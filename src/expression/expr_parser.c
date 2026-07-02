#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "expr_parser.h"
#include "../token.h"
#include "../utility.h"
#include "../vector.h"

typedef struct {
  Vector* e; //< An expression
  Vector* o; //< A stack of operators
  Token prev;
  ExprError error;
  bool has_error;
  bool operations_retrieved;
} DefaultExprParser;

static void DefaultExprParser_deinit(void* impl);
static int DefaultExprParser_feed(void* impl, Token tok);
static Vector* DefaultExprParser_getOperations(void* impl);
static Vector* DefaultExprParser_getExpressions(void* impl);
static ExprError DefaultExprParser_getError(void* impl);

static bool isTerm(Token const* tok);
static bool isOp(Token const* tok);
static int prec(Token const* tok);

ExprParser DefaultExprParser_make(void) {
  Vector* expr = Vector_new(sizeof(Token));
  if (!expr)
    die("Vector_new() failed");

  Vector* operators = Vector_new(sizeof(Token));
  if (!operators)
    die("Vector_new() failed");

  DefaultExprParser* impl = malloc(sizeof(*impl));
  if (!impl) {
    die("DefaultExprParser_make(): malloc failed");
  }
  memset(impl, 0, sizeof(*impl));
  impl->e = expr;
  impl->o = operators;

  ExprParser p = {
    ._m_feed = DefaultExprParser_feed,
    ._m_deinit = DefaultExprParser_deinit,
    ._m_getOperations = DefaultExprParser_getOperations,
    ._m_getExpressions = DefaultExprParser_getExpressions,
    ._m_getError = DefaultExprParser_getError,
    ._m_impl = impl,
  };

  return p;
}

static void DefaultExprParser_deinit(void* impl) {
  DefaultExprParser* p = impl;
  Vector_destroy(p->e);
  if (!p->operations_retrieved) {
    Vector_destroy(p->o);
  }
  free(impl);
}

static inline Token const* top(Vector* v) {
  assert(!Vector_isEmpty(v));
  return Vector_at((Vector*)v, Vector_len(v) - 1);
}

static inline void error(DefaultExprParser* p, ExprErrorType type, Token tok) {
  p->error = (ExprError){.type = type, .tok = tok};
  p->has_error = true;
}

int DefaultExprParser_feed(void* impl, Token tok) {
  DefaultExprParser* p = impl;

  Token* prev = &p->prev;

  if (isTerm(&tok)) {
    Vector_push(p->e, &tok);

  } else if (isOp(&tok)) {
    if (prev->type == TOKEN_UNINITIALIZED || isOp(prev) || prev->type == TOKEN_LEFT_PAREN) {
      if (tok.type != TOKEN_PLUS && tok.type != TOKEN_MINUS && tok.type != TOKEN_BANG && tok.type != TOKEN_TILDE) {
        error(p, EXPR_ERROR_WRONG_UNARY_OP, tok);
        return -1;
      }
      tok.unary = true;
      Vector_push(p->o, &tok);

    } else if (Vector_isEmpty(p->o) || prec(&tok) > prec(top(p->o))) {
      Vector_push(p->o, &tok);

    } else {
      while (!Vector_isEmpty(p->o) && prec(&tok) <= prec(top(p->o))) {
        Token tmp;
        Vector_pop(p->o, &tmp);
        Vector_push(p->e, &tmp);
      }
      Vector_push(p->o, &tok);
    }

  } else if (tok.type == TOKEN_LEFT_PAREN) {
    Vector_push(p->o, &tok);

  } else if (tok.type == TOKEN_RIGHT_PAREN) {
    while (true) {
      if (Vector_isEmpty(p->o)) {
        error(p, EXPR_ERROR_UNBALANCED_RIGHT_PAREN, tok);
        return -1;
      }
      if (top(p->o)->type == TOKEN_LEFT_PAREN) {
        break;
      }
      Token tmp;
      Vector_pop(p->o, &tmp);
      Vector_push(p->e, &tmp);
    }
    Vector_pop(p->o, NULL);
  }

  else if (tok.type == TOKEN_END || tok.type == TOKEN_NEWLINE) {
    while (!Vector_isEmpty(p->o)) {
      if (top(p->o)->type == TOKEN_LEFT_PAREN) {
        error(p, EXPR_ERROR_UNBALANCED_LEFT_PAREN, tok);
        return -1;
      }
      Token tmp;
      Vector_pop(p->o, &tmp);
      Vector_push(p->e, &tmp);
    }

    assert(Vector_isEmpty(p->o));
  }

  else {
    error(p, EXPR_ERROR_UNEXPECTED_TOKEN, tok);
    return -1;
  }

  p->prev = tok;

  return 0;
}

static Vector* DefaultExprParser_getOperations(void* impl) {
  DefaultExprParser* p = impl;
  p->operations_retrieved = true;
  return p->o;
}

static Vector* DefaultExprParser_getExpressions(void* impl) {
  DefaultExprParser* p = impl;
  return p->e;
}

static ExprError DefaultExprParser_getError(void* impl) {
  DefaultExprParser* p = impl;
  return p->error;
}

static bool isTerm(Token const* tok) {
  switch (tok->type) {
  case TOKEN_DECIMAL:
  case TOKEN_HEXADECIMAL:
  case TOKEN_OCTAL:
  case TOKEN_BINARY:
  case TOKEN_CHAR:
  case TOKEN_ID:
    return true;
  case TOKEN_UNINITIALIZED:
  case TOKEN_END:
  case TOKEN_ERROR:
  case TOKEN_STRING:
  case TOKEN_LEFT_PAREN:
  case TOKEN_RIGHT_PAREN:
  case TOKEN_LEFT_BRACE:
  case TOKEN_RIGHT_BRACE:
  case TOKEN_COMMA:
  case TOKEN_MINUS:
  case TOKEN_PLUS:
  case TOKEN_SLASH:
  case TOKEN_STAR:
  case TOKEN_PERCENT:
  case TOKEN_CAP:
  case TOKEN_TILDE:
  case TOKEN_AMPERSAND:
  case TOKEN_BAR:
  case TOKEN_LEFT_SHIFT:
  case TOKEN_RIGHT_SHIFT:
  case TOKEN_DOUBLE_AMPERSAND:
  case TOKEN_DOUBLE_BAR:
  case TOKEN_BANG:
  case TOKEN_BANG_EQUAL:
  case TOKEN_EQUAL_EQUAL:
  case TOKEN_GREATER_EQUAL:
  case TOKEN_LESS_EQUAL:
  case TOKEN_NEWLINE:
  case TOKEN_COLON:
  default:
    return false;
  }
}

static bool isOp(Token const* tok) {
  switch (tok->type) {
  case TOKEN_MINUS:
  case TOKEN_PLUS:
  case TOKEN_SLASH:
  case TOKEN_STAR:
  case TOKEN_PERCENT:
  case TOKEN_CAP:
  case TOKEN_TILDE:
  case TOKEN_AMPERSAND:
  case TOKEN_BAR:
  case TOKEN_DOUBLE_AMPERSAND:
  case TOKEN_DOUBLE_BAR:
  case TOKEN_BANG:
  case TOKEN_BANG_EQUAL:
  case TOKEN_EQUAL_EQUAL:
  case TOKEN_GREATER_EQUAL:
  case TOKEN_LESS_EQUAL:
  case TOKEN_LEFT_SHIFT:
  case TOKEN_RIGHT_SHIFT:
  case TOKEN_LEFT_BRACE:
  case TOKEN_RIGHT_BRACE:
    return true;
  case TOKEN_UNINITIALIZED:
  case TOKEN_END:
  case TOKEN_ERROR:
  case TOKEN_ID:
  case TOKEN_CHAR:
  case TOKEN_STRING:
  case TOKEN_DECIMAL:
  case TOKEN_HEXADECIMAL:
  case TOKEN_OCTAL:
  case TOKEN_BINARY:
  case TOKEN_LEFT_PAREN:
  case TOKEN_RIGHT_PAREN:
  case TOKEN_COMMA:
  case TOKEN_NEWLINE:
  case TOKEN_COLON:
  default:
    return false;
  }
}

static int prec(Token const* tok) {
  switch (tok->type) {
  case TOKEN_DOUBLE_BAR:
    return 10;
  case TOKEN_DOUBLE_AMPERSAND:
    return 20;
  case TOKEN_BAR:
    return 30;
  case TOKEN_CAP:
    return 40;
  case TOKEN_AMPERSAND:
    return 50;
  case TOKEN_EQUAL_EQUAL:
  case TOKEN_BANG_EQUAL:
    return 60;
  case TOKEN_LEFT_BRACE:
  case TOKEN_RIGHT_BRACE:
  case TOKEN_LESS_EQUAL:
  case TOKEN_GREATER_EQUAL:
    return 70;
  case TOKEN_LEFT_SHIFT:
  case TOKEN_RIGHT_SHIFT:
    return 80;
  case TOKEN_MINUS:
  case TOKEN_PLUS:
    return 90;
  case TOKEN_SLASH:
  case TOKEN_STAR:
  case TOKEN_PERCENT:
    return 100;
  case TOKEN_BANG:
  case TOKEN_TILDE:
    return 110;
  case TOKEN_UNINITIALIZED:
  case TOKEN_END:
  case TOKEN_ERROR:
  case TOKEN_ID:
  case TOKEN_CHAR:
  case TOKEN_STRING:
  case TOKEN_DECIMAL:
  case TOKEN_HEXADECIMAL:
  case TOKEN_OCTAL:
  case TOKEN_BINARY:
  case TOKEN_LEFT_PAREN:
  case TOKEN_RIGHT_PAREN:
  case TOKEN_COMMA:
  case TOKEN_NEWLINE:
  case TOKEN_COLON:
  default:
    return 0;
  }
}
