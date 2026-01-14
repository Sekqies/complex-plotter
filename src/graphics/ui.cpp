#include <graphics/ui.h>
#include <parser/parser.h>
#include <transformer/transformer.h>
#include <graphics/graphics.h>
#include <string>

using string = std::string;
using glm::vec2;

const float DEBOUNCE_DELAY = 0.2f;

int input_text_callback(ImGuiInputTextCallbackData* data) {
	if (data->EventFlag != ImGuiInputTextFlags_CallbackResize) {
		return 0;
	}
	string* s = static_cast<string*>(data->UserData);
	s->resize(data->BufTextLen);
	data->Buf = static_cast<char*>(s->data());
	return 0;
}


void render(FunctionState& state, unsigned int& op_tex, unsigned int& const_tex) {
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
}

void render_and_update(FunctionState& state, unsigned int& op_tex, unsigned int& const_tex){
	ImGui::Begin("Function editor");
	ImGui::Text("Enter a complex function f(z)=");
	bool typed = ImGui::InputText("##source",
		static_cast<char*>(state.expression.data()),
		state.expression.capacity() + 1,
		ImGuiInputTextFlags_CallbackResize,
		input_text_callback,
		&state.expression);
	if (typed) {
		state.last_typing_time = glfwGetTime();
		state.needs_reparse = true;
	}
	const float current_time = glfwGetTime();
	const float time_dif = current_time - state.last_typing_time;
	if (state.needs_reparse && time_dif < DEBOUNCE_DELAY) {
		render(state, op_tex, const_tex);
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

	ImGui::End();
}
