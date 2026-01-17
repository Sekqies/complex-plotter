#include <vector>
#include <stack>
#include <memory>
#include <types/types.h>
#include <stdexcept>

using std::vector, std::unique_ptr;

struct AstNode {
	TokenOperator op;
	vector<unique_ptr<AstNode>> children;
};

unique_ptr<AstNode> stack_to_syntax_tree(const vector<TokenOperator>& stack);

void traverse(const AstNode* node, vector<TokenOperator>& result_stack);

vector<TokenOperator> syntax_tree_to_stack(const unique_ptr<AstNode> head);
