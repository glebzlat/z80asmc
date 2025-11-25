#include <assert.h>

#include "expression.h"
#include "lexer.h"
#include "utility.h"

static bool isTerm(Token const* tok);
static bool isOp(Token const* tok);
static int prec(Token const* tok);

static inline Token const* top(Vector* v) {
  assert(!Vector_isEmpty(v));
  return Vector_at((Vector*)v, Vector_len(v) - 1);
}

static inline void error(ExprParser* p, char const* reason, Token tok) {
  p->error = (ExprError){.reason = reason, .tok = tok};
  p->has_error = true;
}

ExprParser ExprParser_make(void) {
  Vector* expr = Vector_new(sizeof(Token));
  if (!expr)
    die("Vector_new() failed");

  Vector* operators = Vector_new(sizeof(Token));
  if (!operators)
    die("Vector_new() failed");

  return (ExprParser){.e = expr, .o = operators};
}

void ExprParser_deinit(ExprParser* p) {
  assert(p);
  Vector_destroy(p->e);
  Vector_destroy(p->o);
}

int ExprParser_get(ExprParser* p, Token tok) {
  assert(p);

  Token* prev = &p->prev;

  if (isTerm(&tok)) {
    Vector_push(p->e, &tok);

  } else if (isOp(&tok)) {
    if (prev->type == TOKEN_UNINITIALIZED || isOp(prev) || prev->type == TOKEN_LEFT_PAREN) {
      if (tok.type != TOKEN_PLUS && tok.type != TOKEN_MINUS) {
        error(p, "operator can't be unary", tok);
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
        error(p, "unbalanced left paren", tok);
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

  else if (tok.type == TOKEN_END) {
    while (!Vector_isEmpty(p->o)) {
      if (top(p->o)->type == TOKEN_LEFT_PAREN) {
        error(p, "unbalanced left paren", tok);
        return -1;
      }
      Token tmp;
      Vector_pop(p->o, &tmp);
      Vector_push(p->e, &tmp);
    }

    assert(Vector_isEmpty(p->o));
  }

  else {
    error(p, "unexpected token", tok);
    return -1;
  }

  p->prev = tok;

  return 0;
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
  case TOKEN_LEFT_BRACE:
  case TOKEN_RIGHT_BRACE:
  case TOKEN_COMMA:
  case TOKEN_LEFT_SHIFT:
  case TOKEN_RIGHT_SHIFT:
  case TOKEN_NEWLINE:
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
  default:
    return 0;
  }
}
