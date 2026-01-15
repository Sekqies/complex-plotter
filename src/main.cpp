#include <glad/glad.h>
#include <shader/shader.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <graphics/graphics.h>
#include <graphics/ui.h>
#include <parser/parser.h>
#include <preprocessor/preprocessor.h>
#include <transformer/transformer.h>
#include <interactions/interactions.h>
#include <stdexcept>
#include <iostream>
#include <types/type_mapper.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>



using vec2 = glm::vec2;

constexpr float WIDTH = 800.00f;
constexpr float HEIGHT = 600.00f;
int main() {
	GLFWwindow* window = initalize_window(WIDTH, HEIGHT, "Domain Coloring");
	preprocess("shaders/plotter.frag", operators);

	Shader shader_program("shaders/plotter.vert", "shaders/plotter.frag");

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	ViewState view_state;
	view_state.width = WIDTH;
	view_state.height = HEIGHT;
	glfwSetWindowUserPointer(window, &view_state);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetFramebufferSizeCallback(window, window_size_callback);

	shader_program.use();

	unsigned int stack_tbo_buffer, stack_tbo_texture;
	unsigned int constants_tbo_buffer, constants_tbo_texture;

	float last_time = 0.0f;
	init_imgui(window);
	FunctionState function_state;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
		init_imgui_loop();

		render_and_update(function_state, stack_tbo_texture, constants_tbo_texture);

		shader_program.use();
		shader_program.setFloat("time", glfwGetTime());
		shader_program.setFloat("u_range", view_state.range);
		shader_program.setVec2("shift", view_state.shift);
		shader_program.setVec2("u_resolution", glm::vec2(view_state.width,view_state.height));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, stack_tbo_texture);
		shader_program.setInt("operator_stack", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, constants_tbo_texture);
		shader_program.setInt("constant_stack", 1);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		render_imgui();
		glfwSwapBuffers(window);
	}
	return 0;
}