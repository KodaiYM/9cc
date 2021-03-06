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
std::unique_ptr<Node> Parser::new_node(Node::node_type    type,
                                       const std::string &value) {
	std::unique_ptr<Node> node(new Node(type));
	node->value = value;
	return node;
}

std::unique_ptr<Node> Parser::program() {
	std::unique_ptr<Node> node(new Node(Node::node_type::statements));

	while (!tokenListIsEmpty()) {
		node->child.push_back(function());
	}

	return node;
}
std::unique_ptr<Node> Parser::function() {
	std::unique_ptr<Node> node(new Node(Node::node_type::function));

	// function name
	node->value = expect_identifier();

	// dummy argument names
	expect("(");
	if (!consume(")")) {
		node->identifier_list.push_back(expect_identifier());
		while (consume(",")) {
			node->identifier_list.push_back(expect_identifier());
		}
		expect(")");
	}

	// function statements
	std::unique_ptr<Node> function_body(new Node(Node::node_type::statements));
	expect("{");
	while (!consume("}")) {
		function_body->child.push_back(statement());
	}
	node->child.push_back(std::move(function_body));

	return node;
}
std::unique_ptr<Node> Parser::statement() {
	std::unique_ptr<Node> node;

	if (consume("{")) {
		node = new_node(Node::node_type::statements);
		// compound statement

		while (!consume("}")) {
			node->child.push_back(statement());
		}
	} else if (consume("return")) {
		node = new_node(Node::node_type::return_, expression());
		expect(";");
	} else if (consume("if")) {
		node = new_node(Node::node_type::if_);

		expect("(");
		node->child.push_back(expression());
		expect(")");
		node->child.push_back(statement());

		// ただの if ではなく if-else の場合
		if (consume("else")) {
			auto ifnode = std::move(node);

			node = new_node(Node::node_type::ifelse_, std::move(ifnode->child.at(0)),
			                std::move(ifnode->child.at(1)), statement());
		}
	} else if (consume("for")) {
		node = new_node(Node::node_type::for_);

		expect("(");

		/* 初期化式 */
		// 式が無ければ、1、あればそれにする
		if (consume(";")) {
			node->child.push_back(new_node(Node::node_type::number, "1"s));
		} else {
			node->child.push_back(expression());
			expect(";");
		}

		/* 条件式 */
		// 式が無ければ、1、あればそれにする
		if (consume(";")) {
			node->child.push_back(new_node(Node::node_type::number, "1"s));
		} else {
			node->child.push_back(expression());
			expect(";");
		}

		/* 変化式 */
		// 式が無ければ、1、あればそれにする
		if (consume(")")) {
			node->child.push_back(new_node(Node::node_type::number, "1"s));
		} else {
			node->child.push_back(expression());
			expect(")");
		}

		// 文
		node->child.push_back(statement());
	} else if (consume("while")) {
		node = new_node(Node::node_type::while_);

		expect("(");

		// 条件式
		node->child.push_back(expression());

		expect(")");

		// 文
		node->child.push_back(statement());
	} else {
		node = expression();
		expect(";");
	}

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

	return address();
}

std::unique_ptr<Node> Parser::address() {
	if (consume("*")) {
		return new_node(Node::node_type::indirection, address());
	}
	if (consume("&")) {
		return new_node(Node::node_type::address, address());
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
	    token.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRST"
	                            "UVWXYZ0123456789_") == token.npos) {
		const auto identifier = expect_identifier();

		if (!consume("(")) {
			// identifier
			return new_node(Node::node_type::identifier, identifier);
		} else {
			// function call
			auto node = new_node(Node::node_type::call, identifier);

			// non-nullary function call
			if (!consume(")")) {
				node->child.push_back(expression());
				while (consume(",")) {
					node->child.push_back(expression());
				}
				expect(")");
			}

			return std::move(node);
		}
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
