#include "itmoscript/parser.h"

#include <stdexcept>

#include "itmoscript/token.h"

namespace itmoscript {

Parser::Parser(const std::vector<Token>& tokens) noexcept : tokens_(tokens) {}

const Token& Parser::peek() const noexcept { return tokens_[index_]; }

const Token& Parser::get() {
    if (index_ < tokens_.size()) return tokens_[index_++];
    return tokens_.back();
}

bool Parser::match(TokenType type) noexcept {
    if (check(type)) {
        get();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const noexcept {
    return peek().type == type;
}

void Parser::expect(TokenType type, const std::string& message) {
    if (!match(type)) {
        throw ParseError(message + " at line " + std::to_string(peek().line));
    }
}

ASTNodePtr Parser::parseProgram() {
    auto node = std::make_unique<ASTNode>(NodeType::Program);
    node->addChild(parseStatementList());
    expect(TokenType::EndOfFile, "Expected end of file");
    return node;
}

ASTNodePtr Parser::parseStatementList() {
    auto list = std::make_unique<ASTNode>(NodeType::StatementList);

    while (match(TokenType::NewLine)) {
    }

    while (!check(TokenType::End) && !check(TokenType::Else) &&
           !check(TokenType::EndOfFile)) {
        size_t oldIndex = index_;

        list->addChild(parseStatement());

        if (index_ == oldIndex) {
            throw ParseError("Uexpected token '" + tokens_[index_].lexeme +
                             "' at " + std::to_string(index_) + ", line " +
                             std::to_string(tokens_[index_].line) +
                             ", column " +
                             std::to_string(tokens_[index_].column));
        }

        while (match(TokenType::NewLine)) {
        }
    }

    return list;
}

ASTNodePtr Parser::parseStatement() {
    if (check(TokenType::If) || check(TokenType::While) ||
        check(TokenType::For))
        return parseCompoundStatement();
    return parseSimpleStatement();
}

ASTNodePtr Parser::parseSimpleStatement() {
    if (check(TokenType::Return)) return parseReturn();
    if (check(TokenType::Break)) return parseBreak();
    if (check(TokenType::Continue)) return parseContinue();
    if (check(TokenType::Identifier) && index_ + 1 < tokens_.size() &&
        (tokens_[index_ + 1].type == TokenType::Equals ||
         tokens_[index_ + 1].type == TokenType::PlusEqual ||
         tokens_[index_ + 1].type == TokenType::MinusEqual ||
         tokens_[index_ + 1].type == TokenType::StarEqual ||
         tokens_[index_ + 1].type == TokenType::SlashEqual ||
         tokens_[index_ + 1].type == TokenType::PercentEqual ||
         tokens_[index_ + 1].type == TokenType::CaretEqual))
        return parseAssignment();
    if (check(TokenType::Identifier) && index_ + 1 < tokens_.size() &&
        tokens_[index_ + 1].type == TokenType::LeftParen)
        return parseFunctionCall();
    return std::make_unique<ASTNode>(NodeType::StatementList);
}

ASTNodePtr Parser::parseCompoundStatement() {
    if (check(TokenType::If)) return parseIf();
    if (check(TokenType::While)) return parseWhile();
    if (check(TokenType::For)) return parseFor();
    throw ParseError("compound expected");
}

ASTNodePtr Parser::parseAssignment() {
    auto t = get();
    auto n = std::make_unique<ASTNode>(NodeType::Assignment, t.lexeme);
    auto o = get();
    n->addChild(std::make_unique<ASTNode>(NodeType::Identifier, t.lexeme));
    n->addChild(std::make_unique<ASTNode>(NodeType::Identifier, o.lexeme));
    n->addChild(parseExpression());
    return n;
}

ASTNodePtr Parser::parseFunctionCall() {
    auto id = get();
    auto node = std::make_unique<ASTNode>(NodeType::FunctionCall, id.lexeme);
    node->addChild(std::make_unique<ASTNode>(NodeType::Identifier, id.lexeme));
    expect(TokenType::LeftParen, "Expected '('");
    if (!check(TokenType::RightParen)) node->addChild(parseArgumentList());
    expect(TokenType::RightParen, "Expected ')'");
    return node;
}

ASTNodePtr Parser::parseReturn() {
    get();
    auto node = std::make_unique<ASTNode>(NodeType::Return);
    node->addChild(parseExpression());
    return node;
}

ASTNodePtr Parser::parseBreak() {
    get();
    return std::make_unique<ASTNode>(NodeType::Break);
}
ASTNodePtr Parser::parseContinue() {
    get();
    return std::make_unique<ASTNode>(NodeType::Continue);
}

ASTNodePtr Parser::parseIf() {
    get();
    auto node = std::make_unique<ASTNode>(NodeType::If);
    node->addChild(parseExpression());
    expect(TokenType::Then, "Expected 'then'");
    node->addChild(parseStatementList());

    while (check(TokenType::Else) && index_ + 1 < tokens_.size() &&
           tokens_[index_ + 1].type == TokenType::If) {
        get();
        get();
        auto elseif = std::make_unique<ASTNode>(NodeType::ElseIf);
        elseif->addChild(parseExpression());
        expect(TokenType::Then, "Expected 'then'");
        elseif->addChild(parseStatementList());
        node->addChild(std::move(elseif));
    }

    if (match(TokenType::Else)) {
        auto elseNode = std::make_unique<ASTNode>(NodeType::Else);
        elseNode->addChild(parseStatementList());
        node->addChild(std::move(elseNode));
    }

    expect(TokenType::End, "Expected 'end'");
    expect(TokenType::If, "Expected 'if'");
    return node;
}

ASTNodePtr Parser::parseWhile() {
    get();
    auto node = std::make_unique<ASTNode>(NodeType::While);
    node->addChild(parseExpression());
    node->addChild(parseStatementList());
    expect(TokenType::End, "Expected 'end'");
    expect(TokenType::While, "Expected 'while'");
    return node;
}

ASTNodePtr Parser::parseFor() {
    get();
    auto node = std::make_unique<ASTNode>(NodeType::For);
    auto id = get();
    node->addChild(std::make_unique<ASTNode>(NodeType::Identifier, id.lexeme));
    expect(TokenType::In, "Expected 'in'");
    node->addChild(parseExpression());
    node->addChild(parseStatementList());
    expect(TokenType::End, "Expected 'end'");
    expect(TokenType::For, "Expected 'for'");
    return node;
}

ASTNodePtr Parser::parseFunctionDefinition() {
    auto name = get();
    expect(TokenType::Equals, "Expected '='");
    expect(TokenType::Function, "Expected 'function'");
    auto node =
        std::make_unique<ASTNode>(NodeType::FunctionDefinition, name.lexeme);
    expect(TokenType::LeftParen, "Expected '('");
    if (!check(TokenType::RightParen)) node->addChild(parseParameterList());
    expect(TokenType::RightParen, "Expected ')'");
    node->addChild(parseStatementList());
    if (check(TokenType::Return)) node->addChild(parseReturn());
    expect(TokenType::End, "Expected 'end'");
    expect(TokenType::Function, "Expected 'function'");
    return node;
}

ASTNodePtr Parser::parseExpression() { return parseLogicalOr(); }

ASTNodePtr Parser::parseLogicalOr() {
    auto node = parseLogicalAnd();
    while (match(TokenType::Or)) {
        auto op = std::make_unique<ASTNode>(NodeType::BinaryOp, "or");
        op->addChild(std::move(node));
        op->addChild(parseLogicalAnd());
        node = std::move(op);
    }
    return node;
}

ASTNodePtr Parser::parseLogicalAnd() {
    auto node = parseLogicalNot();
    while (match(TokenType::And)) {
        auto op = std::make_unique<ASTNode>(NodeType::BinaryOp, "and");
        op->addChild(std::move(node));
        op->addChild(parseLogicalNot());
        node = std::move(op);
    }
    return node;
}

ASTNodePtr Parser::parseLogicalNot() {
    if (match(TokenType::Not)) {
        auto op = std::make_unique<ASTNode>(NodeType::UnaryOp, "not");
        op->addChild(parseLogicalNot());
        return op;
    }
    return parseComparison();
}

ASTNodePtr Parser::parseComparison() {
    auto node = parseAdditive();
    if (check(TokenType::EqualEqual) || check(TokenType::NotEqual) ||
        check(TokenType::Less) || check(TokenType::LessEqual) ||
        check(TokenType::Greater) || check(TokenType::GreaterEqual)) {
        auto opToken = get();
        std::string op = opToken.lexeme;
        auto opNode = std::make_unique<ASTNode>(NodeType::BinaryOp, op);
        opNode->addChild(std::move(node));
        opNode->addChild(parseAdditive());
        node = std::move(opNode);
    }
    return node;
}

ASTNodePtr Parser::parseAdditive() {
    auto node = parseMultiplicative();
    while (match(TokenType::Plus) || match(TokenType::Minus)) {
        auto opToken = tokens_[index_ - 1];
        auto opNode =
            std::make_unique<ASTNode>(NodeType::BinaryOp, opToken.lexeme);
        opNode->addChild(std::move(node));
        opNode->addChild(parseMultiplicative());
        node = std::move(opNode);
    }
    return node;
}

ASTNodePtr Parser::parseMultiplicative() {
    auto node = parseExponent();
    while (match(TokenType::Star) || match(TokenType::Slash) ||
           match(TokenType::Percent)) {
        auto opToken = tokens_[index_ - 1];
        auto opNode =
            std::make_unique<ASTNode>(NodeType::BinaryOp, opToken.lexeme);
        opNode->addChild(std::move(node));
        opNode->addChild(parseExponent());
        node = std::move(opNode);
    }
    return node;
}

ASTNodePtr Parser::parseExponent() {
    auto node = parseUnary();
    while (match(TokenType::Caret)) {
        auto opNode = std::make_unique<ASTNode>(NodeType::BinaryOp, "^");
        opNode->addChild(std::move(node));
        opNode->addChild(parseUnary());
        node = std::move(opNode);
    }
    return node;
}

ASTNodePtr Parser::parseUnary() {
    if (match(TokenType::Plus) || match(TokenType::Minus)) {
        auto opToken = tokens_[index_ - 1];
        auto opNode =
            std::make_unique<ASTNode>(NodeType::UnaryOp, opToken.lexeme);
        opNode->addChild(parseUnary());
        return opNode;
    }
    return parsePrimary();
}

ASTNodePtr Parser::parsePostfix(ASTNodePtr lhs) {
    while (true) {
        if (match(TokenType::LeftBracket)) {
            auto idxNode =
                std::make_unique<ASTNode>(NodeType::BinaryOp, "index");
            idxNode->addChild(std::move(lhs));
            idxNode->addChild(parseSliceOrExpr());
            expect(TokenType::RightBracket, "Expected ']' after index");
            lhs = std::move(idxNode);
        } else if (match(TokenType::LeftParen)) {
            auto callNode =
                std::make_unique<ASTNode>(NodeType::FunctionCall, lhs->value);
            callNode->addChild(std::move(lhs));
            if (!check(TokenType::RightParen))
                callNode->addChild(parseArgumentList());
            expect(TokenType::RightParen, "Expected ')' after arguments");
            lhs = std::move(callNode);
        } else
            break;
    }
    return lhs;
}

ASTNodePtr Parser::parsePrimary() {
    if (match(TokenType::Function)) {
        auto n = std::make_unique<ASTNode>(NodeType::FunctionDefinition);
        expect(TokenType::LeftParen, "Expected '(' after 'function'");
        if (!check(TokenType::RightParen)) n->addChild(parseParameterList());
        expect(TokenType::RightParen, "Expected ')'");
        n->addChild(parseStatementList());
        if (check(TokenType::Return)) n->addChild(parseReturn());
        expect(TokenType::End, "Expected 'end'");
        expect(TokenType::Function, "Expected 'function'");
        return n;
    }

    if (match(TokenType::LeftParen)) {
        auto n = parseExpression();
        expect(TokenType::RightParen, "Expected ')'");
        return parsePostfix(std::move(n));
    }

    if (match(TokenType::Number) || match(TokenType::String) ||
        match(TokenType::Boolean) || match(TokenType::Nil)) {
        auto t = tokens_[index_ - 1];
        auto vt = NodeType::Literal;
        if (t.type == TokenType::Boolean) vt = NodeType::Boolean;
        if (t.type == TokenType::Nil) vt = NodeType::Nil;
        return std::make_unique<ASTNode>(vt, t.lexeme);
    }

    if (match(TokenType::LeftBracket)) {
        auto l = std::make_unique<ASTNode>(NodeType::ListLiteral);

        while (true) {
            while (match(TokenType::NewLine)) {
                ;
            }

            if (check(TokenType::RightBracket)) break;

            l->addChild(parseExpression());

            while (match(TokenType::NewLine)) {
                ;
            }

            if (match(TokenType::Comma)) {
                if (check(TokenType::RightBracket))
                    break;
                else
                    continue;
            } else {
                break;
            }
        }

        expect(TokenType::RightBracket, "Expected ']' after list literal");
        return parsePostfix(std::move(l));
    }

    if (match(TokenType::Identifier)) {
        auto t = tokens_[index_ - 1];
        return parsePostfix(
            std::make_unique<ASTNode>(NodeType::Identifier, t.lexeme));
    }

    throw ParseError("Unexpected token '" + peek().lexeme + "' at line " +
                     std::to_string(peek().line));
}

ASTNodePtr Parser::parseLiteral() {
    if (match(TokenType::Number) || match(TokenType::String) ||
        match(TokenType::Boolean) || match(TokenType::Nil)) {
        auto tok = tokens_[index_ - 1];
        NodeType t = NodeType::Literal;
        if (tok.type == TokenType::Boolean) t = NodeType::Boolean;
        if (tok.type == TokenType::Nil) t = NodeType::Nil;
        return std::make_unique<ASTNode>(t, tok.lexeme);
    }
    if (match(TokenType::LeftBracket)) {
        auto listNode = std::make_unique<ASTNode>(NodeType::ListLiteral);
        while (!check(TokenType::RightBracket)) {
            listNode->addChild(parseExpression());
            if (match(TokenType::Comma)) {
                if (check(TokenType::RightBracket)) break;
            } else {
                break;
            }
        }
        expect(TokenType::RightBracket, "Expected ']' after list literal");
        return listNode;
    }
    if (match(TokenType::Identifier)) {
        auto tok = tokens_[index_ - 1];
        return std::make_unique<ASTNode>(NodeType::Identifier, tok.lexeme);
    }
    throw ParseError("Unexpected token '" + peek().lexeme + "'");
}

ASTNodePtr Parser::parseParameterList() {
    auto node = std::make_unique<ASTNode>(NodeType::ParameterList);
    do {
        auto tok = get();
        if (tok.type != TokenType::Identifier)
            throw ParseError("Expected parameter name");
        node->addChild(
            std::make_unique<ASTNode>(NodeType::Identifier, tok.lexeme));
    } while (match(TokenType::Comma));
    return node;
}

ASTNodePtr Parser::parseArgumentList() {
    auto node = std::make_unique<ASTNode>(NodeType::ArgumentList);
    do {
        node->addChild(parseExpression());
    } while (match(TokenType::Comma));
    return node;
}

ASTNodePtr Parser::parseSliceOrExpr() {
    if (check(TokenType::Colon) || !check(TokenType::RightBracket)) {
        auto start = match(TokenType::Colon) ? nullptr : parseExpression();
        if (match(TokenType::Colon)) {
            auto slice = std::make_unique<ASTNode>(NodeType::BinaryOp, ":");
            slice->addChild(start ? std::move(start)
                                  : std::make_unique<ASTNode>(NodeType::Nil));
            if (!check(TokenType::RightBracket))
                slice->addChild(parseExpression());
            return slice;
        }
        if (start) return start;
    }
    return std::make_unique<ASTNode>(NodeType::Nil);
}

}  