#include "utility.h"
#include <assert.h>
#include <stdarg.h>

#include <expression.h>
#include <lexer.h>

#define STRINGIZE(EXP) #EXP

#define CHECK(ASSERTION)                                                                                               \
  do {                                                                                                                 \
    if (ASSERTION)                                                                                                     \
      ;                                                                                                                \
    else {                                                                                                             \
      fprintf(stderr, "%s:%i: Check " STRINGIZE(#ASSERTION) " failed\n", __FILE__, __LINE__);                          \
      return 1;                                                                                                        \
    }                                                                                                                  \
  } while (0)

typedef struct {
  char const* lit;
  TokenType type;
  bool unary;
} ClueToken;

int testExpression(char const* input, size_t n_tokens, ...);

int main(void) {
  int tests_failed = 0;

  {
    ClueToken t1 = {.lit = "1", .type = TOKEN_DECIMAL}, t2 = {.lit = "2", .type = TOKEN_DECIMAL},
              t3 = {.type = TOKEN_PLUS};
    tests_failed += testExpression("1+2", 3, t1, t2, t3);
  }

  {
    ClueToken t1 = {.lit = "1"}, t2 = {.lit = "2"}, t3 = {.lit = "3"}, t4 = {.type = TOKEN_STAR},
              t5 = {.type = TOKEN_PLUS};
    tests_failed += testExpression("1+2*3", 5, t1, t2, t3, t4, t5);
  }

  {
    ClueToken t1 = {.lit = "1"}, t2 = {.lit = "2"}, t3 = {.type = TOKEN_PLUS}, t4 = {.lit = "3"},
              t5 = {.type = TOKEN_STAR};
    tests_failed += testExpression("(1+2)*3", 5, t1, t2, t3, t4, t5);
  }

  {
    ClueToken t1 = {.lit = "1"}, t2 = {.type = TOKEN_MINUS, .unary = true};
    tests_failed += testExpression("-1", 2, t1, t2);
  }

  {
    ClueToken t1 = {.lit = "1"}, t2 = {.type = TOKEN_PLUS, .unary = true}, t3 = {.type = TOKEN_MINUS, .unary = true};
    tests_failed += testExpression("-(+1)", 3, t1, t2, t3);
  }

  {
    ClueToken t1 = {.lit = "a"};
    tests_failed += testExpression("a", 1, t1);
  }

  {
    ClueToken t1 = {.lit = "01"};
    tests_failed += testExpression("0x01", 1, t1);
  }

  // { // XXX: fails
  //   ClueToken t1 = {.lit = "01"};
  //   tests_failed += testExpression("01", 1, t1);
  // }

  return tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int testExpression(char const* input, size_t n_tokens, ...) {
  Lexer lex = Lexer_make(input);
  ExprParser parser = ExprParser_make();

  while (true) {
    Token tok = Lexer_next(&lex);

    if (ExprParser_get(&parser, tok) == -1) {
      char* tok_str = Token_format(&parser.error.tok);
      fprintf(stderr, "ExprParser_get failed: %s : %s\n", parser.error.reason, tok_str);
      free(tok_str);
      return 1;
    }

    if (tok.type == TOKEN_END)
      break;
  }

  for (size_t i = 0; i < Vector_len(parser.e); ++i) {
    char* tok_str = Token_format(Vector_at(parser.e, i));
    printf("%s ", tok_str);
    free(tok_str);
  }
  printf("\n");

  CHECK(Vector_len(parser.e) == n_tokens);

  va_list ap;
  va_start(ap, n_tokens);
  for (size_t i = 0; i < n_tokens; ++i) {
    Token* tok = Vector_at(parser.e, i);
    ClueToken clue = va_arg(ap, ClueToken);

    if (clue.type)
      CHECK(tok->type == clue.type);
    if (clue.lit)
      CHECK(strncasecmp(tok->value, clue.lit, tok->len) == 0);
    CHECK(tok->unary == clue.unary);
  }
  va_end(ap);

  ExprParser_deinit(&parser);

  return 0;
}
