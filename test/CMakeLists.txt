enable_testing()

include(CTest)

add_executable(TestExpression test_expression.c ${TESTING_SOURCES})
target_include_directories(TestExpression PRIVATE ${INCLUDE_DIRECTORIES})
target_compile_options(TestExpression PRIVATE ${COMPILE_OPTIONS})
add_test(NAME TestExpression COMMAND TestExpression)

add_custom_target(RunTests
    ctest --verbose --progress --output-on-failure --timeout 1
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM USES_TERMINAL
)
