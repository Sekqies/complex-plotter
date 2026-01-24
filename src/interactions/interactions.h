#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

typedef struct ViewState {
	glm::vec2 shift = glm::vec2(0.0f);
	glm::vec2 last_mouse_pos = glm::vec2(0.0f);
	float range = 2.0f;
	float width = 800.0f;
	float height = 600.0f;
	bool is_dragging = false;
	bool is_3d = false;
	bool show_inspector = true;
	bool inspector_follows_mouse = true;
} ViewState;


void scroll_callback(GLFWwindow* window, const double xoffset, const double yoffset);
void mouse_button_callback(GLFWwindow* window, const int button, const int action, const int mods);
void cursor_position_callback(GLFWwindow* window, const double xpos, const double ypos);
void window_size_callback(GLFWwindow* window, const int width, const int height);