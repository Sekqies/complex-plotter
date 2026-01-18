#pragma once
#include <types/types.h>
#include <higher_order/higher_order.h>
#include <transformer/tree.h>
#include <map>

using std::vector, std::unique_ptr, std::shared_ptr, glm::vec2;


unique_ptr<AstNode> differentiate(AstNode* node, const Operator var);

void derivative(unique_ptr<AstNode>& node);