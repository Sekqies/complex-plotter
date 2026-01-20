#version 330 core

in vec2 f_z;
out vec4 FragColor;

const float PI = 3.14159265359;
const float TWO_OVER_PI = 2.0 / PI;

vec3 hsl2rgb(vec3 hsl) {
    vec3 rgb = clamp(abs(mod(hsl.x * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
    return hsl.z + hsl.y * (rgb - 0.5) * (1.0 - abs(2.0 * hsl.z - 1.0));
}

vec3 domain_color(in vec2 z){
    float angle = atan(z.y, z.x);
    float hue = (angle / (2.0 * PI)) + 0.5;
    float light = (TWO_OVER_PI) * atan(length(z));
    return vec3(hue, 1.0, light);
}

void main(){
    vec3 hsl = domain_color(f_z);
    FragColor = vec4(hsl2rgb(hsl),1.0f);
}