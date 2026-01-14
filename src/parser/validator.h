#pragma once
#include <vector>
#include <stack>
#include <parser/types.h>
#include <stdexcept>
using std::vector;

bool is_valid_rpn(const vector<TokenOperator>& operator_stack);