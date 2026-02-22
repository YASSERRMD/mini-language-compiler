#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace minilang {

/**
 * Token types for the Mini language
 */
enum class TokenType : uint8_t {
    // Literals
    NUMBER,
    STRING,
    IDENTIFIER,

    // Keywords
    LET,
    FN,
    IF,
    ELSE,
    WHILE,
    RETURN,
    TRUE,
    FALSE,
    PRINT,

    // Operators
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,

    // Logical
    AND,
    OR,
    BANG,

    // Comparison
    EQUAL_EQUAL,
    BANG_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,

    // Assignment
    EQUAL,

    // Delimiters
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    COMMA,
    SEMICOLON,

    // Special
    EOF_TOKEN,
    ERROR,
};

/**
 * Literal value types
 */
using Literal = std::variant<double, std::string, bool>;

/**
 * Token representing a lexical unit
 */
struct Token {
    TokenType type;
    std::string lexeme;
    size_t line;
    size_t column;
    std::variant<std::monostate, double, std::string, bool> literal;

    Token() = default;

    Token(TokenType t, std::string lex, size_t l, size_t c)
        : type(t), lexeme(std::move(lex)), line(l), column(c), literal(std::monostate{}) {}

    Token(TokenType t, std::string lex, size_t l, size_t c, double lit)
        : type(t), lexeme(std::move(lex)), line(l), column(c), literal(lit) {}

    Token(TokenType t, std::string lex, size_t l, size_t c, std::string lit)
        : type(t), lexeme(std::move(lex)), line(l), column(c), literal(std::move(lit)) {}

    Token(TokenType t, std::string lex, size_t l, size_t c, bool lit)
        : type(t), lexeme(std::move(lex)), line(l), column(c), literal(lit) {}

    /**
     * Get the token type as a string for debugging
     */
    std::string typeString() const;
};

} // namespace minilang
