#include <transformer/transformer.h>
#include <map>
#include <stdexcept>
#include <types/type_mapper.h>
#include <stack>
#include <cmath>
#include <algorithm>

void get_stacks(const vector<TokenOperator>& stack, vector<unsigned char>& operator_stack, vector<glm::vec2>& constant_stack) {
    operator_stack = get_operator_stack(stack);
    constant_stack = get_constant_stack(stack);
}

string stack_to_glsl_string(const vector<TokenOperator>& stack) {
    std::stack<string> str_stack;
    for (const TokenOperator& op : stack) {
        const string str_repr = get_full_operator(op.op).gl_name;
        if (op.op == Operator::CONSTANT) {
            const string out = "vec2(" + std::to_string(op.value.x) + "," + std::to_string(op.value.y) + ")";
            str_stack.push(out);
            continue;
        }
        if (op.arity == Arity::NULLARY) {
            str_stack.push(str_repr);
            continue;
        }
        unsigned int arity_val = static_cast<unsigned int>(op.arity);
        vector<string> args;

        while (arity_val--) {
            if (str_stack.empty()) {
                throw std::runtime_error("RPN Underflow Error: Operator '" + str_repr + "' missing operands.");
            }
            args.push_back(str_stack.top());
            str_stack.pop();
        }
        std::reverse(args.begin(), args.end());
        std::string out = str_repr + "(";
        for (unsigned int i = 0; i < args.size(); ++i) {
            out += args[i];
            if (i != args.size() - 1)
                out += ",";
        }
        out += ")";
        str_stack.push(out);
    }
    if (str_stack.empty()) return "";
    return str_stack.top();
}

string format_hp_float(float val) {
    if (std::abs(val - 3.141592f) < 1e-4f) return "PI";
    if (std::abs(val - 2.718281f) < 1e-4f) return "E";
    if (std::abs(val) < 1e-6f) return "REAL_ZERO";
    if (std::abs(val - 1.0f) < 1e-6f) return "REAL_ONE";

    string str = std::to_string(val);
    if (str.find('.') == string::npos) str += ".0";
    return "float_to_number(" + str + "f)";
}

string stack_to_highp_glsl(const vector<TokenOperator>& stack, const string& out_var_name) {
    string out = "";
    int sp = 0;

    for (const TokenOperator& op : stack) {
        const string str_repr = get_full_operator(op.op).gl_name;

        if (op.op == Operator::CONSTANT) {
            string x_str = format_hp_float(op.value.x);
            string y_str = format_hp_float(op.value.y);
            out += "    E_V[" + std::to_string(sp) + "] = initialize_hp_vec2(" + x_str + ", " + y_str + ");\n";
            sp++;
            continue;
        }
        if (op.arity == Arity::NULLARY) {
            out += "    E_V[" + std::to_string(sp) + "] = " + str_repr + ";\n";
            sp++;
            continue;
        }

        string func = "hp_" + str_repr;

        if (op.arity == Arity::UNARY) {
            if (sp < 1) throw std::runtime_error("RPN Underflow Error: Operator '" + str_repr + "' missing operands.");
            sp--;
            string reg = "E_V[" + std::to_string(sp) + "]";
            out += "    " + func + "(" + reg + ", " + reg + ");\n";
            sp++;
        }
        else if (op.arity == Arity::BINARY) {
            if (sp < 2) throw std::runtime_error("RPN Underflow Error: Operator '" + str_repr + "' missing operands.");
            sp -= 2;
            string left_reg = "E_V[" + std::to_string(sp) + "]";
            string right_reg = "E_V[" + std::to_string(sp + 1) + "]";
            out += "    " + func + "(" + left_reg + ", " + right_reg + ", " + left_reg + ");\n";
            sp++;
        }
    }

    if (sp != 1) {
        throw std::runtime_error("Invalid RPN stack: Result did not resolve to a single value.");
    }

    out += "    " + out_var_name + " = E_V[0];\n";
    return out;
}

string vector_to_glsl_array(const vector<unsigned int>& limbs) {
    string out = "uint[" + std::to_string(limbs.size()) + "](";
    for (const unsigned int& limb : limbs) {
        out += std::to_string(limb) + "u,";
    }
    out.pop_back();
    out += ")";
    return out;
}

string big_number_to_glsl_string(const vector<unsigned int>& limbs, const int sign, const bool is_infinite = false) {
    return "number(" + vector_to_glsl_array(limbs) + "," + std::to_string(sign) + "," + (is_infinite ? "true" : "false") + ")";
}