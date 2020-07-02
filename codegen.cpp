#include "codegen.h"
#include <array>
#include <cassert>
#include <iostream>

using namespace std::string_literals;

// 左辺値なら、その左辺値のアドレスをスタックに積む
// それ以外はエラー
static void setup_identifier(const Node &node) {
	if (Node::node_type::identifier != node.type) {
		std::cerr << "代入式の左辺が識別子ではありません" << std::endl;
		std::exit(EXIT_FAILURE);
	}
	static std::vector<std::string> identifier_list;
	identifier_list.reserve(26);
	auto identifier_it =
	    std::find(identifier_list.begin(), identifier_list.end(), node.value);

	// 初めての識別子
	if (identifier_list.end() == identifier_it) {
		identifier_list.push_back(node.value);
		identifier_it = identifier_list.end() - 1;
	}

	std::cout << "	mov rax, rbp\n"
	          << "	sub rax, "
	          << std::distance(identifier_list.begin(), identifier_it) * 8 << "\n"
	          << "	push rax\n";
}
void gen(const Node &node) {
	if (Node::node_type::identifier == node.type) {
		assert(node.child.empty());

		setup_identifier(node);
		std::cout << "	pop rax\n"
		          << "	mov rax, [rax]\n"
		          << "	push rax\n";

		return;
	}
	if (Node::node_type::number == node.type) {
		assert(node.child.empty());

		std::cout << "	push " << node.value << "\n";

		return;
	}

	// call
	if (Node::node_type::call == node.type) {
		assert(node.child.size() <= 6);

		/* 実引数の計算（右から）*/
		for (auto it = node.child.rbegin(), rend = node.child.rend(); rend != it;
		     ++it) {
			gen(**it);
		}

		/* 計算した実引数をレジスタに規定のレジスタに格納（左から順に取り出すことができる）*/
		constexpr std::string_view target_registers[] = {"rdi", "rsi", "rdx",
		                                                 "rcx", "r8",  "r9"};
		for (size_t i = 0; i < node.child.size(); ++i) {
			std::cout << "	pop " << target_registers[i] << "\n";
		}

		// RSPは16の倍数になっているはずである（呼び出し規約）
		std::cout << "	call " << node.value << "\n"
		          << "	push rax\n";
		return;
	}

	// if-else
	if (Node::node_type::ifelse_ == node.type) {
		static uint32_t label_number = 0;
		const auto      elselabel = ".Lifelseelse"s + std::to_string(label_number);
		const auto      endlabel  = ".Lifelseend"s + std::to_string(label_number);

		assert(node.child.size() == 3);

		// 条件式
		gen(*node.child[0]);

		std::cout << "	pop rax\n"    //条件式の結果を取り出し
		          << "	cmp rax, 0\n" // 0と比較して
		          << "	je " << elselabel << "\n"; // 等しければ else節 に飛ぶ
		gen(*node.child[1]);                       // 真の時実行する文
		std::cout << "	jmp " << endlabel << "\n"; // else の後ろに飛ぶ
		std::cout << elselabel << ":"
		          << "\n";   // else節
		gen(*node.child[2]); // 偽の時実行する文
		std::cout << endlabel << ":" << std::endl;

		++label_number;
		return;
	}

	// if
	if (Node::node_type::if_ == node.type) {
		static uint32_t label_number = 0;
		const auto      label        = ".Lifend"s + std::to_string(label_number);

		assert(node.child.size() == 2);

		// 条件式
		gen(*node.child[0]);

		std::cout << "	pop rax\n"              //条件式の結果を取り出し
		          << "	cmp rax, 0\n"           // 0と比較して
		          << "	je " << label << "\n";  // 等しければ label に飛ぶ
		gen(*node.child[1]);                    // 真の時実行する文
		std::cout << label << ":" << std::endl; // 偽の時ここに飛ぶ

		++label_number;
		return;
	}

	// while
	if (Node::node_type::while_ == node.type) {
		static uint32_t label_number = 0;
		const auto      beginlabel = ".Lwhilebegin"s + std::to_string(label_number);
		const auto      endlabel   = ".Lwhileend"s + std::to_string(label_number);

		assert(node.child.size() == 2);

		std::cout << beginlabel << ":"
		          << "\n";

		// 条件式
		gen(*node.child[0]);

		std::cout << "	pop rax\n"    //条件式の結果を取り出し
		          << "	cmp rax, 0\n" // 0と比較して
		          << "	je " << endlabel << "\n"; // 偽なら終了
		gen(*node.child[1]);                      // 真の時実行する文
		std::cout << "	jmp " << beginlabel << "\n";
		std::cout << endlabel << ":" << std::endl; // 偽の時ここに飛ぶ

		++label_number;
		return;
	}

	// for
	if (Node::node_type::for_ == node.type) {
		static uint32_t label_number = 0;
		const auto      beginlabel   = ".Lforbegin"s + std::to_string(label_number);
		const auto      endlabel     = ".Lforend"s + std::to_string(label_number);

		assert(node.child.size() == 4);

		// 初期化式
		gen(*node.child[0]);

		// 繰り返し開始位置
		std::cout << beginlabel << ":"
		          << "\n";

		// 条件式
		gen(*node.child[1]);

		std::cout << "	pop rax\n"    //条件式の結果を取り出し
		          << "	cmp rax, 0\n" // 0と比較して
		          << "	je " << endlabel << "\n"; // 偽なら終了
		gen(*node.child[3]);                      // 真の時実行する文
		gen(*node.child[2]);                      // 終了時処理
		std::cout << "	jmp " << beginlabel << "\n";
		std::cout << endlabel << ":" << std::endl; // 偽の時ここに飛ぶ

		++label_number;
		return;
	}

	// return
	if (Node::node_type::return_ == node.type) {
		gen(*node.child[0]);
		std::cout << "	pop rax\n"
		          << "	mov rsp, rbp\n"
		          << "	pop rbp\n"
		          << "	ret\n";
		return;
	}

	// assign
	if (Node::node_type::assign == node.type) {
		assert(node.child.size() == 2);

		setup_identifier(*node.child[0]);
		gen(*node.child[1]);

		std::cout << "	pop rdi\n"
		          << "	pop rax\n"
		          << "	mov [rax], rdi\n"
		          << "	push rdi\n";
		return;
	}

	// unary operator
	if (Node::node_type::plus == node.type ||
	    Node::node_type::minus == node.type) {
		assert(node.child.size() == 1);
		gen(*node.child[0]);

		std::cout << "	pop rax\n";
		switch (node.type) {
		case Node::node_type::plus:
			std::cout << "	push rax\n";
			break;
		case Node::node_type::minus:
			std::cout << "	neg rax\n";
			std::cout << "	push rax\n";
			break;
		default:
			assert(false);
		}
		return;
	}

	// binary operator
	if (Node::node_type::equal == node.type ||
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
		gen(*node.child[0]);
		gen(*node.child[1]);

		std::cout << "	pop rdi\n";
		std::cout << "	pop rax\n";

		switch (node.type) {
		case Node::node_type::equal:
			std::cout << "	cmp rax, rdi\n";
			std::cout << "	sete al\n";
			std::cout << "	movzb rax, al\n";
			break;
		case Node::node_type::not_equal:
			std::cout << "	cmp rax, rdi\n";
			std::cout << "	setne al\n";
			std::cout << "	movzb rax, al\n";
			break;
		case Node::node_type::greater_equal:
			std::cout << "	cmp rax, rdi\n";
			std::cout << "	setge al\n";
			std::cout << "	movzb rax, al\n";
			break;
		case Node::node_type::less_equal:
			std::cout << "	cmp rax, rdi\n";
			std::cout << "	setle al\n";
			std::cout << "	movzb rax, al\n";
			break;
		case Node::node_type::greater:
			std::cout << "	cmp rax, rdi\n";
			std::cout << "	setg al\n";
			std::cout << "	movzb rax, al\n";
			break;
		case Node::node_type::less:
			std::cout << "	cmp rax, rdi\n";
			std::cout << "	setl al\n";
			std::cout << "	movzb rax, al\n";
			break;
		case Node::node_type::addition:
			std::cout << "	add rax, rdi\n";
			break;
		case Node::node_type::subtraction:
			std::cout << "	sub rax, rdi\n";
			break;
		case Node::node_type::multiplication:
			std::cout << "	imul rax, rdi\n";
			break;
		case Node::node_type::division:
			std::cout << "	cqo\n";
			std::cout << "	idiv rdi\n";
			break;
		default:
			assert(false);
		}
		std::cout << "	push rax\n";
		return;
	}

	// statements
	if (Node::node_type::statements == node.type) {
		for (const auto &child : node.child) {
			gen(*child); // 結果はスタックに入っているので
			std::cout
			    << "	pop rax\n"; // 必要かどうかは分からないが rax に取り出して置く
		}
		return;
	}

	std::cerr << "not implemented type(" << static_cast<int>(node.type)
	          << ") on codegen" << std::endl;
	std::exit(EXIT_FAILURE);
}
