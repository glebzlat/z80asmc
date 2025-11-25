#include "utility.h"
#include <assert.h>
#include <stdarg.h>

#include <expression.h>
#include <lexer.h>

typedef struct {
  char const* lit;
  TokenType type;
  bool unary;
} ClueToken;

void testExpression(char const* input, size_t n_tokens, ...);

int main(void) {
  testExpression("1+2", 3, (ClueToken){.lit = "1", .type = TOKEN_DECIMAL},
                 (ClueToken){.lit = "2", .type = TOKEN_DECIMAL}, (ClueToken){.type = TOKEN_PLUS});
  testExpression("1+2*3", 5, (ClueToken){.lit = "1"}, (ClueToken){.lit = "2"}, (ClueToken){.lit = "3"},
                 (ClueToken){.type = TOKEN_STAR}, (ClueToken){.type = TOKEN_PLUS});
}

void testExpression(char const* input, size_t n_tokens, ...) {
  Lexer lex = Lexer_make(input);
  ExprParser parser = ExprParser_make();

  while (true) {
    Token tok = Lexer_next(&lex);

    assert(ExprParser_get(&parser, tok) != -1);

    if (tok.type == TOKEN_END)
      break;
  }

  for (size_t i = 0; i < Vector_len(parser.e); ++i) {
    char *tok_str = Token_format(Vector_at(parser.e, i));
    printf("%s ", tok_str);
    free(tok_str);
  }
  printf("\n");

  assert(Vector_len(parser.e) == n_tokens);

  va_list ap;
  va_start(ap, n_tokens);
  for (size_t i = 0; i < n_tokens; ++i) {
    Token* tok = Vector_at(parser.e, i);
    ClueToken clue = va_arg(ap, ClueToken);

    if (clue.type)
      assert(tok->type == clue.type);
    if (clue.lit)
      assert(strncasecmp(tok->value, clue.lit, tok->len) == 0);
    assert(tok->unary == clue.unary);
  }
  va_end(ap);

  ExprParser_deinit(&parser);
}
