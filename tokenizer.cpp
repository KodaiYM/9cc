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
	std::size_t line_num = 0; // token line index
	while (token_str.length()) {
		/* consume to a LF */
		std::string_view token_line;
		{
			auto first_LF_index = token_str.find_first_of("\n");
			if (token_str.npos == first_LF_index) {
				token_line = token_str;
				token_str.remove_prefix(token_str.size());
			} else {
				token_line = token_str.substr(0, first_LF_index);
				token_str.remove_prefix(first_LF_index + 1);
			}
		}

		// initialize remove_length
		remove_length = 0;

		/* consume a line token */
		std::string this_line(token_line);
		while (token_line.length()) {
			/* remove blanks */
			if (std::isblank(token_line.front())) {
				auto not_space_first_it =
				    std::find_if(token_line.begin(), token_line.end(),
				                 [](char c) { return std::isblank(c) == 0; });
				remove_prefix(token_line,
				              std::distance(token_line.begin(), not_space_first_it));

				/* finish */
				if (!token_line.length()) {
					break;
				}
			}

			/* symbol */
			{
				const std::vector<const char *> operators = {
				    "==", "!=", ">=", "<=", ">", "<", "+", "-", "*",
				    "/",  "(",  ")",  "=",  ";", "{", "}", ","};
				if (auto found_op = std::find_if(operators.begin(), operators.end(),
				                                 [&token_line](const auto &op) {
					                                 return token_line.starts_with(op);
				                                 });
				    found_op != operators.end()) {
					std::string op(*found_op);
					token_list.push_back(Token{op, this_line, line_num, remove_length});
					remove_prefix(token_line, op.length());
					continue;
				}
			}

			/* number */
			if (auto &front = token_line.front(); isdigit(front)) {
				auto first_notdigit_index = token_line.find_first_not_of("0123456789");
				if (token_line.npos == first_notdigit_index) {
					first_notdigit_index = token_line.length();
				}

				auto num_str = token_line.substr(0, first_notdigit_index);
				token_list.push_back(
				    Token{std::string(num_str), this_line, line_num, remove_length});
				remove_prefix(token_line, first_notdigit_index);
				continue;
			}

			/* identifier */
			if (auto &front = token_line.front(); isalpha(front)) {
				auto first_not_identifier_index = token_line.find_first_not_of(
				    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
				if (token_line.npos == first_not_identifier_index) {
					first_not_identifier_index = token_line.length();
				}

				auto num_str = token_line.substr(0, first_not_identifier_index);
				token_list.push_back(
				    Token{std::string(num_str), this_line, line_num, remove_length});
				remove_prefix(token_line, first_not_identifier_index);
				continue;
			}

			error("Invalid token: "s + token_line.front(), this_line, line_num,
			      remove_length);
		}

		// increment line num
		++line_num;
	}
}
