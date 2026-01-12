#pragma once
#include <parser/types.h>
#include <vector>

using std::vector;

vector<unsigned char> get_operator_stack(const vector<TokenOperator>& stack);