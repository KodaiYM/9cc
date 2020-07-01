#ifndef PRINT_PARSER_
#define PRINT_PARSER_

#include "parser.h"
#include "tokenizer.h"
std::ostream &operator<<(std::ostream &stream, const Tokenizer &tokenizer);
std::ostream &operator<<(std::ostream &stream, std::list<Token> token_list);
std::ostream &operator<<(std::ostream &stream, const Node &node);

#endif
