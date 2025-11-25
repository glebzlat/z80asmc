#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <stdbool.h>

#include "lexer.h"
#include "vector.h"

typedef struct {
  char const* reason;
  Token tok;
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

#endif // EXPRESSION_H
