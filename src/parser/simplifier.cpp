#include <parser/simplifier.h>
#include <cmath>
complex<float> to_complex(const TokenOperator& t) {
    return complex<float>(t.value.x, t.value.y);
}

TokenOperator from_complex(const complex<float>& c) {
    TokenOperator t;
    t.op = Operator::CONSTANT;
    t.arity = Arity::NULLARY;
    t.value = glm::vec2(c.real(), c.imag());
    t.str_repr = "CONSTANT";
    return t;
}

complex<float> cfloor(const complex<float>& c) {
    return complex<float>(std::floor(c.real()), std::floor(c.imag()));
}

bool is_foldable(const vector<TokenOperator>& fragment) {
    return fragment.size() == 1 && fragment[0].op == Operator::CONSTANT;
}
complex<float> eval_unary_op(Operator op, complex<float> val, bool& success) {
    static const complex<float> one(1.0f, 0.0f);
    static const complex<float> i(0.0f, 1.0f);

    switch (op) {
        // BASIC
    case Operator::NEG: return -val;
    case Operator::MAG: return complex<float>(std::abs(val), 0.0f);
    case Operator::ARG: return complex<float>(std::arg(val), 0.0f);
    case Operator::RECIPROCAL: return one / val;
    case Operator::SQRT: return std::sqrt(val);

        // EXPONENTIAL
    case Operator::LOG: return std::log(val);
    case Operator::EXP: return std::exp(val);

        // TRIGONOMETRIC
    case Operator::SIN: return std::sin(val);
    case Operator::COS: return std::cos(val);
    case Operator::TAN: return std::tan(val);

        // RECIPROCAL TRIGONOMETRIC (1/f)
    case Operator::COSEC: return one / std::sin(val);
    case Operator::SEC:   return one / std::cos(val);
    case Operator::COT:   return one / std::tan(val);

        // INVERSE TRIGONOMETRIC
    case Operator::ASIN: return std::asin(val);
    case Operator::ACOS: return std::acos(val);
    case Operator::ATAN: return std::atan(val);

        // INVERSE RECIPROCAL TRIGONOMETRIC
    case Operator::ACSC: return std::asin(one / val);
    case Operator::ASEC: return std::acos(one / val);
    case Operator::ACOT: return std::atan(one / val);

        // HYPERBOLIC
    case Operator::SINH: return std::sinh(val);
    case Operator::COSH: return std::cosh(val);
    case Operator::TANH: return std::tanh(val);
    case Operator::CSCH: return one / std::sinh(val);
    case Operator::SECH: return one / std::cosh(val);
    case Operator::COTH: return one / std::tanh(val);

    case Operator::ASINH: return std::asinh(val);
    case Operator::ACOSH: return std::acosh(val);
    case Operator::ATANH: return std::atanh(val);
    case Operator::ACSCH: return std::asinh(one / val);
    case Operator::ASECH: return std::acosh(one / val);
    case Operator::ACOTH: return std::atanh(one / val);
    default: success = false;  return val;
    }
}

complex<float> eval_binary_op(Operator op, complex<float> lhs, complex<float> rhs, bool& success) {
    switch (op) {
    case Operator::ADD:  return lhs + rhs;
    case Operator::SUB:  return lhs - rhs;
    case Operator::MULT: return lhs * rhs;
    case Operator::DIV:  return lhs / rhs;
    case Operator::POW:  return std::pow(lhs, rhs);
    case Operator::MOD: {
        if (std::abs(rhs) < 1e-12) return complex<float>(0.0f); 
        complex<float> div = lhs / rhs;
        return lhs - rhs * cfloor(div);
    }

    default: success = false; return lhs;
    }
}

void handle_unary(stack<vector<TokenOperator>>& s, const TokenOperator& op_token) {
    if (s.empty()) {
        throw std::runtime_error("Missing operand for operator '" + op_token.str_repr + "'");
    }

    vector<TokenOperator> op1 = s.top();
    s.pop();

    bool success = true;
    if (!is_foldable(op1)) {
        op1.push_back(op_token);
        s.push(op1);
        return;
    }
    complex<float> val = to_complex(op1[0]);
    complex<float> res = eval_unary_op(op_token.op, val,success);
    if (success) {
        s.push({ from_complex(res) });
        return;
    }
    op1.push_back(op_token);
    s.push(op1);
}

void handle_binary(stack<vector<TokenOperator>>& s, const TokenOperator& op_token) {
    if (s.size() < 2) {
        throw std::runtime_error("Missing operands for binary operator '" + op_token.str_repr + "'");
    }

    vector<TokenOperator> op2 = s.top(); s.pop();
    vector<TokenOperator> op1 = s.top(); s.pop();

    bool success = true;

    if (!is_foldable(op1) || !is_foldable(op2)) {
        op1.insert(op1.end(), op2.begin(), op2.end());
        op1.push_back(op_token);
        s.push(op1);
        return;
    }
    complex<float> v1 = to_complex(op1[0]);
    complex<float> v2 = to_complex(op2[0]);
    complex<float> res = eval_binary_op(op_token.op, v1, v2,success);
    if (success) {
        s.push({ from_complex(res) });
        return;
    }
    op1.insert(op1.end(), op2.begin(), op2.end());
    op1.push_back(op_token);
    s.push(op1);
    
}

vector<TokenOperator> constant_fold(const vector<TokenOperator>& tokens) {
    stack<vector<TokenOperator>> s;

    for (const TokenOperator& token : tokens) {
        if (token.arity == Arity::NULLARY) {
            s.push({ token });
        }
        else if (token.arity == Arity::UNARY) {
            handle_unary(s, token);
        }
        else if (token.arity == Arity::BINARY) {
            handle_binary(s, token);
        }
        else if (token.arity == Arity::PAREN) {
        }
    }

    if (s.empty()) return {};
    return s.top();
}

vector<TokenOperator> simplify(const vector<TokenOperator>& rpn) {
    vector<TokenOperator> out = constant_fold(rpn);
    return out;
}