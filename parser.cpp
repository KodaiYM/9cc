#include "parser.h"
#include "error.h"
#include <cassert>
#include <cstring>
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
	if (tokenListIsEmpty()) {
		return false;
	}

	if (op == getFrontToken().value) {
		popFrontToken();
		return true;
	} else {
		return false;
	}
}
void Parser::expect(std::string_view op) {
	if (tokenListIsEmpty()) {
		const auto &lastPoppedToken = getLastPoppedToken();
		error("Token '"s + op + "' was expected, but not.", lastPoppedToken.line,
		      lastPoppedToken.line_num,
		      lastPoppedToken.pos + lastPoppedToken.value.size());
	}

	if (!consume(op)) {
		const auto &current_token = getFrontToken();
		error("Token '"s + op + "' was expected, but not.", current_token.line,
		      current_token.line_num, current_token.pos);
	}
}
std::string Parser::expect_number() {
	if (tokenListIsEmpty()) {
		const auto &lastPoppedToken = getLastPoppedToken();
		error("A numeric token was expected, but not.", lastPoppedToken.line,
		      lastPoppedToken.line_num,
		      lastPoppedToken.pos + lastPoppedToken.value.size());
	}

	const auto  current_token = getFrontToken();
	const auto &token         = current_token.value;
	popFrontToken();
	if (!std::all_of(token.begin(), token.end(), isdigit)) {
		error("A numeric token was expected, but not.", current_token.line,
		      current_token.line_num, current_token.pos);
	}

	return token;
}
std::string Parser::expect_identifier() {
	if (tokenListIsEmpty()) {
		const auto &lastPoppedToken = getLastPoppedToken();
		error("An identifier token was expected, but not.", lastPoppedToken.line,
		      lastPoppedToken.line_num,
		      lastPoppedToken.pos + lastPoppedToken.value.size());
	}

	const auto  current_token = getFrontToken();
	const auto &token         = current_token.value;
	popFrontToken();

	if ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_"s.find(
	        token.front()) == std::string::npos ||
	    token.find_first_not_of(
	        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789") !=
	        token.npos) {
		error("An identifier token was expected, but not.", current_token.line,
		      current_token.line_num, current_token.pos);
	}

	return token;
}
std::unique_ptr<Node> Parser::new_node(Node::node_type type,
                                       std::string     value) {
	std::unique_ptr<Node> node(new Node(type));
	node->value = value;
	return node;
}

std::unique_ptr<Node> Parser::program() {
	std::unique_ptr<Node> node(new Node(Node::node_type::statements));

	while (!tokenListIsEmpty()) {
		node->child.push_back(statement());
	}

	return node;
}
std::unique_ptr<Node> Parser::statement() {
	auto node = expression();

	expect(";");

	return node;
}
std::unique_ptr<Node> Parser::expression() {
	return assign();
}
std::unique_ptr<Node> Parser::assign() {
	auto node = equation();

	if (consume("=")) {
		node = new_node(Node::node_type::assign, std::move(node), assign());
	}

	return node;
}
std::unique_ptr<Node> Parser::equation() {
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
		auto node = expression();
		expect(")");
		return node;
	}

	/* identifier? */
	const auto &token = getFrontToken().value;

	if ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_"s.find(
	        token.front()) != std::string::npos &&
	    token.find_first_not_of(
	        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") ==
	        token.npos) {
		return new_node(Node::node_type::identifier, expect_identifier());
	}
	return new_node(Node::node_type::number, expect_number());
}

std::unique_ptr<Node> Parser::makeAST() {
	auto AST = program();

	if (!tokenListIsEmpty()) {
		const auto &extra_token = getFrontToken();
		error("extra character", extra_token.line, extra_token.line_num,
		      extra_token.pos);
	}

	return AST;
}
