#include <graphics/ui.h>
#include <parser/parser.h>
#include <transformer/transformer.h>
#include <graphics/graphics.h>
#include <interactions/interactions.h>
#include <string>
#include <iostream>

#include <imgui.h>
#include <imgui_stdlib.h>

#include <interactions/export.h>


using string = std::string;
using glm::vec2;

const float DEBOUNCE_DELAY = 0.05f;

namespace UI {
    bool Button(const char* label, const ImVec2& size = ImVec2(0, 0)) {
        bool clicked = ImGui::Button(label, size);

        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        return clicked;
    }

    bool RadioButton(const char* label, const bool cond) {
        bool clicked = ImGui::RadioButton(label, cond);
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        return clicked;
    }
    bool Checkbox(const char* label, bool* v) {
        bool clicked = ImGui::Checkbox(label, v);
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        return clicked;
    }
    bool CollapsingHeader(const char* label, ImGuiTreeNodeFlags flags = 0) {
        bool is_open = ImGui::CollapsingHeader(label, flags);
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        return is_open;
    }
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
        compiler_shader.compile(expression, state.is_3d);
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
    state.is_3d = view_state.is_3d;
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
        &state.expression,
        ImGuiInputTextFlags_EnterReturnsTrue);

    bool typed = ImGui::IsItemEdited();

    if (typed && !pressed_enter && auto_reparse) {
        state.last_typing_time = glfwGetTime();
        state.needs_reparse = true;
    }

    ImGui::SameLine();
    if (UI::Button("Compile")) {
        pressed_enter = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
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
    if (UI::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        UI::Checkbox("Auto-Reparse (Live Edit)", &auto_reparse);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("If disabled, you must press Enter to see changes.");

        ImGui::Text("View Mode:");
        ImGui::SameLine();
        if (UI::RadioButton("2D Plane", !view_state.is_3d)) {
            view_state.is_3d = false;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        ImGui::SameLine();
        if (UI::RadioButton("3D Surface", view_state.is_3d)) {
            view_state.is_3d = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        UI::Checkbox("Show Value Inspector", &view_state.show_inspector);

        if (view_state.show_inspector) {
            ImGui::Indent();
            UI::Checkbox("Follow Mouse", &view_state.inspector_follows_mouse);
            ImGui::Unindent();
        }
        UI::Checkbox("Show Grid", &view_state.show_grid);
        if (view_state.show_grid) {
            ImGui::Indent();
            UI::Checkbox("Warp Grid", &view_state.warp_grid);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("This maps the grid to f(z), which shows how the functions warps space.");
            ImGui::Unindent();
        }
    }

    if (UI::CollapsingHeader("3D Keybinds")) {
        ImGui::BulletText("WASD: Move");
        ImGui::BulletText("Right click + Drag: Move camera");
        ImGui::BulletText("Shift/Spacebar: Go up");
        ImGui::BulletText("Ctrl: Go down");
    }
    if (UI::CollapsingHeader("Help & Keybinds")) {
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
        ImGui::TextWrapped("%s", supported_operators.c_str());
    }
    if (UI::CollapsingHeader("Presets")) {
        static const char* presets[] = {
                                        "z",
                                        "sin(z)",
                                        "(z^2+1)/(z^2-1)",
                                        "exp(1/z)",
                                        "sin(z)*cos(10/z)"
        };
        static int current_preset = -1;
        if (ImGui::Combo("Choose", &current_preset, presets, IM_ARRAYSIZE(presets))) {
            state.expression = string(presets[current_preset]);
            state.needs_reparse = true;
        }
    }
    if (UI::CollapsingHeader("Export")) {
        static int export_w = 1920;
        static int export_h = 1080;

        ImGui::Text("Resolution:");
        ImGui::SetNextItemWidth(75);
        ImGui::InputInt("Width", &export_w, 0);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(75);
        ImGui::InputInt("Height", &export_h, 0);

        if (UI::Button("1080p")) { export_w = 1920; export_h = 1080; }
        ImGui::SameLine();
        if (UI::Button("4K")) { export_w = 3840; export_h = 2160; }

        ImGui::Spacing();

        if (UI::Button("Export plot as PNG")) {
            export_w = std::max(100, std::min(export_w, 8192));
            export_h = std::max(100, std::min(export_h, 8192));

            view_state.export_width = export_w;
            view_state.export_height = export_h;
            view_state.wants_export = true;
        }
    }
    if (view_state.show_export_success) {
        ImGui::OpenPopup("Export Successful!");
        view_state.show_export_success = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Export Successful!", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Your plot has been saved.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
#ifndef __EMSCRIPTEN__
        ImGui::TextDisabled("Check the folder where your executable is located.");
#else
        ImGui::TextDisabled("Check your browser's downloads folder.");
#endif
        ImGui::Spacing();

        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 120.0f) * 0.5f);
        if (UI::Button("Awesome", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();

        ImGui::EndPopup();
    }

    ImGui::End();
}