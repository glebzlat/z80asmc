#include <stdbool.h>
#include <stdlib.h>

#include <expr_parser.h>
#include <expression/expr_parser.h>
#include <string_lexer.h>

#include "test_suite.h"
#include "token.h"

TEST_P(expr_parser_lex_fail, char const* expr, ExprErrorType err_type, Token err_token) {
  Lexer lex = StringLexer_make(expr);
  ExprParser parser = DefaultExprParser_make();

  bool expr_parser_failed = false;
  Token tok = {0};
  while (tok.type != TOKEN_END) {
    tok = Lexer_next(&lex);
    if (ExprParser_feed(&parser, tok) == -1) {
      expr_parser_failed = true;
      break;
    }
  }

  CHECK_BOOL_EQUAL(expr_parser_failed, true);

  ExprError err = ExprParser_getError(&parser);
  CHECK_INT_EQUAL(err.type, err_type);
  CHECK_TOKEN_TYPE_EQUAL(err.tok.type, err_token.type);
  CHECK_UINT_EQUAL(err.tok.len, err_token.len);
  CHECK_UINT_EQUAL(err.tok.line, err_token.line);
  CHECK_UINT_EQUAL(err.tok.col, err_token.col);
  CHECK_BOOL_EQUAL(err.tok.unary, err_token.unary);
  CHECK_STR_EQUAL_LEN(err.tok.value, err_token.value, err.tok.len);

TEST_CLEANUP:
  ExprParser_deinit(&parser);
  StringLexer_deinit(&lex);
}

int testExpressionFail(char const* expr, ExprErrorType err_type, char const* err_token_repr);

int main(void) {
  TestSuite ts = TestSuite_make();

  {
    Token tok = {.value = "*", .type = TOKEN_STAR, .len = 1, .line = 1, .col = 1};
    TC_P(ts, expr_parser_lex_fail, "*1", EXPR_ERROR_WRONG_UNARY_OP, tok);
  }

  {
    Token tok = {.value = ")", .type = TOKEN_END, .len = 1, .line = 1, .col = 6};
    TC_P(ts, expr_parser_lex_fail, "((1+2)", EXPR_ERROR_UNBALANCED_LEFT_PAREN, tok);
  }

  {
    Token tok = {.value = ")", .type = TOKEN_RIGHT_PAREN, .len = 1, .line = 1, .col = 6};
    TC_P(ts, expr_parser_lex_fail, "(1+2))", EXPR_ERROR_UNBALANCED_RIGHT_PAREN, tok);
  }

  {
    Token tok = {.value = ",", .type = TOKEN_COMMA, .len = 1, .line = 1, .col = 2};
    TC_P(ts, expr_parser_lex_fail, "1,2", EXPR_ERROR_UNEXPECTED_TOKEN, tok);
  }

  return ts.tests_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
