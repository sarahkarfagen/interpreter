#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

#include "itmoscript/token.h"

namespace itmoscript {

class Lexer {
   public:
    explicit Lexer(const std::string& source) noexcept;
    std::vector<Token> tokenize();

   private:
    const std::string source_;
    size_t pos_ = 0;
    int line_ = 1;
    int column_ = 1;

    char peek() const noexcept;
    char get();
    bool match(char expected) noexcept;
    void skipWhitespace();
    void skipComment();
    Token identifier();
    Token number();
    Token string();
    Token makeToken(TokenType type, const std::string& lexeme) const;
};

}  // namespace itmoscript

#endif