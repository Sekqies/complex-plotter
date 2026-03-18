number E_R[6];
uniform uint u_center_x_limb[NUMBER_OF_LIMBS];
uniform int u_center_x_sign;
uniform uint u_center_y_limb[NUMBER_OF_LIMBS];
uniform int u_center_y_sign;
uniform uint u_zoom_limb[NUMBER_OF_LIMBS];
uniform int u_zoom_sign;
uniform vec2 u_resolution;

void get_high_precision_coordinates(in vec2 fragCoord, out hp_vec2 res) {
    number center_x;
    center_x.limb = u_center_x_limb;
    center_x.sign = u_center_x_sign;
    center_x.is_infinite = false;

    number center_y;
    center_y.limb = u_center_y_limb;
    center_y.sign = u_center_y_sign;
    center_y.is_infinite = false;

    number zoom;
    zoom.limb = u_zoom_limb;
    zoom.sign = u_zoom_sign;
    zoom.is_infinite = false;

    vec2 offset = (fragCoord.xy - 0.5 * u_resolution.xy) / u_resolution.y;
    
    float ox = offset.x;
    float oy = offset.y;
    float_to_number(ox, E_R[0]);
    float_to_number(oy, E_R[1]);

    hp_mult(E_R[0], zoom, E_R[2]);
    hp_add(center_x, E_R[2], res.x);

    hp_mult(E_R[1], zoom, E_R[3]);
    hp_add(center_y, E_R[3], res.y);
}

vec3 hsl2rgb(vec3 hsl) {
    vec3 rgb = clamp(abs(mod(hsl.x * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
    return hsl.z + hsl.y * (rgb - 0.5) * (1.0 - abs(2.0 * hsl.z - 1.0));
}

vec3 domain_color(in hp_vec2 z) {
    float raw_angle = 0.0;
    float raw_radius = 0.0;

    hp_atan2(z.y, z.x, E_R[0]);
    number_to_float(E_R[0], raw_angle);

    hp_length(z, E_R[1]);
    number_to_float(E_R[1], raw_radius);

    float hue = (raw_angle + 3.14159265) / (2.0 * 3.14159265);
    
    float light = atan(raw_radius) * 2.0 / 3.14159265;

    return vec3(hue, 1.0, light);
}

out vec4 FragColor;

void main(){
    hp_vec2 z;
    get_high_precision_coordinates(gl_FragCoord.xy, z);
    
    hp_vec2 func_value;
    
    #define INJECTION_POINT HERE

    vec3 hsl = domain_color(func_value);
    vec3 rgb = hsl2rgb(hsl);
    FragColor = vec4(rgb, 1.0);
}