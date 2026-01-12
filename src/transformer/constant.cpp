#include <transformer/constant.h>

/// <summary>
/// Gets a stacks of constants from a TokenOperator rpn stack
/// </summary>
/// <param name="stack">The stack in RPN</param>
/// <returns></returns>
vector<vec2> get_constant_stack(const vector<TokenOperator>& stack) {
	vector<vec2> out;
	for (const TokenOperator& token : stack) {
		if (token.arity != Arity::NULLARY) continue;
		if (token.value != vec2(0.0f)) continue;
		if (token.op != Operator::CONSTANT) continue;
		out.push_back(token.value);
	}
	return out;
}
