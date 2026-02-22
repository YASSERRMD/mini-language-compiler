# Basic test setup
add_executable(test_basic
    test_basic.cpp
)

target_include_directories(test_basic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(test_basic PRIVATE)

add_test(NAME BasicTests COMMAND test_basic)
