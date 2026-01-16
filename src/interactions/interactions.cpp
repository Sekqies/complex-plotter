#include <interactions/interactions.h>
#include <graphics/ui.h>
#include <imgui.h>
ViewState* get_state(GLFWwindow* window) {
	return (ViewState*)glfwGetWindowUserPointer(window);
}



void scroll_callback(GLFWwindow* window, const double xoffset, const double yoffset) {
	ViewState* state = get_state(window);
	if (!state) return;
	const float zoom_factor = 0.9f;
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	float u = (x - state->width * 0.5f) / state->height;
	float v = (state->height * 0.5f - y) / state->height;
	glm::vec2 norm(u, v);
	float prev_range = state->range;

	if (yoffset > 0) {
		state->range *= zoom_factor;
	}
	else 
		state->range /= zoom_factor;
	state->shift += (prev_range - state->range) * norm;
}

void mouse_button_callback(GLFWwindow* window, const int button, int action, int mods) {
	ViewState* state = get_state(window);
	if (!state) return;
	if (ImGui::GetIO().WantCaptureMouse) return;
	if (button != GLFW_MOUSE_BUTTON_LEFT) return;
	if (action == GLFW_RELEASE) {
		state->is_dragging = false;
		return;
	}
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	// this is casting doubles down to float.
	state->last_mouse_pos = glm::vec2(x, y);
	state->is_dragging = true;
}

void cursor_position_callback(GLFWwindow* window, const double xpos, const double ypos) {
	ViewState* state = get_state(window);
	if (!state) return;
	if (!state->is_dragging) return;
	//downcasting again
	const glm::vec2 current_pos(xpos, ypos);
	const glm::vec2 delta = current_pos - state->last_mouse_pos;
	const float scale = state->range / state->height;
	
	state->shift.x -= delta.x * scale;
	state->shift.y += delta.y * scale;
	state->last_mouse_pos = current_pos;
}

void window_size_callback(GLFWwindow* window, const int width, const int height) {
	ViewState* state = get_state(window);
	if (!state) return;
	state->width = static_cast<float>(width);
	state->height = static_cast<float>(height);
	glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS) return;
	if (key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(window, true);
	}
	if (key == GLFW_KEY_SPACE) {
		
	}
}