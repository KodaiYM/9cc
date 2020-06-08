#include "compiler.h"
#include <cassert>
#include <iostream>

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

void token_handler::remove_prefix(std::string_view &token_str,
                                  std::size_t       count) {
	token_str.remove_prefix(count);
	remove_length += count;
}
token_handler::token_handler(std::string_view token_str)
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

		/* RESERVED type */
		if ('+' == front || '-' == front || '*' == front || '/' == front ||
		    '(' == front || ')' == front) {
			token_queue.push(
			    {Token::token_type::RESERVED, std::string{front}, 0, remove_length});
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
			token_queue.push(
			    {Token::token_type::NUMBER, std::string{num_str}, 0, remove_length});
			remove_prefix(token_str, first_notdigit_index);
			continue;
		}

		error("Invalid token: "s + front, token_line, remove_length);
	}
}
void token_handler::error(std::string_view message) const {
	std::cerr << message << std::endl;
	std::exit(EXIT_FAILURE);
}
void token_handler::error(std::string_view message, std::string_view line,
                          std::size_t pos) const {
	std::cerr << line << "\n";
	std::cerr << std::string(pos, ' ') << "^ ";
	std::cerr << message << std::endl;
	std::exit(EXIT_FAILURE);
}
bool token_handler::end_of_token() const {
	return token_queue.empty();
}
bool token_handler::consume(std::string_view op) {
	if (token_queue.empty()) {
		return false;
	}
	auto token = token_queue.front();
	if (Token::token_type::RESERVED == token.type && op == token.str) {
		token_queue.pop();
		return true;
	} else {
		return false;
	}
}
bool token_handler::consume(char op) {
	return consume(std::string{op});
}
void token_handler::expect(std::string_view op) {
	const auto current_token = token_queue.front();
	if (!consume(op)) {
		error("Token '"s + op + "' was expected, but not.", token_str,
		      current_token.pos);
	}
}
void token_handler::expect(char op) {
	expect(std::string{op});
}
std::string token_handler::expect_number() {
	if (token_queue.empty()) {
		throw std::out_of_range("token queue is empty.");
	}

	const auto current_token = token_queue.front();
	token_queue.pop();
	if (current_token.type != Token::token_type::NUMBER) {
		error("A numeric token was expected, but not.", token_str,
		      current_token.pos);
	}

	return current_token.str;
}
std::unique_ptr<Node> token_handler::new_node(Node::node_type type,
                                              std::string     value) {
	switch (type) {
	case Node::node_type::num: {
		std::unique_ptr<Node> node(new Node(type));
		node->value = value;
		return node;
	} break;
	default:
		/* non terminal type */
		throw std::invalid_argument("Node::node_type = "s +
		                            std::to_string(static_cast<int>(type)) +
		                            " is not a terminal node.");
		break;
	}
}

std::unique_ptr<Node> token_handler::primary() {
	if (consume('(')) {
		auto node = expr();
		expect(')');
		return node;
	}

	return new_node(Node::node_type::num, expect_number());
}

std::unique_ptr<Node> token_handler::mul() {
	auto node = primary();

	while (1) {
		if (consume('*')) {
			node = new_node(Node::node_type::mul, std::move(node), primary());
		} else if (consume('/')) {
			node = new_node(Node::node_type::div, std::move(node), primary());
		} else {
			return node;
		}
	}
}

std::unique_ptr<Node> token_handler::expr() {
	auto node = mul();

	while (1) {
		if (consume('+')) {
			node = new_node(Node::node_type::add, std::move(node), mul());
		} else if (consume('-')) {
			node = new_node(Node::node_type::sub, std::move(node), mul());
		} else {
			return node;
		}
	}
}

std::unique_ptr<Node> token_handler::makeAST() {
	auto AST = expr();
	if (!token_queue.empty()) {
		error("extra character", token_str, token_queue.front().pos);
	}
	return AST;
}

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

std::ostream &operator<<(std::ostream &               stream,
                         const std::unique_ptr<Node> &node) {
	static unsigned int  depth  = 0;
	constexpr const auto indent = "  ";

	if (Node::node_type::num == node->type) {
		assert(node->child.empty());
		stream << std::string(depth, ' ') << node->value << std::endl;
		return stream;
	} else {
		std::string type;
		switch (node->type) {
		case Node::node_type::add:
			type = '+';
			break;
		case Node::node_type::sub:
			type = '-';
			break;
		case Node::node_type::mul:
			type = '*';
			break;
		case Node::node_type::div:
			type = '/';
			break;
		default:
			std::cerr << "Invalid type(" << static_cast<int>(node->type)
			          << ") detected." << std::endl;
			std::exit(EXIT_FAILURE);
		}

		assert(node->child.size() == 2);
		// both hand sides are number
		if (Node::node_type::num == node->child.at(0)->type &&
		    Node::node_type::num == node->child.at(1)->type) {
			assert(node->child.at(0)->child.empty());
			assert(node->child.at(1)->child.empty());
			stream << std::string(depth, ' ') << "(" << type << " "
			       << node->child.at(0)->value << " " << node->child.at(1)->value
			       << ")" << std::endl;
		} else {
			stream << std::string(depth, ' ') << "(" << type << "\n";
			++depth;
			for (const auto &child : node->child) {
				stream << child;
			}
			--depth;
			stream << std::string(depth, ' ') << ")" << std::endl;
		}
		return stream;
	}
}
