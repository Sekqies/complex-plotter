#version 300 es
precision highp float;
precision highp int;
precision highp sampler2D;
precision highp usampler2D;

out vec2 pos;

void main(){
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    pos = vec2(x,y);
    gl_Position = vec4(x,y,0.0f,1.0f);
}