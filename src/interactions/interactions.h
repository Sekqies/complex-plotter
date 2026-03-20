#pragma once
#include <glad_include_guard.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glsl_transpiled/glsl_big_number.h>
#include <glsl_generated/types.h>
#include <atomic>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

typedef struct ViewState {
	glm::vec2 shift = glm::vec2(0.0f);
	big_vec2 hp_shift = big_vec2(0.0f,0.0f);
	big_vec2 hp_last_mouse_pos = big_vec2(0.0f,0.0f);
	glm::vec2 last_mouse_pos = glm::vec2(0.0f);
	float range = 2.0f;
	big_float hp_range = big_float(2.0f);
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
	bool wants_high_precision = false;
	bool is_high_precision = false;
	unsigned int hp_texture = 0;
	std::vector<unsigned char> hp_cpu_buffer;
	std::atomic<bool> is_rendering_hp{false};
	std::atomic<int> hp_rows_completed{0};

	int hp_width = 800;
	int hp_height = 600;
	
} ViewState;


void scroll_callback(GLFWwindow* window, const double xoffset, const double yoffset);
void mouse_button_callback(GLFWwindow* window, const int button, const int action, const int mods);
void cursor_position_callback(GLFWwindow* window, const double xpos, const double ypos);
void window_size_callback(GLFWwindow* window, const int width, const int height);
void framebuffer_size_callback(GLFWwindow* window, const int width, const int height);
#ifdef __EMSCRIPTEN__
EM_BOOL browser_resize_callback(int event_type, const EmscriptenUiEvent* ui_event, void* data);
#endif