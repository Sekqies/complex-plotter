#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
GLFWwindow* initalize_window(const float width, const float height, const std::string windowname);

void populate_texture(unsigned int& tbo_buffer, unsigned int& tbo_texture, const std::vector<unsigned char>& bytes);

void populate_texture(unsigned int& tbo_buffer, unsigned int& tbo_texture, const std::vector<glm::vec2>& vec_data);
