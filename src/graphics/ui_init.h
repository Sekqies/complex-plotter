#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void init_imgui(GLFWwindow* window);

void init_imgui_loop();

void render_imgui();

void shutdown_imgui();