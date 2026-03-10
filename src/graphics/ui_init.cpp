#include <graphics/ui_init.h>



void init_imgui(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 2.0f;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	#ifdef __EMSCRIPTEN__
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
