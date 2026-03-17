uniform uint u_center_x_limb[NUMBER_OF_LIMBS];
uniform int u_center_x_sign;

uniform uint u_center_y_limb[NUMBER_OF_LIMBS];
uniform int u_center_y_sign;

uniform uint u_zoom_limb[NUMBER_OF_LIMBS];
uniform int u_zoom_sign;

uniform vec2 u_resolution;

hp_vec2 get_high_precision_coordinates(vec2 fragCoord){
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
    number offset_x_hp = float_to_number(offset.x);
    number offset_y_hp = float_to_number(offset.y);

    number final_x = hp_add(center_x, hp_mult(offset_x_hp, zoom));
    number final_y = hp_add(center_y, hp_mult(offset_y_hp, zoom));

    return initialize_hp_vec2(final_x, final_y);
}

vec3 hsl2rgb(vec3 hsl) {
    vec3 rgb = clamp(abs(mod(hsl.x * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
    return hsl.z + hsl.y * (rgb - 0.5) * (1.0 - abs(2.0 * hsl.z - 1.0));
}

vec3 domain_color(in hp_vec2 z) {
    number angle = hp_atan2(z.y, z.x);
    number hue_hp = hp_div(angle, hp_mult(REAL_TWO, PI));
    number light_hp = hp_mult(TWO_OVER_PI, internal_hp_atan(hp_length(z)));

    float hue = number_to_float(hue_hp);
    float light = number_to_float(light_hp);

    return vec3(hue, 1.0, light);
}

out vec4 FragColor;

void main(){
    hp_vec2 z = get_high_precision_coordinates(gl_FragCoord.xy);
    hp_vec2 func_value;
    #define INJECTION_POINT HERE

    vec3 hsl = domain_color(func_value);
    vec3 rgb = hsl2rgb(hsl);
    FragColor = vec4(rgb, 1.0);

}