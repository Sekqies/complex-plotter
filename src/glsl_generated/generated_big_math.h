#pragma once
#include <glsl_generated/types.h>// --- GLSL Math implementation for big_vec2 ---
inline big_float length(const big_vec2& v) { return boost::multiprecision::sqrt(v.x*v.x + v.y*v.y); }
inline big_float log(big_float f) { return boost::multiprecision::log(f); }
inline big_float exp(big_float f) { return boost::multiprecision::exp(f); }
inline big_float sin(big_float f) { return boost::multiprecision::sin(f); }
inline big_float cos(big_float f) { return boost::multiprecision::cos(f); }
inline big_float sinh(big_float f) { return boost::multiprecision::sinh(f); }
inline big_float cosh(big_float f) { return boost::multiprecision::cosh(f); }
inline big_float atan(big_float y, big_float x) { return boost::multiprecision::atan2(y, x); }
inline big_vec2 vector_floor(big_vec2 v) { return big_vec2(boost::multiprecision::floor(v.x), boost::multiprecision::floor(v.y)); }

// --- Transpiled GLSL Functions ---

inline big_vec2 cadd(big_vec2 a, big_vec2 b){
    return big_vec2(a.x+b.x,a.y + b.y);
}
inline big_vec2 csub(big_vec2 a, big_vec2 b){
    return big_vec2(a.x-b.x,a.y - b.y);;
}

inline big_vec2 cmult(big_vec2 a, big_vec2 b){
    return big_vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

inline big_vec2 carg(big_vec2 z){
    return big_vec2(atan(z.y,z.x),big_float("0.0"));
}

inline big_vec2 cmag(big_vec2 z){
    return big_vec2(length(z),big_float("0.0"));
}

inline big_vec2 cexp(big_vec2 z){
    return exp(z.x)*big_vec2(cos(z.y),sin(z.y));
}

inline big_vec2 clog(big_vec2 z){
    return big_vec2(log(length(z)), atan(z.y,z.x));
}

inline big_vec2 cpow(big_vec2 a, big_vec2 b){
    return cexp(cmult(b,clog(a)));
}

inline big_vec2 cdiv(big_vec2 a, big_vec2 b){
    return cmult(a,big_vec2(b.x,-b.y))/(b.x*b.x+b.y*b.y);
}

inline big_vec2 csin(big_vec2 a){
    return big_vec2(sin(a.x) * cosh(a.y),cos(a.x) * sinh(a.y));
}

inline big_vec2 ccos(big_vec2 a) {
    return big_vec2(cos(a.x) * cosh(a.y), -sin(a.x) * sinh(a.y));
}

inline big_vec2 csec(big_vec2 a) {
    big_float den = cos(big_float("2.0") * a.x) + cosh(big_float("2.0") * a.y);
    big_float scale = big_float("2.0") / den;
    return big_vec2(
        scale * cos(a.x) * cosh(a.y), 
        scale * sin(a.x) * sinh(a.y)
    );
}

inline big_vec2 ccsc(big_vec2 a) {
    big_float den = cosh(big_float("2.0") * a.y) - cos(big_float("2.0") * a.x);
    big_float scale = big_float("2.0") / den;
    return big_vec2(
        scale * sin(a.x) * cosh(a.y),
        -scale * cos(a.x) * sinh(a.y)
    );
}
inline big_vec2 ccot(big_vec2 a) {
    big_float den = cosh(big_float("2.0") * a.y) - cos(big_float("2.0") * a.x);
    return big_vec2(
        sin(big_float("2.0") * a.x) / den,
        -sinh(big_float("2.0") * a.y) / den
    );
}


inline big_vec2 ctan(big_vec2 a) {
    big_float division = cos(big_float("2.0") * a.x) + cosh(big_float("2.0") * a.y);
    return big_vec2(sin(big_float("2.0") * a.x) / division, sinh(big_float("2.0") * a.y) / division);
}


inline big_vec2 csinh(big_vec2 a) {
    return big_vec2(sinh(a.x) * cos(a.y), cosh(a.x) * sin(a.y));
}
inline big_vec2 ccosh(big_vec2 a) {
    return big_vec2(cosh(a.x) * cos(a.y), sinh(a.x) * sin(a.y));
}

inline big_vec2 ctanh(big_vec2 a) {
    big_float division = cosh(big_float("2.0") * a.x) + cos(big_float("2.0") * a.y);
    return big_vec2(sinh(big_float("2.0") * a.x) / division, sin(big_float("2.0") * a.y) / division);
}

inline big_vec2 csqrt(big_vec2 a) {
    return cpow(a,big_vec2(big_float("0.5"),big_float("0.0")));
}

inline big_vec2 reciprocal(big_vec2 z){
    return cdiv(ONE,z);
}


inline big_vec2 casin(big_vec2 z){
    big_vec2 iz = big_vec2(-z.y,z.x);
    big_vec2 root = csqrt(csub(big_vec2(big_float("1.0"),big_float("0.0")),cmult(z,z)));
    return cmult(big_vec2(big_float("0.0"),-big_float("1.0")),clog(cadd(iz,root)));
}

inline big_vec2 cacos(big_vec2 z){
    return csub(big_vec2(PI/big_float("2.0"), big_float("0.0")), casin(z));
}

inline big_vec2 catan(big_vec2 z){
    big_vec2 iz = big_vec2(-z.y,z.x);
    big_vec2 term = cdiv(csub(ONE,iz),cadd(ONE,iz));
    return cmult(big_vec2(big_float("0.0"),big_float("0.5")),clog(term));
}

inline big_vec2 cacsc(big_vec2 z){
    return casin(reciprocal(z));
}

inline big_vec2 casec(big_vec2 z){
    return cacos(reciprocal(z));
}

inline big_vec2 cacot(big_vec2 z){
    return catan(reciprocal(z));
}



inline big_vec2 csech(big_vec2 z){
    return reciprocal(ccosh(z));
}

inline big_vec2 ccsch(big_vec2 z){
    return reciprocal(csinh(z));
}

inline big_vec2 ccoth(big_vec2 z){
    return cdiv(ccosh(z),csinh(z));
}


inline big_vec2 casinh(big_vec2 z){
    big_vec2 root = csqrt(cadd(cmult(z,z),ONE));
    return clog(cadd(z,root));
}

inline big_vec2 cacosh(big_vec2 z){
    big_vec2 root = csqrt(csub(cmult(z,z),ONE));
    return clog(cadd(z,root));
}

inline big_vec2 catanh(big_vec2 z){
    big_vec2 term = cdiv(cadd(ONE,z),csub(ONE,z));
    return cmult(big_vec2(big_float("0.5"),big_float("0.0")),clog(term));
}

inline big_vec2 cacsch(big_vec2 z){
    return casinh(reciprocal(z));
}

inline big_vec2 casech(big_vec2 z){
    return cacosh(reciprocal(z));
}

inline big_vec2 cacoth(big_vec2 z){
    return catanh(reciprocal(z));
}

inline big_vec2 cneg(big_vec2 a){
    return big_vec2(-a.x,-a.y);
}

inline big_vec2 cmod(big_vec2 a, big_vec2 b){
    return csub(a,cmult(b,vector_floor(cdiv(a,b))));
}

inline big_vec2 conj(big_vec2 z){
    return big_vec2(z.x,-z.y);
}

inline big_vec2 re(big_vec2 z){
    return big_vec2(z.x,big_float("0.0"));
}

inline big_vec2 im(big_vec2 z){
    return big_vec2(z.y,big_float("0.0"));
}

#define END_ELEMENTARY_FUNCTION_DEFINITIONS HERE




 int GAMMA_PRECISION = 7;

 big_float INF = big_float("1.0")/big_float("0.0");

 big_float GAMMA_COEFFICIENTS[7] = { 
    big_float("1.000000000190015"), big_float("76.18009172947146"), -big_float("86.50532032941677"),
    big_float("24.01409824083091"), -big_float("1.231739572450155"), big_float("1.208650973866179e-3"), -big_float("5.395239384953e-6")
 };

 big_vec2 SQUARE_ROOT_TWO_PI = big_vec2(boost::multiprecision::sqrt(PI * big_float("2.0")),big_float("0.0"));

inline big_vec2 clngamma(big_vec2 z) {
    big_vec2 sum = big_vec2(GAMMA_COEFFICIENTS[0], big_float("0.0"));
    for(int i = 1; i < 7; ++i) {
        sum = cadd(sum, cdiv(big_vec2(GAMMA_COEFFICIENTS[i], big_float("0.0")), cadd(z, big_vec2(big_float(i), big_float("0.0")))));
    }

    big_vec2 t = cadd(z, big_vec2(big_float("5.5"), big_float("0.0"))); 
    big_vec2 log_term = cmult(cadd(z, big_vec2(big_float("0.5"), big_float("0.0"))), clog(t));
     big_vec2 ln_sqrt_2pi = big_vec2(big_float("0.9189385332"), big_float("0.0"));
    return cadd(csub(cadd(ln_sqrt_2pi, log_term), t), clog(sum));
}

inline big_vec2 cgamma(big_vec2 z) {   
    if(z.x > big_float("0.5"))
        return cexp(clngamma(z));
    big_vec2 gamma_1_z = cexp(clngamma(ONE-z));
    big_vec2 denom = cmult(gamma_1_z,csin(cmult(CPI,z)));
    big_vec2 result = cdiv(CPI,denom);
    if(boost::math::isnan(result.x)){
        result.x = big_float("0.0");
    }
    if(boost::math::isnan(result.y)){
        result.y = big_float("0.0");
    }
    return result;
}

 big_float ZETA_WEIGHTS[10] = { 
    big_float("1.000000"), big_float("0.999023"), big_float("0.989258"), big_float("0.945312"), big_float("0.828125"), 
    big_float("0.623047"), big_float("0.376953"), big_float("0.171875"), big_float("0.054688"), big_float("0.010742")
 };

inline big_vec2 czeta_main_branch(big_vec2 s){
    big_vec2 sum = big_vec2(big_float("0.0"), big_float("0.0"));
    big_float sign = big_float("1.0");
    
    for(int n = 0; n < 10; ++n){
        big_vec2 n_to_neg_s = cpow(big_vec2(big_float(n+1), big_float("0.0")), cmult(s, MINUS_ONE));
        
        n_to_neg_s = cmult(n_to_neg_s, big_vec2(sign * ZETA_WEIGHTS[n], big_float("0.0")));
        sum = cadd(sum, n_to_neg_s);
        
        sign = -sign;
    }
    
    big_vec2 one_minus_s = csub(ONE, s);
    big_vec2 two_pow = cpow(big_vec2(big_float("2.0"), big_float("0.0")), one_minus_s);
    
    return cdiv(sum, csub(ONE, two_pow));
}

inline big_vec2 czeta_negative_branch(big_vec2 s){
    big_vec2 one_minus_s = csub(ONE, s);
    big_vec2 term1 = cpow(big_vec2(big_float("2.0"), big_float("0.0")), s);
    big_vec2 term2 = cpow(big_vec2(PI, big_float("0.0")), csub(s, ONE));
    big_vec2 term3 = csin(cmult(s, big_vec2(PI / big_float("2.0"), big_float("0.0"))));
    big_vec2 term4 = cgamma(one_minus_s);
    big_vec2 term5 = czeta_main_branch(one_minus_s);
    return cmult(term1, cmult(term2, cmult(term3, cmult(term4, term5))));

}

inline big_vec2 czeta(big_vec2 s){
    if (s.x >= big_float("0.0")) {
        return czeta_main_branch(s);
    } 
    else {
        return czeta_negative_branch(s);
    }
}

