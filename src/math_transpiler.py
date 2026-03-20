import sys 
import os
import re

def transpile_glsl_to_cpp(code):
    code = re.sub(r'//.*|/\*[\s\S]*?\*/', '', code)
    code = re.sub(r'=\s*(?:big_)?float\s*\[\s*\]\s*\(((?:.|\n)*?)\)\s*;', r'= { \1 };', code)
    code = re.sub(r'\bvec2\b', 'big_vec2', code)
    code = re.sub(r'\bfloat\b', 'big_float', code)
    code = re.sub(r'\bconst\b', '', code)
    code = re.sub(r'\b([0-9]+\.[0-9]+(?:[eE][+-]?[0-9]+)?)(?:f|F)?\b', r'big_float("\1")', code)
    code = re.sub(r'\bisnan\b', 'boost::math::isnan', code)
    code = re.sub(r'\bsqrt\b', 'boost::multiprecision::sqrt', code)
    code = re.sub(r'\bbig_vec2\s+([a-zA-Z_]\w*)\s*\(', r'inline big_vec2 \1(', code)
    return code

def extract_functions(code):
    funcs = []
    func_regex = re.compile(r'big_vec2\s+(\w+)\s*\(([^)]+)\)')
    
    for match in func_regex.finditer(code):
        func_name = match.group(1)
        params = match.group(2)
        arity = params.count(',') + 1
        funcs.append((func_name, arity))
        
    return funcs

def generate_cpp_files(glsl_source, out_dir):
    cpp_code = transpile_glsl_to_cpp(glsl_source)
    functions = extract_functions(cpp_code)
    os.makedirs(out_dir, exist_ok=True)
    header_path = os.path.join(out_dir, "generated_big_math.h")
    with open(header_path, 'w', encoding='utf-8') as f:
        f.write("#pragma once\n")
        f.write("#include <glsl_generated/types.h>\n")
        f.write("inline big_float length(const big_vec2& v) { return boost::multiprecision::sqrt(v.x*v.x + v.y*v.y); }\n")
        f.write("inline big_float log(big_float f) { return boost::multiprecision::log(f); }\n")
        f.write("inline big_float exp(big_float f) { return boost::multiprecision::exp(f); }\n")
        f.write("inline big_float sin(big_float f) { return boost::multiprecision::sin(f); }\n")
        f.write("inline big_float cos(big_float f) { return boost::multiprecision::cos(f); }\n")
        f.write("inline big_float sinh(big_float f) { return boost::multiprecision::sinh(f); }\n")
        f.write("inline big_float cosh(big_float f) { return boost::multiprecision::cosh(f); }\n")
        f.write("inline big_float atan(big_float y, big_float x) { return boost::multiprecision::atan2(y, x); }\n")
        f.write("inline big_vec2 vector_floor(big_vec2 v) { return big_vec2(boost::multiprecision::floor(v.x), boost::multiprecision::floor(v.y)); }\n\n")
        f.write(cpp_math_code + "\n")

    mapper_path = os.path.join(out_dir, "generated_math_mapper.h")
    with open(mapper_path, 'w', encoding='utf-8') as f:
        f.write("#pragma once \n#include \"generated_big_math.h\"\n")
        f.write("#include \"types/type_mapper.h\"\n")
        f.write("#include <functional>\n")
        f.write("#include <map>\n\n")
        f.write("struct CPU_Interpreter {\n")
        f.write("    std::map<Operator, std::function<big_vec2(big_vec2)>> unary_ops;\n")
        f.write("    std::map<Operator, std::function<big_vec2(big_vec2, big_vec2)>> binary_ops;\n\n")
        f.write("    CPU_Interpreter() {\n")

        for name, arity in functions:
            f.write("        try {\n")
            f.write(f"            Operator op = get_token_operator_from_glsl(\"{name}\").op;\n")
            if arity == 1:
                f.write(f"            unary_ops[op] = [](big_vec2 a) {{ return {name}(a); }};\n")
            elif arity == 2:
                f.write(f"            binary_ops[op] = [](big_vec2 a, big_vec2 b) {{ return {name}(a, b); }};\n")
            f.write("        } catch (...) { }\n\n")

        f.write("    }\n")
        f.write("};\n")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        sys.exit(1)

    input_file = sys.argv[1]
    output_dir = sys.argv[2]

    try:
        with open(input_file, 'r', encoding='utf-8') as f:
            glsl_source = f.read()
        
        generate_cpp_files(glsl_source, output_dir)
    except Exception:
        sys.exit(1)