#pragma once
#include <glad_include_guard.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

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
	bool show_grid = true;
	bool warp_grid = false;
	bool wants_export = false;
	int export_height = 1080;
	int export_width = 1920;
	bool show_export_success = false;
} ViewState;


void scroll_callback(GLFWwindow* window, const double xoffset, const double yoffset);
void mouse_button_callback(GLFWwindow* window, const int button, const int action, const int mods);
void cursor_position_callback(GLFWwindow* window, const double xpos, const double ypos);
void window_size_callback(GLFWwindow* window, const int width, const int height);
void framebuffer_size_callback(GLFWwindow* window, const int width, const int height);
#ifdef __EMSCRIPTEN__
EM_BOOL browser_resize_callback(int event_type, const EmscriptenUiEvent* ui_event, void* data);
#endif