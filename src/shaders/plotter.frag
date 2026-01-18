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

float carg(vec2 z){
    return atan(z.y,z.x);
}

float cmag(vec2 z){
    return length(z);
}

vec2 cexp(vec2 z){
    return exp(z.x)*vec2(cos(z.y),sin(z.y));
}

vec2 clog(vec2 z){
    return vec2(log(length(z)), carg(z));
}

vec2 cpow(vec2 a, vec2 b){
    return cexp(b * clog(a));
}

vec2 cdiv(vec2 a, vec2 b){
    return cmult(a,vec2(b.x,-b.y))/(b.x*b.x+b.y*b.y);
}

vec2 csin(vec2 a){
    return vec2(sin(a.x) * cosh(a.y),cos(a.x) * sinh(a.y));
}

vec2 ccos(vec2 a) {
    return vec2(cos(a.x) * cosh(a.y), -sin(a.x) * sinh(a.y));
}

vec2 csec(vec2 a) {
    float den = cos(2.0f * a.x) + cosh(2.0f * a.y);
    float scale = 2.0f / den;
    return vec2(
        scale * cos(a.x) * cosh(a.y), 
        scale * sin(a.x) * sinh(a.y)
    );
}

vec2 ccsc(vec2 a) {
    float den = cosh(2.0f * a.y) - cos(2.0f * a.x);
    float scale = 2.0f / den;
    return vec2(
        scale * sin(a.x) * cosh(a.y),
        -scale * cos(a.x) * sinh(a.y)
    );
}
vec2 ccot(vec2 a) {
    float den = cosh(2.0f * a.y) - cos(2.0f * a.x);
    return vec2(
        sin(2.0f * a.x) / den,
        -sinh(2.0f * a.y) / den
    );
}


vec2 ctan(vec2 a) {
    float div = cos(2.0f * a.x) + cosh(2.0f * a.y);
    return vec2(sin(2.0f * a.x) / div, sinh(2.0f * a.y) / div);
}


vec2 csinh(vec2 a) {
    return vec2(sinh(a.x) * cos(a.y), cosh(a.x) * sin(a.y));
}
vec2 ccosh(vec2 a) {
    return vec2(cosh(a.x) * cos(a.y), sinh(a.x) * sin(a.y));
}

vec2 ctanh(vec2 a) {
    float div = cosh(2.0f * a.x) + cos(2.0f * a.y);
    return vec2(sinh(2.0f * a.x) / div, sin(2.0f * a.y) / div);
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
#define SIN SHADER_SIN
#define COS SHADER_COS
#define TAN SHADER_TAN
#define CSC SHADER_CSC
#define SEC SHADER_SEC
#define COT SHADER_COT
#define POW SHADER_POW
#define EXP SHADER_EXP
#define LOG SHADER_LOG


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
    #define SHADER_COT 14u
    #define SHADER_CSC 15u
    #define SHADER_SEC 16u
    #define SHADER_SIN 17u
    #define SHADER_COS 18u
    #define SHADER_TAN 19u
    #define SHADER_LOG 20u
    #define SHADER_EXP 21u
    #define SHADER_POW 22u
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
        case SIN:
            return csin(param);
        case COS:
            return ccos(param);
        case TAN:
            return ctan(param);
        case CSC:
            return ccsc(param);
        case SEC:
            return csec(param);
        case COT:
            return ccot(param);
        case EXP:
            return cexp(param);
        case LOG:
            return clog(param);
        
        
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
        case POW:
            return cpow(a,b);
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

vec3 domain_color(in vec2 z){
    float angle = atan(z.y,z.x);
    float hue = (angle/(2.0 * PI)) + 0.5f;
    float light = (TWO_OVER_PI) * atan(length(z));
    return vec3(hue,1.0f,light);
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