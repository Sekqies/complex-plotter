#include <higher_order/higher_order.h>
#include <types/type_mapper.h>

unique_ptr<AstNode> clone(const AstNode* node) {
	if (!node) return nullptr;
	auto new_node = std::make_unique<AstNode>(node->op);
	for (const auto& child : node->children) {
		new_node->children.push_back(clone(child.get()));
	}
	return new_node;
}

/// <summary>
/// Builds a syntax subtree expression from a given parser string.
/// Warning: Avoid passing expressions with other higher order functions, and with arity higher than 2.
/// </summary>
/// <param name="expr">The expression to be evaluated, in infix. This follows the same syntax rules as expression passed to the parser.</param>
/// <returns></returns>
unique_ptr<AstNode> build_expression(const string& expr) {
	vector<TokenOperator> stack = parser::parse(expr);
	return stack_to_syntax_tree(stack);
}

unique_ptr<AstNode> substitute(unique_ptr<AstNode> node, const Operator replaced, const AstNode& replacement) {
	if (!node) return nullptr;
	if (node->op.op == replaced) {
		return clone(&replacement);
	}
	for (auto& child : node->children) {
		child = substitute(std::move(child), replaced, replacement);
	}
	return node;
}

unique_ptr<AstNode> substitute_many(unique_ptr<AstNode> node, const std::map<Operator, const AstNode*>& replacements) {
	if (!node) return nullptr;
	auto it = replacements.find(node->op.op);
	if (it != replacements.end()) {
		return clone(it->second);
	}

	for (auto& child : node->children) {
		child = substitute_many(std::move(child), replacements);
	}
	return node;
}

std::unique_ptr<AstNode> make_node(Operator op, std::unique_ptr<AstNode> left, std::unique_ptr<AstNode> right) {
	TokenOperator tp = get_token_operator(op);
	auto node = std::make_unique<AstNode>(tp);
	node->children.push_back(std::move(left));
	node->children.push_back(std::move(right));
	return node;
}

std::unique_ptr<AstNode> make_constant(const glm::vec2 val) {
	TokenOperator tp = get_token_operator(Operator::CONSTANT);
	auto node = std::make_unique<AstNode>(tp);
	node->op.value = val;
	return node;
}

std::unique_ptr<AstNode> make_constant(const float x, const float y) {
	TokenOperator tp = get_token_operator(Operator::CONSTANT);
	auto node = std::make_unique<AstNode>(tp);
	node->op.value = glm::vec2(x, y);
	return node;
}
