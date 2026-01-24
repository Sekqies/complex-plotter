#version 330 core
layout (location = 0) out vec4 FragData;

uniform float u_range;
uniform vec2 shift;
uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float time;

#define CONSTANT_DEFINITIONS HERE

#define END_CONSTANT_DEFINITIONS HERE

#define FUNCTION_DEFINITIONS HERE

#define END_FUNCTION_DEFINITIONS HERE

#define INTERPRETER_SPECIFIC_FUNCTIONS HERE

#define END_INTERPRETER_SPECIFIC_FUNCTIONS HERE

void main(){
	vec2 z = (u_mouse/u_resolution.y) * u_range;
	z -= (u_resolution / u_resolution.y) * 0.5 * u_range;
	z += shift;

	vec2 val = run_stack(operator_stack,constant_stack,z);

	FragData = vec4(val.x,val.y,z.x,z.y);	
}