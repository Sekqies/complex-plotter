#include <vector>

#pragma once

using std::vector, std::string;

bool is_digit(const char c) {
	const int digit = c - '0';
	return (digit >= 0 && digit <= 9) || c == '.';
}

bool is_character(const char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_number(const std::string& s)
{
	return(strspn(s.c_str(), ".0123456789") == s.size());
}

bool is_word(const std::string& s) {
	for (const char c : s) {
		if (!is_character(c)) return false;
	}
	return true;
}

bool is_unary_context(const vector<TokenOperator>& tokens, size_t index) {
	if (index == 0) return true;

	const TokenOperator& prev = tokens[index - 1];
	if (prev.op == Operator::LPAREN) return true;
	if (prev.arity != Arity::NULLARY) return true;

	return false;
}

