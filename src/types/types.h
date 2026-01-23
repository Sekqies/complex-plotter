#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
enum Arity {
	UNARY = 1,
	BINARY = 2,
	NULLARY = 0,
	PAREN = 3
};

enum Associativity {
	LEFT = 0,
	RIGHT,
	NONE
};

enum Operator {
	NULL_SYMBOL = 0u,
	CONSTANT = 1u,
	CONSTANTH,
	VARIABLEX,
	VARIABLEY,
	VARIABLEZ,
	VARIABLET,
	VARIABLEPLACEHOLDER,
	SECONDVARIABLEPLACEHOLDER,
	ADD,
	SUB,
	NEG,
	MULT,
	DIV,
	EXP,
	LOG,
	POW,
	SQRT,
	RECIPROCAL,
	LPAREN,
	RPAREN,
	COMMA,
	SIN,
	COS,
	TAN,
	COSEC,
	SEC,
	COT,
	ARG,
	MAG,
	ASIN,
	ACOS,
	ATAN,
	ACSC,
	ASEC,
	ACOT,
	SINH,
	COSH,
	TANH,
	CSCH,
	SECH,
	COTH,
	ASINH,
	ACOSH,
	ATANH,
	ACSCH,
	ASECH,
	ACOTH,
	DERIVATIVE
};

typedef struct TokenOperator {
	Arity arity = Arity::NULLARY;
	Associativity as = Associativity::NONE;
	Operator op = Operator::NULL_SYMBOL;
	std::string str_repr = "";
	unsigned int precedence = 0u;
	glm::vec2 value = glm::vec2(0.0f);
	bool higher_order = false;
} TokenOperator;

typedef struct FullOperator {
	TokenOperator token_operator;
	unsigned int interpreter_id;
	std::string gl_name;
	std::string gl_define_name = "";
} FullOperator;

typedef struct AmbiguousOperator {
	TokenOperator op1;
	TokenOperator op2;
	Operator general_rep;
} AmbiguousOperator;

inline std::vector<FullOperator> full_operators = {
	{ {Arity::UNARY, Associativity::RIGHT, Operator::NEG, "~", 4}, 0, "cneg", "NEG"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::MAG, "mag", 4}, 0, "cmag", "MAG"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::ARG, "arg", 4}, 0, "carg", "ARG"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::RECIPROCAL, "reciprocal", 4}, 0, "reciprocal", "RECIPROCAL"},

	// TRIGONOMETRIC
	{ {Arity::UNARY, Associativity::RIGHT, Operator::SIN, "sin", 5}, 0, "csin", "SIN"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::COS, "cos", 5}, 0, "ccos", "COS"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::TAN, "tan", 5}, 0, "ctan", "TAN"},
	// INVERSE TRIGONOMETRIC
	{ {Arity::UNARY, Associativity::RIGHT, Operator::COSEC, "csc", 5}, 0, "ccsc", "CSC"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::SEC, "sec", 5}, 0, "csec", "SEC"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::COT, "cot", 5}, 0, "ccot", "COT"},
	// OPERATORS
	{ {Arity::BINARY, Associativity::LEFT, Operator::SUB, "-", 1}, 0, "csub", "SUB"},
	{ {Arity::BINARY, Associativity::LEFT, Operator::ADD, "+", 1}, 0, "cadd", "ADD"},
	{ {Arity::BINARY, Associativity::LEFT, Operator::MULT, "*", 3}, 0, "cmult", "MULT"},
	{ {Arity::BINARY, Associativity::LEFT, Operator::DIV, "/", 3}, 0, "cdiv", "DIV"},
	{ {Arity::BINARY, Associativity::RIGHT, Operator::POW, "^", 4}, 0, "cpow", "POW"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::SQRT, "sqrt", 5}, 0, "csqrt", "SQRT"},
	// EXPONENTIAL
	{ {Arity::UNARY, Associativity::RIGHT, Operator::LOG, "ln", 4}, 0, "clog", "LOG"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::EXP, "exp", 4}, 0, "cexp", "EXP"},

	// INVERSE TRIGONOMETRIC 
	{ {Arity::UNARY, Associativity::RIGHT, Operator::ASIN, "asin", 5}, 0, "casin", "ASIN"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::ACOS, "acos", 5}, 0, "cacos", "ACOS"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::ATAN, "atan", 5}, 0, "catan", "ATAN"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::ACSC, "acsc", 5}, 0, "cacsc", "ACSC"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::ASEC, "asec", 5}, 0, "casec", "ASEC"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::ACOT, "acot", 5}, 0, "cacot", "ACOT"},

	// HYPERBOLIC
	{ {Arity::UNARY, Associativity::RIGHT, Operator::SINH, "sinh", 5}, 0, "csinh", "SINH"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::COSH, "cosh", 5}, 0, "ccosh", "COSH"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::TANH, "tanh", 5}, 0, "ctanh", "TANH"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::CSCH, "csch", 5}, 0, "ccsch", "CSCH"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::SECH, "sech", 5}, 0, "csech", "SECH"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::COTH, "coth", 5}, 0, "ccoth", "COTH"},

	// INVERSE HYPERBOLIC
	{ {Arity::UNARY, Associativity::RIGHT, Operator::ASINH, "asinh", 5}, 0, "casinh", "ASINH"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::ACOSH, "acosh", 5}, 0, "cacosh", "ACOSH"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::ATANH, "atanh", 5}, 0, "catanh", "ATANH"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::ACSCH, "acsch", 5}, 0, "cacsch", "ACSCH"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::ASECH, "asech", 5}, 0, "casech", "ASECH"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::ACOTH, "acoth", 5}, 0, "cacoth", "ACOTH"},

	//CONSTANTS
	{ {Arity::NULLARY, Associativity::NONE, Operator::CONSTANT, "CONSTANT", 0, glm::vec2(0.0f)}, 0, "", "CONSTANT"},
	{ {Arity::NULLARY, Associativity::NONE, Operator::CONSTANT, "i", 0, glm::vec2(0,1)}, 0, ""},

	//VARIABLES
	{ {Arity::NULLARY, Associativity::NONE, Operator::VARIABLEZ, "z", 0}, 0, "z", "VARIABLEZ"},
	{ {Arity::NULLARY, Associativity::NONE, Operator::VARIABLEX, "x", 0}, 0, "vec2(z.x,0.0f)", "VARIABLEX"},
	{ {Arity::NULLARY, Associativity::NONE, Operator::VARIABLEY, "y", 0}, 0, "vec2(z.y,0.0f)", "VARIABLEY"},
	{ {Arity::NULLARY, Associativity::NONE, Operator::VARIABLET, "t", 0}, 0, "vec2(time,0.0f)", "VARIABLET"},
	
	//PARSER STUFF
	{ {Arity::NULLARY, Associativity::NONE, Operator::VARIABLEPLACEHOLDER, "uplaceholder", 0}, 0, "THIS_SHOULDNT_BE_HERE", "VARIABLEPLACEHOLDER"},
	{ {Arity::NULLARY, Associativity::NONE, Operator::SECONDVARIABLEPLACEHOLDER, "vplaceholder", 0}, 0, "THIS_SHOULDNT_BE_HERE", "SECONDVARIABLEPLACEHOLDER"},
	{ {Arity::PAREN, Associativity::NONE, Operator::LPAREN, "(", 0}, 0, "" },
	{ {Arity::PAREN, Associativity::NONE, Operator::RPAREN, ")", 0}, 0, "" },

	//HIGHER ORDER
	{ {Arity::UNARY, Associativity::RIGHT, Operator::DERIVATIVE, "derivative", 0,glm::vec2(0.0f),true}, 0, ""},


};


inline const std::vector<TokenOperator> operators = []() {
	std::vector<TokenOperator> temp;
	temp.reserve(full_operators.size());
	for (FullOperator& op : full_operators) {
		temp.push_back(op.token_operator);
	}
	return temp;
}();


inline const std::vector<AmbiguousOperator> amb_operators = {
	{
		{Arity::UNARY, Associativity::RIGHT, Operator::NEG, "~", 1},
		{Arity::BINARY, Associativity::LEFT, Operator::SUB, "-", 0},
		Operator::SUB
	}
};