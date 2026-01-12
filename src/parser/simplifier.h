#pragma once
#include <parser/types.h>
#include <vector>
#include <stack>
#include <complex>
#include <cmath>
#include <glm/glm.hpp>

using std::vector, std::stack, std::complex;

complex<float> to_complex(const TokenOperator& t);

TokenOperator from_complex(const complex<float>& c);

bool is_foldable(const vector<TokenOperator>& fragment);

complex<float> eval_unary_op(Operator op, complex<float> val);

complex<float> eval_binary_op(Operator op, complex<float> lhs, complex<float> rhs);

void handle_unary(stack<vector<TokenOperator>>& s, const TokenOperator& op_token);

void handle_binary(stack<vector<TokenOperator>>& s, const TokenOperator& op_token);

vector<TokenOperator> constant_fold(const vector<TokenOperator>& tokens);

vector<TokenOperator> simplify(const vector<TokenOperator>& rpn);