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

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>



using vec2 = glm::vec2;

constexpr float WIDTH = 800.00f;
constexpr float HEIGHT = 600.00f;

float last_x = 800.0f / 2.0f;
float last_y = 600.0f / 2.0f;
bool first_mouse = true;
float last_frame = 0.0f;
float delta_time = 0.0f;


unsigned int stack_tbo_buffer, stack_tbo_texture;
unsigned int constants_tbo_buffer, constants_tbo_texture;


int main() {
	GLFWwindow* window = initalize_window(WIDTH, HEIGHT, "Domain Coloring");
	preprocess("shaders/plotter.frag", operators);
	
	Shader shader_program("shaders/plotter.vert", "shaders/plotter.frag");

	Shader shader_3d;
	const string frag_source = shader_program.fragment_source;
	string vert_source_3d = get_source("shaders/plotter3d.vert");
	const string frag_source_3d = get_source("shaders/plotter3d.frag");
	std::cout << vert_source_3d;
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
	function_state.is_3d = true;

	Mesh grid_mesh = generate_grid_mesh(256);
	render(function_state, stack_tbo_texture, constants_tbo_texture,shader_program);
	update_camera_vectors(camera_state);
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		float current_frame = (float)glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;
		view_state.is_3d = function_state.is_3d;
		init_imgui_loop();
		if (function_state.is_3d) {
			handle_camera_input(window,delta_time);
			if (function_state.is_interpreted) {
				function_state.current_shader = &shader_3d;
			}
			glEnable(GL_DEPTH_TEST);
			render_and_update(function_state, stack_tbo_texture, constants_tbo_texture, shader_3d, compiled_shader_3d);
		}
		else {
			glDisable(GL_DEPTH_TEST);
			render_and_update(function_state, stack_tbo_texture, constants_tbo_texture, shader_program, compiled_shader);
		}
		Shader* current_shader = function_state.current_shader;
		current_shader->use();
		current_shader->setFloat("time", glfwGetTime());
		current_shader->setFloat("u_range", view_state.range);
		current_shader->setVec2("shift", view_state.shift);
		current_shader->setVec2("u_resolution", glm::vec2(view_state.width,view_state.height));

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

		render_imgui();
		glfwSwapBuffers(window);
	}
	return 0;
}