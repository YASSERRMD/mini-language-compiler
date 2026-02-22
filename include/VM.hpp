#pragma once

#include "IRGenerator.hpp"
#include <iostream>
#include <stack>
#include <string>
#include <vector>

namespace minilang {

/**
 * Interpret result
 */
enum class InterpretResult : uint8_t {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR,
};

/**
 * Virtual Machine for executing bytecode
 * Fast stack-based VM optimized for execution speed
 */
class VM {
public:
    VM();
    ~VM() = default;

    /**
     * Interpret a chunk of bytecode
     */
    InterpretResult interpret(const Chunk& chunk);

    /**
     * Get the last error message
     */
    const std::string& getError() const { return m_error; }

    /**
     * Set output stream for print statements
     */
    void setOutput(std::ostream& output) { m_output = &output; }

private:
    std::stack<Value> m_stack;
    size_t m_ip = 0; // Instruction pointer
    const Chunk* m_chunk = nullptr;
    std::string m_error;
    std::ostream* m_output = &std::cout;

    // Stack operations
    void push(Value value);
    Value pop();
    Value peek(size_t distance = 0);
    size_t stackSize() const { return m_stack.size(); }

    // Instruction fetching
    Instruction readInstruction();
    uint8_t readByte();

    // Operations
    bool isFalsey(const Value& value);
    bool valuesEqual(const Value& a, const Value& b);
    void concatenate();
    void runtimeError(const std::string& message);

    // Binary operations
    template <typename Op>
    bool binaryOp(Op op);

    // Debug
    void dumpStack();
    std::string valueToString(const Value& value);
};

} // namespace minilang
