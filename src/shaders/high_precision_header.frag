#version 300 es

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
}