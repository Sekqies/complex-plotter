#pragma once

#include <parser/rpn.h>

namespace parser {
	vector<TokenOperator> parse(const string& s) {
		vector<TokenOperator> tokens = tokenize(s);
		return to_rpn(tokens);
	}
	string stack_to_str(const vector<TokenOperator>& stack) {
		string out = "";
		for (const TokenOperator& op : stack) {
			if (op.op != Operator::CONSTANT) {
				out += op.str_repr + " ";
				continue;
			}
			out += std::to_string(op.value) + " ";
		}
		return out;
	}
}

