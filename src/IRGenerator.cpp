#include "IRGenerator.hpp"
#include <format>

namespace minilang {

IRGenerator::IRGenerator() {
    // Reserve space for locals
    m_locals.reserve(256);
}

Chunk IRGenerator::compile(const Program& program) {
    m_hadError = false;
    m_error.clear();
    m_chunk = Chunk();
    m_locals.clear();
    m_scopeDepth = 0;

    beginScope();

    for (const auto& stmt : program) {
        compileStmt(stmt.get());
        if (m_hadError) {
            return m_chunk;
        }
    }

    endScope();
    emitByte(OpCode::OP_RETURN);
    return m_chunk;
}

Chunk IRGenerator::compileExpression(std::unique_ptr<Expr> expr) {
    m_hadError = false;
    m_error.clear();
    m_chunk = Chunk();
    m_locals.clear();
    m_scopeDepth = 0;

    beginScope();
    compileExpr(expr.get());
    endScope();
    emitByte(OpCode::OP_RETURN);

    return m_chunk;
}

void IRGenerator::beginScope() {
    m_scopeDepth++;
}

void IRGenerator::endScope() {
    m_scopeDepth--;

    // Pop locals from this scope
    while (!m_locals.empty() && m_locals.back().depth > m_scopeDepth) {
        emitByte(OpCode::OP_POP); // We need to add OP_POP
        m_locals.pop_back();
    }
}

void IRGenerator::declareVariable(const std::string& name) {
    if (m_scopeDepth == 0) return;

    // Check for duplicate in current scope
    for (auto it = m_locals.rbegin(); it != m_locals.rend(); ++it) {
        if (it->depth != m_scopeDepth) break;
        if (it->name == name) {
            error(std::format("Variable '{}' already declared in this scope.", name));
            return;
        }
    }

    m_locals.push_back({name, m_scopeDepth, false});
}

int IRGenerator::resolveLocal(const std::string& name) {
    for (int i = static_cast<int>(m_locals.size()) - 1; i >= 0; i--) {
        if (m_locals[i].name == name) {
            return i;
        }
    }
    return -1; // Not found, treat as global
}

void IRGenerator::markInitialized() {
    if (m_locals.empty()) return;
    m_locals.back().depth = m_scopeDepth;
}

void IRGenerator::emitByte(OpCode op, uint8_t operand) {
    m_chunk.write(op, 0, operand);
}

void IRGenerator::emitJump(OpCode op) {
    m_chunk.write(op, 0, 255); // Placeholder
}

void IRGenerator::emitLoop(size_t loopStart) {
    size_t offset = m_chunk.code.size() - loopStart;
    if (offset > 255) {
        error("Loop body too large.");
        return;
    }
    m_chunk.write(OpCode::OP_LOOP, 0, static_cast<uint8_t>(offset));
}

void IRGenerator::patchJump(size_t offset) {
    size_t jump = m_chunk.code.size() - 1 - offset;
    if (jump > 255) {
        error("Jump too far.");
        return;
    }
    m_chunk.code[offset].operand = static_cast<uint8_t>(jump);
}

void IRGenerator::compileExpr(Expr* expr) {
    if (!expr) {
        emitByte(OpCode::OP_NIL);
        return;
    }

    switch (expr->getType()) {
        case ExprType::Binary:
            compileBinaryExpr(static_cast<BinaryExpr*>(expr));
            break;
        case ExprType::Unary:
            compileUnaryExpr(static_cast<UnaryExpr*>(expr));
            break;
        case ExprType::Literal:
            compileLiteralExpr(static_cast<LiteralExpr*>(expr));
            break;
        case ExprType::Variable:
            compileVariableExpr(static_cast<VariableExpr*>(expr));
            break;
        case ExprType::Assignment:
            compileAssignExpr(static_cast<AssignExpr*>(expr));
            break;
        case ExprType::Call:
            compileCallExpr(static_cast<CallExpr*>(expr));
            break;
        case ExprType::Grouping:
            compileGroupingExpr(static_cast<GroupingExpr*>(expr));
            break;
    }
}

void IRGenerator::compileBinaryExpr(BinaryExpr* expr) {
    compileExpr(expr->left.get());
    compileExpr(expr->right.get());

    switch (expr->op.type) {
        case TokenType::PLUS: emitByte(OpCode::OP_ADD); break;
        case TokenType::MINUS: emitByte(OpCode::OP_SUBTRACT); break;
        case TokenType::STAR: emitByte(OpCode::OP_MULTIPLY); break;
        case TokenType::SLASH: emitByte(OpCode::OP_DIVIDE); break;
        case TokenType::PERCENT: emitByte(OpCode::OP_MODULO); break;

        case TokenType::EQUAL_EQUAL: emitByte(OpCode::OP_EQUAL); break;
        case TokenType::BANG_EQUAL: emitByte(OpCode::OP_EQUAL); emitByte(OpCode::OP_NOT); break;
        case TokenType::LESS: emitByte(OpCode::OP_LESS); break;
        case TokenType::LESS_EQUAL: emitByte(OpCode::OP_GREATER); emitByte(OpCode::OP_NOT); break;
        case TokenType::GREATER: emitByte(OpCode::OP_GREATER); break;
        case TokenType::GREATER_EQUAL: emitByte(OpCode::OP_LESS); emitByte(OpCode::OP_NOT); break;

        case TokenType::AND: emitByte(OpCode::OP_AND); break;
        case TokenType::OR: emitByte(OpCode::OP_OR); break;

        default:
            error(std::format("Unknown binary operator: {}", expr->op.lexeme));
            break;
    }
}

void IRGenerator::compileUnaryExpr(UnaryExpr* expr) {
    compileExpr(expr->right.get());

    switch (expr->op.type) {
        case TokenType::MINUS: emitByte(OpCode::OP_NEGATE); break;
        case TokenType::BANG: emitByte(OpCode::OP_NOT); break;
        default:
            error(std::format("Unknown unary operator: {}", expr->op.lexeme));
            break;
    }
}

void IRGenerator::compileLiteralExpr(LiteralExpr* expr) {
    if (std::holds_alternative<double>(expr->value)) {
        m_chunk.writeConstant(Value(std::get<double>(expr->value)), 0);
    } else if (std::holds_alternative<std::string>(expr->value)) {
        m_chunk.writeConstant(Value(std::get<std::string>(expr->value)), 0);
    } else if (std::holds_alternative<bool>(expr->value)) {
        if (std::get<bool>(expr->value)) {
            emitByte(OpCode::OP_TRUE);
        } else {
            emitByte(OpCode::OP_FALSE);
        }
    } else {
        emitByte(OpCode::OP_NIL);
    }
}

void IRGenerator::compileVariableExpr(VariableExpr* expr) {
    int local = resolveLocal(expr->name.lexeme);
    if (local != -1) {
        emitByte(OpCode::OP_GET_LOCAL, static_cast<uint8_t>(local));
    } else {
        // Global variable (not implemented in this simple version)
        error(std::format("Undefined variable: {}", expr->name.lexeme));
    }
}

void IRGenerator::compileAssignExpr(AssignExpr* expr) {
    compileExpr(expr->value.get());

    int local = resolveLocal(expr->name.lexeme);
    if (local != -1) {
        emitByte(OpCode::OP_SET_LOCAL, static_cast<uint8_t>(local));
    } else {
        error(std::format("Undefined variable: {}", expr->name.lexeme));
    }
}

void IRGenerator::compileCallExpr(CallExpr* expr) {
    compileExpr(expr->callee.get());

    for (const auto& arg : expr->arguments) {
        compileExpr(arg.get());
    }

    emitByte(OpCode::OP_CALL, static_cast<uint8_t>(expr->arguments.size()));
}

void IRGenerator::compileGroupingExpr(GroupingExpr* expr) {
    compileExpr(expr->expression.get());
}

void IRGenerator::compileStmt(Stmt* stmt) {
    if (!stmt) return;

    switch (stmt->getType()) {
        case StmtType::Expression:
            compileExpressionStmt(static_cast<ExpressionStmt*>(stmt));
            break;
        case StmtType::Let:
            compileLetStmt(static_cast<LetStmt*>(stmt));
            break;
        case StmtType::Function:
            compileFunctionStmt(static_cast<FunctionStmt*>(stmt));
            break;
        case StmtType::If:
            compileIfStmt(static_cast<IfStmt*>(stmt));
            break;
        case StmtType::While:
            compileWhileStmt(static_cast<WhileStmt*>(stmt));
            break;
        case StmtType::Return:
            compileReturnStmt(static_cast<ReturnStmt*>(stmt));
            break;
        case StmtType::Print:
            compilePrintStmt(static_cast<PrintStmt*>(stmt));
            break;
        case StmtType::Block:
            compileBlockStmt(static_cast<BlockStmt*>(stmt));
            break;
    }
}

void IRGenerator::compileExpressionStmt(ExpressionStmt* stmt) {
    compileExpr(stmt->expression.get());
    emitByte(OpCode::OP_POP); // Discard result
}

void IRGenerator::compileLetStmt(LetStmt* stmt) {
    declareVariable(stmt->name.lexeme);

    if (stmt->initializer) {
        compileExpr(stmt->initializer.get());
    } else {
        emitByte(OpCode::OP_NIL);
    }

    markInitialized();
}

void IRGenerator::compileFunctionStmt(FunctionStmt* stmt) {
    declareVariable(stmt->name.lexeme);
    markInitialized();

    // In a full implementation, we'd compile the function body separately
    // For now, just push nil as placeholder
    emitByte(OpCode::OP_NIL);
}

void IRGenerator::compileIfStmt(IfStmt* stmt) {
    compileExpr(stmt->condition.get());
    emitJump(OpCode::OP_JUMP_IF_FALSE);
    size_t thenJump = m_chunk.code.size() - 1;

    compileStmt(stmt->thenBranch.get());
    emitJump(OpCode::OP_JUMP);
    size_t elseJump = m_chunk.code.size() - 1;

    patchJump(thenJump);

    if (stmt->elseBranch) {
        compileStmt(stmt->elseBranch.get());
    }

    patchJump(elseJump);
}

void IRGenerator::compileWhileStmt(WhileStmt* stmt) {
    size_t loopStart = m_chunk.code.size();

    compileExpr(stmt->condition.get());
    emitJump(OpCode::OP_JUMP_IF_FALSE);
    size_t exitJump = m_chunk.code.size() - 1;

    compileStmt(stmt->body.get());
    emitLoop(loopStart);

    patchJump(exitJump);
}

void IRGenerator::compileReturnStmt(ReturnStmt* stmt) {
    if (stmt->value) {
        compileExpr(stmt->value.get());
    } else {
        emitByte(OpCode::OP_NIL);
    }
    emitByte(OpCode::OP_RETURN);
}

void IRGenerator::compilePrintStmt(PrintStmt* stmt) {
    compileExpr(stmt->expression.get());
    emitByte(OpCode::OP_PRINT);
}

void IRGenerator::compileBlockStmt(BlockStmt* stmt) {
    beginScope();
    for (const auto& s : stmt->statements) {
        compileStmt(s.get());
    }
    endScope();
}

void IRGenerator::error(const std::string& message) {
    m_hadError = true;
    m_error = message;
}

} // namespace minilang
