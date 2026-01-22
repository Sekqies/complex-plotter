#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct CameraState {
    glm::vec3 position = glm::vec3(8.0f, 8.0f, 8.0f);
    glm::vec3 front;
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right;
    glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);

    float yaw;
    float pitch;
    float movement_speed = 2.5f;
    float mouse_sensitivity = 0.05f;
    float zoom = 45.0f;

    CameraState() {
        glm::vec3 direction = glm::normalize(-position);
        front = direction;
        pitch = glm::degrees(asin(direction.y));
        yaw = glm::degrees(atan2(direction.z, direction.x));
        right = glm::normalize(glm::cross(front, world_up));
        up = glm::normalize(glm::cross(right, front));
    }
};

extern CameraState camera_state;


void update_camera_vectors(CameraState& state);

glm::mat4 get_view_matrix(const CameraState& state);

void process_keyboard(CameraState& state, int direction, float delta_time);

void process_mouse_movement(CameraState& state, float x_offset, float y_offset, bool constrain_pitch = true);

void process_mouse_scroll(CameraState& state, float y_offset);

void handle_camera_input(GLFWwindow* window, const float delta_time);

