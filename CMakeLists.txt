cmake_minimum_required(VERSION 3.20)
project(MiniLangCompiler VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source files
set(SOURCES
    src/Lexer.cpp
    src/Parser.cpp
    src/IRGenerator.cpp
    src/VM.cpp
    src/Compiler.cpp
    src/main.cpp
)

# Header files
set(HEADERS
    include/Token.hpp
    include/Lexer.hpp
    include/AST.hpp
    include/Parser.hpp
    include/IRGenerator.hpp
    include/VM.hpp
    include/Compiler.hpp
)

# Create executable
add_executable(minilang ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(minilang PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Compiler warnings
target_compile_options(minilang PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

# Enable tests
enable_testing()
add_subdirectory(tests)
