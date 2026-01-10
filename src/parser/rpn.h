#pragma once
#include <parser/tokenizer.h>
#include <parser/checker.h>
#include <parser/types.h>

#include <stack>
#include <vector>
#include <string>

using std::vector, std::string;

vector<TokenOperator> to_rpn(const vector<TokenOperator> tokens);