#include <parser/validator.h>
#include <types/types.h>

bool is_valid_rpn(const vector<TokenOperator>& operator_stack) {
	int stack_depth = 0;
	std::stack<TokenOperator> stack;
	for (const TokenOperator& op : operator_stack) {
		if (op.arity == Arity::NULLARY) {
			stack_depth++;
			continue;
		}
		if (op.arity == Arity::PAREN) {
			throw std::runtime_error("Parenthesis mismatch");
		}
		const unsigned int value_arity = static_cast<unsigned int>(op.arity);
		if (stack_depth < value_arity) {
			throw std::runtime_error("Wrong arity for operator '" + op.str_repr + "'");
			return false;
		}
		stack_depth -= value_arity - 1;
	}
	return stack_depth == 1;
}
