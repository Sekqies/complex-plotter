#pragma once
#include <glsl_transpiled/glsl_types.h>
#include <glsl_transpiled/glsl_constants.h>
#include <vector>
#include <iostream>

constexpr inline uint NUMBER_OF_LIMBS = 8;
constexpr inline uint FRACTIONAL_SIZE = NUMBER_OF_LIMBS / 2;

struct number {
    std::vector<uint> limb = std::vector<unsigned int>(NUMBER_OF_LIMBS, 0);
    int sign = 1;
    bool is_infinite = false;
};

struct hp_vec2 {
    number x;
    number y;
    hp_vec2(number x,number y) : x(x), y(y){};
};

hp_vec2 initialize_hp_vec2(number x, number y);

number null_number();

number infinite_number();

number number_one();

number hp_neg(number a);

uint hi(uint a);

uint lo(uint a);


int compare_abs(number a, number b);

bool is_zero(number a);

uvec2 sum_with_carry(uint a, uint b);
number abs_sum(number a, number b);
number abs_hp_sub(number a, number b);

number hp_add(number a, number b);



number hp_sub(number a, number b);

uvec2 product_with_remainder(uint a, uint b);

number hp_mult(number a, number b);

number shift_left(number a, int shift);

number shift_right(number a, int shift);

int find_msb(number a);

uint get_half(number a, int index);

void set_half(number& a, int index, uint val);

number mult_scalar_16(number a, uint b_16);

number hp_div(number n, number d);

number div_uint(number n, uint d);

number hp_sqrt(number x);

number number_integer(const uint n);

number float_to_number(float f);

