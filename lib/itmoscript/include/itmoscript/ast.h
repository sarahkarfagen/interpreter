#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <vector>

namespace itmoscript {

enum class NodeType {
    Program,
    StatementList,
    Assignment,
    FunctionCall,
    Return,
    Break,
    Continue,
    If,
    ElseIf,
    Else,
    While,
    For,
    FunctionDefinition,
    ParameterList,
    ArgumentList,
    BinaryOp,
    UnaryOp,
    Literal,
    Identifier,
    ListLiteral,
    Nil,
    Boolean
};

class ASTNode {
   public:
    NodeType type;
    std::string value;
    std::vector<std::unique_ptr<ASTNode>> children;

    explicit ASTNode(NodeType type_, std::string value_ = "") noexcept
        : type(type_), value(std::move(value_)) {}

    ASTNode(const ASTNode&) = delete;
    ASTNode& operator=(const ASTNode&) = delete;
    ASTNode(ASTNode&&) = default;
    ASTNode& operator=(ASTNode&&) = default;

    void addChild(std::unique_ptr<ASTNode> child) {
        children.push_back(std::move(child));
    }
};

using ASTNodePtr = std::unique_ptr<ASTNode>;

}  // namespace itmoscript

#endif