#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
using namespace std::string_literals;

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "引数の個数が正しくありません\n";
		return EXIT_FAILURE;
	}

	std::string_view arg = argv[1];

	std::cout << ".intel_syntax noprefix\n"
	          << ".global main\n"
	          << "\n"
	          << "main:\n";

	auto index = arg.find_first_not_of("0123456789");
	if (index == arg.npos) {
		index = arg.length();
	}
	// get first number and remove number from arg
	auto firstNumber = arg.substr(0, index);
	std::cout << "	mov rax, " << firstNumber << "\n";
	arg.remove_prefix(index);

	while (0 < arg.length()) {
		switch (auto op = arg.front()) {
		case '+':
		case '-': {
			arg.remove_prefix(1);
			auto index = arg.find_first_not_of("0123456789");
			if (arg.npos == index) {
				index = arg.length();
			}
			auto number = arg.substr(0, index);
			std::cout << "\t" << (op == '+' ? "add" : "sub") << " rax, " << number
			          << "\n";
			arg.remove_prefix(index);
		} break;
		default:
			std::cerr << "invalid operator!" << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}
	std::cout << "	ret\n";
	return EXIT_SUCCESS;
}
