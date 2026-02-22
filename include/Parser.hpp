#pragma once

#include "AST.hpp"
#include "Token.hpp"
#include <vector>
#include <stdexcept>

namespace minilang {

/**
 * Parse error exception
 */
class ParseError : public std::runtime_error {
public:
    size_t line;
    size_t column;

    ParseError(const std::string& msg, size_t l, size_t c)
        : std::runtime_error(msg), line(l), column(c) {}
};

/**
 * Recursive descent parser
 * Transforms tokens into an AST
 */
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    ~Parser() = default;

    /**
     * Parse the entire program
     */
    Program parse();

private:
    const std::vector<Token>& m_tokens;
    size_t m_current = 0;

    // Token consumption
    Token advance();
    Token peek() const;
    Token previous() const;
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(std::initializer_list<TokenType> types);
    Token consume(TokenType type, const std::string& message);

    // Error handling
    ParseError error(Token token, const std::string& message);
    void synchronize();

    // Statement parsing (declarations first)
    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> letDeclaration();
    std::unique_ptr<Stmt> functionDeclaration();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> ifStatement();
    std::unique_ptr<Stmt> whileStatement();
    std::unique_ptr<Stmt> returnStatement();
    std::unique_ptr<Stmt> printStatement();
    std::unique_ptr<Stmt> blockStatement();
    std::unique_ptr<Stmt> expressionStatement();

    // Expression parsing
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> logicalOr();
    std::unique_ptr<Expr> logicalAnd();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> primary();
    std::unique_ptr<Expr> call();
    std::unique_ptr<Expr> finishCall(std::unique_ptr<Expr> callee);

    // Helper for grouping
    std::unique_ptr<Expr> finishGrouping();
};

} // namespace minilang
