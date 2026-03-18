#include <glsl_transpiled/glsl_big_number.h>
#include <bit>
#include <cstdint>


hp_vec2 initialize_hp_vec2(number x, number y) {
    return hp_vec2(x, y);
}

number null_number() {
    number res;
    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        res.limb[i] = 0u;
    }
    res.sign = 1;
    res.is_infinite = false;
    return res;
}

number infinite_number() {
    number res;
    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        res.limb[i] = (1u << 31u) - 1u;
    }
    res.sign = 1;
    res.is_infinite = true;
    return res;
}

number number_integer(const uint n){
    number res = null_number();
    res.limb[FRACTIONAL_SIZE] = n;
    return res;
}

number number_one() {
    number res = null_number();
    res.limb[FRACTIONAL_SIZE] = 1u;
    return res;
}

number hp_neg(number a) {
    a.sign *= -1;
    return a;
}

uint hi(uint a) {
    return a >> 16;
}

uint lo(uint a) {
    return a & (0xFFFFu);
}

int compare_abs(number a, number b) {
    for (int i = NUMBER_OF_LIMBS - 1; i >= 0; --i) {
        if (a.limb[i] > b.limb[i]) return 1;
        if (a.limb[i] < b.limb[i]) return -1;
    }
    return 0;
}

bool is_zero(number a) {
    bool notzero = false;
    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        notzero = a.limb[i] != 0u || notzero;
    }
    return !notzero;
}

uvec2 sum_with_carry(uint a, uint b) {
    uvec2 res = uvec2(0u, 0u);
    res.x = a + b;
    res.y = uint(a > res.x || b > res.x);
    return res;
}

number abs_sum(number a, number b) {
    number c = null_number();
    uint carry = 0u;
    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        uvec2 step1 = sum_with_carry(a.limb[i], b.limb[i]);
        uvec2 step2 = sum_with_carry(step1.x, carry);

        c.limb[i] = step2.x;
        carry = step1.y + step2.y;
    }
    return c;
}

number abs_hp_sub(number a, number b) {
    number c = null_number();
    uint borrow = 0u;
    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        uint sub = a.limb[i] - b.limb[i] - borrow;
        borrow = uint((a.limb[i] < b.limb[i] || (a.limb[i] == b.limb[i] && borrow > 0u)));
        c.limb[i] = sub;
    }
    return c;
}

number hp_add(number a, number b) {
    number c = null_number();

    if (a.sign == b.sign) {
        uint64_t carry = 0;
        for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
            uint64_t sum = (uint64_t)a.limb[i] + b.limb[i] + carry;
            c.limb[i] = (uint32_t)sum;
            carry = sum >> 32;
        }
        c.sign = a.sign;
        return c;
    }

    int cmp = compare_abs(a, b);
    number max_val = (cmp >= 0) ? a : b;
    number min_val = (cmp >= 0) ? b : a;

    uint64_t borrow = 0;
    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        uint64_t diff = (uint64_t)max_val.limb[i] - min_val.limb[i] - borrow;
        c.limb[i] = (uint32_t)diff;
        borrow = (diff >> 32) & 1;
    }
    c.sign = max_val.sign;
    return c;
}

number hp_sub(number a, number b) {
    return hp_add(a, hp_neg(b));
}

uvec2 product_with_remainder(uint a, uint b) {
    uint low_part = a * b;

    uint al = lo(a);
    uint ah = hi(a);
    uint bl = lo(b);
    uint bh = hi(b);

    uint p0 = al * bl;
    uint p1 = al * bh;
    uint p2 = ah * bl;
    uint p3 = ah * bh;

    uvec2 m_sum = sum_with_carry(p1, p2);
    uvec2 m_combined = sum_with_carry(m_sum.x, hi(p0));

    uint high_part = p3 + hi(m_combined.x) + ((m_sum.y + m_combined.y) << 16);

    return uvec2(low_part, high_part);
}

number hp_mult(number a, number b) {
    number c = null_number();
    c.sign = a.sign * b.sign;

    if (is_zero(a) || is_zero(b)) {
        c.sign = 1;
        return c;
    }
    if (a.is_infinite || b.is_infinite) return infinite_number();

    uint32_t ext_c[NUMBER_OF_LIMBS + FRACTIONAL_SIZE] = { 0 };

    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        if (a.limb[i] == 0u) continue;
        uint64_t carry = 0;

        for (int j = 0; j < NUMBER_OF_LIMBS; ++j) {
            int ext_target = i + j;
            if (ext_target >= NUMBER_OF_LIMBS + FRACTIONAL_SIZE) break;

            uint64_t prod = (uint64_t)a.limb[i] * b.limb[j] + carry;
            prod += ext_c[ext_target];
            ext_c[ext_target] = (uint32_t)prod;
            carry = prod >> 32;
        }

        int leftover_target = i + NUMBER_OF_LIMBS;
        while (carry > 0 && leftover_target < NUMBER_OF_LIMBS + FRACTIONAL_SIZE) {
            uint64_t sum = (uint64_t)ext_c[leftover_target] + carry;
            ext_c[leftover_target] = (uint32_t)sum;
            carry = sum >> 32;
            leftover_target++;
        }
    }

    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        c.limb[i] = ext_c[i + FRACTIONAL_SIZE];
    }

    if (is_zero(c)) c.sign = 1;
    return c;
}

number shift_left(number a, int shift) {
    if (shift <= 0) return a;
    int limb_shift = shift / 32;
    int bit_shift = shift % 32;
    number c = null_number();
    c.sign = a.sign;
    if (limb_shift >= NUMBER_OF_LIMBS) return c;

    for (int i = NUMBER_OF_LIMBS - 1; i >= limb_shift; --i) {
        uint32_t val = a.limb[i - limb_shift] << bit_shift;
        if (i - limb_shift > 0 && bit_shift > 0) {
            uint64_t prev_limb = a.limb[i - limb_shift - 1];
            val |= static_cast<uint32_t>(prev_limb >> (32 - bit_shift));
        }
        c.limb[i] = val;
    }
    return c;
}

number shift_right(number a, int shift) {
    if (shift <= 0) return a;
    int limb_shift = shift >> 5;
    int bit_shift = shift % 32;
    number c = null_number();
    c.sign = a.sign;
    if (limb_shift >= NUMBER_OF_LIMBS) return c;

    for (int i = 0; i < NUMBER_OF_LIMBS - limb_shift; ++i) {
        uint32_t val = a.limb[i + limb_shift] >> bit_shift;
        if (i + limb_shift < NUMBER_OF_LIMBS - 1 && bit_shift > 0) {
            uint64_t next_limb = a.limb[i + limb_shift + 1];
            val |= static_cast<uint32_t>(next_limb << (32 - bit_shift));
        }
        c.limb[i] = val;
    }
    return c;
}
int find_msb(number a) {
    for (int i = NUMBER_OF_LIMBS - 1; i >= 0; --i) {
        uint x = a.limb[i];
        if (x != 0u) {
            int bit_pos = 0;
            if ((x & 0xFFFF0000u) != 0u) { bit_pos += 16; x >>= 16; }
            if ((x & 0x0000FF00u) != 0u) { bit_pos += 8;  x >>= 8; }
            if ((x & 0x000000F0u) != 0u) { bit_pos += 4;  x >>= 4; }
            if ((x & 0x0000000Cu) != 0u) { bit_pos += 2;  x >>= 2; }
            if ((x & 0x00000002u) != 0u) { bit_pos += 1; }
            return (i * 32) + bit_pos;
        }
    }
    return -1;
}

uint get_half(number a, int index) {
    if (index >= NUMBER_OF_LIMBS * 2 || index < 0) {
        return 0u;
    }
    uint l = a.limb[index / 2];
    return uint(index % 2 == 0) * lo(l) + uint(index % 2 == 1) * hi(l);
}

void set_half(number& a, int index, uint val) {
    if (index >= NUMBER_OF_LIMBS * 2 || index < 0) return;
    int limb_idx = index / 2;
    if (index % 2 == 1) {
        a.limb[limb_idx] = (a.limb[limb_idx] & 0x0000FFFFu) | (val << 16);
    }
    else {
        a.limb[limb_idx] = (a.limb[limb_idx] & 0xFFFF0000u) | (val & 0xFFFFu);
    }
}

number mult_scalar_16(number a, uint b_16) {
    number c = null_number();
    c.sign = a.sign;
    uint carry = 0u;

    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        if (a.limb[i] == 0u && carry == 0u) continue;

        uvec2 prod = product_with_remainder(a.limb[i], b_16);
        uvec2 hp_add1 = sum_with_carry(prod.x, carry);

        c.limb[i] = hp_add1.x;
        carry = prod.y + hp_add1.y;
    }
    return c;
}

number hp_div(number n, number d) {
    number q = null_number();
    q.sign = n.sign * d.sign;

    if (is_zero(d)) {
        q = infinite_number();
        return q;
    }
    if (d.is_infinite) return q;

    int msb_d = find_msb(d);
    if (msb_d == -1) return q;

    int msb_n = find_msb(n);
    if (msb_n == -1) return q;

    const int MAX_HALVES = NUMBER_OF_LIMBS * 2 + FRACTIONAL_SIZE * 2 + 2;
    uint u_halves[MAX_HALVES];

    for (int i = 0; i < MAX_HALVES; ++i) {
        u_halves[i] = 0u;
    }

    for (int i = 0; i < NUMBER_OF_LIMBS * 2; ++i) {
        u_halves[i + FRACTIONAL_SIZE * 2] = get_half(n, i);
    }

    int len_v = (msb_d >> 4) + 1;
    int len_u_unshifted = ((msb_n + FRACTIONAL_SIZE * 32) >> 4) + 1;

    if (len_u_unshifted < len_v) return q;

    if (len_v == 1) {
        uint v0 = get_half(d, 0);
        uint rem = 0u;
        for (int i = len_u_unshifted - 1; i >= 0; --i) {
            uint dividend = (rem << 16) | u_halves[i];
            uint q_i = dividend / v0;
            rem = dividend % v0;

            if (i < NUMBER_OF_LIMBS * 2) {
                set_half(q, i, q_i);
            }
        }
        return q;
    }

    int shift = 15 - (msb_d % 16);
    number v = shift_left(d, shift);

    if (shift > 0) {
        uint carry_shift = 0u;
        for (int i = 0; i < MAX_HALVES; ++i) {
            uint val = (u_halves[i] << shift) | carry_shift;
            carry_shift = u_halves[i] >> (16 - shift);
            u_halves[i] = val & 0xFFFFu;
        }
    }

    int len_u = ((msb_n + FRACTIONAL_SIZE * 32 + shift) >> 4) + 1;
    int m = len_u - len_v;
    int n_len = len_v;

    uint v_n1 = get_half(v, n_len - 1);
    uint v_n2 = get_half(v, n_len - 2);

    for (int j = m; j >= 0; --j) {
        uint u_jn = u_halves[j + n_len];
        uint u_jn1 = u_halves[j + n_len - 1];
        uint u_jn2 = (j + n_len >= 2) ? u_halves[j + n_len - 2] : 0u;

        uint dividend = (u_jn << 16) | u_jn1;
        uint q_hat, r_hat;

        if (u_jn == v_n1) {
            q_hat = 0xFFFFu;
            r_hat = u_jn1 + v_n1;
        }
        else {
            q_hat = dividend / v_n1;
            r_hat = dividend % v_n1;
        }

        while (r_hat < 0x10000u && (q_hat * v_n2) >((r_hat << 16) | u_jn2)) {
            q_hat--;
            r_hat += v_n1;
        }

        uint k = 0u;
        uint borrow = 0u;
        for (int i = 0; i < n_len; ++i) {
            uint p = q_hat * get_half(v, i) + k;
            k = p >> 16;
            uint p_lo = p & 0xFFFFu;

            uint u_ji = u_halves[j + i];
            int diff = int(u_ji) - int(p_lo) - int(borrow);

            u_halves[j + i] = uint(diff) & 0xFFFFu;
            borrow = (diff < 0) ? 1u : 0u;
        }

        int final_diff = int(u_halves[j + n_len]) - int(k) - int(borrow);
        u_halves[j + n_len] = uint(final_diff) & 0xFFFFu;

        if (final_diff < 0) {
            q_hat--;
            uint carry_add = 0u;
            for (int i = 0; i < n_len; ++i) {
                uint sum = u_halves[j + i] + get_half(v, i) + carry_add;
                u_halves[j + i] = sum & 0xFFFFu;
                carry_add = sum >> 16;
            }
            uint sum_last = u_halves[j + n_len] + carry_add;
            u_halves[j + n_len] = sum_last & 0xFFFFu;
        }

        if (j < NUMBER_OF_LIMBS * 2) {
            set_half(q, j, q_hat);
        }
    }
    return q;
}

number div_uint(number n, uint d) {
    number q = null_number();
    q.sign = n.sign;
    uint64_t rem = 0;

    for (int i = NUMBER_OF_LIMBS - 1; i >= 0; --i) {
        uint64_t dividend = (rem << 32) | n.limb[i];
        q.limb[i] = (uint32_t)(dividend / d);
        rem = dividend % d;
    }
    return q;
}



float uintBitsToFloat(const uint n) {
    return std::bit_cast<float>(n);
}

uint floatBitsToUint(const float n){
    return std::bit_cast<uint>(n);
}

number float_to_number(float f) {
    if (f == 0.0) return null_number();

    uint f_bits = floatBitsToUint(f);
    uint sign_bit = f_bits >> 31;
    uint exp_bits = (f_bits >> 23) & 0xFFu;
    uint mantissa_bits = f_bits & 0x7FFFFFu;

    if (exp_bits == 255u) {
        number inf = infinite_number();
        inf.sign = (sign_bit == 1u) ? -1 : 1;
        return inf;
    }

    if (exp_bits == 0u) return null_number();

    number res = null_number();
    res.sign = (sign_bit == 1u) ? -1 : 1;

    uint mantissa = mantissa_bits | 0x800000u;

    res.limb[FRACTIONAL_SIZE] = mantissa;

    int shift = int(exp_bits) - 127 - 23;

    if (shift > 0) {
        res = shift_left(res, shift);
    }
    else if (shift < 0) {
        res = shift_right(res, -shift);
    }

    return res;
}

float number_to_float(number n) {
    if (is_zero(n)) return 0.0;
    if (n.is_infinite) {
        return (n.sign == 1) ? uintBitsToFloat(0x7F800000u) : uintBitsToFloat(0xFF800000u);
    }
    int msb = find_msb(n);
    int true_exp = msb - (FRACTIONAL_SIZE * 32);
    if (true_exp > 127) {
        return (n.sign == 1) ? uintBitsToFloat(0x7F800000u) : uintBitsToFloat(0xFF800000u);
    }
    if (true_exp < -126) {
        return 0.0;
    }

    uint exp_bits = uint(true_exp + 127);
    uint mantissa = 0u;

    int limb_idx = msb / 32;
    int bit_idx = msb % 32;

    if (bit_idx >= 23) {
        mantissa = (n.limb[limb_idx] >> (bit_idx - 23)) & 0x7FFFFFu;
    }
    else {
        uint top = n.limb[limb_idx] << (23 - bit_idx);
        uint bottom = 0u;
        if (limb_idx > 0) {
            bottom = n.limb[limb_idx - 1] >> (32 - (23 - bit_idx));
        }
        mantissa = (top | bottom) & 0x7FFFFFu;
    }
    uint sign_bit = (n.sign == -1) ? 1u : 0u;
    uint float_bits = (sign_bit << 31) | (exp_bits << 23) | mantissa;

    return uintBitsToFloat(float_bits);
}

number hp_sqrt(number x) {
    if (is_zero(x) || x.sign == -1) return null_number();

    float x_float = number_to_float(x);
    float guess_float = sqrt(x_float);
    number n_k = float_to_number(guess_float);

    number n_k_next = null_number();

    for (int i = 0; i < ((NUMBER_OF_LIMBS * 32)/ 23 + 2); ++i) {
        number div_term = hp_div(x, n_k);
        n_k_next = hp_add(n_k, div_term);
        n_k_next = shift_right(n_k_next, 1);
        if (compare_abs(n_k, n_k_next) == 0) {
            break;
        }
        n_k = n_k_next;
    }
    return n_k;
}

