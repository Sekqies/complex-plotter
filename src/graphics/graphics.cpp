#include <graphics/graphics.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <parser/parser.h>
#include <preprocessor/preprocessor.h>
#include <stdexcept>

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

void populate_texture(unsigned int& tbo_buffer, unsigned int& tbo_texture, const std::vector<unsigned char>& bytes) {
	glGenBuffers(1, &tbo_buffer);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo_buffer);
	glBufferData(GL_TEXTURE_BUFFER, bytes.size() * sizeof(unsigned char), bytes.data(), GL_STATIC_DRAW);
	glGenTextures(1, &tbo_texture);
	glBindTexture(GL_TEXTURE_BUFFER, tbo_texture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R8UI, tbo_buffer);
}

void populate_texture(unsigned int& tbo_buffer, unsigned int& tbo_texture, const std::vector<glm::vec2>& vec_data) {
	glGenBuffers(1, &tbo_buffer);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo_buffer);
	glBufferData(GL_TEXTURE_BUFFER, vec_data.size() * sizeof(glm::vec2), vec_data.data(), GL_STATIC_DRAW);
	glGenTextures(1, &tbo_texture);
	glBindTexture(GL_TEXTURE_BUFFER, tbo_texture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, tbo_buffer);
}

void build_shader_source(Shader& shader, const std::string& vert_source, const std::string& frag_source) {
	shader.compile(vert_source, frag_source);
}

void build_shader_path(Shader& shader, const std::string& vert_path, const std::string& frag_path) {
	const std::string vert_source = get_source(vert_path);
	const std::string frag_source = get_source(frag_path);
	return build_shader_source(shader, vert_source, frag_source);
}