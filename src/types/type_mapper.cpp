#include <types/type_mapper.h>

using std::vector;

void initialize_map_id() {
	for (FullOperator& fop : full_operators) {
		fop.interpreter_id = get_opcode(fop.token_operator.op);
	}
}

std::map<Operator, FullOperator> init_full_operator_map() {
	std::map<Operator, FullOperator> m;
	for (const FullOperator& fop : full_operators) {
		if (fop.token_operator.op == Operator::CONSTANT && fop.token_operator.value != glm::vec2(0.0f)) continue;
		m[fop.token_operator.op] = fop;
	}
	return m;
}

std::map<std::string, FullOperator> init_str_operator_map() {
	std::map<std::string, FullOperator> m;
	for (const FullOperator& fop : full_operators) {
		if (fop.token_operator.op == Operator::CONSTANT && fop.token_operator.value != glm::vec2(0.0f)) continue;
		if (fop.token_operator.str_repr.empty()) continue;
		m[fop.token_operator.str_repr] = fop;
	}
	return m;
}


FullOperator get_full_operator(const Operator op) {
	static const std::map<Operator, FullOperator> m = init_full_operator_map();
	const auto it = m.find(op);
	if (it == m.end()) {
		std::string temp_str = std::to_string(static_cast<unsigned int>(op));
		throw std::runtime_error("Operator with opcode " + temp_str + "not found");
	}
	return it->second;
}

FullOperator get_full_operator(const std::string& rep) {
	static const std::map<std::string, FullOperator> m = init_str_operator_map();
	const auto it = m.find(rep);
	if (it == m.end()) {
		throw std::runtime_error("Operator with '" + rep + "' does not exist");
	}
	return it->second;
}

TokenOperator get_token_operator(const Operator op) {
	return get_full_operator(op).token_operator;
}

TokenOperator get_token_operator(const std::string& rep) {
	return get_full_operator(rep).token_operator;
}

