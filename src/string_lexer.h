#ifndef STRING_LEXER_H
#define STRING_LEXER_H

#include "lexer.h"

Lexer StringLexer_make(char const* buf);
void StringLexer_deinit(Lexer* lex);

#endif // STRING_LEXER_H
