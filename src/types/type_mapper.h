#pragma once
#include <types/types.h>
#include <preprocessor/string_builder.h>
#include <stdexcept>

void initialize_map_id();

FullOperator get_full_operator(const Operator op);

FullOperator get_full_operator(const std::string& rep);

TokenOperator get_token_operator(const Operator op);

TokenOperator get_token_operator(const std::string& rep);

