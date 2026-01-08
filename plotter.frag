#version 330

in vec2 pos;
out vec4 FragColor;

const uint STACK_SIZE = 1024;

const uint END = 255u;
const uint NULL_SYMBOL = 0u;
const uint CONSTANT = 1u;
const uint VARIABLEX = 2u;
const uint VARIABLEY = 3u;
const uint VARIABLEZ = 4u;
const uint VARIABLET = 5u;
//Everything below VALUE_BOUNDARY is a constant on the stack
const uint VALUE_BOUNDARY = 10u;
//Everything above VALUE_BONDARY is an operator on the stack

const uint ADD = 10u;
const uint SUB = 11u;
const uint MULT = 12u;
const uint DIV = 13u;
// Everything below BINARY_BOUNDARY and above VALUE_BOUNDARY is a binary operator.
const uint BINARY_BOUNDARY = 20u;

const uint NEG = 21u;


// Other useful constants
const float PI = 3.141591f;
const float TWO_PI_OVER_3 = 2*PI*0.66666f;
const float TWO_OVER_PI = 2.0f / PI;



uniform samplerBuffer constant_stack;
uniform usamplerBuffer operator_stack;
uniform float time;

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

vec2 mult(vec2 a, vec2 b){
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

vec2 div(vec2 a, vec2 b){
    return mult(a,vec2(b.x,-b.y))/(b.x*b.x+b.y*b.y);
}

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
            return mult(a,b);
        case DIV:
            return div(a,b);
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

        vec2 result;
        if(operator < VALUE_BOUNDARY)
            result = evaluate_constant_operator(operator,constants, constant_index, pos);
        else            
            result = evaluate_stack_operator(operator,stack,stack_index);
        
        stack[stack_index++] = result;
    }
    return (stack_index > 0)? stack[stack_index-1] : vec2(0.0f);
}

vec3 hsl2rgb(vec3 hsl) {
    vec3 rgb = clamp(abs(mod(hsl.x * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
    return hsl.z + hsl.y * (rgb - 0.5) * (1.0 - abs(2.0 * hsl.z - 1.0));
}

vec3 domain_color(in vec2 z){
    return vec3(atan(z.y,z.x) + TWO_PI_OVER_3, 1, TWO_OVER_PI * atan(length(z)));
}


void main(){
    vec2 func_value = run_stack(operator_stack,constant_stack,pos);
    vec3 hsl = domain_color(func_value);
    FragColor = vec4(hsl2rgb(hsl),1.0f);
}