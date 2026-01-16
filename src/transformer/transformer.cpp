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
		std::string out = str_repr + "(";
		while (arity_val--) {
			out += str_stack.top();
			if (arity_val != 0)
				out += ",";
			str_stack.pop();
		}
		out += ")";
		str_stack.push(out);
	}
	return str_stack.top();
}
