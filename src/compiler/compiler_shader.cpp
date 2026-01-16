#include <compiler/compiler_shader.h>

CompilerShader::CompilerShader(const std::string& vertex_path, const std::string& frag_path) {
	std::ifstream vfile;
	std::ifstream ffile;
	vfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	ffile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		vfile.open(vertex_path);
		ffile.open(frag_path);
		std::stringstream v_ss, f_ss;

		v_ss << vfile.rdbuf();
		f_ss << ffile.rdbuf();

		vfile.close();
		ffile.close();

		vert_source = v_ss.str();
		frag_source = f_ss.str();
	}
	catch (const std::ifstream::failure& e) {
		std::cerr << "Error: failed reading contents of file '" << vertex_path << "' or '" << frag_path << "'";
	}
	clean_frag_source();
	find_injection_point();
}

void CompilerShader::compile(const std::string glsl_expression) {
	shader.compile(vert_source, generate_full_source(glsl_expression));
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

void CompilerShader::clean_frag_source() {
	constexpr std::array kill_these_guys = { "INTERPRETER_SPECIFIC_FUNCTIONS","INTERPRETER_ASSIGNEMENT" };
	size_t prev_end = 0;
	for (const std::string& s : kill_these_guys) {
		const size_t start = frag_source.find("#define " + s) - 1;
		const size_t end = find_end(frag_source, "#define END_" + s);
		const size_t end_of_line = frag_source.find('\n', end);
		if (start == std::string::npos || end == std::string::npos) {
			std::cerr << "Formatting error for tag " << s;
			continue;
		}
		frag_source.erase(start, end_of_line - start);
	}
}

void CompilerShader::find_injection_point() {
	injection_point = find_end(frag_source, "#define INJECTION_POINT HERE", 0);
	if (injection_point == std::string::npos) {
		std::cerr << "Injection point not found in shader";
	}
}

std::string CompilerShader::generate_full_source(const std::string& expression) {
	size_t line_end = frag_source.find('\n', injection_point);
	std::string copy = frag_source;
	copy.insert(line_end + 1, "\n vec2 func_value = " + expression + ";\n");
	std::cout << copy;
	return copy;
}
