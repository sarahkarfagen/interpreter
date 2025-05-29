#ifndef TOKEN_H
#define TOKEN_H

#include <string>

namespace itmoscript {

enum class TokenType {
    Identifier,
    Number,
    String,
    Boolean,
    Nil,

    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Caret,
    Equals,
    PlusEqual,
    MinusEqual,
    StarEqual,
    SlashEqual,
    PercentEqual,
    CaretEqual,
    EqualEqual,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,

    LeftParen,
    RightParen,
    LeftBracket,
    RightBracket,
    Comma,
    Colon,

    If,
    Then,
    Else,
    While,
    For,
    In,
    Function,
    Return,
    Break,
    Continue,
    End,
    And,
    Or,
    Not,

    EndOfFile,
    Unknown
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

}  // namespace itmoscript

#endif