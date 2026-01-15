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

enum Operator{
	NULL_SYMBOL = 0u,
	CONSTANT=1u,
	VARIABLEX,
	VARIABLEY,
	VARIABLEZ,
	VARIABLET,
	ADD,
	SUB,
	NEG,
	MULT,
	DIV,
	EXP,
	POW,
	LPAREN,
	RPAREN,
	COMMA,
	SIN,
	COS,
	TAN
};

typedef struct TokenOperator {
	Arity arity = Arity::NULLARY;
	Associativity as = Associativity::NONE;
	Operator op = Operator::NULL_SYMBOL;
	std::string str_repr = "";
	unsigned int precedence = 0u;
	glm::vec2 value = glm::vec2(0.0f);
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
	{ {Arity::UNARY, Associativity::RIGHT, Operator::SIN, "sin", 1}, 0, "csin", "SIN"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::COS, "cos", 1}, 0, "ccos", "COS"},
	{ {Arity::UNARY, Associativity::RIGHT, Operator::TAN, "tan", 1}, 0, "ctan", "TAN"},
	{ {Arity::BINARY, Associativity::LEFT, Operator::SUB, "-", 1}, 0, "csub", "SUB"},
	{ {Arity::BINARY, Associativity::LEFT, Operator::ADD, "+", 1}, 0, "cadd", "ADD"},
	{ {Arity::BINARY, Associativity::LEFT, Operator::MULT, "*", 3}, 0, "cmult", "MULT"},
	{ {Arity::BINARY, Associativity::LEFT, Operator::DIV, "/", 3}, 0, "cdiv", "DIV"},
	{ {Arity::BINARY, Associativity::RIGHT, Operator::POW, "^", 4}, 0, "cpow", "POW"},
	{ {Arity::NULLARY, Associativity::NONE, Operator::CONSTANT, "CONSTANT", 0, glm::vec2(0.0f)}, 0, "", "CONSTANT"},
	{ {Arity::NULLARY, Associativity::NONE, Operator::CONSTANT, "i", 0, glm::vec2(0,1)}, 0, ""},
	{ {Arity::NULLARY, Associativity::NONE, Operator::VARIABLEZ, "z", 0}, 0, "z", "VARIABLEZ"},
	{ {Arity::NULLARY, Associativity::NONE, Operator::VARIABLEX, "x", 0}, 0, "z.x", "VARIABLEX"},
	{ {Arity::NULLARY, Associativity::NONE, Operator::VARIABLEY, "y", 0}, 0, "z.y", "VARIABLEY"},
	{ {Arity::NULLARY, Associativity::NONE, Operator::VARIABLET, "t", 0}, 0, "time", "VARIABLET"},
	{ {Arity::PAREN, Associativity::NONE, Operator::LPAREN, "(", 0}, 0, "" },
	{ {Arity::PAREN, Associativity::NONE, Operator::RPAREN, ")", 0}, 0, "" },
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