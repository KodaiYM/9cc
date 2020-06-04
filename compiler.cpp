#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
using namespace std::string_literals;

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "引数の個数が正しくありません\n";
		return EXIT_FAILURE;
	}

	const std::string_view number = argv[1];
	if (!std::all_of(number.begin(), number.end(), isdigit)) {
		std::cerr << "compilation error!";
		return EXIT_FAILURE;
	}

	std::cout << ".intel_syntax noprefix\n"
	             ".global main\n"
	             "\n"
	             "main:\n"
	          << "	mov rax, "s + argv[1] + "\n"
	          << "	ret\n";
	return EXIT_SUCCESS;
}
