#ifndef INCLUDE_GUARD_CODEGEN_
#define INCLUDE_GUARD_CODEGEN_

#include "parser.h"
#include <memory>

// calculate node and "push" result to stack
void gen(const std::unique_ptr<Node> &node);

#endif
