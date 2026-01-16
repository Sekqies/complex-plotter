#pragma once
#include <shader/shader.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <array>


class CompilerShader {
public:
	Shader shader;
	CompilerShader(const std::string& vertex_path, const std::string& frag_path);
	void compile(const std::string glsl_expression);

	static size_t find_end(const std::string& source, const std::string& substr, const size_t start = 0);

private:
	void clean_frag_source();
	void find_injection_point();
	std::string generate_full_source(const std::string& expression);
	std::string frag_source;
	std::string vert_source;
	size_t injection_point;
};