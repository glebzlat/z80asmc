#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "token.h"
#include <assert.h>
#include <stdarg.h>

#include <expr_parser.h>
#include <expression/expr_parser.h>

#include "test_suite.h"
#include "vector.h"

/* Build a non-identifier token at the given end column. Matches the col
 * convention used by StringLexer for every token type except TOKEN_ID. */
static Token make_nonid_token(char const* value, size_t end_col, TokenType type) {
  assert(value);
  return (Token){
      .value = value,
      .len = strlen(value),
      .symlen = 0,
      .line = 1,
      .col = end_col,
      .type = type,
      .unary = false,
  };
}

/* Build an identifier token at the given start column. Matches the col
 * convention used by StringLexer for TOKEN_ID (col = start + 1). */
static Token make_id_token(char const* value, size_t start_col) {
  assert(value);
  return (Token){
      .value = value,
      .len = strlen(value),
      .symlen = 0,
      .line = 1,
      .col = start_col + 1,
      .type = TOKEN_ID,
      .unary = false,
  };
}

/* Build a TOKEN_END sentinel. The StringLexer has a known bug where END
 * inherits the previous token's value/len; we do not mirror that bug here
 * because it is orthogonal to what the Expression Parser consumes. */
static Token make_end_token(void) {
  return (Token){
      .value = "",
      .len = 0,
      .symlen = 0,
      .line = 1,
      .col = 0,
      .type = TOKEN_END,
      .unary = false,
  };
}

/* Compare a single Token field against the expected value. `expect_repr` is
 * a human-readable label for the expected value (used in error messages). */
#define CHECK_TOKEN_FIELD_NUMERIC(FIELD, EXPECT, EXPECT_REPR)                                                          \
  do {                                                                                                                 \
    if (tok->FIELD != (EXPECT)) {                                                                                      \
      _PRINTERR_VA("Check token[%zu].%s failed: actual=%llu expected=%llu (%s)\n", i, #FIELD,                     \
                   (unsigned long long)tok->FIELD, (unsigned long long)(EXPECT), EXPECT_REPR);                         \
      _ts->current_test_failed = true;                                                                                 \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
  } while (0)

#define CHECK_TOKEN_FIELD_VALUE()                                                                                      \
  do {                                                                                                                 \
    if (tok->len != clue->len || strncmp(tok->value, clue->value, tok->len) != 0) {                                    \
      _PRINTERR_VA("Check token[%zu].value failed: actual=\"%.*s\" expected=\"%.*s\"\n", i, (int)tok->len, tok->value, \
                   (int)clue->len, clue->value);                                                                       \
      _ts->current_test_failed = true;                                                                                 \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
  } while (0)

/* Runs the Expression Parser on `input_tokens` (which must end with a
 * TOKEN_END sentinel) and verifies the produced expression list equals
 * `clue_tokens` (length `n_clue`). */
TEST_P(expr_parser_lex_parse_expr, Token const* input_tokens, size_t n_clue, Token const* clue_tokens) {
  ExprParser parser = DefaultExprParser_make();
  Vector* expressions = NULL;

  for (size_t i = 0;; ++i) {
    CHECK_INT_NEQUAL(ExprParser_feed(&parser, input_tokens[i]), -1);
    if (input_tokens[i].type == TOKEN_END)
      break;
  }

  expressions = ExprParser_getExpressions(&parser);

  if (TEST_VERBOSE_LOG()) {
    for (size_t i = 0; i < Vector_len(expressions); ++i) {
      char* s = Token_format(Vector_at(expressions, i));
      printf("%s ", s);
      free(s);
    }
    printf("\n");
  }

  CHECK_UINT_EQUAL(Vector_len(expressions), n_clue);

  for (size_t i = 0; i < n_clue; ++i) {
    Token const* tok = Vector_at(expressions, i);
    Token const* clue = &clue_tokens[i];

    /* Assert every documented Token field except symlen (the lexer does
     * not currently populate it). */
    CHECK_TOKEN_FIELD_VALUE();
    CHECK_TOKEN_FIELD_NUMERIC(len, clue->len, "see value");
    CHECK_TOKEN_FIELD_NUMERIC(col, clue->col, "see clue->col");
    CHECK_TOKEN_FIELD_NUMERIC(line, clue->line, "see clue->line");
    CHECK_TOKEN_FIELD_NUMERIC(type, clue->type, TokenType_str(clue->type));
    CHECK_TOKEN_FIELD_NUMERIC(unary, clue->unary, clue->unary ? "true" : "false");
  }

TEST_CLEANUP:
  if (TEST_FAILED()) {
    ExprError err = ExprParser_getError(&parser);
    char* s = Token_format(&err.tok);
    _PRINTERR_VA("ExprParser_get failed: %s : %s\n", ExprErrorType_toStr(err.type), s);
    free(s);
  }

  if (expressions)
    Vector_destroy(expressions);

  ExprParser_deinit(&parser);
}

int main(void) {
  TestSuite ts = TestSuite_make();

  /* Input: 1 + 2
   * postfix: 1 2 + */
  {
    Token const input[] = {
        make_nonid_token("1", 1, TOKEN_DECIMAL),
        make_nonid_token("+", 2, TOKEN_PLUS),
        make_nonid_token("2", 3, TOKEN_DECIMAL),
        make_end_token(),
    };
    Token const clue[] = {
        make_nonid_token("1", 1, TOKEN_DECIMAL),
        make_nonid_token("2", 3, TOKEN_DECIMAL),
        make_nonid_token("+", 2, TOKEN_PLUS),
    };
    TC_P(ts, expr_parser_lex_parse_expr, input, 3, clue);
  }

  /* Input: 1 + 2 * 3
   * postfix: 1 2 3 * + */
  {
    Token const input[] = {
        make_nonid_token("1", 1, TOKEN_DECIMAL), make_nonid_token("+", 2, TOKEN_PLUS),
        make_nonid_token("2", 3, TOKEN_DECIMAL), make_nonid_token("*", 4, TOKEN_STAR),
        make_nonid_token("3", 5, TOKEN_DECIMAL), make_end_token(),
    };
    Token const clue[] = {
        make_nonid_token("1", 1, TOKEN_DECIMAL), make_nonid_token("2", 3, TOKEN_DECIMAL),
        make_nonid_token("3", 5, TOKEN_DECIMAL), make_nonid_token("*", 4, TOKEN_STAR),
        make_nonid_token("+", 2, TOKEN_PLUS),
    };
    TC_P(ts, expr_parser_lex_parse_expr, input, 5, clue);
  }

  /* Input: ( 1 + 2 ) * 3
   * Postfix: 1 2 + 3 * */
  {
    Token const input[] = {
        make_nonid_token("(", 1, TOKEN_LEFT_PAREN),  make_nonid_token("1", 2, TOKEN_DECIMAL),
        make_nonid_token("+", 3, TOKEN_PLUS),        make_nonid_token("2", 4, TOKEN_DECIMAL),
        make_nonid_token(")", 5, TOKEN_RIGHT_PAREN), make_nonid_token("*", 6, TOKEN_STAR),
        make_nonid_token("3", 7, TOKEN_DECIMAL),     make_end_token(),
    };
    Token const clue[] = {
        make_nonid_token("1", 2, TOKEN_DECIMAL), make_nonid_token("2", 4, TOKEN_DECIMAL),
        make_nonid_token("+", 3, TOKEN_PLUS),    make_nonid_token("3", 7, TOKEN_DECIMAL),
        make_nonid_token("*", 6, TOKEN_STAR),
    };
    TC_P(ts, expr_parser_lex_parse_expr, input, 5, clue);
  }

  /* Input: - 1
   * Postfix: 1 -  (with unary on -) */
  {
    Token const input[] = {
        make_nonid_token("-", 1, TOKEN_MINUS),
        make_nonid_token("1", 2, TOKEN_DECIMAL),
        make_end_token(),
    };
    Token clue[] = {
        make_nonid_token("1", 2, TOKEN_DECIMAL),
        {.value = "-", .len = 1, .symlen = 0, .line = 1, .col = 1, .type = TOKEN_MINUS, .unary = true},
    };
    TC_P(ts, expr_parser_lex_parse_expr, input, 2, clue);
  }

  /* Input: - ( + 1 )
   * Postfix: 1 + -  (both ops unary) */
  {
    Token const input[] = {
        make_nonid_token("-", 1, TOKEN_MINUS),       make_nonid_token("(", 2, TOKEN_LEFT_PAREN),
        make_nonid_token("+", 3, TOKEN_PLUS),        make_nonid_token("1", 4, TOKEN_DECIMAL),
        make_nonid_token(")", 5, TOKEN_RIGHT_PAREN), make_end_token(),
    };
    Token clue[] = {
        make_nonid_token("1", 4, TOKEN_DECIMAL),
        {.value = "+", .len = 1, .symlen = 0, .line = 1, .col = 3, .type = TOKEN_PLUS, .unary = true},
        {.value = "-", .len = 1, .symlen = 0, .line = 1, .col = 1, .type = TOKEN_MINUS, .unary = true},
    };
    TC_P(ts, expr_parser_lex_parse_expr, input, 3, clue);
  }

  /* Input: a
   * Postfix: a */
  {
    Token const input[] = {
        make_id_token("a", 0),
        make_end_token(),
    };
    Token const clue[] = {
        make_id_token("a", 0),
    };
    TC_P(ts, expr_parser_lex_parse_expr, input, 1, clue);
  }

  /* Input: 0x01
   * Postfix: 0x01 */
  {
    Token const input[] = {
        make_nonid_token("01", 4, TOKEN_HEXADECIMAL),
        make_end_token(),
    };
    Token const clue[] = {
        make_nonid_token("01", 4, TOKEN_HEXADECIMAL),
    };
    TC_P(ts, expr_parser_lex_parse_expr, input, 1, clue);
  }

  /* Input: ! a
   * Postfix: a ! (with unary on !) */
  {
    Token const input[] = {
        make_nonid_token("!", 1, TOKEN_BANG),
        make_id_token("a", 1),
        make_end_token(),
    };
    Token clue[] = {
        make_id_token("a", 1),
        {.value = "!", .len = 1, .symlen = 0, .line = 1, .col = 1, .type = TOKEN_BANG, .unary = true},
    };
    TC_P(ts, expr_parser_lex_parse_expr, input, 2, clue);
  }

  /* Input:   ! a && b || c && d
   * Postfix: a ! b && c d && ||  (! is unary) */
  {
    Token const input[] = {
        make_nonid_token("!", 1, TOKEN_BANG),
        make_id_token("a", 1),
        make_nonid_token("&&", 5, TOKEN_DOUBLE_AMPERSAND),
        make_id_token("b", 6),
        make_nonid_token("||", 10, TOKEN_DOUBLE_BAR),
        make_id_token("c", 11),
        make_nonid_token("&&", 15, TOKEN_DOUBLE_AMPERSAND),
        make_id_token("d", 16),
        make_end_token(),
    };
    Token clue[] = {
        make_id_token("a", 1),
        {.value = "!", .len = 1, .symlen = 0, .line = 1, .col = 1, .type = TOKEN_BANG, .unary = true},
        make_id_token("b", 6),
        make_nonid_token("&&", 5, TOKEN_DOUBLE_AMPERSAND),
        make_id_token("c", 11),
        make_id_token("d", 16),
        make_nonid_token("&&", 15, TOKEN_DOUBLE_AMPERSAND),
        make_nonid_token("||", 10, TOKEN_DOUBLE_BAR),
    };
    TC_P(ts, expr_parser_lex_parse_expr, input, 8, clue);
  }

  /* Input:   a << b > c == d
   * Postfix: a b << c > d == */
  {
    Token const input[] = {
        make_id_token("a", 0),  make_nonid_token("<<", 4, TOKEN_LEFT_SHIFT),
        make_id_token("b", 5),  make_nonid_token(">", 8, TOKEN_RIGHT_BRACE),
        make_id_token("c", 9),  make_nonid_token("==", 13, TOKEN_EQUAL_EQUAL),
        make_id_token("d", 14), make_end_token(),
    };
    Token const clue[] = {
        make_id_token("a", 0),
        make_id_token("b", 5),
        make_nonid_token("<<", 4, TOKEN_LEFT_SHIFT),
        make_id_token("c", 9),
        make_nonid_token(">", 8, TOKEN_RIGHT_BRACE),
        make_id_token("d", 14),
        make_nonid_token("==", 13, TOKEN_EQUAL_EQUAL),
    };
    TC_P(ts, expr_parser_lex_parse_expr, input, 7, clue);
  }

  /* Input:   a == b != c == d
   * Postfix: a b == c != d == */
  {
    Token const input[] = {
        make_id_token("a", 0),  make_nonid_token("==", 4, TOKEN_EQUAL_EQUAL),
        make_id_token("b", 5),  make_nonid_token("!=", 9, TOKEN_BANG_EQUAL),
        make_id_token("c", 10), make_nonid_token("==", 14, TOKEN_EQUAL_EQUAL),
        make_id_token("d", 15), make_end_token(),
    };
    Token const clue[] = {
        make_id_token("a", 0),
        make_id_token("b", 5),
        make_nonid_token("==", 4, TOKEN_EQUAL_EQUAL),
        make_id_token("c", 10),
        make_nonid_token("!=", 9, TOKEN_BANG_EQUAL),
        make_id_token("d", 15),
        make_nonid_token("==", 14, TOKEN_EQUAL_EQUAL),
    };
    TC_P(ts, expr_parser_lex_parse_expr, input, 7, clue);
  }

  return ts.tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
