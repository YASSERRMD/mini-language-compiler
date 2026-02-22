#include "Parser.hpp"
#include <format>

namespace minilang {

Parser::Parser(const std::vector<Token>& tokens) : m_tokens(tokens) {}

Program Parser::parse() {
    Program program;

    while (!isAtEnd()) {
        try {
            auto stmt = declaration();
            if (stmt) {
                program.push_back(std::move(stmt));
            }
        } catch (const ParseError& error) {
            // Synchronize and continue parsing
            std::cerr << std::format("[Line {}] Parse Error: {}", error.line, error.what()) << std::endl;
            synchronize();
        }
    }

    return program;
}

Token Parser::advance() {
    if (!isAtEnd()) m_current++;
    return previous();
}

Token Parser::peek() const {
    return m_tokens[m_current];
}

Token Parser::previous() const {
    return m_tokens[m_current - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw error(peek(), message);
}

ParseError Parser::error(Token token, const std::string& message) {
    return ParseError(message, token.line, token.column);
}

void Parser::synchronize() {
    advance();

    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;

        switch (peek().type) {
            case TokenType::FN:
            case TokenType::LET:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::RETURN:
            case TokenType::PRINT:
                return;
            default:
                break;
        }

        advance();
    }
}

std::unique_ptr<Stmt> Parser::declaration() {
    if (match({TokenType::LET})) {
        return letDeclaration();
    }
    if (match({TokenType::FN})) {
        return functionDeclaration();
    }
    return statement();
}

std::unique_ptr<Stmt> Parser::letDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name after 'let'.");

    std::unique_ptr<Expr> initializer = nullptr;
    if (match({TokenType::EQUAL})) {
        initializer = expression();
    }

    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_unique<LetStmt>(name, std::move(initializer));
}

std::unique_ptr<Stmt> Parser::functionDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect function name after 'fn'.");

    consume(TokenType::LPAREN, "Expect '(' after function name.");

    std::vector<Token> params;
    if (!check(TokenType::RPAREN)) {
        do {
            if (params.size() >= 255) {
                throw error(peek(), "Can't have more than 255 parameters.");
            }
            params.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
        } while (match({TokenType::COMMA}));
    }

    consume(TokenType::RPAREN, "Expect ')' after parameters.");
    consume(TokenType::LBRACE, "Expect '{' before function body.");

    std::vector<std::unique_ptr<Stmt>> body;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        auto stmt = declaration();
        if (stmt) {
            body.push_back(std::move(stmt));
        }
    }

    consume(TokenType::RBRACE, "Expect '}' after function body.");
    return std::make_unique<FunctionStmt>(name, std::move(params), std::move(body));
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::RETURN})) return returnStatement();
    if (match({TokenType::PRINT})) return printStatement();
    if (match({TokenType::LBRACE})) return blockStatement();
    return expressionStatement();
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    consume(TokenType::LPAREN, "Expect '(' after 'if'.");
    auto condition = expression();
    consume(TokenType::RPAREN, "Expect ')' after if condition.");

    auto thenBranch = statement();
    std::unique_ptr<Stmt> elseBranch = nullptr;

    if (match({TokenType::ELSE})) {
        elseBranch = statement();
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
    consume(TokenType::LPAREN, "Expect '(' after 'while'.");
    auto condition = expression();
    consume(TokenType::RPAREN, "Expect ')' after while condition.");

    auto body = statement();
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::returnStatement() {
    Token keyword = previous();
    std::unique_ptr<Expr> value = nullptr;

    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }

    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_unique<ReturnStmt>(keyword, std::move(value));
}

std::unique_ptr<Stmt> Parser::printStatement() {
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    return std::make_unique<PrintStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::blockStatement() {
    std::vector<std::unique_ptr<Stmt>> statements;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        auto stmt = declaration();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }

    consume(TokenType::RBRACE, "Expect '}' after block.");
    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    auto expr = logicalOr();

    if (match({TokenType::EQUAL})) {
        Token equals = previous();
        auto value = assignment();

        if (dynamic_cast<VariableExpr*>(expr.get())) {
            auto* varExpr = static_cast<VariableExpr*>(expr.get());
            Token name = varExpr->name;
            expr.release(); // Release from unique_ptr to transfer ownership
            return std::make_unique<AssignExpr>(name, std::move(value));
        }

        throw error(equals, "Invalid assignment target.");
    }

    return expr;
}

std::unique_ptr<Expr> Parser::logicalOr() {
    auto expr = logicalAnd();

    while (match({TokenType::OR})) {
        Token op = previous();
        auto right = logicalAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::logicalAnd() {
    auto expr = equality();

    while (match({TokenType::AND})) {
        Token op = previous();
        auto right = equality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    auto expr = comparison();

    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        Token op = previous();
        auto right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    auto expr = term();

    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        auto right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    auto expr = factor();

    while (match({TokenType::MINUS, TokenType::PLUS})) {
        Token op = previous();
        auto right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    auto expr = unary();

    while (match({TokenType::SLASH, TokenType::STAR, TokenType::PERCENT})) {
        Token op = previous();
        auto right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
        Token op = previous();
        auto right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }

    return call();
}

std::unique_ptr<Expr> Parser::call() {
    auto expr = primary();

    while (true) {
        if (match({TokenType::LPAREN})) {
            expr = finishCall(std::move(expr));
        } else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expr> Parser::finishCall(std::unique_ptr<Expr> callee) {
    std::vector<std::unique_ptr<Expr>> arguments;

    if (!check(TokenType::RPAREN)) {
        do {
            if (arguments.size() >= 255) {
                throw error(peek(), "Can't have more than 255 arguments.");
            }
            arguments.push_back(expression());
        } while (match({TokenType::COMMA}));
    }

    Token paren = consume(TokenType::RPAREN, "Expect ')' after arguments.");
    return std::make_unique<CallExpr>(std::move(callee), paren, std::move(arguments));
}

std::unique_ptr<Expr> Parser::primary() {
    if (match({TokenType::FALSE})) {
        return std::make_unique<LiteralExpr>(false);
    }
    if (match({TokenType::TRUE})) {
        return std::make_unique<LiteralExpr>(true);
    }
    if (match({TokenType::NUMBER})) {
        Token num = previous();
        if (std::holds_alternative<double>(num.literal)) {
            return std::make_unique<LiteralExpr>(std::get<double>(num.literal));
        }
        return std::make_unique<LiteralExpr>(0.0);
    }
    if (match({TokenType::STRING})) {
        Token str = previous();
        if (std::holds_alternative<std::string>(str.literal)) {
            return std::make_unique<LiteralExpr>(std::get<std::string>(str.literal));
        }
        return std::make_unique<LiteralExpr>(std::string(""));
    }
    if (match({TokenType::LPAREN})) {
        auto expr = expression();
        consume(TokenType::RPAREN, "Expect ')' after expression.");
        return std::make_unique<GroupingExpr>(std::move(expr));
    }
    if (match({TokenType::IDENTIFIER})) {
        return std::make_unique<VariableExpr>(previous());
    }

    throw error(peek(), "Expect expression.");
}

} // namespace minilang
