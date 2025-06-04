#include "itmoscript/interpreter.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "itmoscript/aet.h"
#include "itmoscript/environment.h"
#include "itmoscript/lexer.h"
#include "itmoscript/parser.h"
#include "itmoscript/stdlib.h"
#include "itmoscript/value.h"

namespace itmoscript {

/**
 *  Two‐argument version: No runtime input (read() will immediately be EOF).
 */
bool interpret(std::istream& codeIn, std::ostream& out) {
    return interpret(codeIn, std::cin, out);
}

/**
 *  Three‐argument version: codeIn supplies the program text; runtimeIn supplies
 *  data for any read() calls at runtime; out is the destination for
 * print/println.
 */
bool interpret(std::istream& codeIn, std::istream& runtimeIn,
               std::ostream& out) {
    try {
        std::string src((std::istreambuf_iterator<char>(codeIn)),
                        std::istreambuf_iterator<char>());

        Lexer lex(src);
        auto tokens = lex.tokenize();

        Parser parser(tokens);
        auto ast = parser.parseProgram();

        auto root = buildAET(ast.get());

        Environment::Builder eb;
        eb.setInput(runtimeIn).setOutput(out);

        registerStandardLibrary(eb);

        auto env = eb.build();

        root->execute(*env);
        return true;
    } catch (std::runtime_error e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

}  // namespace itmoscript
