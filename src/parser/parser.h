#pragma once

#include <parser/rpn.h>

namespace parser {
	vector<TokenOperator> parse(const string& s);
	string stack_to_str(const vector<TokenOperator>& stack);
}

