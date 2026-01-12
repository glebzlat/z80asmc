#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "map.h"
#include "vector.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct {
  Lexer* lex;
  Vector* buf;
  size_t ptr;
  bool error;
  Vector* errors;
  Map* labels;
  Vector* nodes;
} Parser;

typedef struct {
  char* reason;
  char* line;
  size_t col;
  size_t lineno;
} ParserError;

Parser Parser_make(Lexer* lex);
void Parser_deinit(Parser* p);

void Parser_parse(Parser* p);
bool Parser_hasErrors(Parser const* p);

void ParserError_print(ParserError const* err, FILE* fout);

#endif // PARSER_H
