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
		equal,          // ==
		not_equal,      // !=
		greater_equal,  // >=
		less_equal,     // <=
		greater,        // >
		less,           // <
		addition,       // binary +
		subtraction,    // binary -
		multiplication, // *
		division,       // /
		plus,           // unary +
		minus,          // unary -
		number          // unsigned integer literal
	};
	node_type type;
	Node(node_type type)
	    : type(type) {}

	std::vector<std::unique_ptr<Node>> child;
	std::string                        value;
};

class Parser {
public:
	Parser(const Tokenizer &tokenizer) {
		this->token_list = tokenizer.token_list;
	}
	Parser(Tokenizer &&tokenizer) {
		this->token_list = std::move(tokenizer.token_list);
	}
	Parser(std::list<Token> token_list)
	    : token_list(std::move(token_list)) {}
	Parser(std::list<Token> &&token_list)
	    : token_list(std::move(token_list)) {}

private:
	std::list<Token> token_list;

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
	/**
	 * new terminal node
	 */
	std::unique_ptr<Node> new_node(Node::node_type type, std::string value);

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
	std::unique_ptr<Node> expression();
	std::unique_ptr<Node> equality();
	std::unique_ptr<Node> comparison();
	std::unique_ptr<Node> add();
	std::unique_ptr<Node> mul();
	std::unique_ptr<Node> sign();
	std::unique_ptr<Node> primary();

public:
	std::unique_ptr<Node> makeAST();
};

#endif
