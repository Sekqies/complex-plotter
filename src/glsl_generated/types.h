#pragma once

#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/math/constants/constants.hpp>

using big_float = boost::multiprecision::number<boost::multiprecision::cpp_bin_float<25>>;

struct big_vec2 { 
    big_float x, y; 
    big_vec2() : x(0), y(0) {} 
    big_vec2(big_float _x, big_float _y) : x(_x), y(_y) {}
    big_vec2(float _x, float _y): x(big_float(_x)), y(big_float(_y)){} 
};

// --- Constants ---
#define PI (boost::math::constants::pi<big_float>())
#define CPI (big_vec2(PI, big_float("0.0")))
#define ONE (big_vec2(big_float("1.0"), big_float("0.0")))
#define MINUS_ONE (big_vec2(big_float("-1.0"), big_float("0.0")))
#define ZERO (big_vec2(big_float("0.0"), big_float("0.0")))

// --- Helper Overloads ---
inline big_vec2 operator+(big_vec2 a, big_vec2 b) { return big_vec2(a.x + b.x, a.y + b.y); }
inline big_vec2 operator-(big_vec2 a, big_vec2 b) { return big_vec2(a.x - b.x, a.y - b.y); }
inline big_vec2 operator*(big_float s, big_vec2 v) { return big_vec2(s * v.x, s * v.y); }
inline big_vec2 operator*(big_vec2 v, big_float s) { return big_vec2(v.x * s, v.y * s); }
inline big_vec2 operator/(big_vec2 v, big_float s) { return big_vec2(v.x / s, v.y / s); }
