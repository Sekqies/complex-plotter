#version 330

layout (location=0) in vec4 total_pos;
out vec2 pos;

void main(){
    pos = total_pos.xy;
    gl_Position = total_pos;
}