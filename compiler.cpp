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
	std::string str;  // token string
	std::size_t line; // token line index
	std::size_t pos;  // token position (byte index)
};

class token_handler {
	friend std::ostream &operator<<(std::ostream &       stream,
	                                const token_handler &token);

private:
	std::queue<Token> token_queue;
	const std::string token_str;

private:
	std::size_t remove_length = 0;
	void        remove_prefix(std::string_view &token_str, std::size_t count) {
    token_str.remove_prefix(count);
    remove_length += count;
	}

public:
	token_handler(std::string_view token_str)
	    : token_str(token_str) {
		const auto token_line = token_str;
		while (token_str.length()) {
			/* remove blanks */
			if (std::isblank(token_str.front())) {
				auto not_space_first_it =
				    std::find_if(token_str.begin(), token_str.end(),
				                 [](char c) { return std::isblank(c) == 0; });
				remove_prefix(token_str,
				              std::distance(token_str.begin(), not_space_first_it));

				/* finish */
				if (!token_str.length()) {
					break;
				}
			}

			/* get front character */
			auto &front = token_str.front();

			/* binary operator +, - */
			if ('+' == front || '-' == front) {
				token_queue.push({Token::token_type::RESERVED, std::string{front}, 0,
				                  remove_length});
				remove_prefix(token_str, 1);
				continue;
			}

			/* number */
			if (isdigit(front)) {
				auto first_notdigit_index = token_str.find_first_not_of("0123456789");
				if (token_str.npos == first_notdigit_index) {
					first_notdigit_index = token_str.length();
				}

				auto num_str = token_str.substr(0, first_notdigit_index);
				token_queue.push({Token::token_type::NUMBER, std::string{num_str}, 0,
				                  remove_length});
				remove_prefix(token_str, first_notdigit_index);
				continue;
			}

			error("Invalid token: "s + front, token_line, remove_length);
		}
	}

private:
	/**
	 * message: error message
	 */
	void error(std::string_view message) const {
		std::cerr << message << std::endl;
		std::exit(EXIT_FAILURE);
	}
	/**
	 * message: error message
	 * line: error line string
	 * pos: error position (byte index)
	 */
	void error(std::string_view message, std::string_view line,
	           std::size_t pos) const {
		std::cerr << line << "\n";
		std::cerr << std::string(pos, ' ') << "^ ";
		std::cerr << message << std::endl;
		std::exit(EXIT_FAILURE);
	}

public:
	bool end_of_token() const {
		return token_queue.empty();
	}

	// if current token is expected type RESERVED, then next token and return
	// true else just return false
	bool consume(std::string_view op) {
		if (token_queue.empty()) {
			throw std::out_of_range("token queue is empty.");
		}
		auto token = token_queue.front();
		if (Token::token_type::RESERVED == token.type && op == token.str) {
			token_queue.pop();
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
		const auto current_token = token_queue.front();
		if (!consume(op)) {
			error("Token '"s + op + "' was expected, but not.", token_str,
			      current_token.pos);
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

		const auto current_token = token_queue.front();
		token_queue.pop();
		if (current_token.type != Token::token_type::NUMBER) {
			error("A numeric token was expected, but not.", token_str,
			      current_token.pos);
		}

		return std::stoi(current_token.str);
	}
};

std::ostream &operator<<(std::ostream &stream, const token_handler &handler) {
	stream << "{";

	auto queue = handler.token_queue;
	stream << std::move(queue.front().str);
	queue.pop();
	while (!queue.empty()) {
		stream << ", " << std::move(queue.front().str);
		queue.pop();
	}

	stream << "}";

	return stream;
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

	// first token is number
	std::cout << "	mov rax, " << token.expect_number() << "\n";

	while (!token.end_of_token()) {
		if (token.consume('+')) {
			std::cout << "	add rax, " << token.expect_number() << "\n";
			continue;
		}

		token.expect('-');
		std::cout << "	sub rax, " << token.expect_number() << "\n";
	}

	// main ret
	std::cout << "	ret\n";

	return EXIT_SUCCESS;
}
