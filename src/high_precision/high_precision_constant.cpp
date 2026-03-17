#include <high_precision/high_precision_constant.h>

number compute_pi() {
    number C = number_integer(640320);
    number C3 = hp_mult(hp_mult(C, C), C);

    number sum = null_number();
    number term = number_one();

    number k_num = number_integer(13591409);
    number k_mul = number_integer(545140134);

    number M = number_one();
    number L = k_num;
    number X = number_one();

    number K = number_integer(6);

    for (int k = 0; k < NUMBER_OF_LIMBS / 2; ++k) {

        number term_part = hp_mult(M, L);
        term_part = hp_div(term_part, X);

        if (k % 2 == 1) {
            term_part = hp_neg(term_part);
        }

        sum = hp_add(sum, term_part);

        number k1 = number_integer(k + 1);

        number a = hp_mult(K, hp_sub(K, number_integer(1)));
        number b = hp_mult(K, hp_sub(K, number_integer(2)));

        M = hp_mult(M, hp_mult(a, b));
        M = hp_div(M, hp_mult(k1, hp_mult(k1, k1)));

        L = hp_add(L, k_mul);

        X = hp_mult(X, C3);

        K = hp_add(K, number_integer(12));
    }

    number sqrt_term = hp_sqrt(number_integer(10005));
    number factor = hp_mult(number_integer(426880), sqrt_term);

    return hp_div(factor, sum);
}

number compute_e(){
    number sum = number_one();
    number term = number_one();
    for(uint i = 1; i < NUMBER_OF_LIMBS * 16; ++i){
        term = div_uint(term,i);

        if(is_zero(term)) break;

        sum = hp_add(sum,term);
    }
    return sum;
}

number compute_ln2(){
    number one = number_one();

    number three = hp_add(one,hp_add(one,one));
    number z = hp_div(one, three);
    number z_sq = hp_mult(z, z);

    number sum = z;
    number term = z;

    for(uint i = 3; i < NUMBER_OF_LIMBS * 32; i+=2){
        term = hp_mult(term, z_sq);
        number iteration_term = div_uint(term, i);
        
        if (is_zero(iteration_term)) break;
        
        sum = hp_add(sum, iteration_term);
    }
    return shift_left(sum, 1); 
}