#include <transformer/transformer.h>


void get_stacks(const vector<TokenOperator>& stack, vector<unsigned char>& operator_stack, vector<glm::vec2>& constant_stack) {
	operator_stack = get_operator_stack(stack);
	constant_stack = get_constant_stack(stack);
}
