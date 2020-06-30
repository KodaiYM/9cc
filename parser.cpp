#include "parser.h"
#include "error.h"
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

bool Parser::consume(std::string_view op) {
	if (token_list.empty()) {
		return false;
	}
	auto token = token_list.front();
	if (op == token.value) {
		token_list.pop_front();
		return true;
	} else {
		return false;
	}
}
void Parser::expect(std::string_view op) {
	const auto current_token = token_list.front();
	if (!consume(op)) {
		error("Token '"s + op + "' was expected, but not.", current_token.line,
		      current_token.line_num, current_token.pos);
	}
}
std::string Parser::expect_number() {
	if (token_list.empty()) {
		throw std::out_of_range("token queue is empty.");
	}

	const auto  current_token = token_list.front();
	const auto &token         = current_token.value;
	token_list.pop_front();
	if (!std::all_of(token.begin(), token.end(), isdigit)) {
		error("A numeric token was expected, but not.", current_token.line,
		      current_token.line_num, current_token.pos);
	}

	return token;
}
std::unique_ptr<Node> Parser::new_node(Node::node_type type,
                                       std::string     value) {
	switch (type) {
	case Node::node_type::number: {
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

std::unique_ptr<Node> Parser::expression() {
	return equality();
}
std::unique_ptr<Node> Parser::equality() {
	auto node = comparison();

	while (1) {
		if (consume("==")) {
			node = new_node(Node::node_type::equal, std::move(node), comparison());
		} else if (consume("!=")) {
			node =
			    new_node(Node::node_type::not_equal, std::move(node), comparison());
		} else {
			return node;
		}
	}
}
std::unique_ptr<Node> Parser::comparison() {
	auto node = add();

	while (1) {
		if (consume(">=")) {
			node = new_node(Node::node_type::greater_equal, std::move(node), add());
		} else if (consume("<=")) {
			node = new_node(Node::node_type::less_equal, std::move(node), add());
		} else if (consume(">")) {
			node = new_node(Node::node_type::greater, std::move(node), add());
		} else if (consume("<")) {
			node = new_node(Node::node_type::less, std::move(node), add());
		} else {
			return node;
		}
	}
}
std::unique_ptr<Node> Parser::add() {
	auto node = mul();

	while (1) {
		if (consume("+")) {
			node = new_node(Node::node_type::addition, std::move(node), mul());
		} else if (consume("-")) {
			node = new_node(Node::node_type::subtraction, std::move(node), mul());
		} else {
			return node;
		}
	}
}
std::unique_ptr<Node> Parser::mul() {
	auto node = sign();

	while (1) {
		if (consume("*")) {
			node = new_node(Node::node_type::multiplication, std::move(node), sign());
		} else if (consume("/")) {
			node = new_node(Node::node_type::division, std::move(node), sign());
		} else {
			return node;
		}
	}
}
std::unique_ptr<Node> Parser::sign() {
	if (consume("+")) {
		return new_node(Node::node_type::plus, sign());
	}
	if (consume("-")) {
		return new_node(Node::node_type::minus, sign());
	}

	return primary();
}
std::unique_ptr<Node> Parser::primary() {
	if (consume("(")) {
		auto node = equality();
		expect(")");
		return node;
	}

	return new_node(Node::node_type::number, expect_number());
}

std::unique_ptr<Node> Parser::makeAST() {
	auto AST = expression();
	if (!token_list.empty()) {
		const auto &extra_token = token_list.front();
		error("extra character", extra_token.line, extra_token.line_num,
		      extra_token.pos);
	}
	return AST;
}
