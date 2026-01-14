#include <parser/tokenizer.h>
#include <utility>
#include <queue>
#include <stdexcept>
const std::map<string, TokenOperator>& generate_operator_map(const vector<TokenOperator>& operators) {
	static std::map<string, TokenOperator> out;
	for (const TokenOperator& op : operators) {
		out[op.str_repr] = op;
	}
	return out;
}

bool is_operator(const string& s) {
	static const std::map<string, TokenOperator>& m = generate_operator_map(operators);
	return m.contains(s);
}

TokenOperator get_operator(const string& s) {
	static const std::map<string, TokenOperator>& m = generate_operator_map(operators);
	if (is_number(s)) {
		TokenOperator val = m.at("CONSTANT");
		size_t idx;
		const float value = std::stof(s, &idx);
		if (idx != s.size()) {
			throw std::runtime_error("Invalid number: '" + s + "'");
		}
		val.value = { std::stof(s),0.0f };
		return val;
	}
	if (!m.contains(s)) {
		throw std::runtime_error("Unknown token or variable '" + s + "'");
	}
	return m.at(s);
}

string handle_multicharacter(const string& s, size_t& index, const std::function<bool(const char)>& check) {
	string out = "";
	out += s[index];
	while (index + 1 < s.size() && check(s[index + 1])) {
		++index;
		out += s[index];
	}
	return out;
}

void handle_ambiguous_operator(vector<TokenOperator>& tokens, const vector<AmbiguousOperator>& amb_ops) {
	for (size_t i = 0; i < tokens.size(); ++i) {
		for (const auto& amb : amb_ops) {
			if (tokens[i].op != amb.general_rep) {
				continue;
			}
			if (is_unary_context(tokens, i)) {
				tokens[i] = amb.op1;
			}
			else {
				tokens[i] = amb.op2;
			}
			break;
		}
	}
}
vector<TokenOperator> handle_implicit(vector<TokenOperator>& tokens) {
	using std::pair, std::queue;
	const TokenOperator times = get_operator("*");
	queue<pair<size_t, TokenOperator>> pos;

	for (size_t i = 0; i < tokens.size(); ++i) {
		if (i == tokens.size() - 1) continue;

		const TokenOperator& token = tokens[i];
		if (token.arity != Arity::NULLARY && token.op != Operator::RPAREN) continue;

		const TokenOperator& next = tokens[i + 1];
		if (next.arity == Arity::NULLARY || next.op == Operator::LPAREN)
			pos.push({ i, times });
	}

	vector<TokenOperator> out;
	out.reserve(tokens.size() + pos.size());
	for (size_t i = 0; i < tokens.size(); ++i) {
		out.push_back(tokens[i]);
		if (!pos.empty() && pos.front().first == i) {
			out.push_back(pos.front().second);
			pos.pop();
		}
	}
	return out;
}

vector<TokenOperator> tokenize(const string& s) {
	vector<TokenOperator> tokens;
	for (size_t i = 0; i < s.size(); ++i) {
		const char c = s[i];
		if (c == ' ') continue;
		if (!is_digit(c) && !is_character(c)) {
			if (!is_operator({c})) {
				throw std::runtime_error("Unknown character: '" + string(c,0) + "'");
			}
			tokens.push_back(get_operator({ c }));
		}
		if (is_digit(c)) {
			const string number = handle_multicharacter(s, i, is_digit);
			tokens.push_back(get_operator(number));
		}
		if (is_character(c)) {
			const string word = handle_multicharacter(s, i, is_character);
			tokens.push_back(get_operator(word));
		}
	}
	tokens = handle_implicit(tokens);
	handle_ambiguous_operator(tokens, amb_operators);
	return tokens;
}

