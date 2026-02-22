#pragma once

#include "AST.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace minilang {

/**
 * Bytecode opcodes for the Mini language VM
 */
enum class OpCode : uint8_t {
    // Constants and literals
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,

    // Arithmetic
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,
    OP_NEGATE,

    // Comparison
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL,

    // Logical
    OP_NOT,
    OP_AND,
    OP_OR,

    // Variables
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,

    // Stack
    OP_POP,

    // Control flow
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_RETURN,

    // Built-in
    OP_PRINT,
};

/**
 * Value types in the VM
 */
enum class ValueType : uint8_t {
    NIL,
    BOOL,
    NUMBER,
    STRING,
};

/**
 * Runtime value
 */
struct Value {
    ValueType type;
    std::variant<std::monostate, bool, double, std::string> as;

    Value() : type(ValueType::NIL), as(std::monostate{}) {}
    explicit Value(bool b) : type(ValueType::BOOL), as(b) {}
    explicit Value(double n) : type(ValueType::NUMBER), as(n) {}
    explicit Value(std::string s) : type(ValueType::STRING), as(std::move(s)) {}

    bool isBool() const { return type == ValueType::BOOL; }
    bool isNumber() const { return type == ValueType::NUMBER; }
    bool isString() const { return type == ValueType::STRING; }
    bool isNil() const { return type == ValueType::NIL; }

    bool asBool() const { return std::get<bool>(as); }
    double asNumber() const { return std::get<double>(as); }
    const std::string& asString() const { return std::get<std::string>(as); }
};

/**
 * Bytecode instruction (8-bit opcode + optional operand)
 */
struct Instruction {
    OpCode opcode;
    uint8_t operand; // For jump offsets, local indices, etc.
    Value constant;  // For OP_CONSTANT

    Instruction(OpCode op, uint8_t ops = 0, Value cons = Value())
        : opcode(op), operand(ops), constant(std::move(cons)) {}
};

/**
 * Chunk of bytecode
 */
struct Chunk {
    std::vector<Instruction> code;
    std::vector<size_t> lines; // Debug info
    std::vector<Value> constants;

    void write(OpCode op, size_t line, uint8_t operand = 0) {
        code.emplace_back(op, operand);
        lines.push_back(line);
    }

    void writeConstant(Value constant, size_t line) {
        size_t index = addConstant(std::move(constant));
        write(OpCode::OP_CONSTANT, line, static_cast<uint8_t>(index));
    }

    size_t addConstant(Value value) {
        constants.push_back(std::move(value));
        return constants.size() - 1;
    }
};

/**
 * Local variable in a scope
 */
struct Local {
    std::string name;
    size_t depth;
    bool isCaptured;
};

/**
 * Compiler state
 */
enum class CompilerState : uint8_t {
    SCRIPT,
    FUNCTION,
};

/**
 * IR Generator - Compiles AST to bytecode
 */
class IRGenerator {
public:
    IRGenerator();
    ~IRGenerator() = default;

    /**
     * Compile a program to bytecode
     */
    Chunk compile(const Program& program);

    /**
     * Compile a single expression (for REPL)
     */
    Chunk compileExpression(std::unique_ptr<Expr> expr);

    /**
     * Get the last error message
     */
    const std::string& getError() const { return m_error; }

    /**
     * Check if compilation was successful
     */
    bool hadError() const { return m_hadError; }

private:
    Chunk m_chunk;
    std::vector<Local> m_locals;
    size_t m_scopeDepth = 0;
    bool m_hadError = false;
    std::string m_error;

    // Scope management
    void beginScope();
    void endScope();

    // Local variable management
    void declareVariable(const std::string& name);
    int resolveLocal(const std::string& name);
    void markInitialized();

    // Bytecode emission
    void emitByte(OpCode op, uint8_t operand = 0);
    void emitJump(OpCode op);
    void emitLoop(size_t loopStart);
    void patchJump(size_t offset);

    // Expression compilation
    void compileExpr(Expr* expr);
    void compileBinaryExpr(BinaryExpr* expr);
    void compileUnaryExpr(UnaryExpr* expr);
    void compileLiteralExpr(LiteralExpr* expr);
    void compileVariableExpr(VariableExpr* expr);
    void compileAssignExpr(AssignExpr* expr);
    void compileCallExpr(CallExpr* expr);
    void compileGroupingExpr(GroupingExpr* expr);

    // Statement compilation
    void compileStmt(Stmt* stmt);
    void compileExpressionStmt(ExpressionStmt* stmt);
    void compileLetStmt(LetStmt* stmt);
    void compileFunctionStmt(FunctionStmt* stmt);
    void compileIfStmt(IfStmt* stmt);
    void compileWhileStmt(WhileStmt* stmt);
    void compileReturnStmt(ReturnStmt* stmt);
    void compilePrintStmt(PrintStmt* stmt);
    void compileBlockStmt(BlockStmt* stmt);

    // Error handling
    void error(const std::string& message);
};

} // namespace minilang
