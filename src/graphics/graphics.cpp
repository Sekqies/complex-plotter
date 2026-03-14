#include <graphics/graphics.h>
#include <glad_include_guard.h>
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
	#ifndef __EMSCRIPTEN__
		const int worked = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		if (!worked) {
			std::cerr << "Error at initialize_window: error at initializing GLAD";
			return nullptr;
		}
	#endif
	glViewport(0, 0, width, height);
	return window;
}

void clean_texture_buffers(unsigned int& tbo_buffer, unsigned int& tbo_texture) {
	if (tbo_buffer != 0) {
		glDeleteBuffers(1, &tbo_buffer);
		tbo_buffer = 0;
	}
	if (tbo_texture != 0) {
		glDeleteTextures(1, &tbo_texture);
		tbo_texture = 0;
	}
}

void populate_texture(unsigned int& tbo_buffer, unsigned int& tbo_texture, const std::vector<unsigned char>& bytes) {
	clean_texture_buffers(tbo_buffer, tbo_texture);
	glGenTextures(1, &tbo_texture);
	glBindTexture(GL_TEXTURE_2D, tbo_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	std::vector<unsigned char> padded = bytes;
	padded.resize(1024, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, 32, 32, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, padded.data());
}

void populate_texture(unsigned int& tbo_buffer, unsigned int& tbo_texture, const std::vector<glm::vec2>& vec_data) {
	clean_texture_buffers(tbo_buffer, tbo_texture);
	glGenTextures(1, &tbo_texture);
	glBindTexture(GL_TEXTURE_2D, tbo_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	std::vector<glm::vec2> padded = vec_data;
	padded.resize(1024, glm::vec2(0.0f));

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, 32, 32, 0, GL_RG, GL_FLOAT, padded.data());
}

void build_shader_source(Shader& shader, const std::string& vert_source, const std::string& frag_source) {
	shader.compile(vert_source, frag_source);
}

void build_shader_path(Shader& shader, const std::string& vert_path, const std::string& frag_path) {
	const std::string vert_source = get_source(vert_path);
	const std::string frag_source = get_source(frag_path);
	return build_shader_source(shader, vert_source, frag_source);
}