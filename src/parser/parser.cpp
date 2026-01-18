#include <parser/parser.h>
#include <higher_order/derivative.h>
#include <iostream>
vector<TokenOperator> parser::parse(const string& s, const bool evaluate_higher_order) {
	vector<TokenOperator> tokens = tokenize(s);
	tokens = to_rpn(tokens);
	is_valid_rpn(tokens);
	if (is_higher_order(tokens) && evaluate_higher_order) {
		unique_ptr<AstNode> head = stack_to_syntax_tree(tokens);
		derivative(head);
		tokens = syntax_tree_to_stack(head.get());
	}

	tokens = simplify(tokens);
	std::cout << stack_to_str(tokens) << '\n';
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
