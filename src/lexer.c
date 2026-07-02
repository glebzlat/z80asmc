#include <assert.h>

#include "lexer.h"
#include "interfaces.h"

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
