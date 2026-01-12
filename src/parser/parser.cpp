#include <parser/parser.h>

vector<TokenOperator> parser::parse(const string& s) {
	vector<TokenOperator> tokens = tokenize(s);
	tokens = to_rpn(tokens);
	tokens = simplify(tokens);
	return tokens;
}

string parser::stack_to_str(const vector<TokenOperator>& stack) {
	string out = "";
	for (const TokenOperator& op : stack) {
		if (op.op != Operator::CONSTANT) {
			out += op.str_repr + " ";
			continue;
		}

		out += "(" + std::to_string(op.value.x) + "," + std::to_string(op.value.y) + "i)";
	}
	return out;
}
