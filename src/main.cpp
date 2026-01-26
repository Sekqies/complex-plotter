#include <glad/glad.h>
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



using vec2 = glm::vec2;

constexpr float WIDTH = 800.00f * 1.5f;
constexpr float HEIGHT = 600.00f * 1.5f;

float last_x = WIDTH / 2.0f;
float last_y = HEIGHT / 2.0f;
bool first_mouse = true;
float last_frame = 0.0f;
float delta_time = 0.0f;


unsigned int stack_tbo_buffer, stack_tbo_texture;
unsigned int constants_tbo_buffer, constants_tbo_texture;



int main() {
	GLFWwindow* window = initalize_window(WIDTH, HEIGHT, "Domain Coloring");
	preprocess_string("shaders/plotter.frag", operators);
	
	Shader shader_program;
	build_shader_path(shader_program, "shaders/plotter.vert", "shaders/plotter.frag");

	Shader picker;
	string picker_frag = SRC_PICKER_FRAG;
	inject_at(picker_frag, "FUNCTION_DEFINITIONS HERE", get_block(shader_program.fragment_source, "FUNCTION_DEFINITIONS"));
	inject_at(picker_frag, "INTERPRETER_SPECIFIC_FUNCTIONS", get_block(shader_program.fragment_source, "INTERPRETER_SPECIFIC_FUNCTIONS"));
	inject_at(picker_frag, "CONSTANT_DEFINITIONS HERE", get_block(shader_program.fragment_source, "CONSTANT_DEFINITIONS"));
	picker.compile(shader_program.vertex_source, picker_frag);

	Shader shader_3d;
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

	CompilerShader compiled_shader_3d;
	compiled_shader_3d.prepare_source(vert_source_3d, frag_source_3d, true);

	CompilerShader compiled_shader("shaders/plotter.vert", "shaders/plotter.frag");
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);

	ViewState view_state;
	view_state.width = WIDTH;
	view_state.height = HEIGHT;
	glfwSetWindowUserPointer(window, &view_state);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetFramebufferSizeCallback(window, window_size_callback);

	shader_program.use();

	float last_time = 0.0f;
	init_imgui(window);
	FunctionState function_state;
	function_state.current_shader = &shader_program;

	Mesh grid_mesh = generate_grid_mesh(350);

	render(function_state, stack_tbo_texture, constants_tbo_texture,shader_program);
	update_camera_vectors(camera_state);
	bool pressing_t = false;

	unsigned int picker_tex, picker_fbo;

	init_picker(picker_tex,picker_fbo);
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		bool is_3d = view_state.is_3d;

		float current_frame = (float)glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		if (!pressing_t && !ImGui::GetIO().WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
			pressing_t = true;
		}

		if (pressing_t && !ImGui::GetIO().WantCaptureKeyboard && glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
			pressing_t = false;
			view_state.is_3d = !view_state.is_3d;
		}

		function_state.is_3d = view_state.is_3d;
		init_imgui_loop();
		if (function_state.is_3d) {
			if(!ImGui::GetIO().WantCaptureKeyboard)
				handle_camera_input(window,delta_time);
			if (function_state.is_interpreted) {
				function_state.current_shader = &shader_3d;
			}
			glEnable(GL_DEPTH_TEST);
			render_and_update(function_state,view_state, stack_tbo_texture, constants_tbo_texture, shader_3d, compiled_shader_3d);
		}
		else {
			glDisable(GL_DEPTH_TEST);
			render_and_update(function_state,view_state, stack_tbo_texture, constants_tbo_texture, shader_program, compiled_shader);
		}
		Shader* current_shader = function_state.current_shader;
		current_shader->use();
		current_shader->setFloat("time", glfwGetTime());
		current_shader->setFloat("u_range", view_state.range);
		current_shader->setVec2("shift", view_state.shift);
		current_shader->setVec2("u_resolution", glm::vec2(view_state.width,view_state.height));
		current_shader->setBool("show_grid", view_state.show_grid);
		current_shader->setBool("warp_grid", view_state.warp_grid);
		if (function_state.is_interpreted) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_BUFFER, stack_tbo_texture);
			current_shader->setInt("operator_stack", 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_BUFFER, constants_tbo_texture);
			current_shader->setInt("constant_stack", 1);
		}
		if (function_state.is_3d) {
			glm::mat4 projection = glm::perspective(glm::radians(45.0f), WIDTH / HEIGHT, 0.1f, 100.0f);
			glm::mat4 view = get_view_matrix(camera_state);
			glm::mat4 model = glm::mat4(1.0f);
			current_shader->setMat4("projection", projection);
			current_shader->setMat4("view", view);
			current_shader->setMat4("model", model);
			glBindVertexArray(grid_mesh.VAO);
			glDrawElements(GL_TRIANGLES, grid_mesh.index_count, GL_UNSIGNED_INT, 0);
		}
		else {
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		if (is_3d != view_state.is_3d) {
			function_state.is_3d = view_state.is_3d;
			if (view_state.is_3d) {
				function_state.current_shader = &shader_3d;
			}
			else {
				function_state.current_shader = &shader_program;
			}
			std::cout << "Switched to " << (view_state.is_3d ? "3D" : "2D") << std::endl;
		}


		if (view_state.show_inspector && !view_state.is_3d) {
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);

			PickerResult hover = get_hover_value(xpos, ypos, view_state, stack_tbo_texture, constants_tbo_texture, picker, picker_fbo);
			render_inspector_overlay(hover, view_state);
		}

		render_imgui();
		glfwSwapBuffers(window);
	}
	return 0;
}