#pragma once
#include <parser/parser.h>
#include <sstream>
#include <map>
#include <iostream>

string get_operator_name(const Operator op);

unsigned char get_opcode(Operator op);

string get_preprocessor_string(const vector<TokenOperator>& operators);

string write_interpreter();

string build_high_precision_shader_string(const std::string& highp_header, const std::string& highp_footer, const std::string& highp_function_declarations, const std::string& lowp_function_declarations);