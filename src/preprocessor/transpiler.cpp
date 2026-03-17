#include <preprocessor/transpiler.h>

using std::string, std::vector;

std::set<string> get_function_names(const std::string& source) {
    static const std::regex glsl_regex = []() {
        std::vector<std::string> types = {
            "void", "float", "int", "uint", "bool", "double",
            "vec2", "vec3", "vec4", "ivec2", "ivec3", "ivec4", "uvec2", "uvec3", "uvec4",
            "mat2", "mat3", "mat4", "dmat2", "dmat3", "dmat4", "sampler2D", "samplerCube",
            "number", "hp_vec2", "hp_vec3", "hp_vec4"
        };
        std::string pattern = R"(\b(?:)";
        for (size_t i = 0; i < types.size(); ++i) {
            pattern += types.at(i) + (i == types.size() - 1 ? "" : "|");
        }
        pattern += R"()\s+([\w]+)\s*\()";
        return std::regex(pattern);
        }();
    std::set<string> function_names;

    auto words_begin = std::sregex_iterator(source.begin(), source.end(), glsl_regex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator it = words_begin; it != words_end; ++it) {
        function_names.insert((*it)[1].str());
    }
    return function_names;
}

std::map<string, string> set_to_map(const std::set<string>& word_set, const string& prefix) {
    std::map<string, string> out_map;
    for (const string& s : word_set) {
        if (s.rfind(prefix, 0) == 0) {
            out_map.insert({ s.substr(prefix.size()), s });
        }
    }
    return out_map;
}

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
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            std::string ws;
            while (i < source.size() && std::isspace(source.at(i))) {
                ws += source[i++];
            }
            tokens.push_back(ws);
            i--;
            continue;
        }

        if (std::isalnum(c) || c == '_' || c == '.') {
            if ((c == 'e' || c == 'E') && i + 1 < source.size() &&
                (source[i + 1] == '+' || source[i + 1] == '-')) {
                current += c;
                current += source[++i];
                continue;
            }
            current += c;
        }
        else {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            if (std::isspace(c)) continue;

            if ((c == '+' || c == '-') && i + 1 < source.size() && source[i + 1] == c) {
                tokens.push_back(std::string(2, c));
                i++;
            }
            else {
                tokens.push_back(std::string(1, c));
            }
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

void find_and_replace_tokens(std::vector<std::string>& tokens,
    const std::map<std::string, std::string>& rename_map) {
    std::string output;
    output.reserve(tokens.size() * 5);
    for (auto& token : tokens) {
        auto it = rename_map.find(token);
        if (it != rename_map.end()) {
            token = it->second;
        }
    }
}

string tokens_to_string(const vector<string>& v)
{
    string out = "";
    for (const string& s : v) {
        out += s;
    }
    return out;
}

bool is_sig(const std::string& t) {
    return !t.empty() && t.find_first_not_of(" \t\n\r") != std::string::npos;
}

void handle_ambiguous_types(std::vector<std::string>& tokens) {
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens.at(i) == "number" || tokens.at(i) == "hp_vec2") {
            size_t j = i + 1;
            while (j < tokens.size() && !is_sig(tokens.at(j))) j++;
            if (j < tokens.size() && tokens.at(j) == "(") {
                tokens[i] = (tokens.at(i) == "number") ? "float_to_number" : "hp_vec2";
            }
        }
        else if (is_sig(tokens.at(i)) && (std::isdigit(tokens.at(i)[0]) || (tokens.at(i).size() > 1 && tokens.at(i)[0] == '.' && std::isdigit(tokens.at(i)[1])))) {
            if (tokens.at(i).find('.') != std::string::npos) {
                std::string num = tokens.at(i);
                if (num.back() == 'f' || num.back() == 'F') num.pop_back();
                tokens[i] = "float_to_number(" + num + ")";
            }
        }
    }
}

int get_left_operand(const std::vector<std::string>& tokens, int op_idx) {
    int i = op_idx - 1;
    while (i >= 0 && !is_sig(tokens.at(i))) i--;
    if (i < 0) return 0;
    if (tokens.at(i) == ")") {
        int depth = 0;
        while (i >= 0) {
            if (tokens.at(i) == ")") depth++;
            else if (tokens.at(i) == "(") depth--;
            i--;
            if (depth == 0) break;
        }
        while (i >= 0 && !is_sig(tokens.at(i))) i--;
        if (i >= 0 && (std::isalpha(tokens.at(i)[0]) || tokens.at(i)[0] == '_')) return i;
        return i + 1;
    }
    return i;
}

int get_right_operand(const std::vector<std::string>& tokens, int op_idx) {
    int i = op_idx + 1;
    while (i < tokens.size() && !is_sig(tokens.at(i))) i++;
    if (i >= tokens.size()) return tokens.size() - 1;
    if (tokens.at(i) == "-" || tokens.at(i) == "+") {
        i++;
        while (i < tokens.size() && !is_sig(tokens.at(i))) i++;
    }

    if (i >= tokens.size()) return tokens.size() - 1;

    if (std::isalpha(tokens.at(i)[0]) || tokens.at(i)[0] == '_') {
        int func_idx = i;
        int next_sig = i + 1;
        while (next_sig < tokens.size() && !is_sig(tokens.at(next_sig))) next_sig++;
        if (next_sig < tokens.size() && tokens.at(next_sig) == "(") {
            i = next_sig;
            int depth = 0;
            while (i < tokens.size()) {
                if (tokens.at(i) == "(") depth++;
                else if (tokens.at(i) == ")") depth--;
                if (depth == 0) break;
                i++;
            }
            return i;
        }
        return func_idx;
    }
    if (tokens.at(i) == "(") {
        int depth = 0;
        while (i < tokens.size()) {
            if (tokens.at(i) == "(") depth++;
            else if (tokens.at(i) == ")") depth--;
            if (depth == 0) break;
            i++;
        }
        if (i >= tokens.size()) return tokens.size() - 1;
        return i;
    }
    return i;
}

void process_operator_pass(std::vector<std::string>& tokens, const std::vector<std::string>& ops, const std::map<std::string, std::string>& func_map) {
    for (size_t i = 0; i < tokens.size(); ++i) {
        bool match = false;
        for (const auto& op : ops) {
            if (tokens.at(i) == op) { match = true; break; }
        }
        if (!match) continue;

        int left_idx = get_left_operand(tokens, i);
        int right_idx = get_right_operand(tokens, i);

        if (left_idx < 0) left_idx = 0;
        if (right_idx >= tokens.size()) right_idx = tokens.size() - 1;

        std::string func_name = func_map.at(tokens.at(i));
        std::vector<std::string> new_tokens;

        for (int j = 0; j < left_idx; ++j) new_tokens.push_back(tokens.at(j));
        new_tokens.push_back(func_name);
        new_tokens.push_back("(");
        for (int j = left_idx; j < i; ++j) new_tokens.push_back(tokens.at(j));
        new_tokens.push_back(",");
        for (int j = i + 1; j <= right_idx; ++j) new_tokens.push_back(tokens.at(j));
        new_tokens.push_back(")");
        for (size_t j = right_idx + 1; j < tokens.size(); ++j) new_tokens.push_back(tokens.at(j));

        tokens = new_tokens;
        i = left_idx;
    }
}

void handle_operators(std::vector<std::string>& tokens) {
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens.at(i) == "-") {
            int prev = i - 1;
            while (prev >= 0 && !is_sig(tokens[prev])) prev--;
            if (prev < 0 || tokens[prev] == "=" || tokens[prev] == "return" || tokens[prev] == "(" || tokens[prev] == "," ||
                tokens[prev] == "+" || tokens[prev] == "-" || tokens[prev] == "*" || tokens[prev] == "/") {
                int right_idx = get_right_operand(tokens, i);
                std::vector<std::string> new_tokens;
                for (int j = 0; j < i; ++j) new_tokens.push_back(tokens.at(j));
                new_tokens.push_back("hp_neg");
                new_tokens.push_back("(");
                for (int j = i + 1; j <= right_idx; ++j) new_tokens.push_back(tokens.at(j));
                new_tokens.push_back(")");
                for (size_t j = right_idx + 1; j < tokens.size(); ++j) new_tokens.push_back(tokens.at(j));
                tokens = new_tokens;
                i = right_idx;
            }
        }
    }

    std::map<std::string, std::string> mul_div = { {"*", "hp_mult"}, {"/", "hp_div"} };
    process_operator_pass(tokens, { "*", "/" }, mul_div);

    std::map<std::string, std::string> add_sub = { {"+", "hp_add"}, {"-", "hp_sub"} };
    process_operator_pass(tokens, { "+", "-" }, add_sub);
}

string transpile_to_highp_glsl(const std::string& lowp_glsl_func_declarations, const std::string& highp_glsl_func_declarations) {
    static const std::map<string, string> type_map = {
        {"float", "number"},
        {"uint", "number"},
        {"vec2", "hp_vec2"}
    };
    const string sanitized_lowp = remove_comments(lowp_glsl_func_declarations);
    std::set<string> highp_function_names = get_function_names(highp_glsl_func_declarations);
    vector<string> tokens = tokenize(sanitized_lowp);

    find_and_replace_tokens(tokens, set_to_map(highp_function_names, "hp_"));
    find_and_replace_tokens(tokens, type_map);

    handle_ambiguous_types(tokens);
    handle_operators(tokens);

    string out = highp_glsl_func_declarations + tokens_to_string(tokens);

    return out;
}

