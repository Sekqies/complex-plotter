#include <preprocessor/transpiler.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <regex>
#include <memory>
#include <stdexcept>

using std::string;
using std::vector;

std::string remove_comments(const std::string& code) {
    std::regex comment_regex(R"(//.*|/\*[\s\S]*?\*/)");
    return std::regex_replace(code, comment_regex, "");
}

std::vector<std::string> tokenize(const std::string& source) {
    std::vector<std::string> tokens;
    std::string current;
    for (size_t i = 0; i < source.size(); ++i) {
        char c = source.at(i);
        if (std::isspace(c)) {
            if (!current.empty()) { tokens.push_back(current); current.clear(); }
            continue;
        }
        if (std::isalnum(c) || c == '_' || c == '.') {
            if ((c == 'e' || c == 'E') && i + 1 < source.size() && (source[i + 1] == '+' || source[i + 1] == '-')) {
                current += c; current += source[++i]; continue;
            }
            current += c;
        }
        else {
            if (!current.empty()) { tokens.push_back(current); current.clear(); }
            if ((c == '+' || c == '-') && i + 1 < source.size() && source[i + 1] == c) {
                tokens.push_back(std::string(2, c)); i++;
            }
            else {
                tokens.push_back(std::string(1, c));
            }
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

enum class Type { NUMBER, HP_VEC2, VOID, UNKNOWN };

Type map_type(const string& t) {
    if (t == "float" || t == "number") return Type::NUMBER;
    if (t == "vec2" || t == "hp_vec2") return Type::HP_VEC2;
    if (t == "void") return Type::VOID;
    return Type::UNKNOWN;
}

string typeToStr(Type t) {
    if (t == Type::NUMBER) return "number";
    if (t == Type::HP_VEC2) return "hp_vec2";
    return "void";
}

class RegisterAllocator {
    bool v_used[4] = { false };
    bool r_used[16] = { false };
public:
    string alloc(Type t) {
        if (t == Type::HP_VEC2) {
            for (int i = 0; i < 4; ++i) {
                if (!v_used[i]) { v_used[i] = true; return "V[" + std::to_string(i) + "]"; }
            }
            throw std::runtime_error("Vector register exhaustion");
        }
        else {
            for (int i = 0; i < 16; ++i) {
                if (!r_used[i]) { r_used[i] = true; return "R[" + std::to_string(i) + "]"; }
            }
            throw std::runtime_error("Scalar register exhaustion");
        }
    }
    void freeReg(const string& reg) {
        if (reg.size() >= 4) {
            if (reg[0] == 'V' && reg[1] == '[') v_used[std::stoi(reg.substr(2, 1))] = false;
            else if (reg[0] == 'R' && reg[1] == '[') r_used[std::stoi(reg.substr(2))] = false;
        }
    }
    void reset() {
        for (bool& b : v_used) b = false;
        for (bool& b : r_used) b = false;
    }
};

struct ASTNode {
    virtual ~ASTNode() = default;
};

struct Expr : public ASTNode {
    Type exprType = Type::UNKNOWN;
    virtual string genCode(RegisterAllocator& alloc, string& out) = 0;
};

struct IdentifierExpr : public Expr {
    string name;
    IdentifierExpr(string n, Type t) : name(n) { exprType = t; }
    string genCode(RegisterAllocator&, string&) override { return name; }
};

struct NumberExpr : public Expr {
    string val;
    NumberExpr(string v) : val(v) { exprType = Type::NUMBER; }
    string genCode(RegisterAllocator& alloc, string& out) override {
        string reg = alloc.alloc(Type::NUMBER);
        string clean_val = val;
        if (clean_val.back() == 'f' || clean_val.back() == 'F') clean_val.pop_back();
        if (clean_val.find('.') == string::npos) clean_val += ".0";
        out += "    float_to_number(" + clean_val + ", " + reg + ");\n";
        return reg;
    }
};

struct FieldAccessExpr : public Expr {
    std::unique_ptr<Expr> base;
    string field;
    FieldAccessExpr(std::unique_ptr<Expr> b, string f) : base(std::move(b)), field(f) { exprType = Type::NUMBER; }
    string genCode(RegisterAllocator& alloc, string& out) override {
        string baseReg = base->genCode(alloc, out);
        string res = baseReg + "." + field;
        alloc.freeReg(baseReg);
        return res;
    }
};

struct CallExpr : public Expr {
    string callee;
    vector<std::unique_ptr<Expr>> args;
    CallExpr(string c, Type t) : callee(c) { exprType = t; }
    string genCode(RegisterAllocator& alloc, string& out) override {
        vector<string> argRegs;
        for (auto& arg : args) argRegs.push_back(arg->genCode(alloc, out));
        string target = alloc.alloc(exprType);
        string fname = (callee == "vec2" || callee == "hp_vec2") ? "initialize_hp_vec2" : "hp_" + callee;
        out += "    " + fname + "(";
        for (size_t i = 0; i < argRegs.size(); ++i) {
            out += argRegs[i] + (i == argRegs.size() - 1 ? "" : ", ");
        }
        out += (argRegs.empty() ? "" : ", ") + target + ");\n";
        for (const auto& r : argRegs) alloc.freeReg(r);
        return target;
    }
};

struct BinaryExpr : public Expr {
    string op;
    std::unique_ptr<Expr> left, right;
    BinaryExpr(string o, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r, Type t) : op(o), left(std::move(l)), right(std::move(r)) { exprType = t; }
    string genCode(RegisterAllocator& alloc, string& out) override {
        string lreg = left->genCode(alloc, out);
        string rreg = right->genCode(alloc, out);
        string target = alloc.alloc(exprType);
        string fname;
        if (op == "+") fname = "hp_add";
        else if (op == "-") fname = "hp_sub";
        else if (op == "*") fname = "hp_mult";
        else if (op == "/") fname = "hp_div";
        out += "    " + fname + "(" + lreg + ", " + rreg + ", " + target + ");\n";
        alloc.freeReg(lreg);
        alloc.freeReg(rreg);
        return target;
    }
};

struct UnaryExpr : public Expr {
    string op;
    std::unique_ptr<Expr> operand;
    UnaryExpr(string o, std::unique_ptr<Expr> opd) : op(o), operand(std::move(opd)) { exprType = operand->exprType; }
    string genCode(RegisterAllocator& alloc, string& out) override {
        string reg = operand->genCode(alloc, out);
        string target = alloc.alloc(exprType);
        if (op == "-") {
            out += "    hp_neg(" + reg + ", " + target + ");\n";
        }
        alloc.freeReg(reg);
        return target;
    }
};

struct ConstructExpr : public Expr {
    std::unique_ptr<Expr> x, y;
    ConstructExpr(std::unique_ptr<Expr> xExpr, std::unique_ptr<Expr> yExpr) : x(std::move(xExpr)), y(std::move(yExpr)) { exprType = Type::HP_VEC2; }
    string genCode(RegisterAllocator& alloc, string& out) override {
        string xreg = x->genCode(alloc, out);
        string yreg = y->genCode(alloc, out);
        string target = alloc.alloc(exprType);
        out += "    " + target + ".x = " + xreg + ";\n";
        out += "    " + target + ".y = " + yreg + ";\n";
        alloc.freeReg(xreg);
        alloc.freeReg(yreg);
        return target;
    }
};

struct Stmt : public ASTNode {
    virtual void genCode(RegisterAllocator& alloc, string& out) = 0;
};

struct DeclStmt : public Stmt {
    Type varType;
    string name;
    std::unique_ptr<Expr> init;
    DeclStmt(Type t, string n, std::unique_ptr<Expr> i) : varType(t), name(n), init(std::move(i)) {}
    void genCode(RegisterAllocator& alloc, string& out) override {
        out += "    " + typeToStr(varType) + " " + name + ";\n";
        if (init) {
            string reg = init->genCode(alloc, out);
            out += "    " + name + " = " + reg + ";\n";
            alloc.freeReg(reg);
        }
    }
};

struct ReturnStmt : public Stmt {
    std::unique_ptr<Expr> val;
    ReturnStmt(std::unique_ptr<Expr> v) : val(std::move(v)) {}
    void genCode(RegisterAllocator& alloc, string& out) override {
        string reg = val->genCode(alloc, out);
        out += "    res = " + reg + ";\n    return;\n";
        alloc.freeReg(reg);
    }
};

struct FuncDef : public ASTNode {
    Type retType;
    string name;
    vector<std::pair<Type, string>> params;
    vector<std::unique_ptr<Stmt>> body;
    void genCode(RegisterAllocator& alloc, string& out) {
        out += "void hp_" + name + "(";
        for (auto& p : params) out += "in " + typeToStr(p.first) + " " + p.second + ", ";
        out += "out " + typeToStr(retType) + " res) {\n";
        for (auto& stmt : body) { alloc.reset(); stmt->genCode(alloc, out); }
        out += "}\n\n";
    }
};

class Parser {
    vector<string> tokens;
    size_t pos = 0;
    std::map<string, Type> env;

    string peek() { return pos < tokens.size() ? tokens[pos] : ""; }
    string advance() { return pos < tokens.size() ? tokens[pos++] : ""; }
    bool match(string exp) { if (peek() == exp) { advance(); return true; } return false; }
    Type infer_call_type(const string& name) {
        if (name == "cos" || name == "cosh" || name == "csc" || name == "cot" || name == "clamp" || name == "ceil") {
            return Type::NUMBER;
        }
        if (name == "vec2" || name.rfind("c", 0) == 0 || name == "reciprocal" || name == "conj" || name == "re" || name == "im" || name == "vector_floor") return Type::HP_VEC2;
        return Type::NUMBER;
    }

    std::unique_ptr<Expr> parse_primary() {
        string t = advance();
        if (t == "(") {
            auto e = parse_expr();
            match(")");
            return e;
        }
        if (std::isdigit(t[0]) || (t[0] == '.' && t.size() > 1)) return std::make_unique<NumberExpr>(t);
        if (t == "PI" || t == "ONE") return std::make_unique<IdentifierExpr>(t, t == "ONE" ? Type::HP_VEC2 : Type::NUMBER);

        if (peek() == "(") {
            match("(");
            if (t == "vec2") {
                auto ex = parse_expr(); match(","); auto ey = parse_expr(); match(")");
                return std::make_unique<ConstructExpr>(std::move(ex), std::move(ey));
            }
            auto call = std::make_unique<CallExpr>(t, infer_call_type(t));
            while (peek() != ")") {
                call->args.push_back(parse_expr());
                if (peek() == ",") match(",");
            }
            match(")");
            return call;
        }
        Type type = env.count(t) ? env[t] : Type::UNKNOWN;
        auto id = std::make_unique<IdentifierExpr>(t, type);
        if (match(".")) {
            string field = advance();
            return std::make_unique<FieldAccessExpr>(std::move(id), field);
        }
        return id;
    }

    std::unique_ptr<Expr> parseUnary() {
        if (match("-")) return std::make_unique<UnaryExpr>("-", parseUnary());
        return parse_primary();
    }

    std::unique_ptr<Expr> parseMulDiv() {
        auto left = parseUnary();
        while (peek() == "*" || peek() == "/") {
            string op = advance();
            auto right = parseUnary();
            Type resType = (left->exprType == Type::HP_VEC2 || right->exprType == Type::HP_VEC2) ? Type::HP_VEC2 : Type::NUMBER;
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right), resType);
        }
        return left;
    }

    std::unique_ptr<Expr> parseAddSub() {
        auto left = parseMulDiv();
        while (peek() == "+" || peek() == "-") {
            string op = advance();
            auto right = parseMulDiv();
            Type resType = (left->exprType == Type::HP_VEC2 || right->exprType == Type::HP_VEC2) ? Type::HP_VEC2 : Type::NUMBER;
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right), resType);
        }
        return left;
    }

    std::unique_ptr<Expr> parse_expr() { return parseAddSub(); }

    std::unique_ptr<Stmt> parse_statement() {
        if (peek() == "return") {
            advance();
            auto ret = std::make_unique<ReturnStmt>(parse_expr());
            match(";");
            return ret;
        }
        Type t = map_type(peek());
        if (t != Type::UNKNOWN) {
            advance();
            string name = advance();
            env[name] = t;
            std::unique_ptr<Expr> init = nullptr;
            if (match("=")) init = parse_expr();
            match(";");
            return std::make_unique<DeclStmt>(t, name, std::move(init));
        }
        auto e = parse_expr();
        match(";");
        return nullptr;
    }

public:
    Parser(vector<string> tks) : tokens(tks) {}

    std::unique_ptr<FuncDef> parseFunc() {
        if (pos >= tokens.size()) return nullptr;
        env.clear();
        auto def = std::make_unique<FuncDef>();
        def->retType = map_type(advance());
        def->name = advance();
        match("(");
        while (peek() != ")") {
            Type pt = map_type(advance());
            string pn = advance();
            env[pn] = pt;
            def->params.push_back({ pt, pn });
            if (peek() == ",") match(",");
        }
        match(")"); match("{");
        while (peek() != "}") {
            if (auto s = parse_statement()) def->body.push_back(std::move(s));
        }
        match("}");
        return def;
    }
};

string transpile_to_highp_glsl(const string& lowp_code, const std::string& highp_declarations) {
    string clean = remove_comments(lowp_code);
    vector<string> tokens = tokenize(clean);
    Parser parser(tokens);
    RegisterAllocator alloc;
    string output = "";
    output += highp_declarations;
    while (auto func = parser.parseFunc()) {
        func->genCode(alloc, output);
    }
    return output;
}