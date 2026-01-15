#pragma once

#include <string>
#include <functional>



#include <parser/checker.h>
#include <vector>
#include <map>


using std::string, std::vector;


const std::map<string, TokenOperator>& generate_operator_map(const vector<TokenOperator>& operators);



bool is_operator(const string& s);

TokenOperator get_operator(const string& s);


string handle_multicharacter(const string& s, size_t& index, const std::function<bool(const char)>& check);

void handle_ambiguous_operator(vector<TokenOperator>& tokens, const vector<AmbiguousOperator>& amb_ops);

vector<TokenOperator> tokenize(const string& s);