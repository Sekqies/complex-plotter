#pragma once

#include <glm/common.hpp>

typedef uint32_t uint;

struct uvec2 {
    uint x, y;
    uvec2(uint x, uint y) : x(x), y(y) {}
    uvec2() : x(0), y(0) {} 
};