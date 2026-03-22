#include <graphics/3d/camera_state.h>
CameraState camera_state;

void update_camera_orbit(CameraState& state) {
    state.position.x = state.orbit_target.x + state.orbit_radius * cos(glm::radians(state.yaw)) * cos(glm::radians(state.pitch));
    state.position.y = state.orbit_target.y + state.orbit_radius * sin(glm::radians(state.pitch));
    state.position.z = state.orbit_target.z + state.orbit_radius * sin(glm::radians(state.yaw)) * cos(glm::radians(state.pitch));
    state.front = glm::normalize(state.orbit_target - state.position);
    state.right = glm::normalize(glm::cross(state.front, state.world_up));
    state.up = glm::normalize(glm::cross(state.right, state.front));
}

void update_camera_vectors(CameraState& state) {
    if (state.is_orbit) {
        update_camera_orbit(state);
        return;
    }
    glm::vec3 new_front;
    new_front.x = cos(glm::radians(state.yaw)) * cos(glm::radians(state.pitch));
    new_front.y = sin(glm::radians(state.pitch));
    new_front.z = sin(glm::radians(state.yaw)) * cos(glm::radians(state.pitch));
    state.front = glm::normalize(new_front);
    state.right = glm::normalize(glm::cross(state.front, state.world_up));
    state.up = glm::normalize(glm::cross(state.right, state.front));
}

glm::mat4 get_view_matrix(const CameraState& state) {
    return glm::lookAt(state.position, state.position + state.front, state.up);
}

void process_keyboard(CameraState& state, int direction, float delta_time) {
    float velocity = state.movement_speed * delta_time;
    if (state.is_orbit) {
        glm::vec3 flat_front = glm::normalize(glm::vec3(state.front.x, 0.0f, state.front.z));
        glm::vec3 flat_right = glm::normalize(glm::vec3(state.right.x, 0.0f, state.right.z));

        if (direction == 0) state.orbit_target += flat_front * velocity;
        if (direction == 1) state.orbit_target -= flat_front * velocity;
        if (direction == 2) state.orbit_target -= flat_right * velocity;
        if (direction == 3) state.orbit_target += flat_right * velocity;
        if (direction == 4) state.orbit_target += state.world_up * velocity;
        if (direction == 5) state.orbit_target -= state.world_up * velocity;

        update_camera_vectors(state);
        return;
    }
    if (direction == 0) state.position += state.front * velocity;
    if (direction == 1) state.position -= state.front * velocity;
    if (direction == 2) state.position -= state.right * velocity;
    if (direction == 3) state.position += state.right * velocity;
    if (direction == 4) state.position += state.world_up * velocity; 
    if (direction == 5) state.position -= state.world_up * velocity;
}

void process_mouse_movement(CameraState& state, float x_offset, float y_offset, bool constrain_pitch) {
    x_offset *= state.mouse_sensitivity;
    y_offset *= state.mouse_sensitivity;
    state.yaw += x_offset;
    state.pitch += y_offset;

    if (constrain_pitch) {
        if (state.pitch > 89.0f) state.pitch = 89.0f;
        if (state.pitch < -89.0f) state.pitch = -89.0f;
    }
    update_camera_vectors(state);
}

void process_mouse_scroll(CameraState& state, float y_offset) {
    if (state.is_orbit) {
        state.orbit_radius -= (float)y_offset;
        if (state.orbit_radius < 0.1f) state.orbit_radius = 0.1f;
        update_camera_vectors(state);
    }
    state.zoom -= (float)y_offset;
    if (state.zoom < 1.0f) state.zoom = 1.0f;
    if (state.zoom > 45.0f) state.zoom = 45.0f;
}

void handle_camera_input(GLFWwindow* window, const float delta_time) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        process_keyboard(camera_state, 0, delta_time);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        process_keyboard(camera_state, 1, delta_time);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        process_keyboard(camera_state, 2, delta_time);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        process_keyboard(camera_state, 3, delta_time);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window,GLFW_KEY_SPACE) == GLFW_PRESS)
        process_keyboard(camera_state, 4, delta_time);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        process_keyboard(camera_state, 5, delta_time);
}
