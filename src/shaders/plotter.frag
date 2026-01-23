#version 330

// Other useful constants


#define CONSTANT_DEFINITIONS HERE

const float PI = 3.141591f;
const float TWO_PI_OVER_3 = 2*PI*0.66666f;
const float TWO_OVER_PI = 2.0f / PI;

const vec2 CPI = vec2(PI,0.0f);
const vec2 ONE = vec2(1.0f,0.0f);
const vec2 I = vec2(0.0f,1.0f);

#define END_CONSTANT_DEFINITIONS HERE

in vec2 pos;
out vec4 FragColor;


#define UNIFORM_DECLARATIONS HERE

uniform vec2 u_resolution;
uniform float u_range;
uniform vec2 shift;
uniform float time;

#define END_UNIFORM_DECLARATIONS HERE

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

vec2 carg(vec2 z){
    return vec2(atan(z.y,z.x),0.0f);
}

vec2 cmag(vec2 z){
    return vec2(length(z),0.0f);
}

vec2 cexp(vec2 z){
    return exp(z.x)*vec2(cos(z.y),sin(z.y));
}

vec2 clog(vec2 z){
    return vec2(log(length(z)), carg(z));
}

vec2 cpow(vec2 a, vec2 b){
    return cexp(cmult(b,clog(a)));
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

vec2 csqrt(vec2 a) {
    return cpow(a,vec2(0.5f,0.0f));
}

vec2 reciprocal(vec2 z){
    return cdiv(ONE,z);
}

// Inverse trigonometric
vec2 casin(vec2 z){
    vec2 iz = vec2(-z.y,z.x);
    vec2 root = csqrt(csub(vec2(1.0f,0.0f),cmult(z,z)));
    return cmult(vec2(0.0f,-1.0f),clog(cadd(iz,root)));
}

vec2 cacos(vec2 z){
    return csub(vec2(PI/2.0f, 0.0f), casin(z));
}

vec2 catan(vec2 z){
    vec2 iz = vec2(-z.y,z.x);
    vec2 term = cdiv(csub(ONE,iz),cadd(ONE,iz));
    return cmult(vec2(0.0f,0.5f),clog(term));
}

vec2 cacsc(vec2 z){
    return casin(reciprocal(z));
}

vec2 casec(vec2 z){
    return cacos(reciprocal(z));
}

vec2 cacot(vec2 z){
    return catan(reciprocal(z));
}

// Reciprocal hyperbolic

vec2 csech(vec2 z){
    return reciprocal(ccosh(z));
}

vec2 ccsch(vec2 z){
    return reciprocal(csinh(z));
}

vec2 ccoth(vec2 z){
    return cdiv(ccosh(z),csinh(z));
}

// Inverse hyperbolic
vec2 casinh(vec2 z){
    vec2 root = csqrt(cadd(cmult(z,z),ONE));
    return clog(cadd(z,root));
}

vec2 cacosh(vec2 z){
    vec2 root = csqrt(csub(cmult(z,z),ONE));
    return clog(cadd(z,root));
}

vec2 catanh(vec2 z){
    vec2 term = cdiv(cadd(ONE,z),csub(ONE,z));
    return cmult(vec2(0.5f,0.0f),clog(term));
}

vec2 cacsch(vec2 z){
    return casinh(reciprocal(z));
}

vec2 casech(vec2 z){
    return cacosh(reciprocal(z));
}

vec2 cacoth(vec2 z){
    return catanh(reciprocal(z));
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
#define ARG SHADER_ARG
#define MAG SHADER_MAG
#define ASIN SHADER_ASIN
#define ACOS SHADER_ACOS
#define ATAN SHADER_ATAN
#define ACSC SHADER_ACSC
#define ASEC SHADER_ASEC
#define ACOT SHADER_ACOT
#define CSCH SHADER_CSCH
#define SECH SHADER_SECH
#define COTH SHADER_COTH
#define ASINH SHADER_ASINH
#define ACOSH SHADER_ACOSH
#define ATANH SHADER_ATANH
#define ACSCH SHADER_ACSCH
#define ASECH SHADER_ASECH
#define ACOTH SHADER_ACOTH

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
    #define SHADER_ARG 23u
    #define SHADER_MAG 24u
    #define SHADER_ASIN 25u
    #define SHADER_ACOS 26u
    #define SHADER_ATAN 27u
    #define SHADER_ACSC 28u
    #define SHADER_ASEC 29u
    #define SHADER_ACOT 30u
    #define SHADER_CSCH 31u
    #define SHADER_SECH 32u
    #define SHADER_COTH 33u
    #define SHADER_ASINH 34u
    #define SHADER_ACOSH 35u
    #define SHADER_ATANH 36u
    #define SHADER_ACSCH 37u
    #define SHADER_ASECH 38u
    #define SHADER_ACOTH 39u
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
        case MAG:
            return cmag(param);
        case ARG:
            return carg(param);
        case ASIN:
            return casin(param);
        case ACOS:
            return cacos(param);
        case ATAN:
            return catan(param);
        case ACSC:
            return cacsc(param);
        case ASEC:
            return casec(param);
        case ACOT:
            return cacot(param);
        case CSCH:
            return ccsch(param);
        case SECH:
            return csech(param);
        case COTH:
            return ccoth(param);
        case ASINH:
            return casinh(param);
        case ACOSH:
            return cacosh(param);
        case ATANH:
            return catanh(param);
        case ACSCH:
            return cacsch(param);
        case ASECH:
            return casech(param);
        case ACOTH:
            return cacoth(param);
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

#define COLOR_FUNCTIONS HERE

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

#define END_COLOR_FUNCTIONS HERE

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