#pragma once

#include "IRGenerator.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "VM.hpp"
#include <string>

namespace minilang {

/**
 * Top-level compiler that orchestrates the compilation pipeline
 * Source -> Lexer -> Tokens -> Parser -> AST -> IR Generator -> Bytecode -> VM
 */
class Compiler {
public:
    Compiler();
    ~Compiler() = default;

    /**
     * Compile and run source code
     */
    InterpretResult run(const std::string& source);

    /**
     * Compile source code and return bytecode
     */
    Chunk compile(const std::string& source);

    /**
     * Run pre-compiled bytecode
     */
    InterpretResult run(const Chunk& chunk);

    /**
     * Get the last error message
     */
    const std::string& getError() const { return m_error; }

    /**
     * Check if there was an error
     */
    bool hadError() const { return !m_error.empty(); }

private:
    std::string m_error;
    Lexer* m_lexer = nullptr;
    Parser* m_parser = nullptr;
    IRGenerator* m_irgen = nullptr;
    VM* m_vm = nullptr;
};

} // namespace minilang
