#include "codegen.h"
#include <cassert>
#include <iostream>

// 左辺値なら、その左辺値のアドレスをスタックに積む
// それ以外はエラー
static void setup_identifier(const Node &node) {
	if (Node::node_type::identifier != node.type || node.value.size() != 1) {
		std::cerr << "代入式の左辺が識別子ではありません" << std::endl;
		std::exit(EXIT_FAILURE);
	}
	std::cout << "	mov rax, rbp\n"
	          << "	sub rax, " << (node.value.at(0) - 'a') * 8 << "\n"
	          << "	push rax\n";
}
void gen(const Node &node) {
	if (Node::node_type::identifier == node.type) {
		assert(node.child.empty());
		setup_identifier(node);
		std::cout << "	pop rax\n"
		          << "	mov rax, [rax]\n"
		          << "	push rax\n";
		return;
	}
	if (Node::node_type::number == node.type) {
		assert(node.child.empty());
		std::cout << "	push " << node.value << "\n";
		return;
	}

	// assign
	if (Node::node_type::assign == node.type) {
		assert(node.child.size() == 2);

		setup_identifier(*node.child.at(0));
		gen(*node.child.at(1));

		std::cout << "	pop rdi\n"
		          << "	pop rax\n"
		          << "	mov [rax], rdi\n"
		          << "	push rdi\n";
		return;
	}

	// unary operator
	if (Node::node_type::plus == node.type ||
	    Node::node_type::minus == node.type) {
		assert(node.child.size() == 1);
		gen(*node.child.at(0));

		std::cout << "	pop rax\n";
		switch (node.type) {
		case Node::node_type::plus:
			std::cout << "	push rax\n";
			break;
		case Node::node_type::minus:
			std::cout << "	neg rax\n";
			std::cout << "	push rax\n";
			break;
		default:
			assert(false);
		}
		return;
	}

	// binary operator
	if (Node::node_type::equal == node.type ||
	    Node::node_type::not_equal == node.type ||
	    Node::node_type::greater_equal == node.type ||
	    Node::node_type::less_equal == node.type ||
	    Node::node_type::greater == node.type ||
	    Node::node_type::less == node.type ||
	    Node::node_type::addition == node.type ||
	    Node::node_type::subtraction == node.type ||
	    Node::node_type::multiplication == node.type ||
	    Node::node_type::division == node.type) {
		assert(node.child.size() == 2);
		gen(*node.child.at(0));
		gen(*node.child.at(1));

		std::cout << "	pop rdi\n";
		std::cout << "	pop rax\n";

		switch (node.type) {
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
			assert(false);
		}
		std::cout << "	push rax\n";
		return;
	}

	// assign
	if (Node::node_type::assign == node.type) {
		return;
	}

	// statements
	if (Node::node_type::statements == node.type) {
		for (const auto &child : node.child) {
			gen(*child);
			std::cout << "	pop rax\n";
		}
		return;
	}

	std::cerr << "not implemented type(" << static_cast<int>(node.type) << ")"
	          << std::endl;
	std::exit(EXIT_FAILURE);
}
