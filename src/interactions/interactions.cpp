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

    if (state->is_3d && camera_state.is_orbit) {
        process_mouse_scroll(camera_state, yoffset);
        return;
    }

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    float u = (static_cast<float>(x) - state->width * 0.5f) / state->height;
    float v = (state->height * 0.5f - static_cast<float>(y)) / state->height;

    big_float hp_u = big_float(u);
    big_float hp_v = big_float(v);
    big_float hp_zoom_factor = big_float(0.9f);

    big_float prev_hp_range = state->hp_range;

    if (yoffset > 0) {
        state->hp_range = state->hp_range * hp_zoom_factor;;
    }
    else {
        state->hp_range = state->hp_range / hp_zoom_factor;
    }

    //if (state->is_3d) return;

    big_float hp_range_diff = prev_hp_range -  state->hp_range;
    state->hp_shift.x = state->hp_shift.x + hp_range_diff * hp_u;
    state->hp_shift.y = state->hp_shift.y + hp_range_diff * hp_v;

    state->range = static_cast<float>(state->hp_range);
    state->shift.x = static_cast<float>(state->hp_shift.x);
    state->shift.y = static_cast<float>(state->hp_shift.y);
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

    if (state->is_3d) {
        process_mouse_movement(camera_state,delta.x, -delta.y);
        return;
    }

    float inv_height = 1.0f / state->height;
    big_float hp_inv_height = big_float(inv_height);
    big_float hp_scale = state->hp_range * hp_inv_height;

    big_float hp_delta_x = big_float(delta.x);
    big_float hp_delta_y = big_float(delta.y);

    state->hp_shift.x = state->hp_shift.x - hp_delta_x * hp_scale;
    state->hp_shift.y = state->hp_shift.y + hp_delta_y * hp_scale;

    state->shift.x = static_cast<double>(state->hp_shift.x);
    state->shift.y = static_cast<double>(state->hp_shift.y);
}

void window_size_callback(GLFWwindow* window, const int width, const int height) {
    ViewState* state = get_state(window);
    if (!state) return;
    state->width = static_cast<float>(width);
    state->height = static_cast<float>(height);
}

void apply_resolution_scaling(int current_height) {
    if (current_height <= 0) return; 

    ImGuiIO& io = ImGui::GetIO();
    float base_height = 1080.0f;
    float raw_scale = static_cast<float>(current_height) / base_height;

    if (raw_scale < 0.75f) raw_scale = 0.75f;
    if (raw_scale > 2.5f) raw_scale = 2.5f;

    float scale = std::round(raw_scale * 4.0f) / 4.0f;

    io.FontGlobalScale = scale;

    ImGui::GetStyle() = ImGuiStyle();
    ImGui::GetStyle().ScaleAllSizes(scale);

    ImGuiStyle& style = ImGui::GetStyle();
    if (style.WindowBorderSize < 1.0f) style.WindowBorderSize = 1.0f;
    if (style.FrameBorderSize < 1.0f) style.FrameBorderSize = 1.0f;
    if (style.PopupBorderSize < 1.0f) style.PopupBorderSize = 1.0f;
    if (style.ChildBorderSize < 1.0f) style.ChildBorderSize = 1.0f;
    if (style.TabBorderSize < 1.0f) style.TabBorderSize = 1.0f;
    
#if IMGUI_VERSION_NUM >= 18900
    if (style.SeparatorTextBorderSize < 1.0f) style.SeparatorTextBorderSize = 1.0f;
#endif
}

void framebuffer_size_callback(GLFWwindow* window, const int width, const int height) {
    glViewport(0, 0, width, height);
    //apply_resolution_scaling(height);
}

#ifdef __EMSCRIPTEN__
EM_BOOL browser_resize_callback(int event_type, const EmscriptenUiEvent* ui_event, void* data) {
    GLFWwindow* window = static_cast<GLFWwindow*>(data);
    const double width = ui_event->windowInnerWidth;
    const double height = ui_event->windowInnerHeight;
    const double ratio = 2;

    emscripten_set_canvas_element_size("#canvas", width * ratio, height * ratio);

    int phys_width = static_cast<int>(width * ratio);
    int phys_height = static_cast<int>(height * ratio);
    glfwSetWindowSize(window, phys_width, phys_height);
    return EM_TRUE;
}
#endif