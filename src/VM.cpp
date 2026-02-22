#include "VM.hpp"
#include <format>
#include <functional>

namespace minilang {

VM::VM() {
    // Pre-allocate stack for performance
    // m_stack.reserve(256); // std::stack doesn't have reserve
}

InterpretResult VM::interpret(const Chunk& chunk) {
    m_chunk = &chunk;
    m_ip = 0;
    m_error.clear();

    // Clear stack
    while (!m_stack.empty()) {
        m_stack.pop();
    }

    for (;;) {
        Instruction instruction = readInstruction();

        switch (instruction.opcode) {
            // Constants and literals
            case OpCode::OP_CONSTANT:
                push(m_chunk->constants[instruction.operand]);
                break;

            case OpCode::OP_NIL:
                push(Value());
                break;

            case OpCode::OP_TRUE:
                push(Value(true));
                break;

            case OpCode::OP_FALSE:
                push(Value(false));
                break;

            // Arithmetic
            case OpCode::OP_ADD: {
                Value b = pop();
                Value a = pop();
                if (a.isString() && b.isString()) {
                    push(Value(a.asString() + b.asString()));
                } else if (a.isNumber() && b.isNumber()) {
                    push(Value(a.asNumber() + b.asNumber()));
                } else {
                    runtimeError("Operands must be two numbers or two strings.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }

            case OpCode::OP_SUBTRACT: {
                Value b = pop();
                Value a = pop();
                if (!a.isNumber() || !b.isNumber()) {
                    runtimeError("Operands must be numbers.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                push(Value(a.asNumber() - b.asNumber()));
                break;
            }

            case OpCode::OP_MULTIPLY: {
                Value b = pop();
                Value a = pop();
                if (!a.isNumber() || !b.isNumber()) {
                    runtimeError("Operands must be numbers.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                push(Value(a.asNumber() * b.asNumber()));
                break;
            }

            case OpCode::OP_DIVIDE: {
                Value b = pop();
                Value a = pop();
                if (!a.isNumber() || !b.isNumber()) {
                    runtimeError("Operands must be numbers.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                if (b.asNumber() == 0.0) {
                    runtimeError("Division by zero.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                push(Value(a.asNumber() / b.asNumber()));
                break;
            }

            case OpCode::OP_MODULO: {
                Value b = pop();
                Value a = pop();
                if (!a.isNumber() || !b.isNumber()) {
                    runtimeError("Operands must be numbers.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                if (b.asNumber() == 0.0) {
                    runtimeError("Modulo by zero.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                push(Value(std::fmod(a.asNumber(), b.asNumber())));
                break;
            }

            case OpCode::OP_NEGATE: {
                Value value = pop();
                if (!value.isNumber()) {
                    runtimeError("Operand must be a number.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                push(Value(-value.asNumber()));
                break;
            }

            // Comparison
            case OpCode::OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(Value(valuesEqual(a, b)));
                break;
            }

            case OpCode::OP_LESS: {
                Value b = pop();
                Value a = pop();
                if (!a.isNumber() || !b.isNumber()) {
                    runtimeError("Operands must be numbers.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                push(Value(a.asNumber() < b.asNumber()));
                break;
            }

            case OpCode::OP_GREATER: {
                Value b = pop();
                Value a = pop();
                if (!a.isNumber() || !b.isNumber()) {
                    runtimeError("Operands must be numbers.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                push(Value(a.asNumber() > b.asNumber()));
                break;
            }

            // Logical
            case OpCode::OP_NOT: {
                Value value = pop();
                push(Value(isFalsey(value)));
                break;
            }

            case OpCode::OP_AND: {
                Value b = pop();
                Value a = pop();
                push(Value(!isFalsey(a) && !isFalsey(b)));
                break;
            }

            case OpCode::OP_OR: {
                Value b = pop();
                Value a = pop();
                push(Value(!isFalsey(a) || !isFalsey(b)));
                break;
            }

            // Variables
            case OpCode::OP_GET_LOCAL: {
                // In this simple VM, we use a flat stack
                // A full implementation would have proper frame handling
                runtimeError("Local variables not fully implemented.");
                return InterpretResult::RUNTIME_ERROR;
            }

            case OpCode::OP_SET_LOCAL: {
                runtimeError("Local variables not fully implemented.");
                return InterpretResult::RUNTIME_ERROR;
            }

            case OpCode::OP_POP:
                pop();
                break;

            // Control flow
            case OpCode::OP_JUMP:
                m_ip += instruction.operand;
                break;

            case OpCode::OP_JUMP_IF_FALSE: {
                Value value = peek();
                if (isFalsey(value)) {
                    m_ip += instruction.operand;
                }
                break;
            }

            case OpCode::OP_LOOP: {
                m_ip -= instruction.operand;
                break;
            }

            case OpCode::OP_CALL: {
                uint8_t argCount = instruction.operand;
                runtimeError("Function calls not fully implemented.");
                return InterpretResult::RUNTIME_ERROR;
            }

            case OpCode::OP_RETURN:
                // Return the top value or nil
                return InterpretResult::OK;

            // Built-in
            case OpCode::OP_PRINT: {
                Value value = pop();
                *m_output << valueToString(value) << std::endl;
                break;
            }

            default:
                runtimeError(std::format("Unknown opcode: {}", static_cast<int>(instruction.opcode)));
                return InterpretResult::RUNTIME_ERROR;
        }
    }

    return InterpretResult::OK;
}

void VM::push(Value value) {
    m_stack.push(std::move(value));
}

Value VM::pop() {
    if (m_stack.empty()) {
        runtimeError("Stack underflow.");
        return Value();
    }
    Value value = m_stack.top();
    m_stack.pop();
    return value;
}

Value VM::peek(size_t distance) {
    // This is a simplified version - a full implementation would properly
    // support peeking at arbitrary stack distances
    if (m_stack.empty()) {
        return Value();
    }
    return m_stack.top();
}

Instruction VM::readInstruction() {
    if (m_ip >= m_chunk->code.size()) {
        return Instruction(OpCode::OP_RETURN);
    }
    return m_chunk->code[m_ip++];
}

uint8_t VM::readByte() {
    return readInstruction().operand;
}

bool VM::isFalsey(const Value& value) {
    if (value.isNil()) return true;
    if (value.isBool()) return !value.asBool();
    if (value.isNumber()) return value.asNumber() == 0.0;
    return false;
}

bool VM::valuesEqual(const Value& a, const Value& b) {
    if (a.type != b.type) return false;

    switch (a.type) {
        case ValueType::NIL:
            return true;
        case ValueType::BOOL:
            return a.asBool() == b.asBool();
        case ValueType::NUMBER:
            return a.asNumber() == b.asNumber();
        case ValueType::STRING:
            return a.asString() == b.asString();
    }

    return false;
}

void VM::concatenate() {
    Value b = pop();
    Value a = pop();
    push(Value(a.asString() + b.asString()));
}

void VM::runtimeError(const std::string& message) {
    m_error = message;
}

std::string VM::valueToString(const Value& value) {
    switch (value.type) {
        case ValueType::NIL:
            return "nil";
        case ValueType::BOOL:
            return value.asBool() ? "true" : "false";
        case ValueType::NUMBER: {
            std::string s = std::format("{}", value.asNumber());
            // Remove trailing zeros
            size_t dot = s.find('.');
            if (dot != std::string::npos) {
                size_t last_non_zero = s.find_last_not_of('0');
                if (last_non_zero != std::string::npos && last_non_zero > dot) {
                    s.erase(last_non_zero + 1);
                }
                if (s.back() == '.') {
                    s.pop_back();
                }
            }
            return s;
        }
        case ValueType::STRING:
            return value.asString();
    }
    return "unknown";
}

} // namespace minilang
