#include <stdbool.h>
#include <stdlib.h>

#include <expr_parser.h>
#include <expression/expr_parser.h>
#include <string_lexer.h>

#include "common.h"

int testExpressionFail(char const* expr, ExprErrorType err_type, char const* err_token_repr);

int main(void) {
  int failed_tests = 0;

  failed_tests += testExpressionFail("*1", EXPR_ERROR_WRONG_UNARY_OP, "1:1:TOKEN_STAR:*");
  failed_tests += testExpressionFail("((1+2)", EXPR_ERROR_UNBALANCED_LEFT_PAREN, "1:6:TOKEN_END:)");
  failed_tests += testExpressionFail("(1+2))", EXPR_ERROR_UNBALANCED_RIGHT_PAREN, "1:6:TOKEN_RIGHT_PAREN:)");
  failed_tests += testExpressionFail("1,2", EXPR_ERROR_UNEXPECTED_TOKEN, "1:2:TOKEN_COMMA:,");

  return failed_tests == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int testExpressionFail(char const* expr, ExprErrorType err_type, char const* err_token_repr) {
  Lexer lex = StringLexer_make(expr);
  ExprParser parser = DefaultExprParser_make();

  bool assert_failed = false;

  Token tok = {0};
  while (tok.type != TOKEN_END) {
    tok = Lexer_next(&lex);

    if (ExprParser_feed(&parser, tok) == -1) {
      ExprError err = ExprParser_getError(&parser);
      CHECK_EQUAL(err.type, err_type, NULL);
      char* tok_str = Token_format(&err.tok);
      CHECK_STREQUAL(tok_str, err_token_repr, free(tok_str));
      free(tok_str);
      assert_failed = true;
    }
  }

  ExprParser_deinit(&parser);
  StringLexer_deinit(&lex);

  return !assert_failed;
}
