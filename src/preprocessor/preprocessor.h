#pragma once
#include <parser/parser.h>
#include <preprocessor/string_builder.h>
#include <fstream>
#include <iostream>

string get_source(const string& filename);

void preprocess(const string& filename, const vector<TokenOperator>& operators);