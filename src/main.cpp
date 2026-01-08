#include <glad/glad.h>
#include <shader/shader.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <graphics/graphics.h>

constexpr float WIDTH = 800.00f;
constexpr float HEIGHT = 600.00f;
int main() {
	GLFWwindow* window = initalize_window(WIDTH, HEIGHT, "Domain Coloring");
	Shader shader_program("shaders/plotter.vert", "shaders/plotter.frag");
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	shader_program.use();
	shader_program.setFloat("u_range", 2.0f);
	shader_program.setVec2("u_resolution", glm::vec2(WIDTH, HEIGHT));
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
				
		shader_program.use();
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return 0;
}