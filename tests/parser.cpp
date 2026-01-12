#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <functional>
#include <map>
#include <stack>
using std::string, std::vector, std::cin, std::cout, std::pair, std::queue;

enum Arity {
	UNARY = 0,
	BINARY,
};

enum Associativity {
	LEFT = 0,
	RIGHT
};

typedef struct TokenOperator {
	Arity arity;
	Associativity as;
	string str_repr;
	unsigned int precedence;
} TokenOperator;

typedef struct AmbiguousOperator {
	TokenOperator op1;
	TokenOperator op2;
	string general_rep;
};


const vector<TokenOperator> operators = {
	{Arity::UNARY, Associativity::RIGHT, "~", 1},
	{Arity::BINARY, Associativity::LEFT, "-", 0},
	{Arity::BINARY, Associativity::LEFT, "+", 0},
	{Arity::BINARY, Associativity::LEFT, "*", 2},
	{Arity::BINARY, Associativity::RIGHT, "^", 3}
};

const vector<AmbiguousOperator> amb_operators = {
	{
		{Arity::UNARY, Associativity::RIGHT, "~", 1},
		{Arity::BINARY, Associativity::LEFT, "-", 0},
		"-"
	}
};

std::map<string, TokenOperator> generate_operator_map(const vector<TokenOperator>& operators) {
	std::map<string, TokenOperator> out;
	for (const TokenOperator& op : operators) {
		out[op.str_repr] = op;
	}
	return out;
}

TokenOperator get_operator(const string& s) {
	static std::map<string, TokenOperator> m = generate_operator_map(operators);
	return m.at(s);
}


bool is_digit(const char c) {
	const int digit = c - '0';
	return (digit >= 0 && digit <= 9) || c == '.';
}

bool is_character(const char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

string handle_multicharacter(const string& s, size_t& index, const std::function<bool(const char)>& check) {
	string out = "";
	out += s[index];
	while (index + 2 < s.size() && check(s[index + 1])) {
		++index;
		out += s[index];
	}
	return out;
}

vector<string> tokenize(const string s) {
	vector<string> tokens;
	for (size_t i = 0; i < s.size(); ++i) {
		const char c = s[i];
		if (c == ' ') continue;
		if (!is_digit(c) && !is_character(c)) {
			tokens.push_back({ c });
			continue;
		}
		string out = "";
		if (is_character(c)) {
			out = handle_multicharacter(s, i, is_character);
		}
		if (is_digit(c)) {
			out = handle_multicharacter(s, i, is_digit);
		}
		tokens.push_back(out);
	}
	return tokens;
}

bool is_operator(const string& s) {
	for (const auto& op : operators) {
		if (op.str_repr == s) return true;
	}
	return false;
}

bool is_unary_context(const vector<string>& tokens, size_t index) {
	if (index == 0) return true;

	const string& prev = tokens[index - 1];
	if (prev == "(") return true;
	if (is_operator(prev)) return true;

	return false;
}

void handle_ambiguous_operator(vector<string>& tokens, const vector<AmbiguousOperator>& amb_ops) {
	for (size_t i = 0; i < tokens.size(); ++i) {
		for (const auto& amb : amb_ops) {
			if (tokens[i] != amb.general_rep) {
				continue;
			}
			if (is_unary_context(tokens, i)) {
				tokens[i] = amb.op1.str_repr;
			}
			else {
				tokens[i] = amb.op2.str_repr;
			}
			break;
		}
	}
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

vector<string> to_rpn(const vector<string>& tokens) {
	std::stack<string> operator_stack;
	vector<string> out;

	for (const string& token : tokens) {
		if (is_number(token)) {
			out.push_back(token);
			continue;
		}
		if (is_word(token)) {
			out.push_back(token);
			continue;
		}

		if (token == "(") {
			operator_stack.push(token);
			continue;
		}

		if (token == ")") {
			while (!operator_stack.empty() && operator_stack.top() != "(") {
				out.push_back(operator_stack.top());
				operator_stack.pop();
			}
			if (!operator_stack.empty()) {
				operator_stack.pop();
			}
			continue;
		}
		TokenOperator o1 = get_operator(token);
		while (!operator_stack.empty() && operator_stack.top() != "(") {
			string top_str = operator_stack.top();
			if (!is_operator(top_str)) break;
			TokenOperator o2 = get_operator(top_str);
			bool should_pop = false;
			if (o1.as == Associativity::LEFT) {
				should_pop = (o2.precedence >= o1.precedence);
			}
			else {
				should_pop = (o2.precedence > o1.precedence);
			}
			if (should_pop) {
				out.push_back(top_str);
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


int main() {
	const string s = "sin(z^2 * tan(z)) + cos(z^z)";
	vector<string> tokens = tokenize(s);
	handle_ambiguous_operator(tokens, amb_operators);
	tokens = to_rpn(tokens);
	for (const string& token : tokens) {
		cout << token << " "; 
	}
	return 0;
}
