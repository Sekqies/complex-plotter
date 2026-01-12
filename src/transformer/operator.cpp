#include <transformer/operator.h>
#include <preprocessor/preprocessor.h>
vector<unsigned char> get_operator_stack(const vector<TokenOperator>& stack) {
	vector<unsigned char> out;
	for (const TokenOperator& op : stack) {
		if (op.op == Operator::COMMA || op.arity == Arity::PAREN) continue;
		out.push_back(get_opcode(op.op));
	}
	out.push_back(static_cast<unsigned char>(255u));
	return out;
}
