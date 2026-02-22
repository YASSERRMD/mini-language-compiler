#include "Compiler.hpp"
#include <iostream>
#include <string>

using namespace minilang;

void testArithmetic() {
    std::cout << "Testing arithmetic..." << std::endl;

    Compiler compiler;
    compiler.run("print 1 + 2 * 3;");

    if (compiler.hadError()) {
        std::cerr << "  FAILED: " << compiler.getError() << std::endl;
    } else {
        std::cout << "  PASSED" << std::endl;
    }
}

void testComparison() {
    std::cout << "Testing comparison..." << std::endl;

    Compiler compiler;
    compiler.run("print 5 > 3;");

    if (compiler.hadError()) {
        std::cerr << "  FAILED: " << compiler.getError() << std::endl;
    } else {
        std::cout << "  PASSED" << std::endl;
    }
}

void testStrings() {
    std::cout << "Testing strings..." << std::endl;

    Compiler compiler;
    compiler.run("print \"Hello\" + \" \" + \"World\";");

    if (compiler.hadError()) {
        std::cerr << "  FAILED: " << compiler.getError() << std::endl;
    } else {
        std::cout << "  PASSED" << std::endl;
    }
}

void testIfStatement() {
    std::cout << "Testing if statement..." << std::endl;

    Compiler compiler;
    compiler.run("if (true) { print \"yes\"; } else { print \"no\"; }");

    if (compiler.hadError()) {
        std::cerr << "  FAILED: " << compiler.getError() << std::endl;
    } else {
        std::cout << "  PASSED" << std::endl;
    }
}

void testWhileLoop() {
    std::cout << "Testing while loop..." << std::endl;

    Compiler compiler;
    compiler.run("let x = 0; while (x < 3) { print x; x = x + 1; }");

    if (compiler.hadError()) {
        std::cerr << "  FAILED: " << compiler.getError() << std::endl;
    } else {
        std::cout << "  PASSED" << std::endl;
    }
}

void testLogical() {
    std::cout << "Testing logical operators..." << std::endl;

    Compiler compiler;
    compiler.run("print true && false;");
    compiler.run("print true || false;");
    compiler.run("print !true;");

    if (compiler.hadError()) {
        std::cerr << "  FAILED: " << compiler.getError() << std::endl;
    } else {
        std::cout << "  PASSED" << std::endl;
    }
}

int main() {
    std::cout << "=== MiniLang Basic Tests ===" << std::endl << std::endl;

    testArithmetic();
    testComparison();
    testStrings();
    testIfStatement();
    testWhileLoop();
    testLogical();

    std::cout << std::endl << "=== All Tests Complete ===" << std::endl;
    return 0;
}
