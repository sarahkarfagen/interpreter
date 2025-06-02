

#include "itmoscript/interpreter.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "itmoscript/aet.h"
#include "itmoscript/environment.h"
#include "itmoscript/lexer.h"
#include "itmoscript/parser.h"
#include "itmoscript/stdlib.h"
#include "itmoscript/value.h"

namespace itmoscript {

bool interpret(std::istream& in, std::ostream& out) {
    try {
        std::string src((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());

        Lexer lex(src);
        auto tokens = lex.tokenize();

        Parser parser(tokens);
        auto ast = parser.parseProgram();

        auto root = buildAET(ast.get());

        Environment::Builder eb;
        eb.setOutput(out);

        registerStandardLibrary(eb);

        auto env = eb.build();

        root->execute(*env);
        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace itmoscript
