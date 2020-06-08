#include "codegen.h"
#include <cassert>
#include <iostream>

void gen(const std::unique_ptr<Node> &node) {
	if (Node::node_type::number == node->type) {
		assert(node->child.empty());
		std::cout << "	push " << node->value << "\n";
		return;
	}

	// unary operator
	if (Node::node_type::plus == node->type ||
	    Node::node_type::minus == node->type) {
		assert(node->child.size() == 1);
		gen(node->child.at(0));

		std::cout << "	pop rax\n";
		switch (node->type) {
		case Node::node_type::plus:
			std::cout << "	push rax\n";
			break;
		case Node::node_type::minus:
			std::cout << "	neg rax\n";
			std::cout << "	push rax\n";
			break;
		default:
			std::cerr << "not implemented type(" << static_cast<int>(node->type)
			          << ")" << std::endl;
			std::exit(EXIT_FAILURE);
		}
		return;
	}

	// binary operator.
	assert(node->child.size() == 2);
	gen(node->child.at(0));
	gen(node->child.at(1));

	std::cout << "	pop rdi\n";
	std::cout << "	pop rax\n";

	switch (node->type) {
	case Node::node_type::equal:
		std::cout << "	cmp rax, rdi\n";
		std::cout << "	sete al\n";
		std::cout << "	movzb rax, al\n";
		break;
	case Node::node_type::not_equal:
		std::cout << "	cmp rax, rdi\n";
		std::cout << "	setne al\n";
		std::cout << "	movzb rax, al\n";
		break;
	case Node::node_type::greater_equal:
		std::cout << "	cmp rax, rdi\n";
		std::cout << "	setge al\n";
		std::cout << "	movzb rax, al\n";
		break;
	case Node::node_type::less_equal:
		std::cout << "	cmp rax, rdi\n";
		std::cout << "	setle al\n";
		std::cout << "	movzb rax, al\n";
		break;
	case Node::node_type::greater:
		std::cout << "	cmp rax, rdi\n";
		std::cout << "	setg al\n";
		std::cout << "	movzb rax, al\n";
		break;
	case Node::node_type::less:
		std::cout << "	cmp rax, rdi\n";
		std::cout << "	setl al\n";
		std::cout << "	movzb rax, al\n";
		break;
	case Node::node_type::addition:
		std::cout << "	add rax, rdi\n";
		break;
	case Node::node_type::subtraction:
		std::cout << "	sub rax, rdi\n";
		break;
	case Node::node_type::multiplication:
		std::cout << "	imul rax, rdi\n";
		break;
	case Node::node_type::division:
		std::cout << "	cqo\n";
		std::cout << "	idiv rdi\n";
		break;
	default:
		std::cerr << "not implemented type(" << static_cast<int>(node->type) << ")"
		          << std::endl;
		std::exit(EXIT_FAILURE);
	}

	std::cout << "	push rax\n";
}
