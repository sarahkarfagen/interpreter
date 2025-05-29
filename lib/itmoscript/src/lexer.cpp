#include "itmoscript/lexer.h"

#include <cctype>
#include <stdexcept>
#include <unordered_map>

#include "itmoscript/token.h"

namespace itmoscript {

static const std::unordered_map<std::string, TokenType> keywords = {
    {"if", TokenType::If},
    {"then", TokenType::Then},
    {"else", TokenType::Else},
    {"end", TokenType::End},
    {"while", TokenType::While},
    {"for", TokenType::For},
    {"in", TokenType::In},
    {"function", TokenType::Function},
    {"return", TokenType::Return},
    {"break", TokenType::Break},
    {"continue", TokenType::Continue},
    {"and", TokenType::And},
    {"or", TokenType::Or},
    {"not", TokenType::Not},
    {"true", TokenType::Boolean},
    {"false", TokenType::Boolean},
    {"nil", TokenType::Nil}};

Lexer::Lexer(const std::string& source) noexcept : source_(source) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (pos_ < source_.size()) {
        skipWhitespace();
        if (pos_ >= source_.size()) break;
        char c = peek();
        if (std::isalpha(c) || c == '_') {
            tokens.push_back(identifier());
        } else if (std::isdigit(c)) {
            tokens.push_back(number());
        } else if (c == '"') {
            tokens.push_back(string());
        } else if (c == '/' && pos_ + 1 < source_.size() &&
                   source_[pos_ + 1] == '/') {
            skipComment();
        } else {
            int start_col = column_;
            std::string lexeme;
            TokenType type = TokenType::Unknown;
            char current = get();
            switch (current) {
                case '+':
                    type = match('=') ? TokenType::PlusEqual : TokenType::Plus;
                    break;
                case '-':
                    type =
                        match('=') ? TokenType::MinusEqual : TokenType::Minus;
                    break;
                case '*':
                    type = match('=') ? TokenType::StarEqual : TokenType::Star;
                    break;
                case '/':
                    type =
                        match('=') ? TokenType::SlashEqual : TokenType::Slash;
                    break;
                case '%':
                    type = match('=') ? TokenType::PercentEqual
                                      : TokenType::Percent;
                    break;
                case '^':
                    type =
                        match('=') ? TokenType::CaretEqual : TokenType::Caret;
                    break;
                case '=':
                    type =
                        match('=') ? TokenType::EqualEqual : TokenType::Equals;
                    break;
                case '!':
                    type =
                        match('=') ? TokenType::NotEqual : TokenType::Unknown;
                    break;
                case '<':
                    type = match('=') ? TokenType::LessEqual : TokenType::Less;
                    break;
                case '>':
                    type = match('=') ? TokenType::GreaterEqual
                                      : TokenType::Greater;
                    break;
                case '(':
                    type = TokenType::LeftParen;
                    break;
                case ')':
                    type = TokenType::RightParen;
                    break;
                case '[':
                    type = TokenType::LeftBracket;
                    break;
                case ']':
                    type = TokenType::RightBracket;
                    break;
                case ',':
                    type = TokenType::Comma;
                    break;
                case ':':
                    type = TokenType::Colon;
                    break;
                default:
                    throw std::runtime_error(
                        "Unknown character '" + std::string(1, current) +
                        "' at line " + std::to_string(line_));
            }

            size_t length =
                (type == TokenType::PlusEqual ||
                 type == TokenType::MinusEqual ||
                 type == TokenType::StarEqual ||
                 type == TokenType::SlashEqual ||
                 type == TokenType::PercentEqual ||
                 type == TokenType::CaretEqual ||
                 type == TokenType::EqualEqual || type == TokenType::NotEqual ||
                 type == TokenType::LessEqual ||
                 type == TokenType::GreaterEqual)
                    ? 2
                    : 1;
            lexeme = source_.substr(pos_ - length, length);
            tokens.push_back({type, lexeme, line_, start_col});
        }
    }
    tokens.push_back({TokenType::EndOfFile, "", line_, column_});
    return tokens;
}

char Lexer::peek() const noexcept { return source_[pos_]; }

char Lexer::get() {
    char c = source_[pos_++];
    if (c == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    return c;
}

bool Lexer::match(char expected) noexcept {
    if (pos_ < source_.size() && source_[pos_] == expected) {
        get();
        return true;
    }
    return false;
}

void Lexer::skipWhitespace() {
    while (pos_ < source_.size() && std::isspace(peek())) get();
}

void Lexer::skipComment() {
    get();
    get();
    while (pos_ < source_.size() && peek() != '\n') get();
}

Token Lexer::identifier() {
    int start_col = column_;
    size_t start = pos_;
    while (pos_ < source_.size() && (std::isalnum(peek()) || peek() == '_'))
        get();
    std::string text = source_.substr(start, pos_ - start);
    auto it = keywords.find(text);
    TokenType type =
        (it != keywords.end() ? it->second : TokenType::Identifier);
    return {type, text, line_, start_col};
}

Token Lexer::number() {
    int start_col = column_;
    size_t start = pos_;
    while (pos_ < source_.size() && std::isdigit(peek())) get();
    if (pos_ < source_.size() && peek() == '.') {
        get();
        while (pos_ < source_.size() && std::isdigit(peek())) get();
    }
    if (pos_ < source_.size() && (peek() == 'e' || peek() == 'E')) {
        get();
        if (peek() == '+' || peek() == '-') get();
        while (pos_ < source_.size() && std::isdigit(peek())) get();
    }
    std::string num = source_.substr(start, pos_ - start);
    return {TokenType::Number, num, line_, start_col};
}

Token Lexer::string() {
    int start_col = column_;
    get();
    size_t start = pos_;
    while (pos_ < source_.size() && peek() != '"') {
        if (peek() == '\\') {
            get();
            get();
        } else
            get();
    }
    std::string str = source_.substr(start, pos_ - start);
    if (peek() == '"')
        get();
    else
        throw std::runtime_error("Unterminated string at line " +
                                 std::to_string(line_));
    return {TokenType::String, str, line_, start_col};
}

}  // namespace itmoscript
