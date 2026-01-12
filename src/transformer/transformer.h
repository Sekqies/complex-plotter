#pragma once
#include <transformer/constant.h>
#include <transformer/operator.h>


/// <summary>/// Get an operator (unsigned char) and constant stack (glm::vec2) from a TokenOperator stack/// </summary>/// <param name="stack">The RPN TokenOperator stack</param>/// <param name="operator_stack">A queue of opcodes in RPN. Always send an empty vector here.</param>/// <param name="constant_stack">A queue of constants. Always send an empty vector here.</param>
void get_stacks(const vector<TokenOperator>& stack, vector<unsigned char>& operator_stack, vector<glm::vec2>& constant_stack);