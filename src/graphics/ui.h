#pragma once
#include <glad/glad.h>
#include <shader/shader.h>
#include <compiler/compiler_shader.h>
#include <graphics/ui_init.h>
#include <string>

typedef struct FunctionState {
	Shader* current_shader = nullptr;
	std::string expression = "z";
	std::string error_message = "";
	float last_typing_time = 0.0f;
	bool needs_reparse = true;
	bool is_interpreted = true;
} FunctionState;

void render(FunctionState& state, unsigned int& op_tex, unsigned int& const_tex, Shader& interpreter_shader);

void render_and_update(FunctionState& state, unsigned int& op_tex, unsigned int& const_tex, Shader& interpreter_shader, CompilerShader& compiler_shader);