#include "Compiler.hpp"
#include <fstream>
#include <iostream>
#include <string>

namespace minilang {

/**
 * Run a source file
 */
static bool runFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << path << "'" << std::endl;
        return false;
    }

    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    Compiler compiler;
    InterpretResult result = compiler.run(source);

    if (result == InterpretResult::COMPILE_ERROR) {
        std::cerr << "Compile Error: " << compiler.getError() << std::endl;
        return false;
    }

    if (result == InterpretResult::RUNTIME_ERROR) {
        std::cerr << "Runtime Error: " << compiler.getError() << std::endl;
        return false;
    }

    return true;
}

/**
 * Interactive REPL
 */
static void repl() {
    std::cout << "MiniLang v1.0.0 - C++ Compiler" << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    std::cout << std::endl;

    Compiler compiler;
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            break;
        }

        if (line.empty()) {
            continue;
        }

        compiler.run(line);

        if (compiler.hadError()) {
            std::cerr << "Error: " << compiler.getError() << std::endl;
        }
    }
}

} // namespace minilang

int main(int argc, char* argv[]) {
    using namespace minilang;

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        std::string path = argv[1];
        if (!runFile(path)) {
            return 1;
        }
    } else {
        std::cerr << "Usage: " << argv[0] << " [file]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "  If no file is specified, starts interactive REPL." << std::endl;
        return 1;
    }

    return 0;
}
