#version 330 core
in vec2 z_val;
in vec2 f_z;
out vec4 FragColor;

uniform bool show_grid;
uniform bool warp_grid;


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
    vec2 z = z_val;
        if(show_grid){
        const float axis_width = 1.5f;
        const float grid_width = 1.0f;
        
        vec2 df = fwidth(z);

        vec2 grid_dist = abs(fract(z+0.5f)-0.5f);
        vec2 grid_px = grid_dist/df;
        float grid_val = min(grid_px.x,grid_px.y);

        vec2 axis_px = abs(z) / df;
        float axis_val = min(axis_px.x, axis_px.y);

        float grid_alpha = 1.0 - smoothstep(0.0, grid_width, grid_val);
        float axis_alpha = 1.0 - smoothstep(0.0, axis_width, axis_val);

        vec3 grid_color = vec3(0.0);

        hsl = mix(hsl, grid_color, grid_alpha * 0.3);
        hsl = mix(hsl, grid_color, axis_alpha * 0.9);
    }
    FragColor = vec4(hsl2rgb(hsl),1.0f);
}