#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

#define TOKEN_BUF_LEN 32

typedef struct {
  Lexer* lex;
  Token buf[TOKEN_BUF_LEN];
  size_t ptr;
  bool error;
} Parser;

Parser Parser_make(Lexer* lex);
void Parser_parse(Parser* p);

#endif // PARSER_H
