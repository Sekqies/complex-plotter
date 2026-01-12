#include <preprocessor/string_builder.h>

static std::map<Operator, unsigned char> operator_to_id_mapper;

string get_operator_name(const Operator op) {
    static std::map<Operator, std::string> mapper = {
        { Operator::CONSTANT,  "CONSTANT" },
        { Operator::VARIABLEX, "VARIABLEX" },
        { Operator::VARIABLEY, "VARIABLEY" },
        { Operator::VARIABLEZ, "VARIABLEZ" },
        { Operator::VARIABLET, "VARIABLET" },
        { Operator::ADD,       "ADD" },
        { Operator::SUB,       "SUB" },
        { Operator::NEG,       "NEG" },
        { Operator::MULT,      "MULT" },
        { Operator::DIV,       "DIV" },
        { Operator::EXP,       "EXP" },
        { Operator::POW,       "POW" },
        { Operator::LPAREN,    "LPAREN" },
        { Operator::RPAREN,    "RPAREN" },
        { Operator::COMMA,     "COMMA" },
        { Operator::SIN,       "SIN" },
        { Operator::COS,       "COS" },
        { Operator::TAN,       "TAN" }
    };
    return mapper.at(op);
}

void define_const(std::stringstream& ss, const Operator op) {
    ss << "#define SHADER_" << get_operator_name(op) << " " << static_cast<unsigned int>(op) << "u\n";
}

void define_const(std::stringstream& ss, const string& name, const unsigned int val) {
    ss << "#define SHADER_" << name << " " << val << "u\n";
}

void register_op(std::stringstream& ss, Operator op, unsigned int& count) {

    if (!operator_to_id_mapper.contains(op)) {
        operator_to_id_mapper[op] = count;
    }
    define_const(ss, get_operator_name(op), count);
    count++;
}

unsigned char get_opcode(Operator op) {
    return operator_to_id_mapper.at(op);
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
