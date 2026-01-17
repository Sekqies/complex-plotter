#version 330

// Other useful constants


#define CONSTANT_DEFINITIONS HERE

const float PI = 3.141591f;
const float TWO_PI_OVER_3 = 2*PI*0.66666f;
const float TWO_OVER_PI = 2.0f / PI;

#define END_CONSTANT_DEFINITIONS HERE

in vec2 pos;
out vec4 FragColor;



uniform vec2 u_resolution;
uniform float u_range;
uniform vec2 shift;
uniform float time;

#define FUNCTION_DEFINITIONS HERE

vec2 cadd(vec2 a, vec2 b){
    return a + b;
}
vec2 csub(vec2 a, vec2 b){
    return a - b;
}

vec2 cmult(vec2 a, vec2 b){
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

vec2 cdiv(vec2 a, vec2 b){
    return cmult(a,vec2(b.x,-b.y))/(b.x*b.x+b.y*b.y);
}

vec2 cneg(vec2 a){
    return -a;
}


#define END_FUNCTION_DEFINITIONS HERE


#define INTERPRETER_SPECIFIC_FUNCTIONS HERE


#define START_WRITING_HERE HERE

#define NULL_SYMBOL SHADER_NULL_SYMBOL
#define END SHADER_END
#define CONSTANT SHADER_CONSTANT
#define VARIABLEX SHADER_VARIABLEX
#define VARIABLEY SHADER_VARIABLEY
#define VARIABLEZ SHADER_VARIABLEZ
#define VARIABLET SHADER_VARIABLET
#define VALUE_BOUNDARY SHADER_VALUE_BOUNDARY
#define ADD SHADER_ADD
#define SUB SHADER_SUB
#define MULT SHADER_MULT
#define DIV SHADER_DIV
#define BINARY_BOUNDARY SHADER_BINARY_BOUNDARY
#define NEG SHADER_NEG

#ifndef SHADER_NULL_SYMBOL
    #define SHADER_NULL_SYMBOL 0u
#endif

#ifndef SHADER_END
    #define SHADER_END 1u
#endif

#ifndef SHADER_CONSTANT
    #define SHADER_CONSTANT 2u
#endif

#ifndef SHADER_VARIABLEX
    #define SHADER_VARIABLEX 3u
#endif

#ifndef SHADER_VARIABLEY
    #define SHADER_VARIABLEY 4u
#endif

#ifndef SHADER_VARIABLEZ
    #define SHADER_VARIABLEZ 5u
#endif

#ifndef SHADER_VARIABLET
    #define SHADER_VARIABLET 6u
#endif

#ifndef SHADER_VALUE_BOUNDARY
    #define SHADER_VALUE_BOUNDARY 7u
#endif

#ifndef SHADER_ADD
    #define SHADER_ADD 8u
#endif

#ifndef SHADER_SUB
    #define SHADER_SUB 9u
#endif

#ifndef SHADER_MULT
    #define SHADER_MULT 10u
#endif

#ifndef SHADER_DIV
    #define SHADER_DIV 11u
#endif

#ifndef SHADER_BINARY_BOUNDARY
    #define SHADER_BINARY_BOUNDARY 12u
#endif

#ifndef SHADER_NEG
    #define SHADER_NEG 13u
#endif


const uint STACK_SIZE = 1024;

vec2 evaluate_constant_operator(in uint operator, in samplerBuffer constants, inout int constant_index, in vec2 pos){
    switch(operator){
        case CONSTANT:
            return texelFetch(constants, constant_index++).xy;
        case VARIABLEX:
            return vec2(pos.x,0.0f);
        case VARIABLEY:
            return vec2(pos.y,0.0f);
        case VARIABLEZ:
            return pos;
        case VARIABLET:
            return vec2(time,0.0f);
        default:
            return vec2(0.0f);
    }
}

vec2 pop_one(in vec2 stack[16], inout int stack_index){
    return stack[--stack_index];
}

vec2[2] pop_two(in vec2 stack[16], inout int stack_index){
    vec2 b = pop_one(stack,stack_index);
    vec2 a = pop_one(stack,stack_index);
    return vec2[2](a,b);
}


vec2 evaluate_unary_operator(in uint operator, inout vec2 stack[16], inout int stack_index){
    vec2 param = pop_one(stack,stack_index);
    switch(operator){
        case NEG:
            return -param;
    }
    return param;
}

uniform samplerBuffer constant_stack;
uniform usamplerBuffer operator_stack;


vec2 evaluate_binary_operator(in uint operator, inout vec2 stack[16], inout int stack_index){
    vec2 params[2] = pop_two(stack,stack_index);
    vec2 a = params[0];
    vec2 b = params[1];
    switch(operator){
        case ADD:
            return a + b;
        case SUB:
            return a - b;
        case MULT:
            return cmult(a,b);
        case DIV:
            return cdiv(a,b);
    }
    return vec2(0.0f);
}

vec2 evaluate_stack_operator(in uint operator, inout vec2 stack[16], inout int stack_index){
    if(operator < BINARY_BOUNDARY)
        return evaluate_binary_operator(operator,stack,stack_index);
    return evaluate_unary_operator(operator,stack,stack_index);
}

vec2 run_stack(in usamplerBuffer operators, in samplerBuffer constants, in vec2 pos){
    vec2 stack[16];
    int stack_index = 0;
    int constant_index = 0;
    
    for(int i = 0; i < 1024; ++i){
        uint operator = texelFetch(operators,i).x;
        if(operator == END){
            break;
        }
        if(operator == NULL_SYMBOL){
            continue;
        }
        vec2 result;

        if(operator < VALUE_BOUNDARY)
            result = evaluate_constant_operator(operator,constants, constant_index, pos);
        else            
            result = evaluate_stack_operator(operator,stack,stack_index);
        
        stack[stack_index++] = result;
    }
    return (stack_index > 0)? stack[stack_index-1] : vec2(0.0f);
}

#define END_INTERPRETER_SPECIFIC_FUNCTIONS HERE

vec3 hsl2rgb(vec3 hsl) {
    vec3 rgb = clamp(abs(mod(hsl.x * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
    return hsl.z + hsl.y * (rgb - 0.5) * (1.0 - abs(2.0 * hsl.z - 1.0));
}

float get_discontinuous_light(vec2 f) {
    // 1. Safety Clamp (prevents infinity artifacts)
    float dist = abs(f.x) > 9e+10 || abs(f.y) > 9e+10 ? 9e+10 : length(f); 

    // 2. Logarithmic Cycle
    // log2 means the pattern repeats every time distance doubles (1, 2, 4, 8...)
    float logval = mod(log2(dist), 1.0); 

    float decimal_exp = 1.0;
    if (dist != 0.0) {
        // Logic: decimal_exp becomes (1.0 - fractional_part_of_log)
        // This creates a "sawtooth" wave for brightness
        decimal_exp = -(logval - floor(logval) - 1.0); 
    }

    // 3. Shaping Curve
    // pow(..., 0.2) softens the curve (gamma), and -0.15 shifts the darkness
    return mod(1.0 / (pow(decimal_exp, 0.2) + 1.0) - 0.15, 1.0);
}

// Main Domain Coloring Function
vec3 domain_color(in vec2 z) {
    // 1. Hue (Standard Phase)
    float hue = atan(z.y, z.x) / (2.0 * PI);
    
    // 2. Saturation (Keep distinct)
    float sat = 1.0; 

    // 3. Lightness (Using your discontinuous logic)
    float light = get_discontinuous_light(z);

    return vec3(hue, sat, light);
}

vec2 convert_coordinates(in vec2 pos, in vec2 resolution, in float range){
    return range * (pos - 0.5f * resolution)/resolution.y;
}



void main(){
    //vec2 func_value = run_stack(operator_stack,constant_stack,pos);
    vec2 z = convert_coordinates(gl_FragCoord.xy,u_resolution,u_range) + shift;
    
    #define INTERPRETER_ASSIGNEMENT HERE
    vec2 func_value = run_stack(operator_stack,constant_stack,z);
    #define END_INTERPRETER_ASSIGNEMENT HERE
    
    #define INJECTION_POINT HERE
    
    vec3 hsl = domain_color(func_value);
    FragColor = vec4(hsl2rgb(hsl),1.0f);
}