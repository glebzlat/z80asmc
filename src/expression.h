#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <stdbool.h>

#include "lexer.h"
#include "vector.h"

typedef enum {
  EXPR_NO_ERROR = 0,
  EXPR_ERROR_WRONG_UNARY_OP,
  EXPR_ERROR_UNBALANCED_LEFT_PAREN,
  EXPR_ERROR_UNBALANCED_RIGHT_PAREN,
  EXPR_ERROR_UNEXPECTED_TOKEN,
} ExprErrorType;

typedef struct {
  Token tok;
  ExprErrorType type;
} ExprError;

typedef struct {
  Vector* e; //< An expression
  Vector* o; //< A stack of operators
  Token prev;
  ExprError error;
  bool has_error;
} ExprParser;

ExprParser ExprParser_make(void);
void ExprParser_deinit(ExprParser* p);

int ExprParser_get(ExprParser* p, Token tok);

char const* ExprErrorType_toStr(ExprErrorType type);

#endif // EXPRESSION_H
