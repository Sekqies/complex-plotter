#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <filesystem>
#include <set>

std::string transpile_glsl_to_cpp(std::string code) {
    code = std::regex_replace(code, std::regex(R"(//.*|/\*[\s\S]*?\*/)"), "");
    code = std::regex_replace(code, std::regex(R"(=\s*(?:big_)?float\s*\[\s*\]\s*\(((?:.|\n)*?)\)\s*;)"), "= { $1 };");
    code = std::regex_replace(code, std::regex(R"(\bvec2\b)"), "big_vec2");
    code = std::regex_replace(code, std::regex(R"(\bfloat\b)"), "big_float");
    code = std::regex_replace(code, std::regex(R"(\bconst\b)"), "");
    code = std::regex_replace(code, std::regex(R"(\b([0-9]+\.[0-9]+(?:[eE][+-]?[0-9]+)?)(?:f|F)?\b)"), "big_float(\"$1\")");
    code = std::regex_replace(code, std::regex(R"(\bisnan\b)"), "boost::math::isnan");
    code = std::regex_replace(code, std::regex(R"(\bsqrt\b)"), "boost::multiprecision::sqrt");
    code = std::regex_replace(code, std::regex(R"(\bbig_vec2\s+([a-zA-Z_]\w*)\s*\()"), "inline big_vec2 $1(");
    return code;
}

std::vector<std::pair<std::string, int>> extract_functions(const std::string& code) {
    std::vector<std::pair<std::string, int>> funcs;
    std::regex func_regex(R"(big_vec2\s+(\w+)\s*\(([^)]+)\))");

    auto words_begin = std::sregex_iterator(code.begin(), code.end(), func_regex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator it = words_begin; it != words_end; ++it) {
        std::string func_name = (*it)[1].str();
        std::string params = (*it)[2].str();
        int arity = std::count(params.begin(), params.end(), ',') + 1;
        funcs.push_back({ func_name, arity });
    }
    return funcs;
}

void generate_cpp_files(const std::string& glsl_source, const std::string& out_dir) {
    std::string cpp_math_code = transpile_glsl_to_cpp(glsl_source);
    auto functions = extract_functions(cpp_math_code);

    std::filesystem::create_directories(out_dir);

    std::string header_path = out_dir + "/generated_big_math.h";
    std::ofstream math_out(header_path);
    math_out << "#pragma once\n"
        << "#include <glsl_generated/types.h>"
        << "// --- GLSL Math implementation for big_vec2 ---\n"
        << "inline big_float length(const big_vec2& v) { return boost::multiprecision::sqrt(v.x*v.x + v.y*v.y); }\n"
        << "inline big_float log(big_float f) { return boost::multiprecision::log(f); }\n"
        << "inline big_float exp(big_float f) { return boost::multiprecision::exp(f); }\n"
        << "inline big_float sin(big_float f) { return boost::multiprecision::sin(f); }\n"
        << "inline big_float cos(big_float f) { return boost::multiprecision::cos(f); }\n"
        << "inline big_float sinh(big_float f) { return boost::multiprecision::sinh(f); }\n"
        << "inline big_float cosh(big_float f) { return boost::multiprecision::cosh(f); }\n"
        << "inline big_float atan(big_float y, big_float x) { return boost::multiprecision::atan2(y, x); }\n"
        << "inline big_vec2 vector_floor(big_vec2 v) { return big_vec2(boost::multiprecision::floor(v.x), boost::multiprecision::floor(v.y)); }\n\n"
        << "// --- Transpiled GLSL Functions ---\n"
        << cpp_math_code << "\n";
    math_out.close();

    std::string mapper_path = out_dir + "/generated_math_mapper.h";
    std::ofstream map_out(mapper_path);
    map_out << "#pragma once \n #include \"generated_big_math.h\"\n"
        << "#include \"types/type_mapper.h\"\n"
        << "#include <functional>\n"
        << "#include <map>\n\n"
        << "struct CPU_Interpreter {\n"
        << "    std::map<Operator, std::function<big_vec2(big_vec2)>> unary_ops;\n"
        << "    std::map<Operator, std::function<big_vec2(big_vec2, big_vec2)>> binary_ops;\n\n"
        << "    CPU_Interpreter() {\n";

    for (const auto& [name, arity] : functions) {
        map_out << "        try {\n"
            << "            Operator op = get_token_operator_from_glsl(\"" << name << "\").op;\n";

        if (arity == 1) {
            map_out << "            unary_ops[op] = [](big_vec2 a) { return " << name << "(a); };\n";
        }
        else if (arity == 2) {
            map_out << "            binary_ops[op] = [](big_vec2 a, big_vec2 b) { return " << name << "(a, b); };\n";
        }

        map_out << "        } catch (...) { }\n\n";
    }

    map_out << "    }\n"
        << "};\n";
    map_out.close();
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input_glsl_file> <output_directory>\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_dir = argv[2];

    std::ifstream file(input_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open input file: " << input_file << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    try {
        generate_cpp_files(buffer.str(), output_dir);
        std::cout << "[Transpiler] Successfully generated C++ math files in " << output_dir << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[Transpiler] Error during generation: " << e.what() << "\n";
        return 1;
    }

    return 0;
}