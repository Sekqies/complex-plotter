#include <compiler/compiler_shader.h>
#include <preprocessor/preprocessor.h>
CompilerShader::CompilerShader(const std::string& vertex_path, const std::string& frag_path, const bool use_vertex) {
	vert_source = get_source(vertex_path);
	frag_source = get_source(frag_path);
	prepare_source(vert_source, frag_source, use_vertex);
}


void CompilerShader::prepare_source(std::string& vertex_source, const std::string& fragment_source, const bool use_vertex) {
	this->vert_source = vertex_source;
	this->frag_source = fragment_source;
	std::string* source = &frag_source;
	if (use_vertex) source = &vert_source;
	clean_source(*source);
	find_injection_point(*source);
}

void CompilerShader::compile(const std::string glsl_expression, const bool use_vertex) {
	if (use_vertex) {
		shader.compile(generate_full_source(vert_source, glsl_expression), frag_source);
		return;
	}
	shader.compile(vert_source, generate_full_source(frag_source,glsl_expression));
}

size_t CompilerShader::find_end(const std::string& source, const std::string& substr, const size_t start) {
	size_t pos = source.find(substr, start);
	if (pos == std::string::npos) {
		return std::string::npos;
	}
	if (substr.empty()) {
		return pos;
	}
	return pos + substr.size();
}

void CompilerShader::clean_source(std::string& source) {
	constexpr std::array kill_these_guys = { "INTERPRETER_SPECIFIC_FUNCTIONS","INTERPRETER_ASSIGNEMENT" };
	size_t prev_end = 0;
	for (const std::string& s : kill_these_guys) {
		const size_t start = source.find("#define " + s) - 1;
		const size_t end = find_end(source, "#define END_" + s);
		const size_t end_of_line = source.find('\n', end);
		if (start == std::string::npos || end == std::string::npos) {
			std::cerr << "Formatting error for tag " << s;
			continue;
		}
		source.erase(start, end_of_line - start);
	}
}

void CompilerShader::find_injection_point(std::string& source) {
	injection_point = find_end(source, "#define INJECTION_POINT HERE", 0);
	if (injection_point == std::string::npos) {
		std::cerr << "Injection point not found in shader";
	}
}

std::string CompilerShader::generate_full_source(std::string& source, const std::string& expression) {
	find_injection_point(source);
	size_t line_end = source.find('\n', injection_point);
	std::string copy = source;
	copy.insert(line_end + 1, "\n vec2 func_value = " + expression + ";\n");
	std::cout << copy;
	return copy;
}
