#include <string>
#include <vector>
#include <set>
#include <map>
#include <regex>
#include <cctype>

using std::string, std::vector;

string transpile_to_highp_glsl(const std::string& lowp_glsl_func_declarations, const std::string& highp_glsl_func_declarations);