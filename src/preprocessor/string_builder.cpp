#include <preprocessor/string_builder.h>
#include <types/type_mapper.h>
static std::map<Operator, unsigned char> operator_to_id_mapper;

static bool handled_const = false;

string get_operator_name(const Operator op) {
    const FullOperator fop = get_full_operator(op);
    const std::string& name = fop.gl_define_name;
    std::string gl_name = fop.gl_name;
    if (name.empty()) {
        for (char& c : gl_name) c = std::toupper(c);
        return gl_name;
    }
    return name;
}

void define_const(std::stringstream& ss, const Operator op) {
    ss << "#define " << get_operator_name(op) << " " << static_cast<unsigned int>(op) << "u\n";
}

void define_const(std::stringstream& ss, const string& name, const unsigned int val) {
    ss << "#define " << name << " " << val << "u\n";
}

void register_op(std::stringstream& ss, Operator op, unsigned int& count) {
    if (get_operator_name(op) == "") return;
    if (!operator_to_id_mapper.contains(op)) {
        operator_to_id_mapper[op] = count;
    }
    define_const(ss, get_operator_name(op), count);
    count++;
}

unsigned char get_opcode(Operator op) {
    const auto it = operator_to_id_mapper.find(op);
    if (it == operator_to_id_mapper.end())
        return 0u;
    return it->second;
}

void set_values(const vector<TokenOperator>& nullary, std::stringstream& ss, unsigned int& count) {
    for (const TokenOperator& op : nullary) {
        if (op.arity != NULLARY) continue;
        if (op.value != glm::vec2(0.0f)) continue;
        register_op(ss, op.op, count);
    }
    define_const(ss, "VALUE_BOUNDARY", count++);
}

void set_unary(const vector<TokenOperator>& unary, std::stringstream& ss, unsigned int& count) {
    for (const TokenOperator& op : unary) {
        if (op.arity != UNARY) continue;
        register_op(ss, op.op, count);
    }
    define_const(ss, "UNARY_BOUNDARY", count++);
}

void set_binary(const vector<TokenOperator>& binary, std::stringstream& ss, unsigned int& count) {
    for (const TokenOperator& op : binary) {
        if (op.arity != BINARY) continue;
        register_op(ss, op.op, count);
    }
    define_const(ss, "BINARY_BOUNDARY", count++);
}

string write_case(const FullOperator& op) {
    string arity_str = "";
    if (op.gl_define_name.empty()) return "";
    switch (op.token_operator.arity) {
    case NULLARY:
        if (op.token_operator.op == CONSTANT)
            arity_str = "stack[sp++] = texelFetch(constants,cp++).xy;";
        else
            arity_str = "stack[sp++] = " + op.gl_name;
        break;
    case UNARY:
        arity_str = "stack[sp-1] = " + op.gl_name + "(stack[sp-1]);"; break;
    case BINARY:
        arity_str = "stack[sp-1] = " + op.gl_name + "(stack[sp-1], b);"; ;
    }
    return "case " + op.gl_define_name + ": " + arity_str + "; break;\n";
}

string write_interpreter() {
    string out = R"(
const uint stack_size = 1024u;
vec2 run_stack(in usamplerBuffer operators, in samplerBuffer constants, in vec2 z){
    vec2 stack[16];
    int sp = 0;
    int cp = 0;
    for(int i=0;i<1024;++i){
        uint op = texelFetch(operators,i).x;
        if (op == END) break;
        if (op == NULL_SYMBOL) continue;
)";
    string arity_op[10];
    for (const FullOperator& fp : full_operators) {
        arity_op[fp.token_operator.arity] += write_case(fp);
    }
    out += "\n        if (op < VALUE_BOUNDARY) {\n            switch(op) {\n" + arity_op[0] + "            }\n        }";

    out += "\n        else if (op < BINARY_BOUNDARY) {\n            vec2 b = stack[--sp];\n            switch(op) {\n" + arity_op[2] + "            }\n        }";

    out += "\n        else {\n            switch(op) {\n" + arity_op[1] + "            }\n        }";

    out += R"(
    }
    return stack[sp-1];
}
)";
    return out;
}

string get_preprocessor_string(const vector<TokenOperator>& operators) {
    std::map<Arity, vector<TokenOperator>> arity_expressions;
    for (const TokenOperator& op : operators) {
        if (!arity_expressions.contains(op.arity)) {
            arity_expressions[op.arity] = vector<TokenOperator>();
        }
        arity_expressions[op.arity].push_back(op);
    }
    std::stringstream ss;
    unsigned int count = 0;
    define_const(ss, "NULL_SYMBOL", count++);
    set_values(arity_expressions.at(Arity::NULLARY), ss, count);
    set_binary(arity_expressions.at(Arity::BINARY), ss, count);
    set_unary(arity_expressions.at(Arity::UNARY), ss, count);

    define_const(ss, "END", 255u);
    std::cout << ss.str();

    return ss.str();
}
