#include "Compiler.hpp"

namespace minilang {

Compiler::Compiler() {
    m_lexer = nullptr;
    m_parser = nullptr;
    m_irgen = nullptr;
    m_vm = new VM();
}

InterpretResult Compiler::run(const std::string& source) {
    Chunk chunk = compile(source);
    if (hadError()) {
        return InterpretResult::COMPILE_ERROR;
    }
    return run(chunk);
}

Chunk Compiler::compile(const std::string& source) {
    m_error.clear();

    // Lexical analysis
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    // Check for lexer errors
    for (const auto& token : tokens) {
        if (token.type == TokenType::ERROR) {
            m_error = std::format("[Line {}] Lexer Error: {}", token.line, token.lexeme);
            return Chunk();
        }
    }

    // Parsing
    Parser parser(tokens);
    Program program = parser.parse();

    // Check for parse errors (parser synchronizes and continues)
    // In a full implementation, we'd collect all errors

    // IR Generation
    IRGenerator irgen;
    Chunk chunk = irgen.compile(program);

    if (irgen.hadError()) {
        m_error = irgen.getError();
        return Chunk();
    }

    return chunk;
}

InterpretResult Compiler::run(const Chunk& chunk) {
    if (!m_vm) {
        m_error = "VM not initialized";
        return InterpretResult::RUNTIME_ERROR;
    }

    InterpretResult result = m_vm->interpret(chunk);
    if (result != InterpretResult::OK) {
        m_error = m_vm->getError();
    }

    return result;
}

} // namespace minilang
