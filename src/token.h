#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>
#include <stdbool.h>

typedef enum {
  TOKEN_UNINITIALIZED = 0,
  TOKEN_END,
  TOKEN_ERROR,
  TOKEN_ID,
  TOKEN_CHAR,
  TOKEN_STRING,
  TOKEN_DECIMAL,
  TOKEN_HEXADECIMAL,
  TOKEN_OCTAL,
  TOKEN_BINARY,
  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_COMMA,
  TOKEN_MINUS,
  TOKEN_PLUS,
  TOKEN_SLASH,
  TOKEN_STAR,
  TOKEN_PERCENT,
  TOKEN_CAP,
  TOKEN_TILDE,
  TOKEN_AMPERSAND,
  TOKEN_BAR,
  TOKEN_LEFT_SHIFT,
  TOKEN_RIGHT_SHIFT,
  TOKEN_DOUBLE_AMPERSAND,
  TOKEN_DOUBLE_BAR,
  TOKEN_BANG,
  TOKEN_BANG_EQUAL,
  TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER_EQUAL,
  TOKEN_LESS_EQUAL,
  TOKEN_COLON,
  TOKEN_NEWLINE,
} TokenType;

typedef struct {
  char const* value;
  size_t len;
  size_t line;
  size_t col;
  TokenType type;
  bool unary;  //< Used by ExprParser
} Token;

char* Token_format(Token* tok);
char* Token_str(Token* tok);
char const* TokenType_str(TokenType type);
unsigned long Token_toInt(Token* tok, bool* correct);

#endif // TOKEN_H
