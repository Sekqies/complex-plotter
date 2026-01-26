#include <graphics/ui.h>
#include <parser/parser.h>
#include <transformer/transformer.h>
#include <graphics/graphics.h>
#include <interactions/interactions.h>
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

void compile(FunctionState& state, CompilerShader& compiler_shader, unsigned int& op_tex, unsigned int& const_tex) {
	try {
		vector<TokenOperator> stack = parser::parse(state.expression);
        vector<unsigned char> operator_stack;
        vector<vec2> constant_stack;
        get_stacks(stack, operator_stack, constant_stack);
        unsigned int op_tbo_buf, const_tbo_buf;
        populate_texture(op_tbo_buf, op_tex, operator_stack);
        populate_texture(const_tbo_buf, const_tex, constant_stack);
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


void render_inspector_overlay(const PickerResult& hover, ViewState& view_state) {
    const float padding = 10.0f;
    const ImVec2 offset = ImVec2(15.0f, 15.0f);
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;

    if (view_state.inspector_follows_mouse) {
        const ImVec2 mouse_pos = ImGui::GetMousePos();
        ImGui::SetNextWindowPos(ImVec2(offset.x + mouse_pos.x, offset.y + mouse_pos.y), ImGuiCond_Always);
    }
    else {
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = work_pos.x + work_size.x - padding;
        window_pos.y = work_pos.y + work_size.y - padding;
        window_pos_pivot = ImVec2(1.0f, 1.0f); 
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }
    ImGui::SetNextWindowBgAlpha(0.65f);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoInputs;
    if (ImGui::Begin("Inspector", nullptr, window_flags)) {
        ImGui::Text("Coordinate (z)");
        ImGui::Separator();
        ImGui::Text("%.3f %s %.3fi",
            hover.coordinate.x,
            (hover.coordinate.y >= 0 ? "+" : "-"),
            std::abs(hover.coordinate.y));

        ImGui::Spacing();

        ImGui::Text("Value f(z)");
        ImGui::Separator();
        ImGui::Text("%.3f %s %.3fi",
            hover.value.x,
            (hover.value.y >= 0 ? "+" : "-"),
            std::abs(hover.value.y));
    }
    ImGui::End();
}


void render_and_update(FunctionState& state, ViewState& view_state, unsigned int& op_tex, unsigned int& const_tex, Shader& interpreter_shader, CompilerShader& compiler_shader) {
    ImGui::Begin("Function Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::SameLine();
    if (state.is_interpreted) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "[ INTERPRETED ]");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("May be slow for large expressions. Press enter to increase performance");
    }
    else {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[ COMPILED ]");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Faster for large expressions");
    }
    ImGui::Separator();

    ImGui::Text("f(z) = ");
    ImGui::SameLine();

    static bool auto_reparse = true;

    bool pressed_enter = ImGui::InputText("##source",
        static_cast<char*>(state.expression.data()),
        state.expression.capacity() + 1,
        ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_EnterReturnsTrue,
        input_text_callback,
        &state.expression);

    bool typed = ImGui::IsItemEdited();

    if (typed && !pressed_enter && auto_reparse) {
        state.last_typing_time = glfwGetTime();
        state.needs_reparse = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Compile")) {
        pressed_enter = true; 
    }

    const float current_time = glfwGetTime();
    const float time_dif = current_time - state.last_typing_time;
    const bool debounced = time_dif > DEBOUNCE_DELAY;

    if (pressed_enter) {
        compile(state, compiler_shader, op_tex, const_tex);
    }
    if (state.needs_reparse && debounced && !pressed_enter) {
        render(state, op_tex, const_tex, interpreter_shader);
    }

    if (!state.error_message.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Error: %s", state.error_message.c_str());
        ImGui::Spacing();
    }

    ImGui::Spacing();
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Auto-Reparse (Live Edit)", &auto_reparse);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("If disabled, you must press Enter to see changes.");

        ImGui::Text("View Mode:");
        ImGui::SameLine();
        if (ImGui::RadioButton("2D Plane", !state.is_3d)) {
            view_state.is_3d = false;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("3D Surface", state.is_3d)) {
            view_state.is_3d = true;
        }
        ImGui::Checkbox("Show Value Inspector", &view_state.show_inspector);

        if (view_state.show_inspector) {
            ImGui::Indent();
            ImGui::Checkbox("Follow Mouse", &view_state.inspector_follows_mouse);
            ImGui::Unindent();
        }
        ImGui::Checkbox("Show Grid", &view_state.show_grid);
        if (view_state.show_grid) {
            ImGui::Indent();
            ImGui::Checkbox("Warp Grid", &view_state.warp_grid);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("This maps the grid to f(z), which shows how the functions warps space.");
            ImGui::Unindent();
        }
    }

    if (ImGui::CollapsingHeader("3D Keybinds")) {
        ImGui::BulletText("WASD: Move");
        ImGui::BulletText("Right click + Drag: Move camera");
        ImGui::BulletText("Shift/Spacebar: Go up");
        ImGui::BulletText("Ctrl: Go down");
    }
    if (ImGui::CollapsingHeader("Help & Keybinds")) {
        ImGui::BulletText("Left Click + Drag: Pan Camera");
        ImGui::BulletText("Scroll Wheel: Zoom");
        ImGui::BulletText("Enter: Compile (High Performance)");
        ImGui::BulletText("T: Quick Toggle 2D/3D");
        ImGui::TextDisabled("Supported Symbols:");
        static string supported_operators = []() {
            string out;
            for (const auto& op : full_operators) {
                if (op.token_operator.op != Operator::VARIABLEPLACEHOLDER && op.token_operator.op != Operator::SECONDVARIABLEPLACEHOLDER)
                    out += op.token_operator.str_repr + ", ";
            }
            return out;
            }();
        ImGui::TextWrapped(supported_operators.c_str());
    }
    if (ImGui::CollapsingHeader("Presets")) {
        static const char* presets[] = {
                                        "z",
                                        "sin(z)",
                                        "(z^2+1)/(z^2-1)",
                                        "exp(1/z)",
                                        "sin(z)*cos(10/z)"
        };
        static int current_preset = -1;
        if (ImGui::Combo("Choose", & current_preset, presets, IM_ARRAYSIZE(presets))) {
            state.expression = string(presets[current_preset]);
            state.needs_reparse = true; 
        }
    }

    ImGui::End();
}