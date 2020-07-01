#include "print.h"
#include <cassert>
#include <iostream>

std::ostream &operator<<(std::ostream &stream, const Tokenizer &tokenizer) {
	return stream << tokenizer.token_list;
}
std::ostream &operator<<(std::ostream &stream, std::list<Token> token_list) {
	stream << "{";

	stream << std::move(token_list.front().value);
	token_list.pop_front();
	while (!token_list.empty()) {
		stream << ", " << std::move(token_list.front().value);
		token_list.pop_front();
	}

	stream << "}";

	return stream;
}

std::ostream &operator<<(std::ostream &stream, const Node &node) {
	static unsigned int  depth  = 0;
	constexpr const auto indent = "  ";

	if (Node::node_type::identifier == node.type ||
	    Node::node_type::number == node.type) {
		assert(node.child.empty());
		stream << std::string(depth, ' ') << node.value << std::endl;
		return stream;
	}

	// unary operator
	if (Node::node_type::plus == node.type ||
	    Node::node_type::minus == node.type) {
		std::string type;

		switch (node.type) {
		case Node::node_type::plus:
			type = '+';
			break;
		case Node::node_type::minus:
			type = '-';
			break;
		}

		assert(node.child.size() == 1);
		// one child is number
		if (Node::node_type::number == node.child.at(0)->type) {
			assert(node.child.at(0)->child.empty());
			stream << std::string(depth, ' ') << type << node.child.at(0)->value
			       << std::endl;
		} else {
			stream << std::string(depth, ' ') << type << "("
			       << "\n";
			++depth;
			for (const auto &child : node.child) {
				stream << *child;
			}
			--depth;
			stream << std::string(depth, ' ') << ")" << std::endl;
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

		// both hand sides are number
		if (Node::node_type::number == node.child.at(0)->type &&
		    Node::node_type::number == node.child.at(1)->type) {
			assert(node.child.at(0)->child.empty());
			assert(node.child.at(1)->child.empty());
			stream << std::string(depth, ' ') << "(" << type << " "
			       << node.child.at(0)->value << " " << node.child.at(1)->value << ")"
			       << std::endl;
		} else {
			stream << std::string(depth, ' ') << "(" << type << "\n";
			++depth;
			for (const auto &child : node.child) {
				stream << *child;
			}
			--depth;
			stream << std::string(depth, ' ') << ")" << std::endl;
		}
		return stream;
	}

	// statements
	if (Node::node_type::statements == node.type) {
		for (const auto &child : node.child) {
			stream << *child << std::endl;
		}
		return stream;
	}

	std::cerr << "Invalid type(" << static_cast<int>(node.type)
	          << ") detected when print." << std::endl;
	std::exit(EXIT_FAILURE);
}
