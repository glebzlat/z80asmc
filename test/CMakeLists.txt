enable_testing()

include(CTest)

add_executable(TestExpression test_expression.c ${TESTING_SOURCES})
target_include_directories(TestExpression PRIVATE ${INCLUDE_DIRECTORIES})
target_compile_options(TestExpression PRIVATE ${COMPILE_OPTIONS})
add_test(NAME TestExpression COMMAND TestExpression)
