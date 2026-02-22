# Mini Language Compiler

A fast, lightweight programming language compiler written in C++20. Designed for quick compilation and execution, making it suitable for LLM code generation and execution.

## Architecture

The compiler follows a classic multi-stage pipeline:

```
Source Code → Lexer → Tokens → Parser → AST → IR Generator → Bytecode → VM
```

### Components

- **Lexer** ([Lexer.hpp](include/Lexer.hpp), [Lexer.cpp](src/Lexer.cpp)): Tokenizes source code into a stream of tokens
- **Parser** ([Parser.hpp](include/Parser.hpp), [Parser.cpp](src/Parser.cpp)): Recursive descent parser that builds an Abstract Syntax Tree (AST)
- **IR Generator** ([IRGenerator.hpp](include/IRGenerator.hpp), [IRGenerator.cpp](src/IRGenerator.cpp)): Compiles AST to optimized bytecode
- **VM** ([VM.hpp](include/VM.hpp), [VM.cpp](src/VM.cpp)): Stack-based virtual machine for bytecode execution
- **Compiler** ([Compiler.hpp](include/Compiler.hpp), [Compiler.cpp](src/Compiler.cpp)): Top-level orchestration

## Building

### Requirements

- C++20 compatible compiler (g++ 11+, clang++ 13+)
- CMake 3.20+

### Build Steps

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Usage

### Command Line

```bash
# Run a source file
./build/minilang examples/fibonacci.mini

# Start interactive REPL
./build/minilang
```

### Language Syntax

#### Variables
```cpp
let x = 42;
let name = "Hello";
let flag = true;
```

#### Arithmetic
```cpp
let result = (10 + 5) * 2 - 3;
let remainder = 17 % 5;
```

#### Comparison & Logical
```cpp
if (x > 10 && y < 20) {
    print "Condition met";
}

if (!(a == b) || c != d) {
    print "Another condition";
}
```

#### Loops
```cpp
let i = 0;
while (i < 10) {
    print i;
    i = i + 1;
}
```

#### Functions
```cpp
fn add(a, b) {
    return a + b;
}

fn factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

print add(5, 3);
print factorial(5);
```

## Bytecode Design

The VM uses a compact bytecode format with 8-bit opcodes:

| Opcode | Description |
|--------|-------------|
| `OP_CONSTANT` | Push constant onto stack |
| `OP_ADD` | Binary addition |
| `OP_SUBTRACT` | Binary subtraction |
| `OP_MULTIPLY` | Binary multiplication |
| `OP_DIVIDE` | Binary division |
| `OP_JUMP_IF_FALSE` | Conditional jump |
| `OP_JUMP` | Unconditional jump |
| `OP_LOOP` | Loop back |
| `OP_CALL` | Function call |
| `OP_RETURN` | Return from function |

## Running Tests

```bash
cd build
ctest
```

Or run directly:
```bash
./build/test_basic
```

## Performance Considerations

- **Fast compilation**: No LLVM dependency, direct bytecode generation
- **Stack-based VM**: Simple execution model, easy to optimize
- **C++20**: Uses modern C++ features for zero-cost abstractions
- **Memory efficient**: Minimal allocations in hot paths

## License

MIT License - see [LICENSE](LICENSE) file.
