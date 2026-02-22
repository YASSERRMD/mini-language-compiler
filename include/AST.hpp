#pragma once

#include "Token.hpp"
#include <memory>
#include <vector>
#include <variant>

namespace minilang {

// Forward declarations
class Expr;
class Stmt;

/**
 * Expression types
 */
enum class ExprType {
    Binary,
    Unary,
    Literal,
    Variable,
    Assignment,
    Call,
    Grouping,
};

/**
 * Statement types
 */
enum class StmtType {
    Expression,
    Let,
    Function,
    If,
    While,
    Return,
    Print,
    Block,
};

/**
 * Base expression class using variant for performance
 */
class Expr {
public:
    virtual ~Expr() = default;

    template <typename Visitor>
    auto accept(Visitor& visitor) -> decltype(visitor.visit(nullptr)) {
        // Implemented in derived classes
        return acceptImpl(visitor);
    }

    virtual ExprType getType() const = 0;

protected:
    template <typename Visitor>
    auto acceptImpl(Visitor& visitor) -> decltype(visitor.visit(nullptr)) {
        throw std::runtime_error("acceptImpl not implemented");
    }
};

/**
 * Binary expression: a + b, a == b, etc.
 */
class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}

    ExprType getType() const override { return ExprType::Binary; }
};

/**
 * Unary expression: -a, !a
 */
class UnaryExpr : public Expr {
public:
    Token op;
    std::unique_ptr<Expr> right;

    UnaryExpr(Token o, std::unique_ptr<Expr> r) : op(std::move(o)), right(std::move(r)) {}

    ExprType getType() const override { return ExprType::Unary; }
};

/**
 * Literal expression: numbers, strings, booleans
 */
class LiteralExpr : public Expr {
public:
    std::variant<double, std::string, bool, std::monostate> value;

    explicit LiteralExpr(std::variant<double, std::string, bool, std::monostate> v)
        : value(std::move(v)) {}

    ExprType getType() const override { return ExprType::Literal; }
};

/**
 * Variable expression: identifier reference
 */
class VariableExpr : public Expr {
public:
    Token name;

    explicit VariableExpr(Token n) : name(std::move(n)) {}

    ExprType getType() const override { return ExprType::Variable; }
};

/**
 * Assignment expression: x = 5
 */
class AssignExpr : public Expr {
public:
    Token name;
    std::unique_ptr<Expr> value;

    AssignExpr(Token n, std::unique_ptr<Expr> v) : name(std::move(n)), value(std::move(v)) {}

    ExprType getType() const override { return ExprType::Assignment; }
};

/**
 * Function call expression: foo(a, b)
 */
class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> callee;
    Token paren;
    std::vector<std::unique_ptr<Expr>> arguments;

    CallExpr(std::unique_ptr<Expr> c, Token p, std::vector<std::unique_ptr<Expr>> a)
        : callee(std::move(c)), paren(std::move(p)), arguments(std::move(a)) {}

    ExprType getType() const override { return ExprType::Call; }
};

/**
 * Grouping expression: (expr)
 */
class GroupingExpr : public Expr {
public:
    std::unique_ptr<Expr> expression;

    explicit GroupingExpr(std::unique_ptr<Expr> e) : expression(std::move(e)) {}

    ExprType getType() const override { return ExprType::Grouping; }
};

/**
 * Base statement class
 */
class Stmt {
public:
    virtual ~Stmt() = default;
    virtual StmtType getType() const = 0;
};

/**
 * Expression statement
 */
class ExpressionStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;

    explicit ExpressionStmt(std::unique_ptr<Expr> e) : expression(std::move(e)) {}

    StmtType getType() const override { return StmtType::Expression; }
};

/**
 * Let statement: variable declaration
 */
class LetStmt : public Stmt {
public:
    Token name;
    std::unique_ptr<Expr> initializer;

    LetStmt(Token n, std::unique_ptr<Expr> i) : name(std::move(n)), initializer(std::move(i)) {}

    StmtType getType() const override { return StmtType::Let; }
};

/**
 * Function statement: function definition
 */
class FunctionStmt : public Stmt {
public:
    Token name;
    std::vector<Token> params;
    std::vector<std::unique_ptr<Stmt>> body;

    FunctionStmt(Token n, std::vector<Token> p, std::vector<std::unique_ptr<Stmt>> b)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)) {}

    StmtType getType() const override { return StmtType::Function; }
};

/**
 * If statement
 */
class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;

    IfStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> t, std::unique_ptr<Stmt> e)
        : condition(std::move(c)), thenBranch(std::move(t)), elseBranch(std::move(e)) {}

    StmtType getType() const override { return StmtType::If; }
};

/**
 * While statement
 */
class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;

    WhileStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> b)
        : condition(std::move(c)), body(std::move(b)) {}

    StmtType getType() const override { return StmtType::While; }
};

/**
 * Return statement
 */
class ReturnStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Expr> value;

    ReturnStmt(Token k, std::unique_ptr<Expr> v) : keyword(std::move(k)), value(std::move(v)) {}

    StmtType getType() const override { return StmtType::Return; }
};

/**
 * Print statement
 */
class PrintStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;

    explicit PrintStmt(std::unique_ptr<Expr> e) : expression(std::move(e)) {}

    StmtType getType() const override { return StmtType::Print; }
};

/**
 * Block statement
 */
class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;

    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> s) : statements(std::move(s)) {}

    StmtType getType() const override { return StmtType::Block; }
};

/**
 * Program is a list of statements
 */
using Program = std::vector<std::unique_ptr<Stmt>>;

} // namespace minilang
