#include <graphics/ui.h>
#include <parser/parser.h>
#include <transformer/transformer.h>
#include <graphics/graphics.h>
#include <string>
#include <iostream>

using string = std::string;
using glm::vec2;

const float DEBOUNCE_DELAY = 0.05f;

int input_text_callback(ImGuiInputTextCallbackData* data) {
	if (data->EventFlag != ImGuiInputTextFlags_CallbackResize) {
		return 0;
	}
	string* s = static_cast<string*>(data->UserData);
	s->resize(data->BufTextLen);
	data->Buf = static_cast<char*>(s->data());
	return 0;
}



void render(FunctionState& state, unsigned int& op_tex, unsigned int& const_tex, Shader& interpreter_shader) {
	try {
		vector<TokenOperator> stack = parser::parse(state.expression);
		vector<unsigned char> operator_stack;
		vector<vec2> constant_stack;
		get_stacks(stack, operator_stack, constant_stack);
		unsigned int op_tbo_buf, const_tbo_buf;
		populate_texture(op_tbo_buf, op_tex, operator_stack);
		populate_texture(const_tbo_buf, const_tex, constant_stack);
		state.error_message = "";
		state.needs_reparse = false;
	}
	catch (const std::runtime_error& e) {
		state.error_message = e.what();
		state.needs_reparse = false;
	}
	state.current_shader = &interpreter_shader;
	state.is_interpreted = true;
}

void compile(FunctionState& state, CompilerShader& compiler_shader) {
	try {
		vector<TokenOperator> stack = parser::parse(state.expression);
		const string& expression = stack_to_glsl_string(stack);
		compiler_shader.compile(expression,state.is_3d);
	}
	catch (const std::runtime_error& e) {
		state.error_message = e.what();
		state.needs_reparse = false;
	}
	state.current_shader = &compiler_shader.shader;
	state.is_interpreted = false;
}

void render_and_update(FunctionState& state, unsigned int& op_tex, unsigned int& const_tex, Shader& interpreter_shader, CompilerShader& compiler_shader){
	ImGui::Begin("Function editor");
	ImGui::Text("Enter a complex function f(z)=");
	ImGui::SetNextItemWidth(-FLT_MIN);
	bool pressed_enter = ImGui::InputText("##source",
		static_cast<char*>(state.expression.data()),
		state.expression.capacity() + 1,
		ImGuiInputTextFlags_CallbackResize |
		ImGuiInputTextFlags_EnterReturnsTrue,
		input_text_callback,
		&state.expression);
	bool typed = ImGui::IsItemEdited();

	if (typed && !pressed_enter) {
		state.last_typing_time = glfwGetTime();
		state.needs_reparse = true;
	}

	const float current_time = glfwGetTime();
	const float time_dif = current_time - state.last_typing_time;
	const bool debounced = time_dif > DEBOUNCE_DELAY;

	if (pressed_enter) {
		compile(state, compiler_shader);
	}
	if (state.needs_reparse && debounced && !pressed_enter) {
		render(state, op_tex, const_tex, interpreter_shader);
	}
	if (!state.error_message.empty()) {
		ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Error: %s", state.error_message.c_str());
	}
	else {
		ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Valid function!");
	}
	ImGui::Separator();
	ImGui::Text("Controls:");
	ImGui::BulletText("Pan: Left Click + Drag");
	ImGui::BulletText("Zoom: Scroll Wheel");
	ImGui::BulletText("Enter: Compile (increases performance)");
	ImGui::BulletText("T: Toggle 3D mode");

	ImGui::End();
}
