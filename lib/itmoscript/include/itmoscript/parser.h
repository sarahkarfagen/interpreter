#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <stdexcept>
#include <vector>

#include "itmoscript/ast.h"
#include "itmoscript/token.h"

namespace itmoscript {

class ParseError : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;
};

class Parser {
   public:
    explicit Parser(const std::vector<Token>& tokens) noexcept;
    ASTNodePtr parseProgram();

   private:
    const std::vector<Token>& tokens_;
    size_t index_ = 0;

    const Token& peek() const noexcept;
    const Token& get();
    bool match(TokenType type) noexcept;
    bool check(TokenType type) const noexcept;
    void expect(TokenType type, const std::string& message);

    ASTNodePtr parseStatementList();
    ASTNodePtr parseStatement();
    ASTNodePtr parseSimpleStatement();
    ASTNodePtr parseCompoundStatement();

    ASTNodePtr parseAssignment();
    ASTNodePtr parseFunctionCall();
    ASTNodePtr parseReturn();
    ASTNodePtr parseBreak();
    ASTNodePtr parseContinue();

    ASTNodePtr parseIf();
    ASTNodePtr parseWhile();
    ASTNodePtr parseFor();
    ASTNodePtr parseFunctionDefinition();

    ASTNodePtr parseExpression();
    ASTNodePtr parseLogicalOr();
    ASTNodePtr parseLogicalAnd();
    ASTNodePtr parseLogicalNot();
    ASTNodePtr parseComparison();
    ASTNodePtr parseAdditive();
    ASTNodePtr parseMultiplicative();
    ASTNodePtr parseExponent();
    ASTNodePtr parseUnary();
    ASTNodePtr parsePostfix(ASTNodePtr lhs);
    ASTNodePtr parsePrimary();
    ASTNodePtr parseLiteral();

    ASTNodePtr parseParameterList();
    ASTNodePtr parseArgumentList();
    ASTNodePtr parseSliceOrExpr();
};

}  // namespace itmoscript

#endif