#include "codegen.h"
#include "parser.h"
#include "print.h"
#include "tokenizer.h"
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "There are not enough arguments.\n";
		return EXIT_FAILURE;
	}

	Tokenizer     tokenizer(argv[1]);
	Parser        parser(tokenizer);
	std::ofstream token_file(".token.txt");
	token_file << tokenizer;
	token_file.close();

	// first half of assembler
	std::cout << ".intel_syntax noprefix\n"
	             ".global main\n";

	const auto AST = parser.makeAST(); // Abstract Syntax Tree
	// write out abstract syntax tree
	std::ofstream tree_file(".AST.txt");
	tree_file << *AST;
	tree_file.close();

	// calculate whole node
	gen(*AST);

	return EXIT_SUCCESS;
}
