#ifndef INCLUDE_GUARD_PARSER_
#define INCLUDE_GUARD_PARSER_

#include "tokenizer.h"
#include <cstddef>
#include <memory>
#include <ostream>
#include <queue>
#include <string>
#include <utility>
#include <vector>

struct Node {
	enum class node_type {
		function,       // function-definition
		call,           // function-call
		ifelse_,        // if-else
		if_,            // if
		for_,           // for
		while_,         // while
		statements,     // compound statements
		empty,          // empty statement
		return_,        // return
		assign,         // =
		equal,          // ==
		not_equal,      // !=
		greater_equal,  // >=
		less_equal,     // <=
		greater,        // >
		less,           // <
		addition,       // binary +
		subtraction,    // binary -
		multiplication, // binary *
		division,       // /
		plus,           // unary +
		minus,          // unary -
		address,        // unary &
		indirection,    // unary *
		number,         // unsigned integer literal
		identifier      // identifier
	};
	node_type type;
	Node(node_type type)
	    : type(type) {}
	Node(Node &&node)
	    : type(std::move(node.type))
	    , value(std::move(node.value)) {
		child.reserve(node.child.size());
		child.insert(child.end(), std::make_move_iterator(node.child.begin()),
		             std::make_move_iterator(node.child.end()));
		node.child.clear();
	}

	std::vector<std::unique_ptr<Node>> child;
	std::vector<std::string>
	            identifier_list; // type = function の時の、識別子リスト
	std::string value;
};

class TokenManager {
private:
	std::list<Token> token_list;
	Token            lastPoppedToken;

public:
	TokenManager(const TokenManager &tokenManager) {
		this->token_list = tokenManager.token_list;
	}
	TokenManager(TokenManager &&tokenManager) noexcept {
		this->token_list = std::move(tokenManager.token_list);
	}
	TokenManager(const std::list<Token> &token_list)
	    : token_list(token_list) {}
	TokenManager(std::list<Token> &&token_list)
	    : token_list(std::move(token_list)) {}

	Token &getFrontToken() {
		return token_list.front();
	}
	void popFrontToken() {
		lastPoppedToken = getFrontToken();
		token_list.pop_front();
	}
	const Token &getLastPoppedToken() {
		return lastPoppedToken;
	}
	bool tokenListIsEmpty() {
		return token_list.empty();
	}
};
class Parser : protected TokenManager {
public:
	Parser(const Tokenizer &tokenizer)
	    : TokenManager(tokenizer.token_list) {}
	Parser(Tokenizer &&tokenizer)
	    : TokenManager(std::move(tokenizer.token_list)) {}
	Parser(const std::list<Token> &token_list)
	    : TokenManager(token_list) {}
	Parser(std::list<Token> &&token_list)
	    : TokenManager(std::move(token_list)) {}

private:
	/* Abstract Syntax Tree*/
private:
	// if current token is expected op, then next token and return
	// true else just return false
	bool consume(std::string_view op);

	// if current token is expected type RESERVED, then next token
	void expect(std::string_view op);

	// if current token is number, then next token and return the number
	// else error
	std::string expect_number();

	// if current token is identifier, then next token and return the identifier
	// else error
	std::string expect_identifier();

	/**
	 * new terminal node
	 */
	std::unique_ptr<Node> new_node(Node::node_type    type,
	                               const std::string &value);

	void set_child_node(const std::unique_ptr<Node> &parent) const {
		// 何もしない
		// std::unique_ptr<Node> new_node(Node::node_type) 用
	}
	void set_child_node(const std::unique_ptr<Node> &parent,
	                    std::unique_ptr<Node>        child) const {
		parent->child.push_back(std::move(child));
	}

	template <typename... Rest>
	void set_child_node(const std::unique_ptr<Node> &parent,
	                    std::unique_ptr<Node> child, Rest... restChild) const {
		parent->child.push_back(std::move(child));
		set_child_node(parent, std::forward<Rest>(restChild)...);
	}

	template <typename... Rest>
	[[nodiscard]] std::unique_ptr<Node> new_node(Node::node_type type,
	                                             Rest... restChild) const {
		std::unique_ptr<Node> node(new Node(type));
		node->type = type;
		set_child_node(node, std::forward<Rest>(restChild)...);
		return node;
	}

	/* make nodes */
	std::unique_ptr<Node> program();
	std::unique_ptr<Node> function();
	std::unique_ptr<Node> statement();
	std::unique_ptr<Node> expression();
	std::unique_ptr<Node> assign();
	std::unique_ptr<Node> equation();
	std::unique_ptr<Node> comparison();
	std::unique_ptr<Node> add();
	std::unique_ptr<Node> mul();
	std::unique_ptr<Node> sign();
	std::unique_ptr<Node> address();
	std::unique_ptr<Node> primary();

public:
	std::unique_ptr<Node> makeAST();
};

#endif
