#pragma once
#include <parser/parser.h>
#include <sstream>
#include <map>
#include <iostream>

string get_operator_name(const Operator op);

unsigned char get_opcode(Operator op);

string get_preprocessor_string(const vector<TokenOperator>& operators);