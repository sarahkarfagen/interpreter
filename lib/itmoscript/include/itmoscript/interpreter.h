#ifndef ITMOSCRIPT_INTERPRETER_H
#define ITMOSCRIPT_INTERPRETER_H

#include <istream>
#include <ostream>

namespace itmoscript {

bool interpret(std::istream& in, std::ostream& out);

bool interpret(std::istream& codeIn, std::istream& runtimeIn,
               std::ostream& out);

}  // namespace itmoscript

#endif
