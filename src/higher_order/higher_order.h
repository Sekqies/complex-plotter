#pragma once
#include <types/types.h>
#include <parser/parser.h>
#include <transformer/tree.h>

using std::vector, std::string, std::unique_ptr;


unique_ptr<AstNode> clone(const AstNode* node);


/// <summary>
/// Builds a syntax subtree expression from a given parser string.
/// Warning: Avoid passing expressions with other higher order functions, and with arity higher than 2.
/// </summary>
/// <param name="expr">The expression to be evaluated, in infix. This follows the same syntax rules as expression passed to the parser.</param>
/// <returns></returns>
unique_ptr<AstNode> build_expression(const string& expr);



unique_ptr<AstNode> substitute(unique_ptr<AstNode> node, const Operator replaced, const AstNode& replacement);

unique_ptr<AstNode> substitute_many(unique_ptr<AstNode> node, const std::map<Operator, const AstNode*>& replacements);

std::unique_ptr<AstNode> make_node(Operator op, std::unique_ptr<AstNode> left, std::unique_ptr<AstNode> right);

std::unique_ptr<AstNode> make_constant(const glm::vec2 val);

std::unique_ptr<AstNode> make_constant(const float x , const float y);