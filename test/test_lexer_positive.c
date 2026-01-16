#include <stdlib.h>

#include <lexer.h>

#include "common.h"

typedef struct {
  char const* lit;
  TokenType type;
} ClueToken;

static int testLexer(char const* str, int n_tokens, ClueToken const* tok_arr);

int main(void) {
  int tests_failed = 0;

  {
    ClueToken tokens[] = {{.type = TOKEN_END}};
    TEST_CASE(testLexer("", 1, tokens));
  }

  {
    ClueToken tokens[] = {{.type = TOKEN_END}};
    TEST_CASE(testLexer("; a comment", 1, tokens));
  }

  {
    ClueToken tokens[] = {{.lit = "a", .type = TOKEN_ID}, {.type = TOKEN_END}};
    TEST_CASE(testLexer(";a comment before\na;a comment after", 2, tokens));
  }

  {
    ClueToken tokens[] = {{.lit = "a", .type = TOKEN_CHAR}, {.type = TOKEN_END}};
    TEST_CASE(testLexer("'a'", 2, tokens));
  }

  {
    ClueToken tokens[] = {{.lit = "31415", .type = TOKEN_HEXADECIMAL}, {.type = TOKEN_END}};
    TEST_CASE(testLexer("0x31415", 2, tokens));
  }

  {
    ClueToken tokens[] = {{.lit = "31415", .type = TOKEN_HEXADECIMAL}, {.type = TOKEN_END}};
    TEST_CASE(testLexer("0X31415", 2, tokens));
  }

  {
    ClueToken tokens[] = {{.lit = "31415abc", .type = TOKEN_HEXADECIMAL}, {.type = TOKEN_END}};
    TEST_CASE(testLexer("$31415abc", 2, tokens));
  }

  {
    ClueToken tokens[] = {{.lit = "31415abc", .type = TOKEN_HEXADECIMAL}, {.type = TOKEN_END}};
    TEST_CASE(testLexer("#31415abc", 2, tokens));
  }

  {
    ClueToken tokens[] = {{.lit = "42", .type = TOKEN_OCTAL}, {.type = TOKEN_END}};
    TEST_CASE(testLexer("42o", 2, tokens));
  }

  {
    ClueToken tokens[] = {{.lit = "42", .type = TOKEN_OCTAL}, {.type = TOKEN_END}};
    TEST_CASE(testLexer("42q", 2, tokens));
  }

  // XXX
  // {
  //   ClueToken tokens[] = {{.lit = "010101", .type = TOKEN_BINARY}, {.type = TOKEN_END}};
  //   TEST_CASE(testLexer("0b010101", 2, tokens));
  // }

  {
    ClueToken tokens[] = {{.lit = "010101", .type = TOKEN_BINARY}, {.type = TOKEN_END}};
    TEST_CASE(testLexer("0B010101", 2, tokens));
  }

  {
    ClueToken tokens[] = {{.lit = "010101", .type = TOKEN_BINARY}, {.type = TOKEN_END}};
    TEST_CASE(testLexer("%010101", 2, tokens));
  }

  // XXX
  // {
  //   ClueToken tokens[] = {{.lit = "42", .type = TOKEN_OCTAL}, {.type = TOKEN_END}};
  //   TEST_CASE(testLexer("0q42", 2, tokens));
  // }
  //
  // {
  //   ClueToken tokens[] = {{.lit = "42", .type = TOKEN_OCTAL}, {.type = TOKEN_END}};
  //   TEST_CASE(testLexer("0Q42", 2, tokens));
  // }

  return tests_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

static int testLexer(char const* str, int n_tokens, ClueToken const* tok_arr) {
  Lexer lex = Lexer_make(str);

  for (int i = 0; i < n_tokens; ++i) {
    Token tok = Lexer_next(&lex);
    ClueToken clue = tok_arr[i];
    if (clue.type)
      CHECK_TOKEN_TYPES_EQUAL(tok.type, clue.type, NULL);
    if (clue.lit)
      CHECK_STREQUALN(tok.value, clue.lit, tok.len, NULL);
  }

  return 0;
}
