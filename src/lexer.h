#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "token.h"

typedef struct Lexer Lexer;

/* Lexer interface
 *
 * Actual Lexer implementations must implement Lexer functions, assign
 * them to Lexer function pointers, and save the implementer struct instance
 * as `_m_impl`.
 *
 * Also an implementation must provide a constructor function and may provide
 * a destructor function.
 *
 * ```
 * Lexer ImplLexer_make(...);
 * void ImplLexer_deinit(Lexer* lex);
 * ```
 */
struct Lexer {
  Token (*_m_next)(void* impl);
  size_t (*_m_start)(void* impl);
  size_t (*_m_cur)(void* impl);
  size_t (*_m_bol)(void* impl);
  char* (*_m_line)(void* impl, size_t line);

  void* _m_impl;
};

/** Get the next token from the lexer
 *
 * Denotes scanning errors as tokens with `TOKEN_ERROR` type.
 *
 * @param lex Lexer instance
 * @returns Token
 */
Token Lexer_next(Lexer* lex);

/** Get the start index of the current returned token
 *
 * @param lex Lexer instance
 * @returns Token start index
 */
size_t Lexer_start(Lexer* lex);

/** Get the current index in the string
 *
 * @param lex Lexer instance
 * @returns Current index
 */
size_t Lexer_cur(Lexer* lex);

/** Get the count of Beginnings Of a Line
 *
 * @param lex Lexer instance
 * @returns Beginning Of a Line count
 */
size_t Lexer_bol(Lexer* lex);

/** Get a source line from number
 *
 * Line numbers count from 1.
 *
 * @param lex Lexer instance
 * @param line Source line number
 * @returns Null-terminated source code line
 */
char* Lexer_line(Lexer* lex, size_t line);

#endif
