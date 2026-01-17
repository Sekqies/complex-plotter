#pragma once

#include <parser/rpn.h>
#include <parser/simplifier.h>
#include <parser/validator.h>

namespace parser {
	vector<TokenOperator> parse(const string& s, const bool b = true);
	string stack_to_str(const vector<TokenOperator>& stack);
}

