#include <transformer/tree.h>

unique_ptr<AstNode> stack_to_syntax_tree(const vector<TokenOperator>& stack) {
	std::stack<unique_ptr<AstNode>> operator_stack;
	for (const TokenOperator& op : stack) {
		auto current_node = std::make_unique<AstNode>(op);
		if (op.arity == Arity::NULLARY) {
			operator_stack.push(current_node);
			continue;
		}
		unsigned int arity_val = static_cast<unsigned int>(op.arity);
		AstNode curr = { op };
		vector<unique_ptr<AstNode>> temp_children;
		while (arity_val--) {
			temp_children.push_back(std::move(operator_stack.top()));
			operator_stack.pop();
		}
		// 2 3 ^ is 3 ^ 2, not 2 ^ 3
		std::reverse(temp_children.begin(), temp_children.end());
		for (const auto& child : temp_children) {
			current_node->children.push_back(std::move(child));
		}
		operator_stack.push(std::move(current_node));
	}
	if (operator_stack.size() != 1) {
		throw std::runtime_error("Malformed expression");
	}
	return std::move(operator_stack.top());
}

void traverse(const AstNode* node, vector<TokenOperator>& result_stack) {
	if (!node) return;
	for (const auto& child : node->children) {
		traverse(child.get(), result_stack);
	}
	result_stack.push_back(node->op);
}

vector<TokenOperator> syntax_tree_to_stack(const unique_ptr<AstNode> head) {
	if (!head) {
		throw std::runtime_error("Syntax tree is empty");
	}
	vector<TokenOperator> stack;
	traverse(head.get(), stack);
	return stack;
}
