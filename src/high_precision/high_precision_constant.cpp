#include <high_precision/high_precision_constant.h>

number compute_pi() {
    number a = number_one();

    number half = shift_right(number_one(), 1);
    number b = hp_sqrt(half);

    number t = shift_right(number_one(), 2);

    number p = number_one();
    for (int i = 0; i < LIMB_SIZE; ++i) {
        number a_next = shift_right(hp_add(a, b), 1);
        number b_next = hp_sqrt(hp_mult(a, b));

        number a_diff = hp_sub(a, a_next);
        number a_diff_sq = hp_mult(a_diff, a_diff);

        number t_sub = hp_mult(p, a_diff_sq);
        t = hp_sub(t, t_sub);

        p = shift_left(p, 1);
        a = a_next;
        b = b_next;
    }

    number sum = hp_add(a, b);
    number numerator = hp_mult(sum, sum);
    number denominator = shift_left(t, 2);

    return hp_div(numerator, denominator);
}

number compute_e(){
    number sum = number_one();
    number term = number_one();
    for(uint i = 1; i < LIMB_SIZE * 16; ++i){
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

    for(uint i = 3; i < LIMB_SIZE * 32; i+=2){
        term = hp_mult(term, z_sq);
        number iteration_term = div_uint(term, i);
        
        if (is_zero(iteration_term)) break;
        
        sum = hp_add(sum, iteration_term);
    }
    return shift_left(sum, 1); 
}

