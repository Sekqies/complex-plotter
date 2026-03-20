#pragma once 
 #include "generated_big_math.h"
#include "types/type_mapper.h"
#include <functional>
#include <map>

struct CPU_Interpreter {
    std::map<Operator, std::function<big_vec2(big_vec2)>> unary_ops;
    std::map<Operator, std::function<big_vec2(big_vec2, big_vec2)>> binary_ops;

    CPU_Interpreter() {
        try {
            Operator op = get_token_operator_from_glsl("cadd").op;
            binary_ops[op] = [](big_vec2 a, big_vec2 b) { return cadd(a, b); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("csub").op;
            binary_ops[op] = [](big_vec2 a, big_vec2 b) { return csub(a, b); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("cmult").op;
            binary_ops[op] = [](big_vec2 a, big_vec2 b) { return cmult(a, b); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("carg").op;
            unary_ops[op] = [](big_vec2 a) { return carg(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("cmag").op;
            unary_ops[op] = [](big_vec2 a) { return cmag(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("cexp").op;
            unary_ops[op] = [](big_vec2 a) { return cexp(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("clog").op;
            unary_ops[op] = [](big_vec2 a) { return clog(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("cpow").op;
            binary_ops[op] = [](big_vec2 a, big_vec2 b) { return cpow(a, b); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("cdiv").op;
            binary_ops[op] = [](big_vec2 a, big_vec2 b) { return cdiv(a, b); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("csin").op;
            unary_ops[op] = [](big_vec2 a) { return csin(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("ccos").op;
            unary_ops[op] = [](big_vec2 a) { return ccos(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("csec").op;
            unary_ops[op] = [](big_vec2 a) { return csec(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("ccsc").op;
            unary_ops[op] = [](big_vec2 a) { return ccsc(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("ccot").op;
            unary_ops[op] = [](big_vec2 a) { return ccot(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("ctan").op;
            unary_ops[op] = [](big_vec2 a) { return ctan(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("csinh").op;
            unary_ops[op] = [](big_vec2 a) { return csinh(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("ccosh").op;
            unary_ops[op] = [](big_vec2 a) { return ccosh(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("ctanh").op;
            unary_ops[op] = [](big_vec2 a) { return ctanh(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("csqrt").op;
            unary_ops[op] = [](big_vec2 a) { return csqrt(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("reciprocal").op;
            unary_ops[op] = [](big_vec2 a) { return reciprocal(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("casin").op;
            unary_ops[op] = [](big_vec2 a) { return casin(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("cacos").op;
            unary_ops[op] = [](big_vec2 a) { return cacos(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("catan").op;
            unary_ops[op] = [](big_vec2 a) { return catan(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("cacsc").op;
            unary_ops[op] = [](big_vec2 a) { return cacsc(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("casec").op;
            unary_ops[op] = [](big_vec2 a) { return casec(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("cacot").op;
            unary_ops[op] = [](big_vec2 a) { return cacot(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("csech").op;
            unary_ops[op] = [](big_vec2 a) { return csech(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("ccsch").op;
            unary_ops[op] = [](big_vec2 a) { return ccsch(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("ccoth").op;
            unary_ops[op] = [](big_vec2 a) { return ccoth(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("casinh").op;
            unary_ops[op] = [](big_vec2 a) { return casinh(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("cacosh").op;
            unary_ops[op] = [](big_vec2 a) { return cacosh(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("catanh").op;
            unary_ops[op] = [](big_vec2 a) { return catanh(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("cacsch").op;
            unary_ops[op] = [](big_vec2 a) { return cacsch(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("casech").op;
            unary_ops[op] = [](big_vec2 a) { return casech(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("cacoth").op;
            unary_ops[op] = [](big_vec2 a) { return cacoth(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("cneg").op;
            unary_ops[op] = [](big_vec2 a) { return cneg(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("cmod").op;
            binary_ops[op] = [](big_vec2 a, big_vec2 b) { return cmod(a, b); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("conj").op;
            unary_ops[op] = [](big_vec2 a) { return conj(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("re").op;
            unary_ops[op] = [](big_vec2 a) { return re(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("im").op;
            unary_ops[op] = [](big_vec2 a) { return im(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("clngamma").op;
            unary_ops[op] = [](big_vec2 a) { return clngamma(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("cgamma").op;
            unary_ops[op] = [](big_vec2 a) { return cgamma(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("czeta_main_branch").op;
            unary_ops[op] = [](big_vec2 a) { return czeta_main_branch(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("czeta_negative_branch").op;
            unary_ops[op] = [](big_vec2 a) { return czeta_negative_branch(a); };
        } catch (...) { }

        try {
            Operator op = get_token_operator_from_glsl("czeta").op;
            unary_ops[op] = [](big_vec2 a) { return czeta(a); };
        } catch (...) { }

    }
};
