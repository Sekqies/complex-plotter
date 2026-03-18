number hp_neg(number a){
	a.sign *= -1;
	return a;
}

uint hi(uint a){
	return a >> 16;
}

uint lo(uint a){
	return a & (0xFFFFu);
}


int compare_abs(number a, number b) {
    for(int i = NUMBER_OF_LIMBS - 1; i >= 0; --i) {
        if(a.limb[i] > b.limb[i]) return 1;
        if(a.limb[i] < b.limb[i]) return -1;
    }
    return 0;
}

bool is_zero(number a){
	bool notzero = false;
	for(int i = 0; i < NUMBER_OF_LIMBS; ++i){
		notzero = a.limb[i] != 0u || notzero;
	}
	return !notzero;
}

uvec2 sum_with_carry(uint a, uint b){
	uvec2 res = uvec2(0u,0u);
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
    number c = REAL_ZERO;
    uint borrow = 0u;
    for(int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        uint sub = a.limb[i] - b.limb[i] - borrow;
        borrow = uint((a.limb[i] < b.limb[i] || (a.limb[i] == b.limb[i] && borrow > 0u)));
        c.limb[i] = sub;
    }
    return c;
}

number hp_add(number a, number b){
	number c = REAL_ZERO;
	if(a.sign == b.sign){
		c = abs_sum(a,b);
		c.sign = a.sign;
		return c;
	}
	int cmp = compare_abs(a,b);
	if(cmp >= 0){
		c = abs_hp_sub(a,b);
		c.sign = a.sign;
		return c;
	}
	c = abs_hp_sub(b,a);
	c.sign = b.sign;
	return c;
}



number hp_sub(number a, number b){
	return hp_add(a,hp_neg(b));
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
    number c = REAL_ZERO;
    c.sign = a.sign * b.sign;
    
    if (is_zero(a) || is_zero(b)) return c;
    if (a.is_infinite || b.is_infinite) {
        c.limb = INFINITY.limb;
        return c;
    }
    for (int i = 0; i < NUMBER_OF_LIMBS; ++i) {
        if (a.limb[i] == 0u) continue;
        uint carry = 0u;
        
        for (int j = 0; j < NUMBER_OF_LIMBS; ++j) {
            int target = i + j - FRACTIONAL_SIZE;
            
            if (target >= NUMBER_OF_LIMBS) break; 
            
            uvec2 prod = product_with_remainder(a.limb[i], b.limb[j]);
            if (target < 0) {
                uvec2 hp_add2 = sum_with_carry(prod.x, carry);
                carry = prod.y + hp_add2.y;
            } else {
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
    
    number c = REAL_ZERO;
    c.sign = a.sign;

    if (limb_shift >= NUMBER_OF_LIMBS) return REAL_ZERO;

    for (int i = NUMBER_OF_LIMBS - 1; i >= limb_shift; --i) {
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
    
    number c = REAL_ZERO;
    c.sign = a.sign;

    if (limb_shift >= NUMBER_OF_LIMBS) return REAL_ZERO;
    for (int i = 0; i < NUMBER_OF_LIMBS - limb_shift; ++i) {
        int target = i;
        int source = i + limb_shift;

        uint val = a.limb[source] >> bit_shift;

        if (source < NUMBER_OF_LIMBS - 1 && bit_shift > 0) {
            val |= a.limb[source + 1] << (32 - bit_shift);
        }
        c.limb[target] = val;
    }
    return c;
}

int find_msb(number a) {
    for (int i = NUMBER_OF_LIMBS - 1; i >= 0; --i) {
        uint x = a.limb[i];
        if (x != 0u) {
            int bit_pos = 0;
            if ((x & 0xFFFF0000u) != 0u) { bit_pos += 16; x >>= 16; }
            if ((x & 0x0000FF00u) != 0u) { bit_pos += 8;  x >>= 8;  }
            if ((x & 0x000000F0u) != 0u) { bit_pos += 4;  x >>= 4;  }
            if ((x & 0x0000000Cu) != 0u) { bit_pos += 2;  x >>= 2;  }
            if ((x & 0x00000002u) != 0u) { bit_pos += 1;            }
            return (i * 32) + bit_pos;
        }
    }
    return -1; 
}

uint get_half(number a, int index){
    if (index >= NUMBER_OF_LIMBS * 2 || index < 0) {
            return 0u;
        }
    uint l = a.limb[index / 2];
    return uint(index % 2 == 0) * lo(l) + uint(index%2==1) * hi(l);
}

void set_half(inout number a, int index, uint val) {
    if (index >= NUMBER_OF_LIMBS * 2 || index < 0) return;
    int limb_idx = index / 2;
    if (index % 2 == 1) {
        a.limb[limb_idx] = (a.limb[limb_idx] & 0x0000FFFFu) | (val << 16);
    } else {
        a.limb[limb_idx] = (a.limb[limb_idx] & 0xFFFF0000u) | (val & 0xFFFFu);
    }
}

number mult_scalar_16(number a, uint b_16) {
    number c = REAL_ZERO;
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
    number q = REAL_ZERO;
    q.sign = n.sign * d.sign;

    uint u_halves[NUMBER_OF_LIMBS*2 + FRACTIONAL_SIZE * 2 + 1];

    if (is_zero(d)) {
        q = INFINITY;
        return q;
    }
    if (d.is_infinite) return q;

    int msb_d = find_msb(d);
    if (msb_d == -1) return q;

    int msb_n = find_msb(n);
    if (msb_n == -1) return q;

    for (int i = 0; i < NUMBER_OF_LIMBS*2 + FRACTIONAL_SIZE * 2 + 1; ++i) {
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
        const int MAX_HALVES = int(NUMBER_OF_LIMBS*2 + FRACTIONAL_SIZE * 2 + 1);
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
    number q = REAL_ZERO;
    q.sign = n.sign;
    uint rem = 0u;
    for (int i = NUMBER_OF_LIMBS * 2 - 1; i >= 0; --i) {
        uint dividend = (rem << 16) | get_half(n, i);
        uint q_i = dividend / d;
        rem = dividend % d;
        set_half(q, i, q_i);
    }
    return q;
}

number hp_exp(number x){
    if (is_zero(x)) return REAL_ONE;
    bool is_neg = (x.sign == -1);
    if (is_neg) x.sign = 1;
    const int K_SHIFT_LIMBS = 4;
    number x_small = shift_right(x, K_SHIFT_LIMBS * 32);

    number sum = REAL_ONE;
    number term = REAL_ONE;

    for (uint i = 1u; i < uint(NUMBER_OF_LIMBS * 8); ++i) { 
        term = hp_mult(term, x_small);
        term = div_uint(term, i);
        
        if (is_zero(term)) break; 
        
        sum = hp_add(sum, term);
    }
    for (int i = 0; i < (K_SHIFT_LIMBS * 32); ++i) {
        sum = hp_mult(sum, sum);
    }

    if (is_neg) {
        sum = hp_div(REAL_ONE, sum);
    }
    return sum;
}

number uint_to_number(uint v) {
    number res = REAL_ZERO;
    res.limb[FRACTIONAL_SIZE] = v; 
    return res;
}
number float_to_number(float f) {
    if (f == 0.0) return REAL_ZERO;
    
    uint f_bits = floatBitsToUint(f);
    uint sign_bit = f_bits >> 31;
    uint exp_bits = (f_bits >> 23) & 0xFFu;
    uint mantissa_bits = f_bits & 0x7FFFFFu;
    
    if (exp_bits == 255u) {
        number inf = INFINITY;
        inf.sign = (sign_bit == 1u) ? -1 : 1;
        return inf;
    }
    
    if (exp_bits == 0u) return REAL_ZERO; 

    number res = REAL_ZERO;
    res.sign = (sign_bit == 1u) ? -1 : 1;
    
    uint mantissa = mantissa_bits | 0x800000u;
    
    res.limb[FRACTIONAL_SIZE] = mantissa;
    
    int shift = int(exp_bits) - 127 - 23;
    
    if (shift > 0) {
        res = shift_left(res, shift);
    } else if (shift < 0) {
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

    return uintBitsToFloat(float_bits);
}

number get_ln2_constant(){
    return float_to_number(log(2.0));
}

number hp_log(number x){
    if(x.sign == -1 || is_zero(x)) return INFINITY;

    int msb = find_msb(x);
    int k = msb - (FRACTIONAL_SIZE * 32);

    number m;
    if (k > 0) m = shift_left(x,-k);
    else m = x;

    number z = hp_div(hp_sub(m,REAL_ONE),hp_add(m,REAL_ONE));
    number z_squared = hp_mult(z,z);

    number sum = z;
    number term = z;

    for (uint i = 3u; i < 512u; i += 2u) {
        term = hp_mult(term, z_squared);
        number iteration_term = div_uint(term, i);
       
        if (is_zero(iteration_term)) break;
        
        sum = hp_add(sum, iteration_term);
    }
    sum = shift_left(sum, 1);
    number k_num = uint_to_number(uint(abs(k)));
    k_num.sign = (k >=0)? 1 : -1;
    number k_ln2 = hp_mult(k_num, LN2);
    return hp_add(k_ln2, sum);
}

number hp_pow(number a, number b){
    return hp_exp(hp_mult(hp_log(a),b));
}

number hp_sqrt(number x){
    if(is_zero(x) || x.sign == -1) return REAL_ZERO;
    
    float x_float = number_to_float(x);
    float guess_float = sqrt(x_float);
    number n_k = float_to_number(guess_float);

    number n_k_next = REAL_ZERO;

    for(int i = 0; i < ((NUMBER_OF_LIMBS *32)/ 23 + 2); ++i){
        number div_term = hp_div(x,n_k);
        n_k_next = hp_add(n_k,div_term);
        n_k_next = shift_right(n_k_next,1);
        if(compare_abs(n_k,n_k_next) == 0){
            break;
        }
        n_k = n_k_next;
    }
    return n_k;
}

number hp_floor(number a) {
    if (a.is_infinite || is_zero(a)) return a;

    number c = a;
    bool has_fraction = false;

    for (int i = 0; i < FRACTIONAL_SIZE; ++i) {
        if (c.limb[i] != 0u) {
            has_fraction = true;
            c.limb[i] = 0u;
        }
    }

    if (c.sign == -1 && has_fraction) {
        c = hp_sub(c, REAL_ONE);
    }
    return c;
}

number hp_mod(number a, number b){
    if (is_zero(b)) return REAL_ZERO;
    
    number div_ab = hp_div(a,b);
    number floor_div = hp_floor(div_ab);
    number mult_b_floor = hp_mult(b, floor_div);
    
    return hp_sub(a, mult_b_floor);
}

number get_two_pi_constant(){
    return float_to_number(2.0f * 3.141592);
}

number get_pi_constant(){
    return float_to_number(3.141592);
}

number reduce_trig_range(number x) {
    number two_pi = get_two_pi_constant();
    number pi = PI;
    
    number x_mod = hp_mod(x, two_pi);
    
    if (x_mod.sign == -1 && !is_zero(x_mod)) {
        x_mod = hp_add(x_mod, two_pi);
    }
    
    if (compare_abs(x_mod, pi) == 1) { 
        x_mod = hp_sub(x_mod, two_pi);
    }
    
    return x_mod;
}

number hp_sin(number x){
    if(is_zero(x)) return REAL_ZERO;
    x = reduce_trig_range(x);
    
    number x_sq = hp_mult(x,x);
    number sum = x;
    number term = x;

    for(int i = 1; i < NUMBER_OF_LIMBS * 8; ++i){
        term = hp_mult(term,x_sq);
        uint divisor = uint(2*i) * uint(2*i +1);
        term = div_uint(term,divisor);

        if(is_zero(term)) break;

        if(i%2==1){
            sum = hp_sub(sum,term);
        }
        else{
            sum = hp_add(sum,term);
        }
    }
    return sum;
}

number hp_cos(number x) {
    if (is_zero(x)) return REAL_ONE;
    
    x = reduce_trig_range(x);
    
    number x_sq = hp_mult(x, x);
    number sum = REAL_ONE;
    number term = REAL_ONE;
    
    for (int i = 1; i < 256; ++i) {
        term = hp_mult(term, x_sq);
        
        uint divisor = uint(2 * i - 1) * uint(2 * i);
        term = div_uint(term, divisor);
        
        if (is_zero(term)) break; 
        
        if (i % 2 == 1) {
            sum = hp_sub(sum, term);
        } else {
            sum = hp_add(sum, term);
        }
    }
    return sum;
}

number internal_hp_atan(number z){
    if(is_zero(z)) return REAL_ZERO;

    number z_sq = hp_mult(z,z);
    number REAL_ONE = REAL_ONE;
    number hypotenuse = hp_sqrt(hp_add(REAL_ONE,z_sq));
    number den = hp_add(REAL_ONE,hypotenuse);
    z = hp_div(z,den);

    z_sq = hp_mult(z,z);
    number sum = z;
    number term = z;

    for(uint i = 1u; i < uint(NUMBER_OF_LIMBS) * 8u; ++i){
        term = hp_mult(term, z_sq);
        uint divisor = (2u*i) + 1u;
        number iteration_term = div_uint(term,divisor);

        if(is_zero(iteration_term)) break;

        if(i%2u == 1u){
            sum = hp_sub(sum,iteration_term);
        }
        else{
            sum = hp_add(sum,iteration_term);
        }
    }
    return shift_left(sum, 1);
}

number hp_atan2(number y, number x) {
    bool x_zero = is_zero(x);
    bool y_zero = is_zero(y);

    number pi = PI;
    number pi_over_2 = shift_right(pi, 1);

    if (x_zero && y_zero) return REAL_ZERO; 
    if (x_zero) {
        number res = pi_over_2;
        res.sign = y.sign; 
        return res;
    }
    if (y_zero) {
        if (x.sign == 1) return REAL_ZERO; 
        return pi;                             
    }

    number abs_y = y; abs_y.sign = 1;
    number abs_x = x; abs_x.sign = 1;
    
    number base_angle;

    if (compare_abs(abs_y, abs_x) == 1) { 
        number z = hp_div(abs_x, abs_y);
        base_angle = hp_sub(pi_over_2, internal_hp_atan(z));
    } else {
        number z = hp_div(abs_y, abs_x);
        base_angle = internal_hp_atan(z);
    }

    number final_angle = base_angle;

    if (x.sign == -1) {
        final_angle = hp_sub(pi, base_angle);
    }

    final_angle.sign = y.sign;

    return final_angle;
}

number hp_atan(number y, number x){
    return hp_atan2(y,x);
}


number hp_length(hp_vec2 z){
    return hp_sqrt(hp_add(hp_mult(z.x,z.x),hp_mult(z.y,z.y)));
}

number hp_sinh(number x){
    return shift_right(hp_sub(hp_exp(x),hp_exp(hp_neg(x))),1);
}

number hp_cosh(number x){
     return shift_right(hp_add(hp_exp(x),hp_exp(hp_neg(x))),1);
}
int hp_compare(number a, number b) {
    bool a_zero = is_zero(a);
    bool b_zero = is_zero(b);
    if (a_zero && b_zero) return 0;
    
    if (a.sign != b.sign) {
        return (a.sign == 1) ? 1 : -1;
    }
    
    int abs_cmp = compare_abs(a, b);
    
    return abs_cmp * a.sign;
}
number hp_step(number edge, number x) {
    if (hp_compare(x, edge) >= 0) {
        return REAL_ONE;
    }
    return REAL_ZERO;
}

number hp_mix(number x, number y, number a) {
    number diff = hp_sub(y, x);
    number scaled_diff = hp_mult(a, diff);    
    return hp_add(x, scaled_diff);
}

number hp_smoothstep(number edge0, number edge1, number x) {
    number t = hp_div(hp_sub(x, edge0), hp_sub(edge1, edge0));
    
    number zero = REAL_ZERO;
    number REAL_ONE = REAL_ONE;
    if (hp_compare(t, zero) < 0) t = zero;
    if (hp_compare(t, REAL_ONE) > 0) t = REAL_ONE;
    
    number t_sq = hp_mult(t, t);
    
    number two = hp_add(REAL_ONE, REAL_ONE);
    number three = hp_add(two, REAL_ONE);
    
    number two_t = hp_mult(two, t);
    number three_minus_two_t = hp_sub(three, two_t);
    
    return hp_mult(t_sq, three_minus_two_t);
}

hp_vec2 hp_floor(hp_vec2 a) {
    return initialize_hp_vec2(hp_floor(a.x), hp_floor(a.y));
}

hp_vec2 hp_div(hp_vec2 a, number b) {
    return initialize_hp_vec2(hp_div(a.x, b), hp_div(a.y, b));
}

hp_vec2 hp_mult(hp_vec2 a, number b) {
    return initialize_hp_vec2(hp_mult(a.x, b), hp_mult(a.y, b));
}

hp_vec2 hp_mult(number a, hp_vec2 b) {
    return initialize_hp_vec2(hp_mult(a, b.x), hp_mult(a, b.y));
}