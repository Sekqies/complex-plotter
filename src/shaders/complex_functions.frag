
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
    return vec2(log(length(z)), atan(z.y,z.x));
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
    return csub(a,cmult(b,vector_floor(cdiv(a,b))));
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
