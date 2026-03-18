#include <interactions/interactions.h>
#include <graphics/ui.h>
#include <graphics/3d/camera_state.h>
#include <imgui.h>

ViewState* get_state(GLFWwindow* window) {
    return (ViewState*)glfwGetWindowUserPointer(window);
}

void scroll_callback(GLFWwindow* window, const double xoffset, const double yoffset) {
    ViewState* state = get_state(window);
    if (ImGui::GetIO().WantCaptureMouse || !state) return;

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    float u = (static_cast<float>(x) - state->width * 0.5f) / state->height;
    float v = (state->height * 0.5f - static_cast<float>(y)) / state->height;

    number hp_u = float_to_number(u);
    number hp_v = float_to_number(v);
    number hp_zoom_factor = float_to_number(0.9f);

    number prev_hp_range = state->hp_range;

    if (yoffset > 0) {
        state->hp_range = hp_mult(state->hp_range, hp_zoom_factor);
    }
    else {
        state->hp_range = hp_div(state->hp_range, hp_zoom_factor);
    }

    if (state->is_3d) return;

    number hp_range_diff = hp_sub(prev_hp_range, state->hp_range);
    state->hp_shift.x = hp_add(state->hp_shift.x, hp_mult(hp_range_diff, hp_u));
    state->hp_shift.y = hp_add(state->hp_shift.y, hp_mult(hp_range_diff, hp_v));

    state->range = number_to_float(state->hp_range);
    state->shift.x = number_to_float(state->hp_shift.x);
    state->shift.y = number_to_float(state->hp_shift.y);
    std::cout << "Range,shift.x,shift.y: " << state->range << " " << state->shift.x << " " << state->shift.y << '\n';
    std::cout << "mouse coordinates: " << u << " " << v << '\n';
}

void mouse_button_callback(GLFWwindow* window, const int button, int action, int mods) {
    ViewState* state = get_state(window);
    if (!state || ImGui::GetIO().WantCaptureMouse) return;

    if (button != GLFW_MOUSE_BUTTON_LEFT && button != GLFW_MOUSE_BUTTON_RIGHT) return;

    if (action == GLFW_RELEASE) {
        state->is_dragging = false;
        return;
    }

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    state->last_mouse_pos = glm::vec2(static_cast<float>(x), static_cast<float>(y));
    state->is_dragging = true;
}

void cursor_position_callback(GLFWwindow* window, const double xpos, const double ypos) {
    ViewState* state = get_state(window);
    if (!state || !state->is_dragging) return;

    const glm::vec2 current_pos(static_cast<float>(xpos), static_cast<float>(ypos));
    const glm::vec2 delta = current_pos - state->last_mouse_pos;
    state->last_mouse_pos = current_pos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        process_mouse_movement(camera_state, delta.x, -delta.y);
        return;
    }

    // 1. Calculate HP scale (range / height)
    // Optimization: Use multiplication by inverse height instead of full Knuth division
    float inv_height = 1.0f / state->height;
    number hp_inv_height = float_to_number(inv_height);
    number hp_scale = hp_mult(state->hp_range, hp_inv_height);

    number hp_delta_x = float_to_number(delta.x);
    number hp_delta_y = float_to_number(delta.y);

    // 2. Update HP Shift: shift.x -= delta.x * scale, shift.y += delta.y * scale
    state->hp_shift.x = hp_sub(state->hp_shift.x, hp_mult(hp_delta_x, hp_scale));
    state->hp_shift.y = hp_add(state->hp_shift.y, hp_mult(hp_delta_y, hp_scale));

    // 3. Downconvert for standard shaders and UI
    state->shift.x = number_to_float(state->hp_shift.x);
    state->shift.y = number_to_float(state->hp_shift.y);
}

void window_size_callback(GLFWwindow* window, const int width, const int height) {
    ViewState* state = get_state(window);
    if (!state) return;
    state->width = static_cast<float>(width);
    state->height = static_cast<float>(height);
}

void framebuffer_size_callback(GLFWwindow* window, const int width, const int height) {
    glViewport(0, 0, width, height);
}

#ifdef __EMSCRIPTEN__
EM_BOOL browser_resize_callback(int event_type, const EmscriptenUiEvent* ui_event, void* data) {
    GLFWwindow* window = static_cast<GLFWwindow*>(data);
    const double width = ui_event->windowInnerWidth;
    const double height = ui_event->windowInnerHeight;
    const double ratio = emscripten_get_device_pixel_ratio();

    emscripten_set_canvas_element_size("#canvas", width * ratio, height * ratio);

    int phys_width = static_cast<int>(width * ratio);
    int phys_height = static_cast<int>(height * ratio);
    glfwSetWindowSize(window, phys_width, phys_height);
    return EM_TRUE;
}
#endif