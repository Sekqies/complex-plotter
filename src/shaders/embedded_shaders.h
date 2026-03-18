#pragma once
#include <string>

inline std::string SRC_HIGH_PRECISION_FOOTER_FRAG = 
    R"shdr(uniform uint u_center_x_limb[NUMBER_OF_LIMBS];
uniform int u_center_x_sign;

uniform uint u_center_y_limb[NUMBER_OF_LIMBS];
uniform int u_center_y_sign;

uniform uint u_zoom_limb[NUMBER_OF_LIMBS];
uniform int u_zoom_sign;

uniform vec2 u_resolution;

hp_vec2 get_high_precision_coordinates(vec2 fragCoord){
    number center_x;
    center_x.limb = u_center_x_limb;
    center_x.sign = u_center_x_sign;
    center_x.is_infinite = false;

    number center_y;
    center_y.limb = u_center_y_limb;
    center_y.sign = u_center_y_sign;
    center_y.is_infinite = false;

    number zoom;
    zoom.limb = u_zoom_limb;
    zoom.sign = u_zoom_sign;
    zoom.is_infinite = false;

    vec2 offset = (fragCoord.xy - 0.5 * u_resolution.xy) / u_resolution.y;
    number offset_x_hp = float_to_number(offset.x);
    number offset_y_hp = float_to_number(offset.y);

    number final_x = hp_add(center_x, hp_mult(offset_x_hp, zoom));
    number final_y = hp_add(center_y, hp_mult(offset_y_hp, zoom));

    return initialize_hp_vec2(final_x, final_y);
}

vec3 hsl2rgb(vec3 hsl) {
    vec3 rgb = clamp(abs(mod(hsl.x * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
    return hsl.z + hsl.y * (rgb - 0.5) * (1.0 - abs(2.0 * hsl.z - 1.0));
}

vec3 domain_color(in hp_vec2 z) {
    number angle = hp_atan2(z.y, z.x);
    number hue_hp = hp_div(angle, hp_mult(REAL_TWO, PI));
    number light_hp = hp_mult(TWO_OVER_PI, internal_hp_atan(hp_length(z)));

    float hue = number_to_float(hue_hp);
    float light = number_to_float(light_hp);

    return vec3(hue, 1.0, light);
}

out vec4 FragColor;

void main(){
    hp_vec2 z = get_high_precision_coordinates(gl_FragCoord.xy);
    hp_vec2 func_value;
    #define INJECTION_POINT HERE

    vec3 hsl = domain_color(func_value);
    vec3 rgb = hsl2rgb(hsl);
    FragColor = vec4(rgb, 1.0);

})shdr" ;

inline std::string SRC_HIGH_PRECISION_FUNCTIONS_FRAG = 
    R"shdr(number hp_neg(number a){
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
        uint u_jn2 = (j + n_len >= 2) ? u_halves[j + n_len - 2] : 0u)shdr" R"shdr(;

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
        term = div_uint(term, div)shdr" R"shdr(isor);
        
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
})shdr" ;

inline std::string SRC_HIGH_PRECISION_HEADER_FRAG = 
    R"shdr(#version 300 es

precision highp float;

const int NUMBER_OF_LIMBS = 8;
const int FRACTIONAL_SIZE = NUMBER_OF_LIMBS/2;

struct number{
	uint limb[NUMBER_OF_LIMBS];
	int sign;
    bool is_infinite;
};

struct hp_vec2{
	number x;
	number y;
};

hp_vec2 initialize_hp_vec2(number x, number y){
    hp_vec2 res;
    res.x = x;
    res.y = y;
    return res;
}

number initialize_number(uint limb[NUMBER_OF_LIMBS], int sign, bool is_infinite){
    number res;
    res.limb = limb;
    res.sign = sign;
    res.is_infinite = is_infinite;
    return res;
}

number null_number(){
	number res;
	for(int i = 0; i < NUMBER_OF_LIMBS; ++i){
		res.limb[i] = 0u;
	}
	res.sign = 1;
	return res;
}

number infinite_number(){
    number res;
    for(int i = 0; i < NUMBER_OF_LIMBS; ++i){
        res.limb[i] = (1u<<31u)-1u;
    }
    res.sign = 1;
    res.is_infinite = true;
    return res;
}

number number_one() {
    number res = null_number();
    res.limb[FRACTIONAL_SIZE] = 1u; 
    return res;
})shdr" ;

inline std::string SRC_PICKER_FRAG = 
    R"shdr(#version 300 es
precision highp float;
precision highp int;
precision highp sampler2D;
precision highp usampler2D;
layout (location = 0) out vec4 FragData;

uniform float u_range;
uniform vec2 shift;
uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float time;

#define CONSTANT_DEFINITIONS HERE

#define END_CONSTANT_DEFINITIONS HERE

#define FUNCTION_DEFINITIONS HERE

#define END_FUNCTION_DEFINITIONS HERE

#define INTERPRETER_SPECIFIC_FUNCTIONS HERE

#define END_INTERPRETER_SPECIFIC_FUNCTIONS HERE

void main(){
	vec2 z = (u_mouse/u_resolution.y) * u_range;
	z -= (u_resolution / u_resolution.y) * 0.5 * u_range;
	z += shift;

	vec2 val = run_stack(operator_stack,constant_stack,z);

	FragData = vec4(val.x,val.y,z.x,z.y);	
})shdr" ;

inline std::string SRC_PLOTTER_FRAG = 
    R"shdr(#version 300 es
precision highp float;
precision highp int;
precision highp sampler2D;
precision highp usampler2D;

// Other useful constants


#define CONSTANT_DEFINITIONS HERE

const float PI = 3.141591f;
const float TWO_PI_OVER_3 = 2.0f*PI*0.66666f;
const float TWO_OVER_PI = 2.0f / PI;

const vec2 ZERO = vec2(0.0,0.0);
const vec2 CPI = vec2(PI,0.0f);
const vec2 ONE = vec2(1.0f,0.0f);
const vec2 MINUS_ONE = vec2(-1.0f,0.0f);
const vec2 I = vec2(0.0f,1.0f);

#define END_CONSTANT_DEFINITIONS HERE

in vec2 pos;
out vec4 FragColor;


#define UNIFORM_DECLARATIONS HERE

uniform vec2 u_resolution;
uniform float u_range;
uniform vec2 shift;
uniform float time;
uniform bool show_grid;
uniform bool warp_grid;

#define END_UNIFORM_DECLARATIONS HERE

#define FUNCTION_DEFINITIONS HERE

#define ELEMENTARY_FUNCTION_DEFINITIONS HERE


vec2 cadd(vec2 a, vec2 b){
    return vec2(a.x+b.x,a.y + b.y);
}
vec2 csub(vec2 a, vec2 b){
    return vec2(a.x-b.x,a.y - b.y);;
}

vec2 cmult(vec2 a, vec2 b){
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

vec2 carg(vec2 z){
    return vec2(atan(z.y,z.x),0.0f);
}

vec2 cmag(vec2 z){
    return vec2(length(z),0.0f);
}

vec2 cexp(vec2 z){
    return exp(z.x)*vec2(cos(z.y),sin(z.y));
}

vec2 clog(vec2 z){
    return vec2(log(length(z)), carg(z).x);
}

vec2 cpow(vec2 a, vec2 b){
    return cexp(cmult(b,clog(a)));
}

vec2 cdiv(vec2 a, vec2 b){
    return cmult(a,vec2(b.x,-b.y))/(b.x*b.x+b.y*b.y);
}

vec2 csin(vec2 a){
    return vec2(sin(a.x) * cosh(a.y),cos(a.x) * sinh(a.y));
}

vec2 ccos(vec2 a) {
    return vec2(cos(a.x) * cosh(a.y), -sin(a.x) * sinh(a.y));
}

vec2 csec(vec2 a) {
    float den = cos(2.0f * a.x) + cosh(2.0f * a.y);
    float scale = 2.0f / den;
    return vec2(
        scale * cos(a.x) * cosh(a.y), 
        scale * sin(a.x) * sinh(a.y)
    );
}

vec2 ccsc(vec2 a) {
    float den = cosh(2.0f * a.y) - cos(2.0f * a.x);
    float scale = 2.0f / den;
    return vec2(
        scale * sin(a.x) * cosh(a.y),
        -scale * cos(a.x) * sinh(a.y)
    );
}
vec2 ccot(vec2 a) {
    float den = cosh(2.0f * a.y) - cos(2.0f * a.x);
    return vec2(
        sin(2.0f * a.x) / den,
        -sinh(2.0f * a.y) / den
    );
}


vec2 ctan(vec2 a) {
    float division = cos(2.0f * a.x) + cosh(2.0f * a.y);
    return vec2(sin(2.0f * a.x) / division, sinh(2.0f * a.y) / division);
}


vec2 csinh(vec2 a) {
    return vec2(sinh(a.x) * cos(a.y), cosh(a.x) * sin(a.y));
}
vec2 ccosh(vec2 a) {
    return vec2(cosh(a.x) * cos(a.y), sinh(a.x) * sin(a.y));
}

vec2 ctanh(vec2 a) {
    float division = cosh(2.0f * a.x) + cos(2.0f * a.y);
    return vec2(sinh(2.0f * a.x) / division, sin(2.0f * a.y) / division);
}

vec2 csqrt(vec2 a) {
    return cpow(a,vec2(0.5f,0.0f));
}

vec2 reciprocal(vec2 z){
    return cdiv(ONE,z);
}

// Inverse trigonometric
vec2 casin(vec2 z){
    vec2 iz = vec2(-z.y,z.x);
    vec2 root = csqrt(csub(vec2(1.0f,0.0f),cmult(z,z)));
    return cmult(vec2(0.0f,-1.0f),clog(cadd(iz,root)));
}

vec2 cacos(vec2 z){
    return csub(vec2(PI/2.0f, 0.0f), casin(z));
}

vec2 catan(vec2 z){
    vec2 iz = vec2(-z.y,z.x);
    vec2 term = cdiv(csub(ONE,iz),cadd(ONE,iz));
    return cmult(vec2(0.0f,0.5f),clog(term));
}

vec2 cacsc(vec2 z){
    return casin(reciprocal(z));
}

vec2 casec(vec2 z){
    return cacos(reciprocal(z));
}

vec2 cacot(vec2 z){
    return catan(reciprocal(z));
}

// Reciprocal hyperbolic

vec2 csech(vec2 z){
    return reciprocal(ccosh(z));
}

vec2 ccsch(vec2 z){
    return reciprocal(csinh(z));
}

vec2 ccoth(vec2 z){
    return cdiv(ccosh(z),csinh(z));
}

// Inverse hyperbolic
vec2 casinh(vec2 z){
    vec2 root = csqrt(cadd(cmult(z,z),ONE));
    return clog(cadd(z,root));
}

vec2 cacosh(vec2 z){
    vec2 root = csqrt(csub(cmult(z,z),ONE));
    return clog(cadd(z,root));
}

vec2 catanh(vec2 z){
    vec2 term = cdiv(cadd(ONE,z),csub(ONE,z));
    return cmult(vec2(0.5f,0.0f),clog(term));
}

vec2 cacsch(vec2 z){
    return casinh(reciprocal(z));
}

vec2 casech(vec2 z){
    return cacosh(reciprocal(z));
}

vec2 cacoth(vec2 z){
    return catanh(reciprocal(z));
}

vec2 cneg(vec2 a){
    return vec2(-a.x,-a.y);
}

vec2 cmod(vec2 a, vec2 b){
    return csub(a,cmult(b,floor(cdiv(a,b))));
}

vec2 conj(vec2 z){
    return vec2(z.x,-z.y);
}

vec2 re(vec2 z){
    return vec2(z.x,0.0f);
}

vec2 im(vec2 z){
    return vec2(z.y,0.0f);
}

#define END_ELEMENTARY_FUNCTION_DEFINITIONS HERE


// Non-elementary functions

const int GAMMA_PRECISION = 7;

const float INF = 1.0/0.0;

const float GAMMA_COEFFICIENTS[7] = float[](
    1.000000000190015, 76.18009172947146, -86.50532032941677,
    24.01409824083091, -1.231739572450155, 1.208650973866179e-3, -5.395239384953e-6
);

const vec2 SQUARE_ROOT_TWO_PI = vec2(sqrt(PI * 2.0f),0.0f);

vec2 clngamma(vec2 z) {
    vec2 sum = vec2(GAMMA_COEFFICIENTS[0], 0.0);
    for(int i = 1; i < 7; ++i) {
        sum = cadd(sum, cdiv(vec2(GAMMA_COEFFICIENTS[i], 0.0), cadd(z, vec2(float(i), 0.0))));
    }

    vec2 t = cadd(z, vec2(5.5, 0.0)); 
    vec2 log_term = cmult(cadd(z, vec2(0.5, 0.0)), clog(t));
    const vec2 ln_sqrt_2pi = vec2(0.9189385332, 0.0);
    return cadd(csub(cadd(ln_sqrt_2pi, log_term), t), clog(sum));
}

vec2 cgamma(vec2 z) {   
    if(z.x > 0.5)
        return cexp(clngamma(z));
    vec2 gamma_1_z = cexp(clngamma(ONE-z));
    vec2 denom = cmult(gamma_1_z,csin(cmult(CPI,z)));
    vec2 result = cdiv(CPI,denom);
    if(isnan(result.x)){
        result.x = 0.0;
    }
    if(isnan(result.y)){
        result.y = 0.0;
    }
    return result;
}

const float ZETA_WEIGHTS[10] = float[](
    1.000000, 0.999023, 0.989258, 0.945312, 0.828125, 
    0.623047, 0.376953, 0.171875, 0.054688, 0.010742
);

vec2 czeta_main_branch(vec2 s){
    vec2 sum = vec2(0.0, 0.0);
    float sign = 1.0;
    
    for(int n = 0; n < 10; ++n){
        vec2 n_to_neg_s = cpow(vec2(float(n+1), 0.0), cmult(s, MINUS_ONE));
        
        n_to_neg_s = cmult(n_to_neg_s, vec2(sign * ZETA_WEIGHTS[n], 0.0));
        sum = cadd(sum, n_to_neg_s);
        
        sign = -sign;
    }
    
    vec2 one_minus_s = csub(ONE, s);
    vec2 two_pow = cpow(vec2(2.0, 0.0), one_minus_s);
    
    return cdiv(sum, csub(ONE, two_pow));
}

vec2 czeta_negative_branch(vec2 s){
    vec2 one_minus_s = csub(ONE, s);
    vec2 term1 = cpow(vec2(2.0, 0.0), s);
    vec2 term2 = cpow(vec2(PI, 0.0), csub(s, ONE));
    vec2 term3 = csin(cmult(s, vec2(PI / 2.0, 0.0)));
    vec2 term4 = cgamma(one_minus_s);
    vec2 term5 = czeta_main_branch(one_minus_s);
    return cmult(term1, cmult(term2, cmult(term3, cmult(term4, term5))));

}

vec2 czeta(vec2 s){
    if (s.x >= 0.0) {
        return czeta_main_branch(s);
    } 
    else {
        return czeta_negative_branch(s);
    }
}



#define END_FUNCTION_DEFINITIONS HERE


#define INTERPRETER_SPECIFIC_FUNCTIONS HERE

#define START_WRITING_HERE HERE

uniform usampler2D operator_stack;
uniform sampler2D constant_stack;

vec2 run_stack(in usampler2D operator_stack, in sampler2D constant_stack, in vec2 z);

#define INTERPRETER_DEFINITION HERE

#define END_INTERPRETER_SPECIFIC_FUNCTIONS HERE

#define COLOR_FUNCTIONS HERE

vec3 hsl2rgb(vec3 hsl) {
    vec3 rgb = clamp(abs(mod(hsl.x * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
    return hsl.z + hsl.y * (rgb - 0.5) * (1.0 - abs(2.0 * hsl.z - 1.0));
}

vec3 domain_color(in vec2 z){
    float angle = atan(z.y,z.x);
    float hue = (angle/(2.0 * PI));
    float light = (TWO_OVER_PI) * atan(length(z));
    return vec3(hue,1.0f,light);
}

#define END_COLOR_FUNCTIONS HERE

vec2 convert_coordinates(in vec2 pos, in vec2 resolution, in float range){
    return range * (pos - 0.5f * resolution)/resolution.y;
}




void main(){
    //vec2 func_value = run_stack(operator_stack,constant_stack,pos);
    vec2 z = convert_coordinates(gl_FragCoord.xy,u_resolution,u_range) + shift;
    
    #define INTERPRETER_ASSIGNEMENT HERE
    vec2 func_value = run_stack(operator_stack,constant_stack,z);
    func_value = clamp(func_value, -1e38, 1e38);
    #define END_INTERPRETER_A)shdr" R"shdr(SSIGNEMENT HERE
    
    #define INJECTION_POINT HERE
    
    vec3 hsl = domain_color(func_value);

    if(show_grid){
        vec2 target = z;
        if(warp_grid) target = func_value;
        const float axis_width = 1.5f;
        const float grid_width = 1.0f;
        
        vec2 df = fwidth(target);

        vec2 grid_dist = abs(fract(target+0.5f)-0.5f);
        vec2 grid_px = grid_dist/df;
        float grid_val = min(grid_px.x,grid_px.y);

        vec2 axis_px = abs(target) / df;
        float axis_val = min(axis_px.x, axis_px.y);

        float grid_alpha = 1.0 - smoothstep(0.0, grid_width, grid_val);
        float axis_alpha = 1.0 - smoothstep(0.0, axis_width, axis_val);

        vec3 grid_color = vec3(0.0);

        hsl = mix(hsl, grid_color, grid_alpha * 0.3);
        hsl = mix(hsl, grid_color, axis_alpha * 0.9);
    }

    FragColor = vec4(hsl2rgb(hsl),1.0f);
}
)shdr" ;

inline std::string SRC_PLOTTER_VERT = 
    R"shdr(#version 300 es
precision highp float;
precision highp int;
precision highp sampler2D;
precision highp usampler2D;

out vec2 pos;

void main(){
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    pos = vec2(x,y);
    gl_Position = vec4(x,y,0.0f,1.0f);
})shdr" ;

inline std::string SRC_PLOTTER3D_FRAG = 
    R"shdr(#version 300 es
precision highp float;
precision highp int;
precision highp sampler2D;
precision highp usampler2D;
in vec2 z_val;
in vec2 f_z;
out vec4 FragColor;

uniform bool show_grid;
uniform bool warp_grid;


const float PI = 3.14159265359;
const float TWO_OVER_PI = 2.0 / PI;

vec3 hsl2rgb(vec3 hsl) {
    vec3 rgb = clamp(abs(mod(hsl.x * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
    return hsl.z + hsl.y * (rgb - 0.5) * (1.0 - abs(2.0 * hsl.z - 1.0));
}

vec3 domain_color(in vec2 z){
    float angle = atan(z.y, z.x);
    float hue = (angle / (2.0 * PI));
    float light = (TWO_OVER_PI) * atan(length(z));
    return vec3(hue, 1.0, light);
}

void main(){
    vec3 hsl = domain_color(f_z);
    vec2 z = z_val;
        if(show_grid){
        const float axis_width = 1.5f;
        const float grid_width = 1.0f;
        
        vec2 df = fwidth(z);

        vec2 grid_dist = abs(fract(z+0.5f)-0.5f);
        vec2 grid_px = grid_dist/df;
        float grid_val = min(grid_px.x,grid_px.y);

        vec2 axis_px = abs(z) / df;
        float axis_val = min(axis_px.x, axis_px.y);

        float grid_alpha = 1.0 - smoothstep(0.0, grid_width, grid_val);
        float axis_alpha = 1.0 - smoothstep(0.0, axis_width, axis_val);

        vec3 grid_color = vec3(0.0);

        hsl = mix(hsl, grid_color, grid_alpha * 0.3);
        hsl = mix(hsl, grid_color, axis_alpha * 0.9);
    }
    FragColor = vec4(hsl2rgb(hsl),1.0f);
})shdr" ;

inline std::string SRC_PLOTTER3D_VERT = 
    R"shdr(#version 300 es
precision highp float;
precision highp int;
precision highp sampler2D;
precision highp usampler2D;

#define HERE ;

layout (location=0) in vec3 aPos;

out vec2 f_z;
out vec2 z_val;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 u_resolution;
uniform float u_range;
uniform vec2 shift;
uniform float time;



vec2 run_stack(usampler2D operator_stack, sampler2D constant_stack, vec2 z);

#define CONSTANT_DEFINITIONS HERE

#define END_CONSTANT_DEFINITIONS HERE


#define FUNCTION_DEFINITIONS HERE

#define END_FUNCTION_DEFINITIONS HERE


#define INTERPRETER_SPECIFIC_FUNCTIONS HERE

#define END_INTERPRETER_SPECIFIC_FUNCTIONS HERE

void main(){
	vec2 z = (vec2(aPos.x,aPos.z) * u_range) + shift;

	#define INTERPRETER_ASSIGNEMENT HERE
	vec2 func_value = run_stack(operator_stack,constant_stack,z);
	#define END_INTERPRETER_ASSIGNEMENT HERE

	#define INJECTION_POINT HERE

	float height = length(func_value);

	gl_Position = projection * view * model * vec4(aPos.x,height,aPos.z,1.0f);

	f_z = func_value;
	z_val = z;
}


)shdr" ;

