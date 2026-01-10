#pragma once
#include <parser/tokenizer.h>
#include <parser/checker.h>
#include <parser/types.h>

#include <stack>
#include <vector>
#include <string>

using std::vector, std::string;

vector<TokenOperator> to_rpn(const vector<TokenOperator> tokens) {
	std::stack<TokenOperator> operator_stack;
	vector<TokenOperator> out;
	for (const TokenOperator& token : tokens) {
		if (token.op == Operator::COMMA) {
			continue;
		}
		if (token.arity == NULLARY) {
			out.push_back(token);
			continue;
		}
		if (token.arity == UNARY) {
			operator_stack.push(token);
		}
		if (token.op == LPAREN) {
			operator_stack.push(token);
			continue;
		}
		if (token.op == RPAREN) {
			while (operator_stack.top().op != Operator::LPAREN){
				out.push_back(operator_stack.top());
				operator_stack.pop();
			}
			operator_stack.pop();
			continue;
		}
		TokenOperator o1 = token;
		while (!operator_stack.empty() && operator_stack.top().op != LPAREN) {
			TokenOperator o2 = operator_stack.top();
			if (o2.arity == Arity::NULLARY && o2.arity == Arity::PAREN) {
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
	}
	while (!operator_stack.empty()) {
		out.push_back(operator_stack.top());
		operator_stack.pop();
	}
	return out;
}