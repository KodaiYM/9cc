#ifndef INCLUDE_GUARD_TOKENIZER_
#define INCLUDE_GUARD_TOKENIZER_

#include <cstddef>
#include <list>
#include <string>

struct Token {
	std::string value;    // token string
	std::string line;     // line with this token
	std::size_t line_num; // token line index
	std::size_t pos;      // token position of line (byte index)
};
class Tokenizer {
	friend class Parser;
	friend std::ostream &operator<<(std::ostream &   stream,
	                                const Tokenizer &tokenizer);

private:
	std::list<Token>  token_list;
	const std::string token_str;
	std::size_t       remove_length = 0;
	void              remove_prefix(std::string_view &str, std::size_t count);

public:
	Tokenizer(std::string_view token_str);
};

#endif
