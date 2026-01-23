#include <higher_order/derivative.h>

unique_ptr<AstNode> differentiate(AstNode* node, const Operator var) {
	static const std::map<Operator, unique_ptr<AstNode>> deriv_templates = [] {
		std::map<Operator, unique_ptr<AstNode>> m;
		m[Operator::SIN] = build_expression("cos(uplaceholder)");
		m[Operator::COS] = build_expression("-sin(uplaceholder)");
		m[Operator::TAN] = build_expression("sec(uplaceholder)^2");
		m[Operator::LOG] = build_expression("1/uplaceholder");
		
		m[Operator::COSEC] = build_expression("-csc(uplaceholder) * cot(uplaceholder)");

		return m;
		}();
	if (node->op.op == Operator::CONSTANT) {
		return make_constant(0.0f, 0.0f);
	}
	if (node->op.op == var) {
		return make_constant(1.0f, 0.0f);
	}
	if (node->op.arity == Arity::NULLARY) {
		return make_constant(0.0f, 0.0f);
	}

	auto it = deriv_templates.find(node->op.op);
	if (it != deriv_templates.end()) {
		auto d = clone(it->second.get());

		AstNode* arg = node->children[0].get();
		d = substitute(std::move(d), Operator::VARIABLEPLACEHOLDER, *arg);

		auto darg = differentiate(arg, var);
		return make_node(MULT, std::move(d), std::move(darg));
	}

	switch (node->op.op) {
	case ADD: case SUB: {
		auto l = differentiate(node->children[0].get(), var);
		auto r = differentiate(node->children[1].get(), var);
		return make_node(node->op.op, std::move(l), std::move(r));
	}
	case MULT: {
		AstNode* child = node->children[0].get();
		auto f = clone(node->children[0].get());
		auto g = clone(node->children[1].get());

		auto df = differentiate(node->children[0].get(), var);
		auto dg = differentiate(node->children[1].get(), var);

		auto l = make_node(Operator::MULT, std::move(df), std::move(g));
		auto r = make_node(Operator::MULT, std::move(f), std::move(dg));

		return make_node(Operator::ADD, std::move(l), std::move(r));
	}
	case DIV: {
		auto f = clone(node->children[0].get());
		auto g = clone(node->children[1].get());
		auto g2 = clone(node->children[1].get());
		auto g3 = clone(node->children[1].get());

		auto df = differentiate(node->children[0].get(), var);
		auto dg = differentiate(node->children[1].get(), var);

		auto l = make_node(Operator::MULT, std::move(df), std::move(g));
		auto r = make_node(Operator::MULT, std::move(f), std::move(dg));
		auto top = make_node(Operator::SUB, std::move(l), std::move(r));
		auto bot = make_node(Operator::MULT, std::move(g2), std::move(g3));

		return make_node(Operator::DIV, std::move(top), std::move(bot));
	}
	case POW: {
		auto f = clone(node->children[0].get());
		auto f2 = clone(node->children[0].get());
		auto f3 = clone(node->children[0].get());

		auto g = clone(node->children[1].get());
		auto g2 = clone(node->children[1].get());

		auto df = differentiate(node->children[0].get(), var);
		auto df2 = clone(df.get());
		auto dg = differentiate(node->children[1].get(), var);
		auto dg2 = clone(dg.get());

		auto out = make_node(Operator::POW, std::move(f), std::move(g));

		auto top_l = make_node(Operator::MULT, std::move(g2), std::move(df));
		auto l = make_node(Operator::DIV, std::move(top_l), std::move(f2));

		auto log_r = make_node(Operator::LOG, std::move(f3), nullptr);
		auto r = make_node(Operator::MULT, std::move(dg), std::move(log_r));

		auto in = make_node(Operator::ADD, std::move(l), std::move(r));

		return make_node(Operator::MULT, std::move(out), std::move(in));
	}
	}
	const float H = 1e-5;

	auto h_node = make_constant(H, 0.0f);
	auto two_h = make_constant(2 * H, 0.0f);

	auto var_node = make_node(var, nullptr, nullptr);

	auto x_plus_h = make_node(ADD, clone(var_node.get()), clone(h_node.get()));
	auto x_minus_h = make_node(SUB, clone(var_node.get()), clone(h_node.get()));
	auto f_plus = substitute(clone(node), var, *x_plus_h);
	auto f_minus = substitute(clone(node), var, *x_minus_h);
	auto numerator = make_node(SUB, std::move(f_plus), std::move(f_minus));
	return make_node(DIV, std::move(numerator), std::move(two_h));
}

void derivative(unique_ptr<AstNode>& node) {
	for (auto& child : node->children) {
		derivative(child);
	}
	if (node->op.op != Operator::DERIVATIVE) {
		return;
	}
	node = std::move(differentiate(node->children[0].get(), Operator::VARIABLEZ));
}
