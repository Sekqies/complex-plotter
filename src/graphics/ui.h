#pragma once
#include <glad/glad.h>
#include <shader/shader.h>
#include <compiler/compiler_shader.h>
#include <graphics/ui_init.h>
#include <string>
#include <interactions/interactions.h>
#include <graphics/picker.h>

struct PickerResult;

typedef struct FunctionState {
	Shader* current_shader = nullptr;
	std::string expression = "z";
	std::string error_message = "";
	float last_typing_time = 0.0f;
	bool needs_reparse = true;
	bool is_interpreted = true;
	bool is_3d = false;
} FunctionState;

void render(FunctionState& state, unsigned int& op_tex, unsigned int& const_tex, Shader& interpreter_shader);

void compile(FunctionState& state, CompilerShader& compiler_shader, unsigned int& op_tex, unsigned int& const_tex);

void render_and_update(FunctionState& state,ViewState& view_state, unsigned int& op_tex, unsigned int& const_tex, Shader& interpreter_shader, CompilerShader& compiler_shader);

void render_inspector_overlay(const PickerResult& hover, ViewState& view_state);

