cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(z80asmc LANGUAGES C)

option(Z80ASMC_TESTING "Compile and run tests" OFF)

set(TESTING_SOURCES
    src/lexer.c
    src/parser.c
    src/utility.c
    src/vector.c
    src/map.c
    src/instruction.c
    src/string_lexer.c
    src/token.c
    src/expr_parser.c
    src/expr_parser_builder.c
    src/expression/expr_parser.c
    src/expression/expr_parser_builder.c
)

set(INCLUDE_DIRECTORIES
    src
)

list(TRANSFORM TESTING_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
list(TRANSFORM INCLUDE_DIRECTORIES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

set(SOURCES src/main.c ${TESTING_SOURCES})

set(COMPILE_OPTIONS
    -Wall
    -Wpedantic
    -Wextra
    -Wstrict-prototypes
    -Wwrite-strings
    -Wshadow
    -Winit-self
    -Wcast-align
    -Wmissing-prototypes
    -Wstrict-overflow=3
    -Wcast-qual
    -Wswitch-enum
    -Wconversion
    -fstack-protector-strong
    -Wdouble-promotion
    -Wformat-overflow
    -Wparentheses
)
set(LINK_OPTIONS)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRECTORIES})
target_compile_options(${PROJECT_NAME} PUBLIC ${COMPILE_OPTIONS})
target_link_options(${PROJECT_NAME} PUBLIC ${LINK_OPTIONS})

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_EXTENSIONS OFF)

if (Z80ASMC_TESTING)
    add_subdirectory(test)
endif()

find_program(CMAKE_C_CPPCHECK NAMES cppcheck)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/cppcheck")

add_custom_target(RunCppcheck
    ${CMAKE_C_CPPCHECK}
        --project=${PROJECT_SOURCE_DIR}/project.cppcheck
        --error-exitcode=1
    DEPENDS ${CMAKE_C_CPPCHECK}
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND COMPILE_OPTIONS -g)

    set(SANITIZER_OPTIONS
        -fno-omit-frame-pointer
        -fsanitize=undefined
        -fsanitize=signed-integer-overflow
        -fsanitize=float-cast-overflow
        -fsanitize-address-use-after-scope
        -fno-sanitize-recover
    )

    list(APPEND COMPILE_OPTIONS ${SANITIZER_OPTIONS})
    list(APPEND LINK_OPTIONS ${SANITIZER_OPTIONS})
endif()
