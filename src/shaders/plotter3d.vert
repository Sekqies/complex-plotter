#version 330 core

#define HERE ;

layout (location=0) in vec3 aPos;

out vec2 f_z;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 u_resolution;
uniform float u_range;
uniform vec2 shift;
uniform float time;



vec2 run_stack(samplerBuffer operator_stack, usamplerBuffer constant_stack, vec2 z);

#define CONSTANT_DEFINITIONS HERE

#define END_CONSTANT_DEFINITIONS HERE


#define FUNCTION_DEFINITIONS HERE

#define END_FUNCTION_DEFINITIONS HERE


#define INTERPRETER_SPECIFIC_FUNCTIONS HERE

#define END_INTERPRETER_SPECIFIC_FUNCTIONS HERE

void main(){
	vec2 z = (vec2(aPos.x,aPos.z) * u_range) + shift;

	#define INTERPRETER_ASSIGNEMENT HERE
	vec2 func_value = run_stack(operator_stack,constant_stack,z);
	#define END_INTERPRETER_ASSIGNEMENT HERE

	#define INJECTION_POINT HERE

	float height = length(func_value);

	gl_Position = projection * view * model * vec4(aPos.x,height,aPos.z,1.0f);

	f_z = func_value;
}


