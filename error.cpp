#include "error.h"
#include <iostream>

void error(std::string_view message) {
	std::cerr << message << std::endl;
	std::exit(EXIT_FAILURE);
}
void error(std::string_view message, std::string_view line, std::size_t pos) {
	std::cerr << line << "\n";
	std::cerr << std::string(pos, ' ') << "^ ";
	std::cerr << message << std::endl;
	std::exit(EXIT_FAILURE);
}
