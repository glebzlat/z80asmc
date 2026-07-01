#include "lexer.h"
#include <assert.h>

#define CALL(LEX_OBJ, METHOD) do {\
  assert(LEX_OBJ); \
  assert((LEX_OBJ)->METHOD); \
  assert((LEX_OBJ)->_m_impl); \
  return (LEX_OBJ)->METHOD((LEX_OBJ)->_m_impl); \
} while (0)

#define CALL_VA(LEX_OBJ, METHOD, ...) do {\
  assert(LEX_OBJ); \
  assert((LEX_OBJ)->METHOD); \
  assert((LEX_OBJ)->_m_impl); \
  return (LEX_OBJ)->METHOD((LEX_OBJ)->_m_impl, __VA_ARGS__); \
} while (0)

Token Lexer_next(Lexer* lex) {
  CALL(lex, _m_next);
}

size_t Lexer_start(Lexer* lex) {
  CALL(lex, _m_start);
}

size_t Lexer_cur(Lexer* lex) {
  CALL(lex, _m_cur);
}

size_t Lexer_bol(Lexer* lex) {
  CALL(lex, _m_bol);
}

char* Lexer_line(Lexer* lex, size_t line) {
  CALL_VA(lex, _m_line, line);
}
