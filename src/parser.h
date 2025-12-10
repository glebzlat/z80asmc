#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "map.h"
#include "vector.h"
#include <stdbool.h>
#include <stdio.h>

#define TOKEN_BUF_LEN 32

typedef struct {
  Lexer* lex;
  Token buf[TOKEN_BUF_LEN];
  size_t ptr;
  bool error;
  Vector* errors;
  Map* labels;
} Parser;

typedef struct {
  char* reason;
  char* line;
  size_t col;
  size_t lineno;
} ParserError;

typedef struct {
  size_t line;
  size_t addr;
  bool has_addr;
} Label;

Parser Parser_make(Lexer* lex);
void Parser_deinit(Parser* p);

void Parser_parse(Parser* p);
bool Parser_hasErrors(Parser const* p);

void ParserError_print(ParserError const* err, FILE* fout);

#endif // PARSER_H
