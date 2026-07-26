#include <assert.h>
#include <limits.h>
#include <string.h>

#include "token.h"
#include "utility.h"

static uint8_t escToInt(char const* ch, size_t len);

char* Token_format(Token* tok) {
  assert(tok);
  char* buf = NULL;
  if (tok->type == TOKEN_ERROR) {
    buf = dsprintf("%zu:%zu:%s:%.*s", tok->line, tok->col, TokenType_str(tok->type), (int)tok->len, tok->value);
  } else {
    assert(tok->len < INT_MAX);
    char const* unary_str = tok->unary ? "u:" : "";
    buf = dsprintf("%zu:%zu:%s%s:%.*s", tok->line, tok->col, unary_str, TokenType_str(tok->type), (int)tok->len,
                   tok->value);
  }
  return buf;
}

char* Token_str(Token* tok) {
  assert(tok);

  char* str = malloc(tok->len + 1);
  if (!str)
    return NULL;

  strncpy(str, tok->value, tok->len);
  str[tok->len] = '\0';

  return str;
}

char const* TokenType_str(TokenType type) {
  switch (type) {
  case TOKEN_UNINITIALIZED:
    return "TOKEN_UNINITIALIZED";
  case TOKEN_END:
    return "TOKEN_END";
  case TOKEN_ERROR:
    return "TOKEN_ERROR";
  case TOKEN_ID:
    return "TOKEN_ID";
  case TOKEN_CHAR:
    return "TOKEN_CHAR";
  case TOKEN_STRING:
    return "TOKEN_STRING";
  case TOKEN_DECIMAL:
    return "TOKEN_DECIMAL";
  case TOKEN_HEXADECIMAL:
    return "TOKEN_HEXADECIMAL";
  case TOKEN_OCTAL:
    return "TOKEN_OCTAL";
  case TOKEN_BINARY:
    return "TOKEN_BINARY";
  case TOKEN_LEFT_PAREN:
    return "TOKEN_LEFT_PAREN";
  case TOKEN_RIGHT_PAREN:
    return "TOKEN_RIGHT_PAREN";
  case TOKEN_LEFT_BRACE:
    return "TOKEN_LEFT_BRACE";
  case TOKEN_RIGHT_BRACE:
    return "TOKEN_RIGHT_BRACE";
  case TOKEN_COMMA:
    return "TOKEN_COMMA";
  case TOKEN_MINUS:
    return "TOKEN_MINUS";
  case TOKEN_PLUS:
    return "TOKEN_PLUS";
  case TOKEN_SLASH:
    return "TOKEN_SLASH";
  case TOKEN_STAR:
    return "TOKEN_STAR";
  case TOKEN_PERCENT:
    return "TOKEN_PERCENT";
  case TOKEN_CAP:
    return "TOKEN_CAP";
  case TOKEN_TILDE:
    return "TOKEN_TILDE";
  case TOKEN_AMPERSAND:
    return "TOKEN_AMPERSAND";
  case TOKEN_BAR:
    return "TOKEN_BAR";
  case TOKEN_DOUBLE_AMPERSAND:
    return "TOKEN_DOUBLE_AMPERSAND";
  case TOKEN_DOUBLE_BAR:
    return "TOKEN_DOUBLE_BAR";
  case TOKEN_BANG:
    return "TOKEN_BANG";
  case TOKEN_BANG_EQUAL:
    return "TOKEN_BANG_EQUAL";
  case TOKEN_EQUAL_EQUAL:
    return "TOKEN_EQUAL_EQUAL";
  case TOKEN_GREATER_EQUAL:
    return "TOKEN_GREATER_EQUAL";
  case TOKEN_LESS_EQUAL:
    return "TOKEN_LESS_EQUAL";
  case TOKEN_NEWLINE:
    return "TOKEN_NEWLINE";
  case TOKEN_LEFT_SHIFT:
    return "TOKEN_LEFT_SHIFT";
  case TOKEN_RIGHT_SHIFT:
    return "TOKEN_RIGHT_SHIFT";
  case TOKEN_COLON:
    return "TOKEN_COLON";
  default:
    /* Invariant */
    die("TokenType_str: unknown token type");
  }
}

unsigned long Token_toInt(Token* tok, bool* correct) {
  assert(tok);
  uint8_t base = 0;

  *correct = false;
  switch (tok->type) {
  case TOKEN_HEXADECIMAL:
    base = 16;
    break;
  case TOKEN_DECIMAL:
    base = 10;
    break;
  case TOKEN_OCTAL:
    base = 8;
    break;
  case TOKEN_BINARY:
    base = 2;
    break;
  case TOKEN_CHAR:
    if (tok->len < 1)
      return 0;

    *correct = true;
    return escToInt(tok->value, tok->len);
  case TOKEN_UNINITIALIZED:
  case TOKEN_END:
  case TOKEN_ERROR:
  case TOKEN_ID:
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
  case TOKEN_COLON:
  case TOKEN_NEWLINE:
  default:
    /* Invariant */
    die("Token_toInt(): token type is not integer convertible");
  }

  return strtou32(tok->value, tok->len, correct, base);
}

static uint8_t escToInt(char const* ch, size_t len) {
  assert(ch);
  if (ch[0] == '\\' && len >= 2) {
    switch (ch[1]) {
    case '\\':
      return 92;
    case '?':
      return 63;
    case '\'':
      return 39;
    case '"':
      return 34;
    case '0':
      return 0;
    case 'a':
    case 'A':
      return 7;
    case 'b':
    case 'B':
      return 8;
    case 'd':
    case 'D':
      return 127;
    case 'e':
    case 'E':
      return 27;
    case 'f':
    case 'F':
      return 12;
    case 'n':
    case 'N':
      return 10;
    case 'r':
    case 'R':
      return 13;
    case 't':
    case 'T':
      return 9;
    case 'v':
    case 'V':
      return 11;
    default:
      /* Invariant: lexer must not permit incorrect escape sequences */
      die("escToInt(): incorrect escape sequence");
    }
  }
  return (uint8_t)ch[0];
}
