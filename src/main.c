#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utility.h"
#include "parser.h"

int main(int argc, char** argv) {
  int exitcode = 0;

  FILE* fin = fopen(argv[1], "r");
  if (!fin)
    die("fopen() failed");

  char data[100];
  size_t n_read = fread(data, 1, 100, fin);
  data[n_read] = '\0';

  Lexer lex = Lexer_make(data);

  Parser p = Parser_make(&lex);
  Parser_parse(&p);

  if (Parser_hasErrors(&p)) {
    for (size_t i = 0; i < Vector_len(p.errors); ++i)
      ParserError_print(Vector_at(p.errors, i), stderr);
    exitcode = 1;
  }

  Parser_deinit(&p);
  fclose(fin);

  return exitcode;
}
