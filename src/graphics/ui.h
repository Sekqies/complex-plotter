#pragma once
#include <glad/glad.h>
#include <graphics/ui_init.h>
#include <string>

typedef struct FunctionState {
	std::string expression = "z";
	std::string error_message = "";
	float last_typing_time = 0.0f;
	bool needs_reparse = true;
} FunctionState;

void render_and_update(FunctionState& state, unsigned int& op_tex, unsigned int& const_tex);