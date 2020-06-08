#include "compiler.h"
#include <cassert>
#include <fstream>
#include <iostream>

void gen(const std::unique_ptr<Node> &node) {
	if (Node::node_type::num == node->type) {
		assert(node->child.empty());
		std::cout << "	push " << node->value << "\n";
		return;
	}

	// otherwise, binary operator.
	assert(node->child.size() == 2);
	gen(node->child.at(0));
	gen(node->child.at(1));

	std::cout << "	pop rdi\n";
	std::cout << "	pop rax\n";

	switch (node->type) {
	case Node::node_type::add:
		std::cout << "	add rax, rdi\n";
		break;
	case Node::node_type::sub:
		std::cout << "	sub rax, rdi\n";
		break;
	case Node::node_type::mul:
		std::cout << "	imul rax, rdi\n";
		break;
	case Node::node_type::div:
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

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "There are not enough arguments.\n";
		return EXIT_FAILURE;
	}

	token_handler token(argv[1]);

	// first half of assembler
	std::cout << ".intel_syntax noprefix\n"
	             ".global main\n"
	             "main:\n";

	auto AST = token.makeAST(); // Abstract Syntax Tree
	gen(AST);

	// set result to rax
	std::cout << "	pop rax\n";

	// main ret
	std::cout << "	ret\n";

	// write out abstract syntax tree
	std::ofstream file("AST.txt");
	file << AST;
	file.close();

	return EXIT_SUCCESS;
}
