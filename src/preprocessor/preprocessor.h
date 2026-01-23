#pragma once
#include <parser/parser.h>
#include <preprocessor/string_builder.h>
#include <fstream>
#include <iostream>

string get_source(const string& filename);

void preprocess_string(const string& filename, const vector<TokenOperator>& operators);

void preprocess_file(const string& filename, const vector<TokenOperator>& operators);

string build_shader_string(const string& new_shader, const string& origin);

string get_block(const string& source, const string& tag);

string erase_block(const string& source, const string& tag);

string inject_at(const string& source, const string& tag, const string& payload);

void inject_at(string& source, const string& tag, const string& payload);