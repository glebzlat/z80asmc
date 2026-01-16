enable_testing()

include(CTest)

function(add_test_exe test_name)
    add_executable(${test_name} ${ARGN})
    target_include_directories(${test_name} PRIVATE ${INCLUDE_DIRECTORIES})
    target_compile_options(${test_name} PRIVATE ${COMPILE_OPTIONS})
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_test_exe(TestExpressionPositive test_expression_positive.c ${TESTING_SOURCES})
add_test_exe(TestExpressionNegative test_expression_negative.c ${TESTING_SOURCES})
add_test_exe(TestLexerPositive test_lexer_positive.c ${TESTING_SOURCES})

add_custom_target(RunTests
    ctest --verbose --progress --output-on-failure --timeout 1
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM USES_TERMINAL
)
