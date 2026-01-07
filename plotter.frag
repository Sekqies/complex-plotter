#version 330

out vec4 FragColor;

const uint STACK_SIZE = 1024;

const uint END = 255u;
const uint NULL_SYMBOL = 0u;
const uint CONSTANT = 1u;
const uint VARIABLEX = 2u;
const uint VARIABLEY = 3u;
const uint VARIABLEZ = 4u;
const uint VARIABLET = 5u;
// Everything below VALUE_BOUNDARY is to be considered a "push" to the stack
const uint VALUE_BOUNDARY = 10u;
// Everything above VALUE_BOUNDARY is an operator on the stack

const uint ADD = 10u;
const uint SUB = 11u;
const uint MULT = 12u;
const uint DIV = 13u;
const uint BINARY_BOUNDARY = 20u;

const uint NEG = 21u;

uniform samplerBuffer constant_stack;
uniform usamplerBuffer operator_stack;
uniform float time;

vec2 eval_symbol 
// sin(x) + 3 + sin(z) * 5
// x sin 3 + z sin + 5 *

vec2 evaluate_constant_operator(in uint operator, in samplerBuffer constants, inout uint constant_index, in vec2 pos){
    switch(operator){
        case CONSTANT:
            return texelFetch(constants,int(constant_index++)).xy;
            break;
        case VARIABLEX:
            return vec2(pos.x,0.0f);
            break;
        case VARIABLEY:
            return vec2(pos.y,0.0f);
            break;
        case VARIABLEZ:
            return pos;
            break;
        case VARIABLET:
            return vec2(time,0.0f);
            break;
        default:
            return vec2(0.0f);
    }
}

vec2 pop_one(in vec2 stack[16], inout uint stack_index){
    return stack[--stack_index];
}
vec2[2] pop_two(in vec2 stack[16], inout uint stack_index){
    vec2 a = pop_one(stack,stack_index);
    vec2 b = pop_one(stack,stack_index);
    return vec2[2](a,b);
}
vec2 evaluate_unary_operator(in uint operator, inout vec2 stack[16], inout uint stack_index){
    vec2 param = pop_one(stack,stack_index);
    switch(operator){
        case NEG:
            return -param;
            break;
    }
}

vec2 mult(vec2 a, vec2 b){
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

vec2 div(vec2 a, vec2 b){
    return mult(a,vec2(b.x,-b.y))/(b.x*b.x+b.y*b.y);
}

vec2 evaluate_binary_operator(in uint operator, inout vec2 stack[16], inout uint stack_index){
    vec2 params[2] = pop_two(stack,stack_index);
    vec2 a = params[1];
    vec2 b = params[0];
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

vec2 evaluate_stack_operator(in uint operator, inout vec2 stack[16], inout uint stack_index){
    if(operator < BINARY_BOUNDARY)
        return evaluate_binary_operator(operator,stack,stack_index);
    return evaluate_unary_operator(operator,stack,stack_index);
}


vec2 run_stack(in usamplerBuffer operators, in samplerBuffer constants, in vec3 pos){
    vec2 stack[16];
    uint stack_index = 0u;
    uint constant_index = 0u;
    vec2 output = vec2(0.0f);
    for(int i = 0; i < 1024; ++i){
        uint operator = texelFetch(operators,i).x;
        if(operator == END){
            break;
        }
        vec2 result;
        if(operator < VALUE_BOUNDARY)
            result = evaluate_constant_operator(operator,constants, constant_index, pos);
        else{
            result = stack[stack_index++] = evaluate_stack_operator(operator,stack,stack_index);
        }
        stack[stack_index++] = result;
    }
    return (stack_index > 0)? stack[stack_index-1] : vec2(0.0f);
}

void main(){
    
}