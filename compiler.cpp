#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

using namespace std::string_literals;

namespace std {
/* basic_string + basic_string_view */
template <class charT, class traits, class Allocator>
basic_string<charT, traits, Allocator>
operator+(basic_string_view<charT, traits>              lhs,
          const basic_string<charT, traits, Allocator> &rhs) {
	return std::basic_string<charT, traits, Allocator>(lhs) + rhs;
}
template <class charT, class traits, class Allocator>
basic_string<charT, traits, Allocator>
operator+(basic_string_view<charT, traits>         lhs,
          basic_string<charT, traits, Allocator> &&rhs) {
	return std::basic_string<charT, traits, Allocator>(lhs) + std::move(rhs);
}
template <class charT, class traits, class Allocator>
basic_string<charT, traits, Allocator>
operator+(const basic_string<charT, traits, Allocator> &lhs,
          basic_string_view<charT, traits>              rhs) {
	return lhs + std::basic_string<charT, traits, Allocator>(rhs);
}
template <class charT, class traits, class Allocator>
basic_string<charT, traits, Allocator>
operator+(basic_string<charT, traits, Allocator> &&lhs,
          basic_string_view<charT, traits>         rhs) {
	return std::move(lhs) + std::basic_string<charT, traits, Allocator>(rhs);
}
} // namespace std

struct Token {
	enum class token_type {
		RESERVED, // symbol
		NUMBER    // number
	} type;
	std::string str; // token string
};

class token_handler {
private:
	std::queue<Token> token_queue;

public:
	token_handler(std::string_view token_str) {
		while (token_str.length()) {
			/* remove blanks */
			if (std::isblank(token_str.front())) {
				auto not_space_first_it =
				    std::find_if(token_str.begin(), token_str.end(),
				                 [](char c) { return std::isblank(c) == 0; });
				token_str.remove_prefix(
				    std::distance(token_str.begin(), not_space_first_it));

				/* finish */
				if (0 == token_str.length()) {
					break;
				}
			}

			/* get front character */
			auto &front = token_str.front();

			/* binary operator +, - */
			if ('+' == front || '-' == front) {
				token_str.remove_prefix(1);
				token_queue.push({Token::token_type::RESERVED, std::string{front}});
				continue;
			}

			/* number */
			if (isdigit(front)) {
				auto first_notdigit_index = token_str.find_first_not_of("0123456789");
				if (token_str.npos == first_notdigit_index) {
					first_notdigit_index = token_str.length();
				}

				auto num_str = token_str.substr(0, first_notdigit_index);
				token_str.remove_prefix(first_notdigit_index);
				token_queue.push({Token::token_type::NUMBER, std::string{num_str}});
				continue;
			}

			error("Invalid token: "s + front);
		}
	}

private:
	void error(std::string_view str) const {
		std::cerr << str << std::endl;
		std::exit(EXIT_FAILURE);
	}

public:
	bool end_of_token() const {
		return token_queue.empty();
	}

	// if current token is expected type RESERVED, then next token and return
	// true else return false
	bool consume(std::string_view op) {
		if (token_queue.empty()) {
			throw std::out_of_range("token queue is empty.");
		}
		auto token = token_queue.front();
		token_queue.pop();
		if (Token::token_type::RESERVED == token.type && op == token.str) {
			return true;
		} else {
			return false;
		}
	}
	bool consume(char op) {
		return consume(std::string{op});
	}

	// if current token is expected type RESERVED, then next token
	void expect(std::string_view op) {
		if (!consume(op)) {
			error("Token '"s + op + "' was expected, but not.");
		}
	}
	void expect(char op) {
		expect(std::string{op});
	}

	// if current token is number, then next token and return the number
	// else error
	int expect_number() {
		if (token_queue.empty()) {
			throw std::out_of_range("token queue is empty.");
		}

		auto token = token_queue.front();
		token_queue.pop();
		if (token.type != Token::token_type::NUMBER) {
			error("A numeric token was expected, but not.");
		}

		return std::stoi(token.str);
	}
};

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

	// first token is number
	std::cout << "	mov rax, " << token.expect_number() << "\n";

	while (!token.end_of_token()) {
		if (token.consume('+')) {
			std::cout << "	add rax, " << token.expect_number() << "\n";
		}

		token.expect('-');
		std::cout << "	sub rax, " << token.expect_number() << "\n";
	}

	// main ret
	std::cout << "	ret\n";

	return EXIT_SUCCESS;
}
