#pragma once
#include <string>

inline std::string SRC_HIGH_PRECISION_FRAG = R"(#version 300 es

const int LIMB_SIZE = 32;
const int FRACTIONAL_SIZE = LIMB_SIZE/2;

struct number{
	uint limb[LIMB_SIZE];
	int sign;
    bool is_infinite;
};

struct hp_vec2{
	number x;
	number y;
};

number null_number(){
	number res;
	for(int i = 0; i < LIMB_SIZE; ++i){
		res.limb[i] = 0u;
	}
	res.sign = 1;
	return res;
}

number infinite_number(){
    number res;
    for(int i = 0; i < LIMB_SIZE; ++i){
        res.limb[i] = (1u<<32u)-1u;
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

number neg(number a){
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
    for(int i = LIMB_SIZE - 1; i >= 0; --i) {
        if(a.limb[i] > b.limb[i]) return 1;
        if(a.limb[i] < b.limb[i]) return -1;
    }
    return 0;
}

bool is_zero(number a){
	bool notzero = false;
	for(int i = 0; i < LIMB_SIZE; ++i){
		notzero = a.limb[i] != 0u || notzero;
	}
	return !notzero;
}

uvec2 add_with_carry(uint a, uint b){
	uvec2 res = uvec2(0u,0u);
	res.x = a + b;
	res.y = uint(a > res.x || b > res.x);
	return res;
}
number abs_add(number a, number b){
	number c = null_number();
	uint carry = 0u;
	for(int i = 0; i < LIMB_SIZE; ++i){
		uvec2 res = add_with_carry(add_with_carry(a.limb[i],b.limb[i]).x,carry);
		uint sum = res.x;
		carry = res.y;
		c.limb[i] = sum;
	}
	return c;
}
number abs_sub(number a, number b) {
    number c = null_number();
    uint borrow = 0u;
    for(int i = 0; i < LIMB_SIZE; ++i) {
        uint sub = a.limb[i] - b.limb[i] - borrow;
        borrow = uint((a.limb[i] < b.limb[i] || (a.limb[i] == b.limb[i] && borrow > 0u)));
        c.limb[i] = sub;
    }
    return c;
}

number add(number a, number b){
	number c = null_number();
	if(a.sign == b.sign){
		c = abs_add(a,b);
		c.sign = a.sign;
		return c;
	}
	int cmp = compare_abs(a,b);
	if(cmp >= 0){
		c = abs_sub(a,b);
		c.sign = a.sign;
		return c;
	}
	c = abs_sub(b,a);
	c.sign = b.sign;
	return c;
}



number sub(number a, number b){
	return add(a,neg(b));
}

uvec2 multiply_with_remainder(uint a, uint b) {
    uint low_part = a * b;

    uint al = lo(a);
    uint ah = hi(a);
    uint bl = lo(b);
    uint bh = hi(b);

    uint p0 = al * bl;
    uint p1 = al * bh;
    uint p2 = ah * bl;
    uint p3 = ah * bh;

    uvec2 m_sum = add_with_carry(p1, p2);
    uvec2 m_combined = add_with_carry(m_sum.x, hi(p0));

    uint high_part = p3 + hi(m_combined.x) + ((m_sum.y + m_combined.y) << 16);

    return uvec2(low_part, high_part);
}

number mult(number a, number b) {
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
            
            uvec2 prod = multiply_with_remainder(a.limb[i], b.limb[j]);
            if (target < 0) {
                uvec2 add2 = add_with_carry(prod.x, carry);
                carry = prod.y + add2.y;
            } else {
                uvec2 add1 = add_with_carry(c.limb[target], prod.x);
                uvec2 add2 = add_with_carry(add1.x, carry);
                c.limb[target] = add2.x;
                carry = prod.y + add1.y + add2.y;
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
    uint l = a.limb[index / 2];
    return uint(index % 2 == 0) * hi(l) + uint(index%2==1) * lo(l);
}

void set_half(inout number a, int index, uint val) {
    if (index >= LIMB_SIZE * 2 || index < 0) return;
    int limb_idx = index / 2;
    if (index % 2 == 1) {
        a.limb[limb_idx] = (a.limb[limb_idx] & 0x0000FFFFu) | (val << 16);
    } else {
        a.limb[limb_idx] = (a.limb[limb_idx] & 0xFFFF0000u) | (val & 0xFFFFu);
    }
}

number mult_scalar_16(number a, uint b_16) {
    number c = null_number();
    c.sign = a.sign;
    uint carry = 0u;
    
    for (int i = 0; i < LIMB_SIZE; ++i) {
        if (a.limb[i] == 0u && carry == 0u) continue;
        
        uvec2 prod = multiply_with_remainder(a.limb[i], b_16);
        uvec2 add1 = add_with_carry(prod.x, carry);
        
        c.limb[i] = add1.x;
        carry = prod.y + add1.y;
    }
    return c;
}

number div(number n, number d){
    number q = null_number();
    q.sign = n.sign * d.sign;

    n = shift_left(n, FRACTIONAL_SIZE * 32);

    if(is_zero(d)){
        q = infinite_number();
        return q;
    };
    if(d.is_infinite) return q;

    int msb_d = find_msb(d);
    if(msb_d == -1) return q;

    int msb_n = find_msb(n);
    if(msb_n == -1) return q;

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

            uint u_ji = get_half(u, j + i);
            int diff = int(u_ji) - int(p_lo) - int(borrow);
            
            set_half(u, j + i, uint(diff) & 0xFFFFu);
            borrow = (diff < 0) ? 1u : 0u;
        }
        int final_diff = int(get_half(u, j + n_len)) - int(k) - int(borrow);
        set_half(u, j + n_len, uint(final_diff) & 0xFFFFu);
        if (final_diff < 0) {
            q_hat--;
            uint carry_add = 0u;
            for (int i = 0; i < n_len; ++i) {
                uint sum = get_half(u, j + i) + get_half(v, i) + carry_add;
                set_half(u, j + i, sum & 0xFFFFu);
                carry_add = sum >> 16;
            }
            uint sum_last = get_half(u, j + n_len) + carry_add;
            set_half(u, j + n_len, sum_last & 0xFFFFu);
        }
        set_half(q, j, q_hat);
    }
    return q;
}


)";

inline std::string SRC_PICKER_FRAG = R"(#version 300 es
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
})";

inline std::string SRC_PLOTTER_FRAG = R"(#version 300 es
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

vec2 cadd(vec2 a, vec2 b){
    return a + b;
}
vec2 csub(vec2 a, vec2 b){
    return a - b;
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
    float div = cos(2.0f * a.x) + cosh(2.0f * a.y);
    return vec2(sin(2.0f * a.x) / div, sinh(2.0f * a.y) / div);
}


vec2 csinh(vec2 a) {
    return vec2(sinh(a.x) * cos(a.y), cosh(a.x) * sin(a.y));
}
vec2 ccosh(vec2 a) {
    return vec2(cosh(a.x) * cos(a.y), sinh(a.x) * sin(a.y));
}

vec2 ctanh(vec2 a) {
    float div = cosh(2.0f * a.x) + cos(2.0f * a.y);
    return vec2(sinh(2.0f * a.x) / div, sin(2.0f * a.y) / div);
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
    return -a;
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
    #define END_INTERPRETER_ASSIGNEMENT HERE
    
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
)";

inline std::string SRC_PLOTTER_VERT = R"(#version 300 es
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
})";

inline std::string SRC_PLOTTER3D_FRAG = R"(#version 300 es
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
})";

inline std::string SRC_PLOTTER3D_VERT = R"(#version 300 es
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


)";

