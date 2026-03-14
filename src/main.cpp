#include <glad_include_guard.h>
#include <shader/shader.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <graphics/graphics.h>
#include <graphics/ui.h>
#include <parser/parser.h>
#include <preprocessor/preprocessor.h>
#include <transformer/transformer.h>
#include <interactions/interactions.h>
#include <stdexcept>
#include <iostream>
#include <compiler/compiler_shader.h>
#include <graphics/3d/mesh.h>
#include <graphics/3d/camera_state.h>
#include <shaders/embedded_shaders.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#ifdef __EMSCRIPTEN__
	#include <emscripten.h>
	#include <emscripten/html5.h>
#endif



using vec2 = glm::vec2;

constexpr float WIDTH = 800.00f * 2.0f;
constexpr float HEIGHT = WIDTH * 3/4;

float last_x = WIDTH / 2.0f;
float last_y = HEIGHT / 2.0f;
bool first_mouse = true;
float last_frame = 0.0f;
float delta_time = 0.0f;


unsigned int stack_tbo_buffer, stack_tbo_texture;
unsigned int constants_tbo_buffer, constants_tbo_texture;


struct AppContext {
	GLFWwindow* window;
	ViewState* view_state;
	FunctionState* function_state;
	Shader* shader_program;
	Shader* shader_3d;
	CompilerShader* compiled_shader;
	CompilerShader* compiled_shader_3d;
	Shader* picker;
	Mesh* grid_mesh;
	unsigned int VAO;
	unsigned int picker_fbo;
	bool pressing_t;
};


void main_loop_step(AppContext* ctx) {
	glfwPollEvents();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	bool is_3d = ctx->view_state->is_3d;

	float current_frame = (float)glfwGetTime();
	delta_time = current_frame - last_frame;
	last_frame = current_frame;

	if (!ctx->pressing_t && !ImGui::GetIO().WantCaptureKeyboard && glfwGetKey(ctx->window, GLFW_KEY_T) == GLFW_PRESS) {
		ctx->pressing_t = true;
	}

	if (ctx->pressing_t && !ImGui::GetIO().WantCaptureKeyboard && glfwGetKey(ctx->window, GLFW_KEY_T) == GLFW_RELEASE) {
		ctx->pressing_t = false;
		ctx->view_state->is_3d = !ctx->view_state->is_3d;
	}

	ctx->function_state->is_3d = ctx->view_state->is_3d;
	init_imgui_loop();

	if (ctx->function_state->is_3d) {
		if (!ImGui::GetIO().WantCaptureKeyboard)
			handle_camera_input(ctx->window, delta_time);
		if (ctx->function_state->is_interpreted) {
			ctx->function_state->current_shader = ctx->shader_3d;
		}
		glEnable(GL_DEPTH_TEST);
		render_and_update(*(ctx->function_state), *(ctx->view_state), stack_tbo_texture, constants_tbo_texture, *(ctx->shader_3d), *(ctx->compiled_shader_3d));
	}
	else {
		glDisable(GL_DEPTH_TEST);
		render_and_update(*(ctx->function_state), *(ctx->view_state), stack_tbo_texture, constants_tbo_texture, *(ctx->shader_program), *(ctx->compiled_shader));
	}

	Shader* current_shader = ctx->function_state->current_shader;
	current_shader->use();
	current_shader->setFloat("time", glfwGetTime());
	current_shader->setFloat("u_range", ctx->view_state->range);
	current_shader->setVec2("shift", ctx->view_state->shift);
	current_shader->setVec2("u_resolution", glm::vec2(ctx->view_state->width, ctx->view_state->height));
	current_shader->setBool("show_grid", ctx->view_state->show_grid);
	current_shader->setBool("warp_grid", ctx->view_state->warp_grid);

	if (ctx->function_state->is_interpreted) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, stack_tbo_texture);
		current_shader->setInt("operator_stack", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, constants_tbo_texture);
		current_shader->setInt("constant_stack", 1);
	}

	if (ctx->function_state->is_3d) {
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), ctx->view_state->width / ctx->view_state->height, 0.1f, 100.0f);
		glm::mat4 view = get_view_matrix(camera_state);
		glm::mat4 model = glm::mat4(1.0f);
		current_shader->setMat4("projection", projection);
		current_shader->setMat4("view", view);
		current_shader->setMat4("model", model);
		glBindVertexArray(ctx->grid_mesh->VAO);
		glDrawElements(GL_TRIANGLES, ctx->grid_mesh->index_count, GL_UNSIGNED_INT, 0);
	}
	else {
		glBindVertexArray(ctx->VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	if (is_3d != ctx->view_state->is_3d) {
		ctx->function_state->is_3d = ctx->view_state->is_3d;
		if (ctx->view_state->is_3d) {
			if (ctx->function_state->is_interpreted)
				ctx->function_state->current_shader = ctx->shader_3d;
			else {
				compile(*(ctx->function_state), *(ctx->compiled_shader_3d), stack_tbo_texture, constants_tbo_texture);
				ctx->function_state->current_shader = &(ctx->compiled_shader_3d->shader);
			}
		}
		else {
			if (ctx->function_state->is_interpreted)
				ctx->function_state->current_shader = ctx->shader_program;
			else {
				compile(*(ctx->function_state), *(ctx->compiled_shader), stack_tbo_texture, constants_tbo_texture);
				ctx->function_state->current_shader = &(ctx->compiled_shader->shader);
			}
		}
		std::cout << "Switched to " << (ctx->view_state->is_3d ? "3D" : "2D") << std::endl;
	}

	if (ctx->view_state->show_inspector && !ctx->view_state->is_3d) {
		double xpos, ypos;
		glfwGetCursorPos(ctx->window, &xpos, &ypos);

		PickerResult hover = get_hover_value(xpos, ypos, *(ctx->view_state), stack_tbo_texture, constants_tbo_texture, *(ctx->picker), ctx->picker_fbo);
		render_inspector_overlay(hover, *(ctx->view_state));
	}

	render_imgui();
	ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
    #ifdef __EMSCRIPTEN__
    EM_ASM_({
        const canvas = document.getElementById('canvas');
        if (!canvas) return;
        
        let css_cursor = 'default';
        switch ($0) {
            case 0: css_cursor = 'default'; break;      
            case 1: css_cursor = 'text'; break;        
            case 2: css_cursor = 'move'; break;        
            case 3: css_cursor = 'ns-resize'; break;  
            case 4: css_cursor = 'ew-resize'; break;   
            case 5: css_cursor = 'nesw-resize'; break;
            case 6: css_cursor = 'nwse-resize'; break;
            case 7: css_cursor = 'pointer'; break;
            case 8: css_cursor = 'not-allowed'; break;
            case -1: css_cursor = 'none'; break;
        }
        if (canvas.style.cursor !== css_cursor) {
            canvas.style.cursor = css_cursor;
        }
    }, cursor);
	#endif
	glfwSwapBuffers(ctx->window);
}

void main_loop_step_void(void* arg) {
	try {
		main_loop_step(static_cast<AppContext*>(arg));
	}
	catch (const std::exception& e) {
		std::cerr << "Uncaught exception in the main loop: " << e.what() << std::endl;
	}
}

int main() {
	GLFWwindow* window = initalize_window(WIDTH, HEIGHT, "Domain Coloring");
	preprocess_string("shaders/plotter.frag", operators);
	
	static Shader shader_program;
	build_shader_path(shader_program, "shaders/plotter.vert", "shaders/plotter.frag");

	static Shader picker;
	string picker_frag = SRC_PICKER_FRAG;
	inject_at(picker_frag, "FUNCTION_DEFINITIONS HERE", get_block(shader_program.fragment_source, "FUNCTION_DEFINITIONS"));
	inject_at(picker_frag, "INTERPRETER_SPECIFIC_FUNCTIONS", get_block(shader_program.fragment_source, "INTERPRETER_SPECIFIC_FUNCTIONS"));
	inject_at(picker_frag, "CONSTANT_DEFINITIONS HERE", get_block(shader_program.fragment_source, "CONSTANT_DEFINITIONS"));
	picker.compile(shader_program.vertex_source, picker_frag);

	static Shader shader_3d;
	const string frag_source = shader_program.fragment_source;
	string vert_source_3d = get_source("shaders/plotter3d.vert");
	const string frag_source_3d = get_source("shaders/plotter3d.frag");
	try {
		vert_source_3d = build_shader_string(vert_source_3d, frag_source);
		std::cout << vert_source_3d;
	}
	catch (std::runtime_error& er) {
		std::cout << er.what();
	}
	shader_3d.compile(vert_source_3d, frag_source_3d);

	static CompilerShader compiled_shader_3d;
	compiled_shader_3d.prepare_source(vert_source_3d, frag_source_3d, true);

	static CompilerShader compiled_shader("shaders/plotter.vert", "shaders/plotter.frag");
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);

	static ViewState view_state;
	view_state.width = WIDTH;
	view_state.height = HEIGHT;
	shader_program.use();

	float last_time = 0.0f;
	static FunctionState function_state;
	function_state.current_shader = &shader_program;

	static Mesh grid_mesh = generate_grid_mesh(350);

	render(function_state, stack_tbo_texture, constants_tbo_texture,shader_program);
	update_camera_vectors(camera_state);
	bool pressing_t = false;

	unsigned int picker_tex, picker_fbo;

	init_picker(picker_tex,picker_fbo);

	static AppContext ctx;
	ctx.window = window;
	ctx.view_state = &view_state;
	ctx.function_state = &function_state;
	ctx.shader_program = &shader_program;
	ctx.shader_3d = &shader_3d;
	ctx.compiled_shader = &compiled_shader;
	ctx.compiled_shader_3d = &compiled_shader_3d;
	ctx.picker = &picker;
	ctx.grid_mesh = &grid_mesh;
	ctx.VAO = VAO;
	ctx.picker_fbo = picker_fbo;
	ctx.pressing_t = false;

	glfwSetWindowUserPointer(window, &view_state);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	init_imgui(window);

	
#ifdef __EMSCRIPTEN__
	//glViewport(0, 0, (int)view_state.width, (int)view_state.height);
	emscripten_set_canvas_element_size("#canvas", (int)view_state.width, (int)view_state.height);
	emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, window, EM_FALSE, browser_resize_callback);
	EmscriptenUiEvent fake_event;
	fake_event.windowInnerWidth = EM_ASM_INT({ return window.innerWidth; });
	fake_event.windowInnerHeight = EM_ASM_INT({ return window.innerHeight; });
	browser_resize_callback(EMSCRIPTEN_EVENT_RESIZE, &fake_event, window);
	emscripten_set_main_loop_arg(main_loop_step_void, &ctx, 0, false);
#else
	while (!glfwWindowShouldClose(window)) {
		main_loop_step(&ctx);
	}
#endif
	return 0;
}