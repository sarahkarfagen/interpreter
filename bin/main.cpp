#include <fstream>
#include <iostream>
#include <sstream>

#include "itmoscript/ast.h"
#include "itmoscript/lexer.h"
#include "itmoscript/parser.h"

void printAST(const itmoscript::ASTNode* node, int indent = 0) {
    for (int i = 0; i < indent; ++i) std::cout << "  ";
    std::cout << static_cast<int>(node->type) << " (" << node->value << ")\n";
    for (const auto& child : node->children) printAST(child.get(), indent + 1);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>\n";
        return 1;
    }

    std::ifstream in(argv[1]);
    if (!in) {
        std::cerr << "Cannot open file: " << argv[1] << "\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string source = buffer.str();

    try {
        itmoscript::Lexer lexer(source);
        auto tokens = lexer.tokenize();
        itmoscript::Parser parser(tokens);
        auto ast = parser.parseProgram();
        printAST(ast.get());
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
