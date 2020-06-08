#include <cstddef>
#include <memory>
#include <queue>
#include <string>
#include <vector>

struct Token {
	enum class token_type { RESERVED, NUMBER } type;

	std::string str;  // token string
	std::size_t line; // token line index
	std::size_t pos;  // token position (byte index)
};
struct Node {
	enum class node_type {
		add, // +
		sub, // -
		mul, // *
		div, // /
		num  // unsigned integer literal
	};
	node_type type;
	Node(node_type type)
	    : type(type) {}

	std::vector<std::unique_ptr<Node>> child;
	std::string                        value;
};

class token_handler {
	friend std::ostream &operator<<(std::ostream &       stream,
	                                const token_handler &token);

private:
	std::queue<Token> token_queue;
	const std::string token_str;

private:
	std::size_t remove_length = 0;
	void        remove_prefix(std::string_view &token_str, std::size_t count);

public:
	token_handler(std::string_view token_str);

private:
	/**
	 * message: error message
	 */
	void error(std::string_view message) const;
	/**
	 * message: error message
	 * line: error line string
	 * pos: error position (byte index)
	 */
	void error(std::string_view message, std::string_view line,
	           std::size_t pos) const;

	/* Abstract Syntax Tree*/
private:
	bool end_of_token() const;

	// if current token is expected type RESERVED, then next token and return
	// true else just return false
	bool consume(std::string_view op);
	bool consume(char op);

	// if current token is expected type RESERVED, then next token
	void expect(std::string_view op);
	void expect(char op);

	// if current token is number, then next token and return the number
	// else error
	std::string expect_number();
	/**
	 * new terminal node
	 */
	std::unique_ptr<Node> new_node(Node::node_type type, std::string value);

	void set_child_node(std::unique_ptr<Node> &parent,
	                    std::unique_ptr<Node> &child) const {
		parent->child.push_back(std::move(child));
	}

	template <typename... Rest>
	void set_child_node(std::unique_ptr<Node> &parent,
	                    std::unique_ptr<Node> &child, Rest &... restChild) const {
		parent->child.push_back(std::move(child));
		set_child_node(parent, restChild...);
	}

	template <typename... Rest>
	[[nodiscard]] std::unique_ptr<Node> new_node(Node::node_type type,
	                                             Rest... restChild) const {
		std::unique_ptr<Node> node(new Node(type));
		node->type = type;
		set_child_node(node, restChild...);
		return node;
	}

	/* make nodes */
	std::unique_ptr<Node> primary();
	std::unique_ptr<Node> unary();
	std::unique_ptr<Node> mul();
	std::unique_ptr<Node> expr();

public:
	std::unique_ptr<Node> makeAST();
};
std::ostream &operator<<(std::ostream &               stream,
                         const std::unique_ptr<Node> &node);
