#include <interactions/interactions.h>
#include <graphics/ui.h>
#include <graphics/3d/camera_state.h>
#include <imgui.h>
ViewState* get_state(GLFWwindow* window) {
	return (ViewState*)glfwGetWindowUserPointer(window);
}



void scroll_callback(GLFWwindow* window, const double xoffset, const double yoffset) {
	ViewState* state = get_state(window);
	if (ImGui::GetIO().WantCaptureMouse) return;
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
	if (state->is_3d) return;

	state->shift += (prev_range - state->range) * norm;
}

void mouse_button_callback(GLFWwindow* window, const int button, int action, int mods) {
	ViewState* state = get_state(window);
	if (!state) return;

	if (ImGui::GetIO().WantCaptureMouse) return;
	if (button != GLFW_MOUSE_BUTTON_LEFT && button != GLFW_MOUSE_BUTTON_RIGHT) return;
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
	state->last_mouse_pos = current_pos;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		process_mouse_movement(camera_state, delta.x, -delta.y);
		return;
	}

	const float scale = state->range / state->height;
	
	state->shift.x -= delta.x * scale;
	state->shift.y += delta.y * scale;
}

void window_size_callback(GLFWwindow* window, const int width, const int height) {
	ViewState* state = get_state(window);
	if (!state) return;
	state->width = static_cast<float>(width);
	state->height = static_cast<float>(height);
	glViewport(0, 0, width, height);
}
