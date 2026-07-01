#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "string_lexer.h"
#include "utility.h"

typedef struct {
  char const* buf;
  size_t start;
  size_t cur;
  size_t line;
  size_t bol;
} StringLexer;

static Token StringLexer_next(void* impl);
static size_t StringLexer_start(void* impl);
static size_t StringLexer_cur(void* impl);
static size_t StringLexer_bol(void* impl);
static char* StringLexer_line(void* impl, size_t line);

static bool isAtEnd(StringLexer* lex);
static char peek(StringLexer* lex);
static char advance(StringLexer* lex);
static bool eatWhitespace(StringLexer* lex);
static bool matchChar(StringLexer* lex, char c);
static bool matchLiteral(StringLexer* lex, char const* lit);
static bool matchRange(StringLexer* lex, char start, char end);
static bool matchRanges(StringLexer* lex, size_t n, ...);
static Token parseNumber(StringLexer* lex);
static Token parseLiteral(StringLexer* lex);
static Token parseChar(StringLexer* lex);
static Token parseString(StringLexer* lex);
static Token makeErrorToken(StringLexer* lex, char const* msg);
static Token makeToken(StringLexer* lex, TokenType type);
static Token makeTokenIdx(StringLexer* lex, TokenType type, size_t start, size_t end);

static char const chars[] = "abdefnrtvABDEFNRTV0'\"\\";

Lexer StringLexer_make(char const* buf) {
  assert(buf);

  StringLexer* impl = malloc(sizeof(*impl));
  if (!impl) {
    die("StringLexer_make(): malloc failed");
  }
  impl->buf = buf;

  Lexer lex = {
      ._m_next = StringLexer_next,
      ._m_start = StringLexer_start,
      ._m_cur = StringLexer_cur,
      ._m_bol = StringLexer_bol,
      ._m_line = StringLexer_line,
      ._m_impl = impl,
  };

  return lex;
}

void StringLexer_deinit(Lexer* lex) {
  assert(lex);
  free(lex->_m_impl);
}

Token StringLexer_next(void* impl) {
  StringLexer* lex = impl;

  while (eatWhitespace(lex))
    ;

  if (isAtEnd(lex))
    return makeToken(lex, TOKEN_END);

  lex->start = lex->cur;

  Token result;
  if ((result = parseNumber(lex)).type != TOKEN_UNINITIALIZED ||
      (result = parseLiteral(lex)).type != TOKEN_UNINITIALIZED)
    return result;

  char c = advance(lex);
  switch (c) {
  case '(':
    return makeToken(lex, TOKEN_LEFT_PAREN);
  case ')':
    return makeToken(lex, TOKEN_RIGHT_PAREN);
  case ',':
    return makeToken(lex, TOKEN_COMMA);
  case '-':
    return makeToken(lex, TOKEN_MINUS);
  case '+':
    return makeToken(lex, TOKEN_PLUS);
  case '/':
    return makeToken(lex, TOKEN_SLASH);
  case '*':
    return makeToken(lex, TOKEN_STAR);
  case '%':
    return makeToken(lex, TOKEN_PERCENT);
  case '^':
    return makeToken(lex, TOKEN_CAP);
  case '~':
    return makeToken(lex, TOKEN_TILDE);
  case '&':
    if (matchChar(lex, '&'))
      return makeToken(lex, TOKEN_DOUBLE_AMPERSAND);
    return makeToken(lex, TOKEN_AMPERSAND);
  case '|':
    if (matchChar(lex, '|'))
      return makeToken(lex, TOKEN_DOUBLE_BAR);
    return makeToken(lex, TOKEN_BAR);
  case '!':
    if (matchChar(lex, '='))
      return makeToken(lex, TOKEN_BANG_EQUAL);
    return makeToken(lex, TOKEN_BANG);
  case '=':
    if (matchChar(lex, '='))
      return makeToken(lex, TOKEN_EQUAL_EQUAL);
    return makeErrorToken(lex, "expected equal sign");
  case '<':
    if (matchChar(lex, '='))
      return makeToken(lex, TOKEN_LESS_EQUAL);
    if (matchChar(lex, '<'))
      return makeToken(lex, TOKEN_LEFT_SHIFT);
    return makeToken(lex, TOKEN_LEFT_BRACE);
  case '>':
    if (matchChar(lex, '='))
      return makeToken(lex, TOKEN_GREATER_EQUAL);
    if (matchChar(lex, '>'))
      return makeToken(lex, TOKEN_RIGHT_SHIFT);
    return makeToken(lex, TOKEN_RIGHT_BRACE);
  case '\'':
    lex->start = lex->cur;
    result = parseChar(lex);
    if (result.type != TOKEN_ERROR) {
      if (matchChar(lex, '\'')) {
        return result;
      } else {
        return makeErrorToken(lex, "expected closing mark");
      }
    }
    return result;
  case '"':
    lex->start = lex->cur;
    result = parseString(lex);
    if (result.type != TOKEN_ERROR) {
      if (matchChar(lex, '"')) {
        return result;
      } else {
        return makeErrorToken(lex, "expected closing mark");
      }
    }
    return result;
  case ':':
    return makeToken(lex, TOKEN_COLON);
  case '\n':
    return makeToken(lex, TOKEN_NEWLINE);
  }

  return makeErrorToken(lex, "unknown token");
}

static size_t StringLexer_start(void* impl) {
  StringLexer* lex = impl;
  return lex->start;
}

static size_t StringLexer_cur(void* impl) {
  StringLexer* lex = impl;
  return lex->cur;
}

static size_t StringLexer_bol(void* impl) {
  StringLexer* lex = impl;
  return lex->bol;
}

char* StringLexer_line(void* impl, size_t line) {
  assert(line > 0);
  StringLexer* lex = impl;

  /* Count lines and stop when n_line == line */
  size_t start = 0, n_line = 1;
  for (; lex->buf[start] != '\0'; ++start) {
    if (lex->buf[start] == '\n')
      n_line += 1;
    if (n_line == line)
      break;
  }

  /* Actual number of lines is less than the given number */
  if (n_line < line)
    return NULL;

  if (start == '\n')
    start += 1;

  /* Get the rest of the line */
  size_t end = start + 1;
  while (lex->buf[end] != '\n' && lex->buf[end] != '\0')
    end += 1;

  size_t len = end - start;
  char* buf = malloc(len + 1);
  if (!buf)
    return NULL;

  strncpy(buf, lex->buf + start, len);
  buf[len] = '\0';

  return buf;
}

static bool isAtEnd(StringLexer* lex) { return lex->buf[lex->cur] == '\0'; }

static char peek(StringLexer* lex) { return lex->buf[lex->cur]; }

static char advance(StringLexer* lex) {
  if (isAtEnd(lex))
    return '\0';

  if (lex->buf[lex->cur] == '\n') {
    lex->bol = lex->cur + 1;
    lex->line += 1;
  }

  return lex->buf[lex->cur++];
}

/*
 * Return true if we have consumed a comment. Then the lexer will try one more
 * time to cover the case if there is a subsequent comment and thus omit
 * all comment lines.
 */
static bool eatWhitespace(StringLexer* lex) {
  char c = peek(lex);

  switch (c) {
  case ' ':
  case '\r':
  case '\t':
    advance(lex);
    break;
  case ';':
    while (!isAtEnd(lex) && advance(lex) != '\n')
      ;
    return true;
  default:
    return false;
  }

  return eatWhitespace(lex);
}

static bool matchChar(StringLexer* lex, char c) {
  if (peek(lex) == c) {
    advance(lex);
    return true;
  }
  return false;
}

static bool matchLiteral(StringLexer* lex, char const* lit) {
  size_t save = lex->cur;
  for (size_t i = 0; i < strlen(lit); ++i) {
    if (peek(lex) != lit[i]) {
      lex->cur = save;
      return false;
    }
    advance(lex);
  }
  return true;
}

static bool matchRange(StringLexer* lex, char start, char end) {
  assert(start <= end);
  if (peek(lex) >= start && peek(lex) <= end) {
    advance(lex);
    return true;
  }
  return false;
}

static bool matchRanges(StringLexer* lex, size_t n, ...) {
  va_list ap;
  bool result = false;

  va_start(ap, n);
  for (size_t i = 0; i < n; ++i) {
    int start = va_arg(ap, int), end = va_arg(ap, int);
    assert(start > CHAR_MIN && start < CHAR_MAX);
    assert(end > CHAR_MIN && end < CHAR_MAX);
    assert(start <= end);
    if (matchRange(lex, (char)start, (char)end)) {
      result = true;
      break;
    }
  }
  va_end(ap);

  return result;
}

static Token parseNumber(StringLexer* lex) {
  bool ok = false;
  Token tok = {0};

  // ("0b" / '%') [01]+
  if (matchLiteral(lex, "0b") || matchLiteral(lex, "0B") || matchChar(lex, '%')) {
    lex->start = lex->cur;
    while (matchRange(lex, '0', '1'))
      ok = true;
    if (ok)
      tok = makeToken(lex, TOKEN_BINARY);
    else
      tok = makeErrorToken(lex, "incorrect binary number");
    return tok;
  }

  // "0q" [0-7]+
  if (matchLiteral(lex, "0q") || matchLiteral(lex, "0Q")) {
    lex->start = lex->cur;
    while (matchRange(lex, '0', '7'))
      ok = true;
    if (ok)
      tok = makeToken(lex, TOKEN_OCTAL);
    else
      tok = makeErrorToken(lex, "incorrect octal number");
    return tok;
  }

  // [0-1]+ 'b'
  if (matchRange(lex, '0', '1')) {
    while (matchRange(lex, '0', '1'))
      ;
    if (peek(lex) == 'b') {
      tok = makeToken(lex, TOKEN_BINARY);
      advance(lex);
      return tok;
    }
    lex->cur = lex->start;
  }

  // [0-7]+ [oq]
  if (matchRange(lex, '0', '7')) {
    while (matchRange(lex, '0', '7'))
      ;
    if (peek(lex) == 'q' || peek(lex) == 'o' || peek(lex) == 'Q' || peek(lex) == 'O') {
      tok = makeToken(lex, TOKEN_OCTAL);
      advance(lex);
      return tok;
    }
    lex->cur = lex->start;
  }

  // [1-9][0-9]*
  if (matchRange(lex, '1', '9')) {
    while (matchRange(lex, '0', '9'))
      ;
    tok = makeToken(lex, TOKEN_DECIMAL);
    if (peek(lex) == 'd' || peek(lex) == 'D')
      advance(lex);
    return tok;
  }

  // ("0x" / '$' / '#') [a-fA-F0-9]+
  if (matchLiteral(lex, "0x") || matchLiteral(lex, "0X") || matchChar(lex, '$') || matchChar(lex, '#')) {
    lex->start = lex->cur;
    while (matchRanges(lex, 3, 'a', 'f', 'A', 'F', '0', '9'))
      ok = true;
    if (!ok)
      tok = makeErrorToken(lex, "incorrect hexadecimal number");
    else
      tok = makeToken(lex, TOKEN_HEXADECIMAL);
    return tok;
  }

  return tok;
}

static Token parseLiteral(StringLexer* lex) {
  // [a-zA-Z_][a-zA-Z0-9_]*
  Token tok = {0};
  if (matchRanges(lex, 3, 'a', 'z', 'A', 'Z', '_', '_')) {
    size_t col = lex->cur - lex->bol;
    while (matchRanges(lex, 4, 'a', 'z', 'A', 'Z', '0', '9', '_', '_'))
      ;
    tok = makeToken(lex, TOKEN_ID);
    tok.col = col;
  }
  return tok;
}

static Token parseChar(StringLexer* lex) {
  if (isAtEnd(lex) || peek(lex) == '\n')
    return makeErrorToken(lex, "expected a character");
  size_t const chars_len = sizeof(chars) - 1;
  if (matchChar(lex, '\\')) {
    for (size_t i = 0; i < chars_len; ++i)
      if (matchChar(lex, chars[i]))
        return makeToken(lex, TOKEN_CHAR);
    return makeErrorToken(lex, "incorrect escape char");
  }
  advance(lex);
  return makeToken(lex, TOKEN_CHAR);
}

static Token parseString(StringLexer* lex) {
  Token tok;
  size_t col = lex->cur - lex->bol;
  while (true) {
    if (peek(lex) == '"') {
      tok = makeToken(lex, TOKEN_CHAR);
      break;
    }
    tok = parseChar(lex);
    if ((tok.type == TOKEN_CHAR && tok.value[tok.len] == '"') || tok.type == TOKEN_ERROR)
      break;
  }
  tok.col = col;
  if (tok.type != TOKEN_ERROR)
    tok.type = TOKEN_STRING;
  return tok;
}

static Token makeErrorToken(StringLexer* lex, char const* msg) {
  return (Token){
      .type = TOKEN_ERROR,
      .col = lex->cur - lex->bol,
      .line = lex->line + 1,
      .len = lex->cur - lex->start,
      .value = msg,
  };
}

static Token makeToken(StringLexer* lex, TokenType type) { return makeTokenIdx(lex, type, lex->start, lex->cur); }

static Token makeTokenIdx(StringLexer* lex, TokenType type, size_t start, size_t end) {
  return (Token){
      .type = type,
      .col = end - lex->bol,
      .line = lex->line + 1,
      .len = end - start,
      .value = lex->buf + start,
  };
}
