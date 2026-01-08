#include <graphics/graphics.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

using std::string;
GLFWwindow* initalize_window(const float width, const float height, const string windowname) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(width, height, windowname.c_str(), nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "Error at initialize_window: error at creating window";
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	const int worked = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	if (!worked) {
		std::cerr << "Error at initialize_window: error at initializing GLAD";
		return nullptr;
	}
	glViewport(0, 0, width, height);
	return window;
}