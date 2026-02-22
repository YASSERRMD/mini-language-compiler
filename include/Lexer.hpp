#pragma once

#include "Token.hpp"
#include <string>
#include <vector>

namespace minilang {

/**
 * Lexer that tokenizes source code into tokens
 * Optimized for speed with minimal allocations
 */
class Lexer {
public:
    explicit Lexer(std::string source);
    ~Lexer() = default;

    /**
     * Tokenize the entire source and return all tokens
     */
    std::vector<Token> tokenize();

    /**
     * Get the next token (for incremental lexing)
     */
    Token nextToken();

    /**
     * Check if there are more tokens to process
     */
    bool hasMore() const { return !isAtEnd(); }

private:
    std::string m_source;
    size_t m_start = 0;
    size_t m_current = 0;
    size_t m_line = 1;
    size_t m_column = 1;
    std::vector<Token> m_tokens;

    // Helper methods
    bool isAtEnd() const { return m_current >= m_source.size(); }
    char advance() { m_column++; return m_source[m_current++]; }
    char peek() const { return isAtEnd() ? '\0' : m_source[m_current]; }
    char peekNext() const {
        return (m_current + 1 >= m_source.size()) ? '\0' : m_source[m_current + 1];
    }
    bool match(char expected);
    void skipWhitespace();
    void skipComment();

    Token makeToken(TokenType type) const;
    Token makeToken(TokenType type, double literal) const;
    Token makeToken(TokenType type, std::string literal) const;
    Token makeToken(TokenType type, bool literal) const;
    Token errorToken(const std::string& message) const;

    // Lexing methods
    Token scanToken();
    Token scanIdentifier();
    Token scanNumber();
    Token scanString();
};

} // namespace minilang
