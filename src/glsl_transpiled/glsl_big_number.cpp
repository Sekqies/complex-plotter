#include <glsl_transpiled/glsl_big_number.h>

hp_vec2 initialize_hp_vec2(number x, number y) {
    hp_vec2 res;
    res.x = x;
    res.y = y;
    return res;
}

number null_number() {
    number res;
    for (int i = 0; i < LIMB_SIZE; ++i) {
        res.limb[i] = 0u;
    }
    res.sign = 1;
    return res;
}

number infinite_number() {
    number res;
    for (int i = 0; i < LIMB_SIZE; ++i) {
        res.limb[i] = (1u << 31u) - 1u;
    }
    res.sign = 1;
    res.is_infinite = true;
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
    for (int i = LIMB_SIZE - 1; i >= 0; --i) {
        if (a.limb[i] > b.limb[i]) return 1;
        if (a.limb[i] < b.limb[i]) return -1;
    }
    return 0;
}

bool is_zero(number a) {
    bool notzero = false;
    for (int i = 0; i < LIMB_SIZE; ++i) {
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
    for (int i = 0; i < LIMB_SIZE; ++i) {
        uvec2 res = sum_with_carry(sum_with_carry(a.limb[i], b.limb[i]).x, carry);
        uint sum = res.x;
        carry = res.y;
        c.limb[i] = sum;
    }
    return c;
}

number abs_hp_sub(number a, number b) {
    number c = null_number();
    uint borrow = 0u;
    for (int i = 0; i < LIMB_SIZE; ++i) {
        uint sub = a.limb[i] - b.limb[i] - borrow;
        borrow = uint((a.limb[i] < b.limb[i] || (a.limb[i] == b.limb[i] && borrow > 0u)));
        c.limb[i] = sub;
    }
    return c;
}

number hp_add(number a, number b) {
    number c = null_number();
    if (a.sign == b.sign) {
        c = abs_sum(a, b);
        c.sign = a.sign;
        return c;
    }
    int cmp = compare_abs(a, b);
    if (cmp >= 0) {
        c = abs_hp_sub(a, b);
        c.sign = a.sign;
        return c;
    }
    c = abs_hp_sub(b, a);
    c.sign = b.sign;
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

    if (is_zero(a) || is_zero(b)) return c;
    if (a.is_infinite || b.is_infinite) {
        c.limb = infinite_number().limb;
        return c;
    }
    for (int i = 0; i < LIMB_SIZE; ++i) {
        if (a.limb[i] == 0u) continue;
        uint carry = 0u;

        for (int j = 0; j < LIMB_SIZE; ++j) {
            int target = i + j - FRACTIONAL_SIZE;

            if (target >= LIMB_SIZE) break;

            uvec2 prod = product_with_remainder(a.limb[i], b.limb[j]);
            if (target < 0) {
                uvec2 hp_add2 = sum_with_carry(prod.x, carry);
                carry = prod.y + hp_add2.y;
            }
            else {
                uvec2 hp_add1 = sum_with_carry(c.limb[target], prod.x);
                uvec2 hp_add2 = sum_with_carry(hp_add1.x, carry);
                c.limb[target] = hp_add2.x;
                carry = prod.y + hp_add1.y + hp_add2.y;
            }
        }
    }
    c.sign = c.sign * int(!is_zero(c));
    return c;
}

number shift_left(number a, int shift) {
    if (shift <= 0) return a;

    int limb_shift = shift / 32;
    int bit_shift = shift % 32;

    number c = null_number();
    c.sign = a.sign;

    if (limb_shift >= LIMB_SIZE) return null_number();

    for (int i = LIMB_SIZE - 1; i >= limb_shift; --i) {
        int target = i;
        int source = i - limb_shift;

        uint val = a.limb[source] << bit_shift;

        if (source > 0 && bit_shift > 0) {
            val |= a.limb[source - 1] >> (32 - bit_shift);
        }

        c.limb[target] = val;
    }

    return c;
}

number shift_right(number a, int shift) {
    if (shift <= 0) return a;

    int limb_shift = shift >> 5;
    int bit_shift = shift % 32;

    number c = null_number();
    c.sign = a.sign;

    if (limb_shift >= LIMB_SIZE) return null_number();
    for (int i = 0; i < LIMB_SIZE - limb_shift; ++i) {
        int target = i;
        int source = i + limb_shift;

        uint val = a.limb[source] >> bit_shift;

        if (source < LIMB_SIZE - 1 && bit_shift > 0) {
            val |= a.limb[source + 1] << (32 - bit_shift);
        }
        c.limb[target] = val;
    }
    return c;
}

int find_msb(number a) {
    for (int i = LIMB_SIZE - 1; i >= 0; --i) {
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
    uint l = a.limb[index / 2];
    return uint(index % 2 == 0) * hi(l) + uint(index % 2 == 1) * lo(l);
}

void set_half(number& a, int index, uint val) {
    if (index >= LIMB_SIZE * 2 || index < 0) return;
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

    for (int i = 0; i < LIMB_SIZE; ++i) {
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

    n = shift_left(n, FRACTIONAL_SIZE * 32);

    if (is_zero(d)) {
        q = infinite_number();
        return q;
    };
    if (d.is_infinite) return q;

    int msb_d = find_msb(d);
    if (msb_d == -1) return q;

    int msb_n = find_msb(n);
    if (msb_n == -1) return q;

    int len_v = (msb_d >> 4) + 1;
    int len_u = (msb_n >> 4) + 1;

    if (len_u < len_v) return q;

    if (len_v == 1) {
        uint v0 = get_half(d, 0);
        uint rem = 0u;
        for (int i = len_u - 1; i >= 0; --i) {
            uint dividend = (rem << 16) | get_half(n, i);
            uint q_i = dividend / v0;
            rem = dividend % v0;
            set_half(q, i, q_i);
        }
        return q;
    }
    int shift = 15 - (msb_d % 16);
    number u = shift_left(n, shift);
    number v = shift_left(d, shift);

    int m = len_u - len_v;
    int n_len = len_v;

    uint v_n1 = get_half(v, n_len - 1);
    uint v_n2 = get_half(v, n_len - 2);

    for (int j = m; j >= 0; --j) {
        uint u_jn = get_half(u, j + n_len);
        uint u_jn1 = get_half(u, j + n_len - 1);
        uint u_jn2 = get_half(u, j + n_len - 2);

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

            uint u_ji = get_half(u, j + i);
            int diff = int(u_ji) - int(p_lo) - int(borrow);

            set_half(u, j + i, uint(diff) & 0xFFFFu);
            borrow = (diff < 0) ? 1u : 0u;
        }
        int final_diff = int(get_half(u, j + n_len)) - int(k) - int(borrow);
        set_half(u, j + n_len, uint(final_diff) & 0xFFFFu);
        if (final_diff < 0) {
            q_hat--;
            uint carry_hp_add = 0u;
            for (int i = 0; i < n_len; ++i) {
                uint sum = get_half(u, j + i) + get_half(v, i) + carry_hp_add;
                set_half(u, j + i, sum & 0xFFFFu);
                carry_hp_add = sum >> 16;
            }
            uint sum_last = get_half(u, j + n_len) + carry_hp_add;
            set_half(u, j + n_len, sum_last & 0xFFFFu);
        }
        set_half(q, j, q_hat);
    }
    return q;
}

number div_uint(number n, uint d) {
    number q = null_number();
    q.sign = n.sign;
    uint rem = 0u;
    for (int i = LIMB_SIZE * 2 - 1; i >= 0; --i) {
        uint dividend = (rem << 16) | get_half(n, i);
        uint q_i = dividend / d;
        rem = dividend % d;
        set_half(q, i, q_i);
    }
    return q;
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

#include <bit>

float uintBitsToFloat(const uint n) {
    return std::bit_cast<float>(n);
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

    for (int i = 0; i < (LIMB_SIZE / 23 + 2); ++i) {
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

inline number::number() : limb(LIMB_SIZE, 0), sign(1), is_infinite(false) {}
