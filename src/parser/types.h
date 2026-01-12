#include <vector>
#include <string>
#include <glm/glm.hpp>
#pragma once
enum Arity {
	UNARY = 0,
	BINARY,
	NULLARY,
	PAREN
};

enum Associativity {
	LEFT = 0,
	RIGHT,
	NONE
};

enum Operator {
	CONSTANT,
	CONSTANTI,
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
	Arity arity;
	Associativity as;
	Operator op;
	std::string str_repr;
	unsigned int precedence;
	glm::vec2 value = { 0.0f,0.0f };
} TokenOperator;

typedef struct AmbiguousOperator {
	TokenOperator op1;
	TokenOperator op2;
	Operator general_rep;
} AmbiguousOperator;

const std::vector<TokenOperator> operators = {
	{Arity::UNARY, Associativity::RIGHT, Operator::NEG, "~", 4},
	{Arity::UNARY, Associativity::RIGHT, Operator::SIN, "sin", 1},
	{Arity::UNARY, Associativity::RIGHT, Operator::COS, "cos", 1},
	{Arity::UNARY, Associativity::RIGHT, Operator::TAN, "tan", 1},
	{Arity::BINARY, Associativity::LEFT, Operator::SUB, "-", 1},
	{Arity::BINARY, Associativity::LEFT, Operator::ADD, "+", 1},
	{Arity::BINARY, Associativity::LEFT, Operator::MULT, "*", 3},
	{Arity::BINARY, Associativity::RIGHT, Operator::POW, "^", 4},
	{Arity::NULLARY, Associativity::NONE, Operator::CONSTANT, "CONSTANT", 0},
	{Arity::NULLARY, Associativity::NONE, Operator::CONSTANTI, "i", 0},
	{Arity::NULLARY, Associativity::NONE, Operator::VARIABLEZ, "z", 0},
	{Arity::NULLARY, Associativity::NONE, Operator::VARIABLEX, "x", 0},
	{Arity::NULLARY, Associativity::NONE, Operator::VARIABLEY, "y", 0},
	{Arity::NULLARY, Associativity::NONE, Operator::VARIABLET, "t", 0},
	{Arity::PAREN, Associativity::NONE, Operator::LPAREN, "(", 0},
	{Arity::PAREN, Associativity::NONE, Operator::RPAREN, ")", 0}
};

const std::vector<AmbiguousOperator> amb_operators = {
	{
		{Arity::UNARY, Associativity::RIGHT, Operator::NEG, "~", 1},
		{Arity::BINARY, Associativity::LEFT, Operator::SUB, "-", 0},
		Operator::SUB
	}
};