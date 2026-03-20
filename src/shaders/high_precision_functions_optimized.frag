number R[16];
hp_vec2 V[4];
hp_vec2 E_V[4];

uint EXT_MULT_REG[NUMBER_OF_LIMBS + FRACTIONAL_SIZE];
uint U_HALVES_REG[NUMBER_OF_LIMBS * 2 + FRACTIONAL_SIZE * 2 + 1];

void hp_neg(in number a, out number res) {
    R[0] = a;
    R[0].sign *= -1;
    res = R[0];
}

void hi(in uint a, out uint res) {
    res = a >> 16;
}

void lo(in uint a, out uint res) {
    res = a & 0xFFFFu;
}

void compare_abs(in number a, in number b, out int res) {
    res = 0;
    for (int i = NUMBER_OF_LIMBS - 1; i >= 0; --i) {
        if (a.limb[i] > b.limb[i]) {
            res = 1;
            return;
        }
        if (a.limb[i] < b.limb[i]) {
            res = -1;
            return;
        }
    }
}

void is_zero(in number a, out bool res) {
    res = true;
    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        if (a.limb[i] != 0u) {
            res = false;
            return;
        }
    }
}

void sum_with_carry(in uint a, in uint b, out uvec2 res) {
    res = uvec2(0u, 0u);
    res.x = a + b;
    res.y = uint(a > res.x || b > res.x);
}

void abs_sum(in number a, in number b, out number res) {
    R[0] = REAL_ZERO;
    uint carry = 0u;
    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        uvec2 step1 = uvec2(0u);
        uvec2 step2 = uvec2(0u);
        sum_with_carry(a.limb[i], b.limb[i], step1);
        sum_with_carry(step1.x, carry, step2);
        R[0].limb[i] = step2.x;
        carry = step1.y + step2.y;
    }
    res = R[0];
}

void abs_hp_sub(in number a, in number b, out number res) {
    R[0] = REAL_ZERO;
    uint borrow = 0u;
    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        uint sub = a.limb[i] - b.limb[i] - borrow;
        borrow = uint((a.limb[i] < b.limb[i] || (a.limb[i] == b.limb[i] && borrow > 0u)));
        R[0].limb[i] = sub;
    }
    res = R[0];
}

void hp_add(in number a, in number b, out number res) {
    if (a.sign == b.sign) {
        abs_sum(a, b, R[1]);
        R[1].sign = a.sign;
        res = R[1];
        return;
    }
    int cmp = 0;
    compare_abs(a, b, cmp);
    if (cmp >= 0) {
        abs_hp_sub(a, b, R[1]);
        R[1].sign = a.sign;
        res = R[1];
        return;
    }
    abs_hp_sub(b, a, R[1]);
    R[1].sign = b.sign;
    res = R[1];
}

void hp_sub(in number a, in number b, out number res) {
    hp_neg(b, R[15]);
    hp_add(a, R[15], res);
}

void product_with_remainder(in uint a, in uint b, out uvec2 res) {
    uint low_part = a * b;
    uint al = 0u;
    uint ah = 0u;
    uint bl = 0u;
    uint bh = 0u;
    lo(a, al);
    hi(a, ah);
    lo(b, bl);
    hi(b, bh);

    uint p0 = al * bl;
    uint p1 = al * bh;
    uint p2 = ah * bl;
    uint p3 = ah * bh;

    uvec2 m_sum = uvec2(0u);
    sum_with_carry(p1, p2, m_sum);
    
    uint hi_p0 = 0u;
    hi(p0, hi_p0);
    
    uvec2 m_combined = uvec2(0u);
    sum_with_carry(m_sum.x, hi_p0, m_combined);

    uint hi_m_combined = 0u;
    hi(m_combined.x, hi_m_combined);
    
    uint high_part = p3 + hi_m_combined + ((m_sum.y + m_combined.y) << 16);
    res = uvec2(low_part, high_part);
}

void hp_mult(in number a, in number b, out number res) {
    R[0] = REAL_ZERO;
    R[0].sign = a.sign * b.sign;
    
    bool a_zero = false;
    bool b_zero = false;
    is_zero(a, a_zero);
    is_zero(b, b_zero);
    
    if (a_zero || b_zero) {
        res = R[0];
        return;
    }
    if (a.is_infinite || b.is_infinite) {
        R[0].limb = INFINITY.limb;
        res = R[0];
        return;
    }

    for(int i = 0; i < NUMBER_OF_LIMBS + FRACTIONAL_SIZE; ++i) {
        EXT_MULT_REG[i] = 0u;
    }

    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        if (a.limb[i] == 0u) continue;
        uint carry = 0u;
        
        for (int j = 0; j < NUMBER_OF_LIMBS; ++j) {
            int ext_target = i + j;
            if (ext_target >= NUMBER_OF_LIMBS + FRACTIONAL_SIZE) break; 
            
            uvec2 prod = uvec2(0u);
            product_with_remainder(a.limb[i], b.limb[j], prod);
            
            uvec2 hp_add1 = uvec2(0u);
            uvec2 hp_add2 = uvec2(0u);
            
            sum_with_carry(EXT_MULT_REG[ext_target], prod.x, hp_add1);
            sum_with_carry(hp_add1.x, carry, hp_add2);
            
            EXT_MULT_REG[ext_target] = hp_add2.x;
            carry = prod.y + hp_add1.y + hp_add2.y;
        }

        int leftover_target = i + NUMBER_OF_LIMBS;
        while (carry > 0u && leftover_target < NUMBER_OF_LIMBS + FRACTIONAL_SIZE) {
            uvec2 hp_add1 = uvec2(0u);
            sum_with_carry(EXT_MULT_REG[leftover_target], carry, hp_add1);
            EXT_MULT_REG[leftover_target] = hp_add1.x;
            carry = hp_add1.y;
            leftover_target++;
        }
    }

    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        R[0].limb[i] = EXT_MULT_REG[i + FRACTIONAL_SIZE];
    }

    bool z_c = false;
    is_zero(R[0], z_c);
    R[0].sign = R[0].sign * int(!z_c);
    res = R[0];
}

void shift_left(in number a, in int shift, out number res) {
    if (shift <= 0) {
        res = a;
        return;
    }
    
    int limb_shift = shift / 32;
    int bit_shift = shift % 32;
    
    R[0] = REAL_ZERO;
    R[0].sign = a.sign;

    if (limb_shift >= NUMBER_OF_LIMBS) {
        res = R[0];
        return;
    }

    for (int i = NUMBER_OF_LIMBS - 1; i >= limb_shift; --i) {
        int source = i - limb_shift;
        uint val = a.limb[source] << bit_shift;

        if (source > 0 && bit_shift > 0) {
            val |= a.limb[source - 1] >> (32 - bit_shift);
        }
        R[0].limb[i] = val;
    }
    res = R[0];
}

void shift_right(in number a, in int shift, out number res) {
    if (shift <= 0) {
        res = a;
        return;
    }
    
    int limb_shift = shift >> 5;
    int bit_shift = shift % 32;
    
    R[0] = REAL_ZERO;
    R[0].sign = a.sign;

    if (limb_shift >= NUMBER_OF_LIMBS) {
        res = R[0];
        return;
    }
    for (int i = 0; i < NUMBER_OF_LIMBS - limb_shift; ++i) {
        int source = i + limb_shift;
        uint val = a.limb[source] >> bit_shift;

        if (source < NUMBER_OF_LIMBS - 1 && bit_shift > 0) {
            val |= a.limb[source + 1] << (32 - bit_shift);
        }
        R[0].limb[i] = val;
    }
    res = R[0];
}

void find_msb(in number a, out int res) {
    for (int i = NUMBER_OF_LIMBS - 1; i >= 0; --i) {
        uint x = a.limb[i];
        if (x != 0u) {
            int bit_pos = 0;
            if ((x & 0xFFFF0000u) != 0u) { bit_pos += 16; x >>= 16; }
            if ((x & 0x0000FF00u) != 0u) { bit_pos += 8;  x >>= 8;  }
            if ((x & 0x000000F0u) != 0u) { bit_pos += 4;  x >>= 4;  }
            if ((x & 0x0000000Cu) != 0u) { bit_pos += 2;  x >>= 2;  }
            if ((x & 0x00000002u) != 0u) { bit_pos += 1;            }
            res = (i * 32) + bit_pos;
            return;
        }
    }
    res = -1; 
}

void get_half(in number a, in int index, out uint res) {
    if (index >= NUMBER_OF_LIMBS * 2 || index < 0) {
        res = 0u;
        return;
    }
    uint l = a.limb[index / 2];
    uint lo_l = 0u;
    uint hi_l = 0u;
    lo(l, lo_l);
    hi(l, hi_l);
    res = uint(index % 2 == 0) * lo_l + uint(index % 2 == 1) * hi_l;
}

void set_half(inout number a, in int index, in uint val) {
    if (index >= NUMBER_OF_LIMBS * 2 || index < 0) return;
    int limb_idx = index / 2;
    if (index % 2 == 1) {
        a.limb[limb_idx] = (a.limb[limb_idx] & 0x0000FFFFu) | (val << 16);
    } else {
        a.limb[limb_idx] = (a.limb[limb_idx] & 0xFFFF0000u) | (val & 0xFFFFu);
    }
}

void mult_scalar_16(in number a, in uint b_16, out number res) {
    R[0] = REAL_ZERO;
    R[0].sign = a.sign;
    uint carry = 0u;
    
    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        if (a.limb[i] == 0u && carry == 0u) continue;
        
        uvec2 prod = uvec2(0u);
        uvec2 hp_add1 = uvec2(0u);
        product_with_remainder(a.limb[i], b_16, prod);
        sum_with_carry(prod.x, carry, hp_add1);
        
        R[0].limb[i] = hp_add1.x;
        carry = prod.y + hp_add1.y;
    }
    res = R[0];
}

void hp_div(in number n, in number d, out number res) {
    R[2] = REAL_ZERO;
    R[2].sign = n.sign * d.sign;

    bool d_zero = false;
    is_zero(d, d_zero);
    if (d_zero) {
        R[2] = INFINITY;
        res = R[2];
        return;
    }
    if (d.is_infinite) { res = R[2]; return; }

    int msb_d = 0; 
    find_msb(d, msb_d);
    if (msb_d == -1) { res = R[2]; return; }

    int msb_n = 0; 
    find_msb(n, msb_n);
    if (msb_n == -1) { res = R[2]; return; }

    int MAX_HALVES = int(NUMBER_OF_LIMBS * 2 + FRACTIONAL_SIZE * 2 + 1);
    for (int i = 0; i < MAX_HALVES; ++i) {
        U_HALVES_REG[i] = 0u;
    }

    for (int i = 0; i < NUMBER_OF_LIMBS * 2; ++i) {
        uint half_v = 0u;
        get_half(n, i, half_v);
        U_HALVES_REG[i + FRACTIONAL_SIZE * 2] = half_v;
    }

    int len_v = (msb_d >> 4) + 1;
    int len_u_unshifted = ((msb_n + FRACTIONAL_SIZE * 32) >> 4) + 1;

    if (len_u_unshifted < len_v) { res = R[2]; return; }

    if (len_v == 1) {
        uint v0 = 0u;
        int idx = 0;
        get_half(d, idx, v0);
        uint rem = 0u;
        for (int i = len_u_unshifted - 1; i >= 0; --i) {
            uint dividend = (rem << 16) | U_HALVES_REG[i];
            uint q_i = dividend / v0;
            rem = dividend % v0;
            if (i < NUMBER_OF_LIMBS * 2) {
                set_half(R[2], i, q_i);
            }
        }
        res = R[2];
        return;
    }

    int shift = 15 - (msb_d % 16);
    shift_left(d, shift, R[15]);

    if (shift > 0) {
        uint carry_shift = 0u;
        for (int i = 0; i < MAX_HALVES; ++i) {
            uint val = (U_HALVES_REG[i] << shift) | carry_shift;
            carry_shift = U_HALVES_REG[i] >> (16 - shift);
            U_HALVES_REG[i] = val & 0xFFFFu;
        }
    }

    int len_u = ((msb_n + FRACTIONAL_SIZE * 32 + shift) >> 4) + 1;
    int m = len_u - len_v;
    int n_len = len_v;

    uint v_n1 = 0u;
    uint v_n2 = 0u;
    int idx_n1 = n_len - 1;
    int idx_n2 = n_len - 2;
    get_half(R[15], idx_n1, v_n1);
    get_half(R[15], idx_n2, v_n2);

    for (int j = m; j >= 0; --j) {
        uint u_jn = U_HALVES_REG[j + n_len];
        uint u_jn1 = U_HALVES_REG[j + n_len - 1];
        uint u_jn2 = (j + n_len >= 2) ? U_HALVES_REG[j + n_len - 2] : 0u;

        uint dividend = (u_jn << 16) | u_jn1;
        uint q_hat = 0u;
        uint r_hat = 0u;

        if (u_jn == v_n1) {
            q_hat = 0xFFFFu;
            r_hat = u_jn1 + v_n1;
        } else {
            q_hat = dividend / v_n1;
            r_hat = dividend % v_n1;
        }

        while (r_hat < 0x10000u && (q_hat * v_n2) > ((r_hat << 16) | u_jn2)) {
            q_hat--;
            r_hat += v_n1;
        }

        uint k = 0u;
        uint borrow = 0u;
        for (int i = 0; i < n_len; ++i) {
            uint v_half = 0u;
            get_half(R[15], i, v_half);
            uint p = q_hat * v_half + k;
            k = p >> 16;
            uint p_lo = p & 0xFFFFu;

            uint u_ji = U_HALVES_REG[j + i];
            int diff = int(u_ji) - int(p_lo) - int(borrow);

            U_HALVES_REG[j + i] = uint(diff) & 0xFFFFu;
            borrow = (diff < 0) ? 1u : 0u;
        }
        
        int final_diff = int(U_HALVES_REG[j + n_len]) - int(k) - int(borrow);
        U_HALVES_REG[j + n_len] = uint(final_diff) & 0xFFFFu;

        if (final_diff < 0) {
            q_hat--;
            uint carry_add = 0u;
            for (int i = 0; i < n_len; ++i) {
                uint v_half = 0u;
                get_half(R[15], i, v_half);
                uint sum = U_HALVES_REG[j + i] + v_half + carry_add;
                U_HALVES_REG[j + i] = sum & 0xFFFFu;
                carry_add = sum >> 16;
            }
            uint sum_last = U_HALVES_REG[j + n_len] + carry_add;
            U_HALVES_REG[j + n_len] = sum_last & 0xFFFFu;
        }

        if (j < NUMBER_OF_LIMBS * 2) {
            set_half(R[2], j, q_hat);
        }
    }
    res = R[2];
}

void div_uint(in number n, in uint d, out number res) {
    R[0] = REAL_ZERO;
    R[0].sign = n.sign;
    uint rem = 0u;
    for (int i = NUMBER_OF_LIMBS * 2 - 1; i >= 0; --i) {
        uint half_v = 0u;
        get_half(n, i, half_v);
        uint dividend = (rem << 16) | half_v;
        uint q_i = dividend / d;
        rem = dividend % d;
        set_half(R[0], i, q_i);
    }
    res = R[0];
}

void uint_to_number(in uint v, out number res) {
    R[0] = REAL_ZERO;
    R[0].limb[FRACTIONAL_SIZE] = v; 
    res = R[0];
}

void float_to_number(in float f, out number res) {
    if (f == 0.0) { res = REAL_ZERO; return; }
    
    uint f_bits = floatBitsToUint(f);
    uint sign_bit = f_bits >> 31;
    uint exp_bits = (f_bits >> 23) & 0xFFu;
    uint mantissa_bits = f_bits & 0x7FFFFFu;
    
    if (exp_bits == 255u) {
        R[0] = INFINITY;
        R[0].sign = (sign_bit == 1u) ? -1 : 1;
        res = R[0];
        return;
    }
    
    if (exp_bits == 0u) { res = REAL_ZERO; return; } 

    R[0] = REAL_ZERO;
    R[0].sign = (sign_bit == 1u) ? -1 : 1;
    uint mantissa = mantissa_bits | 0x800000u;
    R[0].limb[FRACTIONAL_SIZE] = mantissa;
    
    int shift = int(exp_bits) - 127 - 23;
    
    if (shift > 0) {
        shift_left(R[0], shift, R[15]);
        R[0] = R[15];
    } else if (shift < 0) {
        int neg_shift = -shift;
        shift_right(R[0], neg_shift, R[15]);
        R[0] = R[15];
    }
    res = R[0];
}

void number_to_float(in number n, out float res) {
    bool is_z = false;
    is_zero(n, is_z);
    if (is_z) { res = 0.0; return; }    
    if (n.is_infinite) {
        res = (n.sign == 1) ? uintBitsToFloat(0x7F800000u) : uintBitsToFloat(0xFF800000u);
        return;
    }
    int msb = 0;
    find_msb(n, msb);
    int true_exp = msb - (FRACTIONAL_SIZE * 32);
    if (true_exp > 127) {
        res = (n.sign == 1) ? uintBitsToFloat(0x7F800000u) : uintBitsToFloat(0xFF800000u);
        return;
    }
    if (true_exp < -126) { res = 0.0; return; }

    uint exp_bits = uint(true_exp + 127);
    uint mantissa = 0u;

    int limb_idx = msb / 32;
    int bit_idx = msb % 32;

    if (bit_idx >= 23) {
        mantissa = (n.limb[limb_idx] >> (bit_idx - 23)) & 0x7FFFFFu;
    } else {
        uint top = n.limb[limb_idx] << (23 - bit_idx);
        uint bottom = 0u;
        if (limb_idx > 0) {
            bottom = n.limb[limb_idx - 1] >> (32 - (23 - bit_idx));
        }
        mantissa = (top | bottom) & 0x7FFFFFu;
    }
    uint sign_bit = (n.sign == -1) ? 1u : 0u;    
    uint float_bits = (sign_bit << 31) | (exp_bits << 23) | mantissa;

    res = uintBitsToFloat(float_bits);
}

void hp_exp(in number x, out number res) {
    bool is_z = false;
    is_zero(x, is_z);
    if (is_z) { res = REAL_ONE; return; }
    
    bool is_neg = (x.sign == -1);
    if (is_neg) x.sign = 1;
    
    int shift_amt = 4 * 32;
    shift_right(x, shift_amt, R[4]); 

    R[5] = REAL_ONE; 
    R[6] = REAL_ONE; 

    uint limit = uint(NUMBER_OF_LIMBS * 8);
    for (uint i = 1u; i < limit; ++i) { 
        hp_mult(R[6], R[4], R[7]);
        R[6] = R[7];
        div_uint(R[6], i, R[7]);
        R[6] = R[7];
        
        bool term_z = false;
        is_zero(R[6], term_z);
        if (term_z) break; 
        
        hp_add(R[5], R[6], R[7]);
        R[5] = R[7];
    }
    for (int i = 0; i < shift_amt; ++i) {
        hp_mult(R[5], R[5], R[7]);
        R[5] = R[7];
    }

    if (is_neg) {
        hp_div(REAL_ONE, R[5], R[7]);
        R[5] = R[7];
    }
    res = R[5];
}

void get_ln2_constant(out number res) {
    float ln2_f = log(2.0);
    float_to_number(ln2_f, res);
}

void hp_log(in number x, out number res) {
    bool is_z = false;
    is_zero(x, is_z);
    if (x.sign == -1 || is_z) { res = INFINITY; return; }

    int msb = 0;
    find_msb(x, msb);
    int k = msb - (FRACTIONAL_SIZE * 32);

    if (k > 0) {
        int neg_k = -k;
        shift_left(x, neg_k, R[4]);
    } else {
        R[4] = x;
    }

    hp_sub(R[4], REAL_ONE, R[6]);
    hp_add(R[4], REAL_ONE, R[7]);
    hp_div(R[6], R[7], R[8]);
    
    hp_mult(R[8], R[8], R[9]);

    R[10] = R[8];
    R[11] = R[8];

    for (uint i = 3u; i < 512u; i += 2u) {
        hp_mult(R[11], R[9], R[12]);
        R[11] = R[12];
        div_uint(R[11], i, R[12]);
        
        bool term_z = false;
        is_zero(R[12], term_z);
        if (term_z) break;
        
        hp_add(R[10], R[12], R[13]);
        R[10] = R[13];
    }
    int shift = 1;
    shift_left(R[10], shift, R[12]);
    R[10] = R[12];
    
    uint abs_k = uint(abs(k));
    uint_to_number(abs_k, R[12]);
    R[12].sign = (k >= 0) ? 1 : -1;
    
    hp_mult(R[12], LN2, R[13]);
    hp_add(R[13], R[10], res);
}

void hp_pow(in number a, in number b, out number res) {
    hp_log(a, R[12]);
    hp_mult(R[12], b, R[13]);
    hp_exp(R[13], res);
}

void hp_sqrt(in number x, out number res) {
    bool is_z = false;
    is_zero(x, is_z);
    if (is_z || x.sign == -1) { res = REAL_ZERO; return; }
    
    float x_float = 0.0;
    number_to_float(x, x_float);
    float guess_float = sqrt(x_float);
    float_to_number(guess_float, R[4]);

    R[5] = REAL_ZERO;

    int loops = ((NUMBER_OF_LIMBS * 32) / 23) + 2;
    for (int i = 0; i < loops; ++i) {
        hp_div(x, R[4], R[6]);
        hp_add(R[4], R[6], R[5]);
        int shift = 1;
        shift_right(R[5], shift, R[7]);
        R[5] = R[7];
        
        int cmp = 0;
        compare_abs(R[4], R[5], cmp);
        if (cmp == 0) break;
        R[4] = R[5];
    }
    res = R[4];
}

void hp_floor(in number a, out number res) {
    bool is_z = false;
    is_zero(a, is_z);
    if (a.is_infinite || is_z) { res = a; return; }

    R[0] = a;
    bool has_fraction = false;

    for (int i = 0; i < FRACTIONAL_SIZE; ++i) {
        if (R[0].limb[i] != 0u) {
            has_fraction = true;
            R[0].limb[i] = 0u;
        }
    }

    if (R[0].sign == -1 && has_fraction) {
        hp_sub(R[0], REAL_ONE, R[15]);
        R[0] = R[15];
    }
    res = R[0];
}

void hp_mod(in number a, in number b, out number res) {
    bool is_z = false;
    is_zero(b, is_z);
    if (is_z) { res = REAL_ZERO; return; }
    
    hp_div(a, b, R[11]);
    hp_floor(R[11], R[12]);
    hp_mult(b, R[12], R[13]);
    hp_sub(a, R[13], res);
}

void get_two_pi_constant(out number res) {
    float pi2 = 2.0 * 3.141592;
    float_to_number(pi2, res);
}

void get_pi_constant(out number res) {
    float pif = 3.141592;
    float_to_number(pif, res);
}

void reduce_trig_range(in number x, out number res) {
    get_two_pi_constant(R[4]);
    R[5] = PI;
    
    hp_mod(x, R[4], R[14]); 
    
    bool is_z = false;
    is_zero(R[14], is_z);
    if (R[14].sign == -1 && !is_z) {
        hp_add(R[14], R[4], R[15]);
        R[14] = R[15];
    }
    
    int cmp = 0;
    compare_abs(R[14], R[5], cmp);
    if (cmp == 1) { 
        hp_sub(R[14], R[4], R[15]);
        R[14] = R[15];
    }
    res = R[14];
}

void hp_sin(in number x, out number res) {
    bool is_z = false;
    is_zero(x, is_z);
    if (is_z) { res = REAL_ZERO; return; }
    
    reduce_trig_range(x, R[4]);
    x = R[4];
    
    hp_mult(x, x, R[5]);
    R[6] = x;
    R[7] = x;

    int limit = NUMBER_OF_LIMBS * 8;
    for (int i = 1; i < limit; ++i) {
        hp_mult(R[7], R[5], R[8]);
        R[7] = R[8];
        uint divisor = uint(2 * i) * uint(2 * i + 1);
        div_uint(R[7], divisor, R[9]);
        R[7] = R[9];

        bool term_z = false;
        is_zero(R[7], term_z);
        if (term_z) break;

        if (i % 2 == 1) {
            hp_sub(R[6], R[7], R[10]);
            R[6] = R[10];
        } else {
            hp_add(R[6], R[7], R[11]);
            R[6] = R[11];
        }
    }
    res = R[6];
}

void hp_cos(in number x, out number res) {
    bool is_z = false;
    is_zero(x, is_z);
    if (is_z) { res = REAL_ONE; return; }
    
    reduce_trig_range(x, R[4]);
    x = R[4];
    
    hp_mult(x, x, R[5]);
    R[6] = REAL_ONE;
    R[7] = REAL_ONE;
    
    for (int i = 1; i < 256; ++i) {
        hp_mult(R[7], R[5], R[8]);
        R[7] = R[8];
        
        uint divisor = uint(2 * i - 1) * uint(2 * i);
        div_uint(R[7], divisor, R[9]);
        R[7] = R[9];
        
        bool term_z = false;
        is_zero(R[7], term_z);
        if (term_z) break; 
        
        if (i % 2 == 1) {
            hp_sub(R[6], R[7], R[10]);
            R[6] = R[10];
        } else {
            hp_add(R[6], R[7], R[11]);
            R[6] = R[11];
        }
    }
    res = R[6];
}

void internal_hp_atan(in number z, out number res) {
    bool is_z = false;
    is_zero(z, is_z);
    if (is_z) { res = REAL_ZERO; return; }

    hp_mult(z, z, R[4]);
    
    hp_add(REAL_ONE, R[4], R[6]);
    hp_sqrt(R[6], R[7]);
    hp_add(REAL_ONE, R[7], R[8]);
    hp_div(z, R[8], R[9]);
    z = R[9];

    hp_mult(z, z, R[4]);
    R[6] = z;
    R[7] = z;

    uint limit = uint(NUMBER_OF_LIMBS) * 8u;
    for (uint i = 1u; i < limit; ++i) {
        hp_mult(R[7], R[4], R[8]);
        R[7] = R[8];
        uint divisor = (2u * i) + 1u;
        div_uint(R[7], divisor, R[9]);

        bool term_z = false;
        is_zero(R[9], term_z);
        if (term_z) break;

        if (i % 2u == 1u) {
            hp_sub(R[6], R[9], R[10]);
            R[6] = R[10];
        } else {
            hp_add(R[6], R[9], R[11]);
            R[6] = R[11];
        }
    }
    int shift = 1;
    shift_left(R[6], shift, res);
}

void hp_atan2(in number y, in number x, out number res) {
    bool x_zero = false;
    bool y_zero = false;
    is_zero(x, x_zero);
    is_zero(y, y_zero);

    R[4] = PI;
    int shift = 1;
    shift_right(R[4], shift, R[5]); 

    if (x_zero && y_zero) { res = REAL_ZERO; return; }
    if (x_zero) {
        R[6] = R[5];
        R[6].sign = y.sign; 
        res = R[6]; return;
    }
    if (y_zero) {
        if (x.sign == 1) { res = REAL_ZERO; return; }
        res = R[4]; return;                            
    }

    R[6] = y; 
    R[6].sign = 1;
    R[7] = x; 
    R[7].sign = 1;
    
    int cmp = 0;
    compare_abs(R[6], R[7], cmp);
    if (cmp == 1) { 
        hp_div(R[7], R[6], R[8]);
        internal_hp_atan(R[8], R[10]);

        R[5] = PI;
        shift_right(R[5], 1, R[5]);
        hp_sub(R[5], R[10], R[10]);
    } else {
        hp_div(R[6], R[7], R[8]);
        internal_hp_atan(R[8], R[10]);
    }

    R[11] = R[10];

    if (x.sign == -1) {
        R[4] = PI;
        hp_sub(R[4], R[10], R[12]);
        R[11] = R[12];
    }

    R[11].sign = y.sign;
    res = R[11];
}

void hp_atan(in number y, in number x, out number res) {
    hp_atan2(y, x, res);
}

void hp_length(in hp_vec2 z, out number res) {
    hp_mult(z.x, z.x, R[4]);
    hp_mult(z.y, z.y, R[5]);
    hp_add(R[4], R[5], R[6]);
    hp_sqrt(R[6], res);
}

void hp_sinh(in number x, out number res) {
    hp_exp(x, R[8]);
    hp_neg(x, R[9]);
    hp_exp(R[9], R[10]);
    hp_sub(R[8], R[10], R[11]);
    int shift = 1;
    shift_right(R[11], shift, res);
}

void hp_cosh(in number x, out number res) {
    hp_exp(x, R[8]);
    hp_neg(x, R[9]);
    hp_exp(R[9], R[10]);
    hp_add(R[8], R[10], R[11]);
    int shift = 1;
    shift_right(R[11], shift, res);
}

void hp_compare(in number a, in number b, out int res) {
    bool a_zero = false;
    bool b_zero = false;
    is_zero(a, a_zero);
    is_zero(b, b_zero);
    if (a_zero && b_zero) { res = 0; return; }
    
    if (a.sign != b.sign) {
        res = (a.sign == 1) ? 1 : -1;
        return;
    }
    
    int abs_cmp = 0;
    compare_abs(a, b, abs_cmp);
    res = abs_cmp * a.sign;
}

void hp_step(in number edge, in number x, out number res) {
    int cmp = 0;
    hp_compare(x, edge, cmp);
    if (cmp >= 0) {
        res = REAL_ONE;
        return;
    }
    res = REAL_ZERO;
}

void hp_mix(in number x, in number y, in number a, out number res) {
    hp_sub(y, x, R[4]);
    hp_mult(a, R[4], R[5]);    
    hp_add(x, R[5], res);
}

void hp_smoothstep(in number edge0, in number edge1, in number x, out number res) {
    hp_sub(x, edge0, R[4]);
    hp_sub(edge1, edge0, R[5]);
    hp_div(R[4], R[5], R[6]);
    
    R[7] = REAL_ZERO;
    R[8] = REAL_ONE;
    
    int cmp1 = 0;
    int cmp2 = 0;
    hp_compare(R[6], R[7], cmp1);
    if (cmp1 < 0) R[6] = R[7];
    hp_compare(R[6], R[8], cmp2);
    if (cmp2 > 0) R[6] = R[8];
    
    hp_mult(R[6], R[6], R[9]);
    
    hp_add(R[8], R[8], R[10]);
    hp_add(R[10], R[8], R[11]);
    
    hp_mult(R[10], R[6], R[12]);
    hp_sub(R[11], R[12], R[13]);
    
    hp_mult(R[9], R[13], res);
}

void hp_vector_floor(in hp_vec2 a, out hp_vec2 res) {
    hp_floor(a.x, V[0].x);
    hp_floor(a.y, V[0].y);
    res = V[0];
}

void hp_div(in hp_vec2 a, in number b, out hp_vec2 res) {
    hp_div(a.x, b, V[0].x);
    hp_div(a.y, b, V[0].y);
    res = V[0];
}

void hp_mult(in hp_vec2 a, in number b, out hp_vec2 res) {
    hp_mult(a.x, b, V[0].x);
    hp_mult(a.y, b, V[0].y);
    res = V[0];
}

void hp_mult(in number a, in hp_vec2 b, out hp_vec2 res) {
    hp_mult(a, b.x, V[0].x);
    hp_mult(a, b.y, V[0].y);
    res = V[0];
}