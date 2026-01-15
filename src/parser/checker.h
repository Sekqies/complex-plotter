#include <types/types.h>
#pragma once

using std::vector, std::string;

bool is_digit(const char c);

bool is_character(const char c);

bool is_number(const std::string& s);

bool is_word(const std::string& s);

bool is_unary_context(const vector<TokenOperator>& tokens, size_t index);

