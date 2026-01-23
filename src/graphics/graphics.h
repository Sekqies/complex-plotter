#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <shader/shader.h>
#include <string>
#include <vector>
GLFWwindow* initalize_window(const float width, const float height, const std::string windowname);

void populate_texture(unsigned int& tbo_buffer, unsigned int& tbo_texture, const std::vector<unsigned char>& bytes);

void populate_texture(unsigned int& tbo_buffer, unsigned int& tbo_texture, const std::vector<glm::vec2>& vec_data);

void build_shader_source(Shader& shader, const std::string& vert_source, const std::string& frag_source);

void build_shader_path(Shader& shader, const std::string& vert_path, const std::string& frag_path);

