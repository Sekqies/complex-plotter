#include <parser/simplifier.h>

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

bool is_foldable(const vector<TokenOperator>& fragment) {
    return fragment.size() == 1 && fragment[0].op == Operator::CONSTANT;
}

complex<float> eval_unary_op(Operator op, complex<float> val) {
    switch (op) {
    case Operator::NEG: return -val;
    case Operator::SIN: return std::sin(val);
    case Operator::COS: return std::cos(val);
    case Operator::TAN: return std::tan(val);
    default: return val;
    }
}

complex<float> eval_binary_op(Operator op, complex<float> lhs, complex<float> rhs) {
    switch (op) {
    case Operator::ADD: return lhs + rhs;
    case Operator::SUB: return lhs - rhs;
    case Operator::MULT: return lhs * rhs;
    case Operator::DIV: return lhs / rhs;
    case Operator::POW: return std::pow(lhs, rhs);
    default: return lhs;
    }
}

void handle_unary(stack<vector<TokenOperator>>& s, const TokenOperator& op_token) {
    if (s.empty()) return;

    vector<TokenOperator> op1 = s.top();
    s.pop();

    if (is_foldable(op1)) {
        complex<float> val = to_complex(op1[0]);
        complex<float> res = eval_unary_op(op_token.op, val);
        s.push({ from_complex(res) });
    }
    else {
        op1.push_back(op_token);
        s.push(op1);
    }
}

void handle_binary(stack<vector<TokenOperator>>& s, const TokenOperator& op_token) {
    if (s.size() < 2) return;

    vector<TokenOperator> op2 = s.top(); s.pop();
    vector<TokenOperator> op1 = s.top(); s.pop();

    if (is_foldable(op1) && is_foldable(op2)) {
        complex<float> v1 = to_complex(op1[0]);
        complex<float> v2 = to_complex(op2[0]);
        complex<float> res = eval_binary_op(op_token.op, v1, v2);
        s.push({ from_complex(res) });
    }
    else {
        op1.insert(op1.end(), op2.begin(), op2.end());
        op1.push_back(op_token);
        s.push(op1);
    }
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
    }

    if (s.empty()) return {};
    return s.top();
}

vector<TokenOperator> simplify(const vector<TokenOperator>& rpn) {
    vector<TokenOperator> out = constant_fold(rpn);
    return out;
}
