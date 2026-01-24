#include <preprocessor/preprocessor.h>
#include <types/type_mapper.h>
#include <array>
#include <stdexcept>
#include <shaders/embedded_shaders.h>

string get_source(const string& filename) {
    static std::map<string, string&> known_files = {
        {"shaders/plotter.frag",SRC_PLOTTER_FRAG},
        {"shaders/plotter.vert",SRC_PLOTTER_VERT},
        {"shaders/plotter3d.frag",SRC_PLOTTER3D_FRAG},
        {"shaders/plotter3d.vert",SRC_PLOTTER3D_VERT}
    };
    const auto it = known_files.find(filename);
    if (it != known_files.end()) {
        return it->second;
    }


    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open source file '" + filename + "'");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


void preprocess_string(const string& filename, const vector<TokenOperator>& operators) {
    string preprocessed_string = get_preprocessor_string(operators);
    string raw_source = get_source(filename);
    initialize_map_id();
    const string marker = "START_WRITING_HERE HERE";
    const string interpreter = write_interpreter();
    inject_at(raw_source, "START_WRITING_HERE HERE", "\n" + preprocessed_string + "\n");
    inject_at(raw_source, "INTERPRETER_DEFINITION HERE", "\n" + interpreter + "\n");
    if (filename == "shaders/plotter.frag") {
        SRC_PLOTTER_FRAG = raw_source;
    }
    if (filename == "shaders/plotter.vert") SRC_PLOTTER_VERT = raw_source;
    if (filename == "shaders/plotter3d.vert") SRC_PLOTTER3D_VERT = raw_source;
    if (filename == "shaders/plotter3d.frag") SRC_PLOTTER3D_FRAG = raw_source;
    std::cout << raw_source;
    return;
}

void preprocess_file(const string& filename, const vector<TokenOperator>& operators) {
    string preprocessed_string = get_preprocessor_string(operators);
    initialize_map_id();
    string raw_source = get_source(filename);
    const string marker = "START_WRITING_HERE HERE";
    const string interpreter = write_interpreter();
    inject_at(raw_source, "START_WRITING_HERE HERE", "\n" + preprocessed_string + "\n");
    inject_at(raw_source, "INTERPRETER_DEFINITION HERE", "\n" + interpreter + "\n");
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Could not open output file";
        return;
    }
    out << interpreter;
    out.close();
}


string build_shader_string(const string& new_shader, const string& origin) {
    const vector<string> tags = { "INTERPRETER_SPECIFIC_FUNCTIONS", "FUNCTION_DEFINITIONS", "CONSTANT_DEFINITIONS"};
    string out = new_shader;
    for (const string& tag : tags) {
        const string& block = get_block(origin, tag);
        inject_at(out, tag, block);
    }
    return out;
}


static bool found_bounds(const string& source, const string& tag, size_t& out_start, size_t& out_length) {
    string start_marker = "#define " + tag;
    string end_marker = "#define END_" + tag;

    size_t start = source.find(start_marker);
    if (start == string::npos) return false;

    size_t end_tag_start = source.find(end_marker, start);
    if (end_tag_start == string::npos) return false;

    size_t end_of_line = source.find('\n', end_tag_start);
    if (end_of_line == string::npos) end_of_line = source.size();

    out_start = start;
    out_length = end_of_line - start;
    return true;
}

string get_block(const string& source, const string& tag) {
    size_t start, length;
    if (!found_bounds(source, tag, start, length)) return "";

    size_t newline_pos = source.find('\n', start);
    if (newline_pos == string::npos) return ""; 
    size_t content_start = newline_pos + 1;
    string end_str = "#define END_" + tag;
    size_t content_end = source.find(end_str, content_start);
    if (content_end == string::npos) return "";
    return source.substr(content_start, content_end - content_start);
}

string erase_block(const string& source, const string& tag) {
    string copy = source;
    size_t start, length;
    while (found_bounds(copy, tag, start, length)) {
        copy.erase(start, length);
    }
    return copy;
}

void erase_block(string& source, const string& tag) {
    size_t start, length;
    while (found_bounds(source, tag, start, length)) {
        source.erase(start, length);
    }
}

string inject_at(const string& source, const string& tag, const string& payload) {
    string copy = source;
    const string marker = "#define " + tag;
    size_t pos = source.find(marker);
    if (pos == string::npos) {
        throw std::runtime_error("Injection marker '" + tag + "' not found");
    }
    size_t end = source.find('\n', pos);
    if (end == string::npos) end = pos + marker.length();

    copy.insert(end + 1, "\n" + payload + "\n");
    return copy;
}

void inject_at(string& source, const string& tag, const string& payload) {
    const string marker = "#define " + tag;
    size_t pos = source.find(marker);
    if (pos == string::npos) {
        std::cout << tag;
        throw std::runtime_error("Injection marker '" + tag + "' not found");
    }
    size_t end = source.find('\n', pos);
    if (end == string::npos) end = pos + marker.length();

    source.insert(end + 1, "\n" + payload + "\n");
}