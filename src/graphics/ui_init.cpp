#include <graphics/ui_init.h>
#include <string>
#include <iostream>
#include <iomanip>
#ifdef __EMSCRIPTEN__
#include <emscripten_browser_clipboard.h>
#include <emscripten.h>

#endif



void init_imgui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    float xscale = 1.0f;
    float yscale = 1.0f;

#ifdef __EMSCRIPTEN__
    xscale = emscripten_get_device_pixel_ratio();
#else
    glfwGetWindowContentScale(window, &xscale, &yscale);
#endif
    io.FontGlobalScale = xscale; 

    //ImGui::GetStyle().ScaleAllSizes(xscale); 

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(window, true);

#ifdef __EMSCRIPTEN__
    emscripten_browser_clipboard::paste([](std::string&& paste_data, void* callback_data) {
        clip_content = std::move(paste_data);
        just_pasted = true;
        }, nullptr); 
    
    ImGui_ImplOpenGL3_Init("#version 300 es");
#else
    ImGui_ImplOpenGL3_Init();
#endif
}

void init_imgui_loop() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void render_imgui() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void shutdown_imgui() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
