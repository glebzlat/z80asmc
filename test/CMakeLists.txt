enable_testing()

include(CTest)

add_executable(TestExpressionPositive test_expression_positive.c ${TESTING_SOURCES})
target_include_directories(TestExpressionPositive PRIVATE ${INCLUDE_DIRECTORIES})
target_compile_options(TestExpressionPositive PRIVATE ${COMPILE_OPTIONS})
add_test(NAME TestExpressionPositive COMMAND TestExpressionPositive)

add_custom_target(RunTests
    ctest --verbose --progress --output-on-failure --timeout 1
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM USES_TERMINAL
)
