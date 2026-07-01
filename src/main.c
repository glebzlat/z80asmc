#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instruction.h"
#include "utility.h"
#include "parser.h"
#include "string_lexer.h"

int main(int argc, char** argv) {
  int exitcode = 0;

  FILE* fin = fopen(argv[1], "r");
  if (!fin)
    die("fopen() failed");

  char* data = ffullread(fin);
  if (!data) {
    die("failed to read the file");
  }

  Lexer lex = StringLexer_make(data);

  Parser p = Parser_make(&lex);
  Parser_parse(&p);

  if (Parser_hasErrors(&p)) {
    for (size_t i = 0; i < Vector_len(p.errors); ++i)
      ParserError_print(Vector_at(p.errors, i), stderr);
    exitcode = 1;
  }

  MapIter it = MapIter_init(p.labels);
  while (MapIter_next(&it)) {
    printf("Label %s\n", it.key);
  }

  for (size_t i = 0; i < Vector_len(p.nodes); ++i)
    IRNode_print(stdout, Vector_at(p.nodes, i));

  Parser_deinit(&p);
  StringLexer_deinit(&lex);
  fclose(fin);
  free(data);

  return exitcode;
}
