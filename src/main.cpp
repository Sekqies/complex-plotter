#include <glad/glad.h>
#include <shader/shader.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <graphics/graphics.h>
#include <parser/parser.h>
#include <iostream>


using vec2 = glm::vec2;

constexpr float WIDTH = 800.00f;
constexpr float HEIGHT = 600.00f;
int main() {
	std::cout << parser::stack_to_str(parser::parse("(2+sin(3i))*5"));
	GLFWwindow* window = initalize_window(WIDTH, HEIGHT, "Domain Coloring");
	Shader shader_program("shaders/plotter.vert", "shaders/plotter.frag");
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	shader_program.use();
	shader_program.setFloat("u_range", 2.0f);
	shader_program.setVec2("shift", glm::vec2(0.0f, 0.0f));
	shader_program.setVec2("u_resolution", glm::vec2(WIDTH, HEIGHT));

	std::vector<unsigned char> operation_stack = { 4,4,12,255 };
	std::vector<glm::vec2> constant_stack = {vec2(1,1),vec2(0,5)};
	unsigned int stack_tbo_buffer, stack_tbo_texture;
	unsigned int constants_tbo_buffer, constants_tbo_texture;
	populate_texture(stack_tbo_buffer, stack_tbo_texture, operation_stack);
	populate_texture(constants_tbo_buffer, constants_tbo_texture, constant_stack);

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		
		shader_program.use();
		shader_program.setFloat("time", glfwGetTime());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, stack_tbo_texture);
		shader_program.setInt("operator_stack", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, constants_tbo_texture);
		shader_program.setInt("constant_stack", 1);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return 0;
}