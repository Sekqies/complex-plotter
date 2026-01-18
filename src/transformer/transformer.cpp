#include <transformer/transformer.h>
#include <map>
#include <stdexcept>
#include <types/type_mapper.h>
#include <stack>

void get_stacks(const vector<TokenOperator>& stack, vector<unsigned char>& operator_stack, vector<glm::vec2>& constant_stack) {
	operator_stack = get_operator_stack(stack);
	constant_stack = get_constant_stack(stack);
}

string stack_to_glsl_string(const vector<TokenOperator>& stack) {
	std::stack<string> str_stack;
	for (const TokenOperator& op : stack) {
		const string str_repr = get_full_operator(op.op).gl_name;
		if (op.op == Operator::CONSTANT) {
			const string out = "vec2(" + std::to_string(op.value.x) + "," + std::to_string(op.value.y) + ")";
			str_stack.push(out);
			continue;
		}
		if (op.arity == Arity::NULLARY) {
			str_stack.push(str_repr);
			continue;
		}
		unsigned int arity_val = static_cast<unsigned int>(op.arity);
		vector<string> args;

		while (arity_val--) {
			args.push_back(str_stack.top());
			str_stack.pop();
		}
		std::reverse(args.begin(), args.end());
		std::string out = str_repr + "(";
		for (unsigned int i = 0; i < args.size(); ++i) {
			out += args[i];
			if (i != args.size() - 1)
				out += ",";
		}
		out += ")";
		str_stack.push(out);
	}
	if (str_stack.empty()) return "";
	return str_stack.top();
}
