#include "token.h"
#include "utility.h"
#include <assert.h>
#include <stdarg.h>

#include <expr_parser.h>
#include <expression/expr_parser.h>
#include <lexer.h>
#include <string_lexer.h>

#include "test_suite.h"
#include "vector.h"

typedef struct {
  char const* lit;
  TokenType type;
  bool unary;
} ClueToken;

TEST_P(expr_parser_lex_parse_expr, char const* input, size_t n_tokens, ClueToken const* tok_arr) {
  Lexer lex = StringLexer_make(input);
  ExprParser parser = DefaultExprParser_make();
  Vector* expressions = NULL;

  while (true) {
    Token tok = Lexer_next(&lex);
    CHECK_INT_NEQUAL(ExprParser_feed(&parser, tok), -1);

    if (tok.type == TOKEN_END)
      break;
  }

  expressions = ExprParser_getExpressions(&parser);
  if (TEST_VERBOSE_LOG()) {
    for (size_t i = 0; i < Vector_len(expressions); ++i) {
      char* tok_str = Token_format(Vector_at(expressions, i));
      printf("%s ", tok_str);
      free(tok_str);
    }
    printf("\n");
  }

  CHECK_UINT_EQUAL(Vector_len(expressions), n_tokens);

  for (size_t i = 0; i < n_tokens; ++i) {
    Token* tok = Vector_at(expressions, i);
    ClueToken clue = tok_arr[i];

    CHECK_TOKEN_TYPE_EQUAL(tok->type, clue.type);
    if (clue.lit)
      CHECK(strncasecmp(tok->value, clue.lit, tok->len) == 0);
    CHECK_BOOL_EQUAL(tok->unary, clue.unary);
  }

TEST_CLEANUP:
  if (TEST_FAILED()) {
    ExprError err = ExprParser_getError(&parser);
    char* tok_str = Token_format(&err.tok);
    _PRINTERR_VA("ExprParser_get failed: %s : %s\n", ExprErrorType_toStr(err.type), tok_str);
    free(tok_str);
  }

  if (expressions)
    Vector_destroy(expressions);

  ExprParser_deinit(&parser);
  StringLexer_deinit(&lex);
}

int main(void) {
  TestSuite ts = TestSuite_make();

  {
    // 1 2 +
    ClueToken tokens[] = {
        {.lit = "1", .type = TOKEN_DECIMAL}, {.lit = "2", .type = TOKEN_DECIMAL}, {.type = TOKEN_PLUS}};
    TC_P(ts, expr_parser_lex_parse_expr, "1+2", 3, tokens);
  }

  {
    // 1 2 3 * +
    ClueToken tokens[] = {{.lit = "1", .type = TOKEN_DECIMAL},
                          {.lit = "2", .type = TOKEN_DECIMAL},
                          {.lit = "3", .type = TOKEN_DECIMAL},
                          {.type = TOKEN_STAR},
                          {.type = TOKEN_PLUS}};
    TC_P(ts, expr_parser_lex_parse_expr, "1+2*3", 5, tokens);
  }

  {
    // 1 2 + 3 *
    ClueToken tokens[] = {{.lit = "1", .type = TOKEN_DECIMAL},
                          {.lit = "2", .type = TOKEN_DECIMAL},
                          {.type = TOKEN_PLUS},
                          {.lit = "3", .type = TOKEN_DECIMAL},
                          {.type = TOKEN_STAR}};
    TC_P(ts, expr_parser_lex_parse_expr, "(1+2)*3", 5, tokens);
  }

  {
    // 1 -
    ClueToken tokens[] = {{.lit = "1", .type = TOKEN_DECIMAL}, {.type = TOKEN_MINUS, .unary = true}};
    TC_P(ts, expr_parser_lex_parse_expr, "-1", 2, tokens);
  }

  {
    // 1 + -
    ClueToken tokens[] = {
        {.lit = "1", .type = TOKEN_DECIMAL}, {.type = TOKEN_PLUS, .unary = true}, {.type = TOKEN_MINUS, .unary = true}};
    TC_P(ts, expr_parser_lex_parse_expr, "-(+1)", 3, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "a", .type = TOKEN_ID}};
    TC_P(ts, expr_parser_lex_parse_expr, "a", 1, tokens);
  }

  {
    ClueToken tokens[] = {{.lit = "01", .type = TOKEN_HEXADECIMAL}};
    TC_P(ts, expr_parser_lex_parse_expr, "0x01", 1, tokens);
  }

  {
    // a !
    ClueToken tokens[] = {{.lit = "a", .type = TOKEN_ID}, {.type = TOKEN_BANG, .unary = true}};
    TC_P(ts, expr_parser_lex_parse_expr, "!a", 2, tokens);
  }

  {
    // a ! b && c d && ||
    ClueToken tokens[] = {{.lit = "a", .type = TOKEN_ID},   {.type = TOKEN_BANG, .unary = true},
                          {.lit = "b", .type = TOKEN_ID},   {.type = TOKEN_DOUBLE_AMPERSAND},
                          {.lit = "c", .type = TOKEN_ID},   {.lit = "d", .type = TOKEN_ID},
                          {.type = TOKEN_DOUBLE_AMPERSAND}, {.type = TOKEN_DOUBLE_BAR}};
    TC_P(ts, expr_parser_lex_parse_expr, "!a && b || c && d", 8, tokens);
  }

  {
    // a b << c > d ==
    ClueToken tokens[] = {{.lit = "a", .type = TOKEN_ID}, {.lit = "b", .type = TOKEN_ID},
                          {.type = TOKEN_LEFT_SHIFT},     {.lit = "c", .type = TOKEN_ID},
                          {.type = TOKEN_RIGHT_BRACE},    {.lit = "d", .type = TOKEN_ID},
                          {.type = TOKEN_EQUAL_EQUAL}};
    TC_P(ts, expr_parser_lex_parse_expr, "a << b > c == d", 7, tokens);
  }

  {
    // a b == c != d ==
    ClueToken tokens[] = {{.lit = "a", .type = TOKEN_ID}, {.lit = "b", .type = TOKEN_ID},
                          {.type = TOKEN_EQUAL_EQUAL},    {.lit = "c", .type = TOKEN_ID},
                          {.type = TOKEN_BANG_EQUAL},     {.lit = "d", .type = TOKEN_ID},
                          {.type = TOKEN_EQUAL_EQUAL}};
    TC_P(ts, expr_parser_lex_parse_expr, "a == b != c == d", 7, tokens);
  }

  return ts.tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
