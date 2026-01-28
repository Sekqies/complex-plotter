#include <parser/rpn.h>
#include <stdexcept>
vector<TokenOperator> to_rpn(const vector<TokenOperator>& tokens) {
	std::stack<TokenOperator> operator_stack;
	bool expecting_operand = true;
	vector<TokenOperator> out;
	for (const TokenOperator& token : tokens) {
		if (token.op == Operator::COMMA) {
			if (token.op == Operator::COMMA) {
				while (!operator_stack.empty() && operator_stack.top().op != Operator::LPAREN) {
					out.push_back(operator_stack.top());
					operator_stack.pop();
				}
				if (operator_stack.empty()) {
					throw std::runtime_error("Misplaced comma or missing parenthesis");
				}
				expecting_operand = true;
				continue;
			}
		}
		if (token.arity == NULLARY) {
			if (!expecting_operand) {
				throw std::runtime_error("Unexpected operand '" + token.str_repr + "' missing operator");
			}
			expecting_operand = false;
			out.push_back(token);
			continue;
		}
		if (token.arity == UNARY) {
			operator_stack.push(token);
			expecting_operand = true;
			continue;
		}
		if (token.op == LPAREN) {
			operator_stack.push(token);
			continue;
		}
		if (token.op == RPAREN) {
			if (expecting_operand) {
				throw std::runtime_error("Unexpected ')': expected operand");
			}
			while (!operator_stack.empty() && operator_stack.top().op != Operator::LPAREN) {
				out.push_back(operator_stack.top());
				operator_stack.pop();
			}
			if (operator_stack.empty()) {
				throw std::runtime_error("Mismatched parenthesis ')' (missing opening paren)");
			}
			operator_stack.pop();
			expecting_operand = false;
			continue;
		}

		if (expecting_operand) {
			throw std::runtime_error("Unexpected operator: missing operand");
		}

		TokenOperator o1 = token;
		while (!operator_stack.empty() && operator_stack.top().op != LPAREN) {
			TokenOperator o2 = operator_stack.top();
			if (o2.arity == Arity::NULLARY || o2.arity == Arity::PAREN) {
				break;
			}
			bool should_pop = false;
			if (o1.as == Associativity::LEFT) {
				should_pop = (o2.precedence >= o1.precedence);
			}
			else {
				should_pop = (o2.precedence > o1.precedence);
			}
			if (should_pop) {
				out.push_back(o2);
				operator_stack.pop();
			}
			else {
				break;
			}
		}
		operator_stack.push(token);
		expecting_operand = true;
	}
	while (!operator_stack.empty()) {
		if (operator_stack.top().arity == Arity::PAREN) {
			throw std::runtime_error("Mismatched parenthesis '(' (missing closing paren)");
		}
		out.push_back(operator_stack.top());
		operator_stack.pop();
	}
	if (expecting_operand) {
		throw std::runtime_error("Trailing operator");
	}
	return out;
}
