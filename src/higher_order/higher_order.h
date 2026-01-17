#include <types/types.h>
#include <parser/parser.h>
#include <transformer/tree.h>

using std::vector, std::string, std::unique_ptr;


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
AstNode build_expression(const string& expr) {
	vector<TokenOperator> stack = parser::parse(expr);
	return *stack_to_syntax_tree(stack);
}



unique_ptr<AstNode> substitute(unique_ptr<AstNode> node, const Operator replaced, const AstNode& replacement) {
	if (!node) return nullptr;
	if (node->op.op == replaced) {
		return clone(&replacement);
	}
	for (auto& child : node->children) {
		child = substitute(std::move(node), replaced, replacement);
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