#include "codegen.h"
#include <cassert>
#include <forward_list>
#include <iostream>

using namespace std::string_literals;

static std::forward_list<std::vector<std::string>> identifier_list;

/**
 * ブロックの追加
 */
static void push_block() {
	identifier_list.emplace_front();
}

/**
 * ブロックの削除
 */
static void pop_block() {
	identifier_list.pop_front();
}

/**
 * identifier: 識別子
 * identifier を 最新のブロック に所属する識別子として登録する
 */
static void register_identifier(const std::string &identifier) {
	assert(!identifier_list.empty());

	// 初めての識別子
	if (auto &front_block = identifier_list.front();
	    std::find(front_block.begin(), front_block.end(), identifier) ==
	    front_block.end()) {
		front_block.push_back(identifier);
	}
}
/**
 * identifier: 識別子
 * identifier を 最新のブロック に所属する識別子として登録する
 * 最新のブロックから identifier を検索してスタックにそのアドレスを返却する
 */
static void setup_identifier(const std::string &identifier) {
	assert(!identifier_list.empty());

	auto &front_block = identifier_list.front();
	if (auto identifier_it =
	        std::find(front_block.cbegin(), front_block.cend(), identifier);
	    identifier_it == front_block.end()) {
		std::cerr << "識別子が見つかりませんでした" << std::endl;
		std::exit(EXIT_FAILURE);
	} else {
		std::cout << "	mov rax, rbp\n"
		          << "	sub rax, "
		          << (std::distance(front_block.cbegin(), identifier_it) + 1) * 8
		          << "\n"
		          << "	push rax\n";
	}
}

void gen(const Node &node) {
	if (Node::node_type::identifier == node.type) {
		assert(node.child.empty());

		setup_identifier(node.value);
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

	// 引数に対応するレジスタ
	constexpr const char *target_registers[] = {"rdi", "rsi", "rdx",
	                                            "rcx", "r8",  "r9"};

	// function-definition
	if (Node::node_type::function == node.type) {
		assert(node.child.size() == 1);

		std::cout << node.value << ":"
		          << "\n";

		push_block();

		/* 仮引数の識別子を登録 */
		for (const auto &dummy_argument_name : node.identifier_list) {
			register_identifier(dummy_argument_name);
		}

		/* あるノード下の識別子の数を数えつつ、識別子の登録もする */
		constexpr auto funcHowManyIdentifiers = [](auto &&     func,
		                                           const Node &node) -> size_t {
			if (Node::node_type::identifier == node.type) {
				register_identifier(node.value);
				return 1;
			}

			size_t howManyIdentifiers = 0;
			for (const auto &child : node.child) {
				howManyIdentifiers += func(func, *child);
			}
			return howManyIdentifiers;
		};
		const size_t howManyIdentifiers =
		    funcHowManyIdentifiers(funcHowManyIdentifiers, node);

		// プロローグ
		std::cout << "	push rbp\n"
		          << "	mov rbp, rsp\n"
		          << "	sub rsp, " << howManyIdentifiers * 8 << "\n"; // 変数の数

		/* 仮引数に実引数を代入 */
		for (size_t i = 0; i < node.identifier_list.size(); ++i) {
			setup_identifier(node.identifier_list[i]);
			std::cout << "	pop rax\n"
			          << "	mov [rax], " << target_registers[i] << "\n";
		}

		/* 関数本体の実行 */
		for (const auto &child : node.child) {
			gen(*child);
		}

		// エピローグ
		std::cout << "	mov rsp, rbp\n"
		          << "	pop rbp\n"
		          << "	ret\n";

		pop_block();

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
		for (size_t i = 0; i < node.child.size(); ++i) {
			std::cout << "	pop " << target_registers[i] << "\n";
		}

		// RSPは16の倍数になっているはずである（最初のローカル変数の確保で、16の倍数になるよう調整しているはずだから）（呼び出し規約）
		// うーん、まずいこともあるなぁ…
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
		assert(node.child[0]->type == Node::node_type::identifier);

		setup_identifier(node.child[0]->value);
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
