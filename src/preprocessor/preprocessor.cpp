#include <preprocessor/preprocessor.h>

string get_source(const string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open source file '" + filename + "'");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void preprocess(const string& filename, const vector<TokenOperator>& operators) {
    string preprocessed_string = get_preprocessor_string(operators);
    string raw_source = get_source(filename);
    const string marker = "START_WRITING_HERE HERE";
    size_t start_pos = raw_source.find(marker) + marker.length();
    // Let this exception be an exception! While I don't like the exception handling paradigm
    //, it is a necessary evil for file handling
    try {
        raw_source = get_source(filename);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return;
    }
    if (start_pos == string::npos) {
        std::cerr << "Marker does not exist";
        return;
    }
    raw_source.insert(start_pos, "\n" + preprocessed_string + "\n");
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Could not open output file";
        return;
    }
    out << raw_source;
    out.close();
}
