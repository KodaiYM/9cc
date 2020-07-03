#include "print.h"
#include <cassert>
#include <iostream>

std::ostream &operator<<(std::ostream &stream, const Tokenizer &tokenizer) {
	return stream << tokenizer.token_list;
}
std::ostream &operator<<(std::ostream &stream, std::list<Token> token_list) {
	while (!token_list.empty()) {
		stream << std::move(token_list.front().value) << "\n";
		token_list.pop_front();
	}

	return stream;
}

// 終端ノードかどうか
static constexpr bool terminationNode(const Node &node) {
	return Node::node_type::identifier == node.type ||
	       Node::node_type::number == node.type;
}
std::ostream &operator<<(std::ostream &stream, const Node &node) {
	static unsigned int depth = 0;

	if (terminationNode(node)) {
		assert(node.child.empty());
		stream << std::string(2 * depth, ' ') << node.value << std::endl;
		return stream;
	}

	// function-definition
	if (Node::node_type::function == node.type) {
		assert(node.child.size() == 1);
		assert(node.child[0]->type == Node::node_type::statements);

		stream << node.value << "(";
		for (auto begin    = node.identifier_list.begin(),
		          ident_it = node.identifier_list.begin(),
		          end      = node.identifier_list.end();
		     end != ident_it; ++ident_it) {
			if (begin != ident_it) {
				stream << ", ";
			}
			stream << *ident_it;
		}
		stream << ") {"
		       << "\n";
		++depth;
		stream << *node.child[0];
		--depth;
		stream << "}" << std::endl;

		return stream;
	}

	// function-call
	if (Node::node_type::call == node.type) {
		stream << std::string(2 * depth, ' ') << node.value << "("
		       << "\n";
		auto child_it = node.child.begin();
		while (node.child.end() != child_it) {
			if (node.child.begin() != child_it) {
				stream << std::string(2 * depth, ' ') << ","
				       << "\n";
			}

			++depth;
			stream << **child_it;
			--depth;

			++child_it;
		}
		stream << std::string(2 * depth, ' ') << ")"
		       << "\n";
		return stream;
	}

	// if-else
	if (Node::node_type::ifelse_ == node.type) {
		assert(node.child.size() == 3);

		/* if */
		stream << std::string(2 * depth, ' ') << "if ("
		       << "\n";
		++depth;
		stream << *node.child[0]; // 条件式
		--depth;
		stream << std::string(2 * depth, ' ') << ") {"
		       << "\n";
		++depth;
		stream << *node.child[1]; // 文
		--depth;
		stream << "} else {"
		       << "\n";

		/* else */
		++depth;
		stream << *node.child[2]; // 文
		--depth;
		stream << "}" << std::endl;

		return stream;
	}

	// if
	if (Node::node_type::if_ == node.type) {
		assert(node.child.size() == 2);

		stream << std::string(2 * depth, ' ') << "if ("
		       << "\n";
		++depth;
		stream << *node.child[0];
		--depth;
		stream << std::string(2 * depth, ' ') << ") {"
		       << "\n";
		++depth;
		stream << *node.child[1];
		--depth;
		stream << "}" << std::endl;

		return stream;
	}

	// while
	if (Node::node_type::while_ == node.type) {
		assert(node.child.size() == 2);

		stream << std::string(2 * depth, ' ') << "while ("
		       << "\n";
		/* 条件式 */
		++depth;
		stream << *node.child[0];
		--depth;
		stream << std::string(2 * depth, ' ') << ") {"
		       << "\n";
		/* 文 */
		++depth;
		stream << *node.child[1];
		--depth;
		stream << std::string(2 * depth, ' ') << "}" << std::endl;

		return stream;
	}

	// for
	if (Node::node_type::for_ == node.type) {
		assert(node.child.size() == 4);

		stream << std::string(2 * depth, ' ') << "for ("
		       << "\n"
		       << *node.child[0] << "; "
		       << "\n"
		       << *node.child[1] << "; "
		       << "\n"
		       << *node.child[2] << ") {"
		       << "\n";
		++depth;
		stream << *node.child[3];
		--depth;
		stream << std::string(2 * depth, ' ') << "}";

		return stream;
	}

	// return
	if (Node::node_type::return_ == node.type) {
		assert(node.child.size() == 1);

		// only one child is termination node
		if (terminationNode(*node.child[0])) {
			assert(node.child[0]->child.empty());
			stream << std::string(2 * depth, ' ') << "return " << node.child[0]->value
			       << ";" << std::endl;
		} else {
			stream << std::string(2 * depth, ' ') << "return ("
			       << "\n";
			++depth;
			for (const auto &child : node.child) {
				stream << *child;
			}
			--depth;
			stream << std::string(2 * depth, ' ') << ");" << std::endl;
		}

		return stream;
	}

	// unary operator
	if (Node::node_type::plus == node.type ||
	    Node::node_type::minus == node.type ||
	    Node::node_type::address == node.type ||
	    Node::node_type::indirection == node.type) {
		std::string type;

		switch (node.type) {
		case Node::node_type::plus:
			type = '+';
			break;
		case Node::node_type::minus:
			type = '-';
			break;
		case Node::node_type::address:
			type = '&';
			break;
		case Node::node_type::indirection:
			type = '*';
			break;
		}

		assert(node.child.size() == 1);
		// one child is termination node
		if (terminationNode(*node.child[0])) {
			assert(node.child[0]->child.empty());
			stream << std::string(2 * depth, ' ') << type << node.child[0]->value
			       << std::endl;
		} else {
			stream << std::string(2 * depth, ' ') << type << "("
			       << "\n";
			++depth;
			for (const auto &child : node.child) {
				stream << *child;
			}
			--depth;
			stream << std::string(2 * depth, ' ') << ")" << std::endl;
		}

		return stream;
	}

	// binary operator
	if (Node::node_type::assign == node.type ||
	    Node::node_type::equal == node.type ||
	    Node::node_type::not_equal == node.type ||
	    Node::node_type::greater_equal == node.type ||
	    Node::node_type::less_equal == node.type ||
	    Node::node_type::greater == node.type ||
	    Node::node_type::less == node.type ||
	    Node::node_type::addition == node.type ||
	    Node::node_type::subtraction == node.type ||
	    Node::node_type::multiplication == node.type ||
	    Node::node_type::division == node.type) {
		assert(node.child.size() == 2);

		std::string type;
		switch (node.type) {
		case Node::node_type::assign:
			type = "=";
			break;
		case Node::node_type::equal:
			type = "==";
			break;
		case Node::node_type::not_equal:
			type = "==";
			break;
		case Node::node_type::greater_equal:
			type = ">=";
			break;
		case Node::node_type::less_equal:
			type = "<=";
			break;
		case Node::node_type::greater:
			type = '>';
			break;
		case Node::node_type::less:
			type = '<';
			break;
		case Node::node_type::addition:
			type = '+';
			break;
		case Node::node_type::subtraction:
			type = '-';
			break;
		case Node::node_type::multiplication:
			type = '*';
			break;
		case Node::node_type::division:
			type = '/';
			break;
		default:
			assert(false);
		}

		// both hand sides are termination node
		if (terminationNode(*node.child[0]) && terminationNode(*node.child[1])) {
			assert(node.child[0]->child.empty());
			assert(node.child[1]->child.empty());
			stream << std::string(2 * depth, ' ') << "(" << type << " "
			       << node.child[0]->value << " " << node.child[1]->value << ")"
			       << std::endl;
		} else {
			stream << std::string(2 * depth, ' ') << "(" << type << "\n";
			++depth;
			for (const auto &child : node.child) {
				stream << *child;
			}
			--depth;
			stream << std::string(2 * depth, ' ') << ")" << std::endl;
		}

		return stream;
	}

	// statements
	if (Node::node_type::statements == node.type) {
		for (const auto &child : node.child) {
			stream << *child;
		}

		return stream;
	}

	std::cerr << "Invalid type(" << static_cast<int>(node.type)
	          << ") detected when print." << std::endl;
	std::exit(EXIT_FAILURE);
}
