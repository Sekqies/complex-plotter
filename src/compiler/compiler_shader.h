#pragma once
#include <shader/shader.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <array>


class CompilerShader {
public:
	std::string frag_source;
	std::string vert_source;
	Shader shader;
	CompilerShader(const std::string& vertex_path, const std::string& frag_path, const bool use_vertex = false);
	
	CompilerShader() = default;
	
	void compile(const std::string glsl_expression, const bool use_vertex);

	void prepare_source(std::string& vertex_source, const std::string& frag_source, const bool use_vertex = false);

	static size_t find_end(const std::string& source, const std::string& substr, const size_t start = 0);

private:
	void clean_source(std::string& source);
	void find_injection_point(std::string& source);
	std::string generate_full_source(std::string& source, const std::string& expression);
	size_t injection_point;
};