#include <stdlib.h>

#include <lexer.h>
#include <string_lexer.h>

#include "test_suite.h"

typedef struct {
  char const* lit;
  TokenType type;
} ClueToken;

TEST_P(lexer_produce_tokens, char const* str, int n_tokens, ClueToken const* tok_arr) {
  Lexer lex = StringLexer_make(str);

  for (int i = 0; i < n_tokens; ++i) {
    Token tok = Lexer_next(&lex);
    ClueToken clue = tok_arr[i];
    CHECK_TOKEN_TYPE_NEQUAL(tok.type, TOKEN_ERROR);
    if (clue.type)
      CHECK_TOKEN_TYPE_EQUAL(tok.type, clue.type);
    if (clue.lit)
      CHECK_STR_EQUAL_LEN(tok.value, clue.lit, tok.len);
  }

TEST_CLEANUP:
  StringLexer_deinit(&lex);
}

int main(void) {
  TestSuite ts = {0};

  {
    ClueToken tokens[] = {{.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "", 1, tokens);
  }

  {
    ClueToken tokens[] = {{.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "; a comment", 1, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "a", .type = TOKEN_ID}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, ";a comment before\na;a comment after", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "a", .type = TOKEN_CHAR}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "'a'", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "31415", .type = TOKEN_HEXADECIMAL}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "0x31415", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "31415", .type = TOKEN_HEXADECIMAL}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "0X31415", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "31415abc", .type = TOKEN_HEXADECIMAL}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "$31415abc", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "31415abc", .type = TOKEN_HEXADECIMAL}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "#31415abc", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "42", .type = TOKEN_OCTAL}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "42o", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "42", .type = TOKEN_OCTAL}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "42q", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "010101", .type = TOKEN_BINARY}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "0b010101", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "010101", .type = TOKEN_BINARY}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "0B010101", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "010101", .type = TOKEN_BINARY}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "%010101", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "42", .type = TOKEN_OCTAL}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "0q42", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "42", .type = TOKEN_OCTAL}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "0Q42", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "abc", .type = TOKEN_ID}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "abc", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "abc", .type = TOKEN_STRING}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "\"abc\"", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "a", .type = TOKEN_STRING}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "\"a\"", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "\\\"", .type = TOKEN_STRING}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "\"\\\"\"", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "", .type = TOKEN_STRING}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "\"\"", 2, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "\n", .type = TOKEN_NEWLINE}, {.type = TOKEN_END}};
    TC_P(ts, lexer_produce_tokens, "\n", 2, tokens);
  }

  return ts.tests_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
