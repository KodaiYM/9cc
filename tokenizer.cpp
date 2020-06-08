#include "tokenizer.h"
#include "error.h"
#include <vector>

using namespace std::string_literals;

void Tokenizer::remove_prefix(std::string_view &str, std::size_t count) {
	str.remove_prefix(count);
	remove_length += count;
}
Tokenizer::Tokenizer(std::string_view token_str)
    : token_str(token_str) {
	const auto token_line = token_str;
	while (token_str.length()) {
		/* remove blanks */
		if (std::isblank(token_str.front())) {
			auto not_space_first_it =
			    std::find_if(token_str.begin(), token_str.end(),
			                 [](char c) { return std::isblank(c) == 0; });
			remove_prefix(token_str,
			              std::distance(token_str.begin(), not_space_first_it));

			/* finish */
			if (!token_str.length()) {
				break;
			}
		}

		/* symbol */
		{
			const std::vector<const char *> operators = {
			    "==", "!=", ">=", "<=", ">", "<", "+", "-", "*", "/", "(", ")"};
			if (auto found_op = std::find_if(operators.begin(), operators.end(),
			                                 [&token_str](const auto &op) {
				                                 return token_str.starts_with(op);
			                                 });
			    found_op != operators.end()) {
				std::string op(*found_op);
				token_list.push_back(
				    Token{op, std::string(token_line), 0, remove_length});
				remove_prefix(token_str, op.length());
				continue;
			}
		}

		/* number */
		if (auto &front = token_str.front(); isdigit(front)) {
			auto first_notdigit_index = token_str.find_first_not_of("0123456789");
			if (token_str.npos == first_notdigit_index) {
				first_notdigit_index = token_str.length();
			}

			auto num_str = token_str.substr(0, first_notdigit_index);
			token_list.push_back(Token{std::string(num_str), std::string(token_line),
			                           0, remove_length});
			remove_prefix(token_str, first_notdigit_index);
			continue;
		}

		/* identifier */

		error("Invalid token: "s + token_str.front(), token_line, remove_length);
	}
}
