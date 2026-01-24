#pragma once
#include <graphics/ui.h>
#include <shader/shader.h>
#include <GLFW/glfw3.h>

struct PickerResult {
	glm::vec2 value;      
	glm::vec2 coordinate; 
	bool valid;          
};

PickerResult get_hover_value(
	double raw_x,                  
	double raw_y,                 
	ViewState& view_state,        
	unsigned int stack_tex,
	unsigned int const_tex,
	Shader& picker_shader,
	unsigned int picker_fbo
);

void init_picker(unsigned int& picker_tex, unsigned int& picker_fbo);