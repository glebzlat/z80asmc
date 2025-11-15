#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "utility.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wswitch-enum"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wswitch-enum"
#endif

static bool isAtEnd(Lexer* lex);
static char peek(Lexer* lex);
static char advance(Lexer* lex);
static bool eatWhitespace(Lexer* lex);
static bool matchChar(Lexer* lex, char c);
static bool matchLiteral(Lexer* lex, char const* lit);
static bool matchRange(Lexer* lex, char start, char end);
static bool matchRanges(Lexer* lex, size_t n, ...);
static Token parseNumber(Lexer* lex);
static Token parseLiteral(Lexer* lex);
static Token parseChar(Lexer* lex);
static Token parseString(Lexer* lex);
static Token makeErrorToken(Lexer* lex, char const* msg);
static Token makeToken(Lexer* lex, TokenType type);
static Token makeTokenIdx(Lexer* lex, TokenType type, size_t start, size_t end);
static int escToInt(char const* ch);

static char const chars[] = "abdefnrtvABDEFNRTV0'\"\\";

char* Token_format(Token* tok) {
  assert(tok);
  char* buf = NULL;
  if (tok->type == TOKEN_ERROR) {
    buf = dsprintf("%li:%li:%s:%s", tok->line, tok->col, TokenType_str(tok->type), tok->value);
  } else {
    assert(tok->len < INT_MAX);
    buf = dsprintf("%li:%li:%s:%.*s", tok->line, tok->col, TokenType_str(tok->type), (int)tok->len, tok->value);
  }
  if (!buf)
    die("dsprintf() failed");
  return buf;
}

char const* TokenType_str(TokenType type) {
  switch (type) {
  case TOKEN_UNINITIALIZED:
    return "TOKEN_UNINITIALIZED";
  case TOKEN_END:
    return "TOKEN_END";
  case TOKEN_ERROR:
    return "TOKEN_ERROR";
  case TOKEN_ID:
    return "TOKEN_ID";
  case TOKEN_CHAR:
    return "TOKEN_CHAR";
  case TOKEN_STRING:
    return "TOKEN_STRING";
  case TOKEN_DECIMAL:
    return "TOKEN_DECIMAL";
  case TOKEN_HEXADECIMAL:
    return "TOKEN_HEXADECIMAL";
  case TOKEN_OCTAL:
    return "TOKEN_OCTAL";
  case TOKEN_BINARY:
    return "TOKEN_BINARY";
  case TOKEN_LEFT_PAREN:
    return "TOKEN_LEFT_PAREN";
  case TOKEN_RIGHT_PAREN:
    return "TOKEN_RIGHT_PAREN";
  case TOKEN_LEFT_BRACE:
    return "TOKEN_LEFT_BRACE";
  case TOKEN_RIGHT_BRACE:
    return "TOKEN_RIGHT_BRACE";
  case TOKEN_COMMA:
    return "TOKEN_COMMA";
  case TOKEN_MINUS:
    return "TOKEN_MINUS";
  case TOKEN_PLUS:
    return "TOKEN_PLUS";
  case TOKEN_SLASH:
    return "TOKEN_SLASH";
  case TOKEN_STAR:
    return "TOKEN_STAR";
  case TOKEN_PERCENT:
    return "TOKEN_PERCENT";
  case TOKEN_CAP:
    return "TOKEN_CAP";
  case TOKEN_TILDE:
    return "TOKEN_TILDE";
  case TOKEN_AMPERSAND:
    return "TOKEN_AMPERSAND";
  case TOKEN_BAR:
    return "TOKEN_BAR";
  case TOKEN_DOUBLE_AMPERSAND:
    return "TOKEN_DOUBLE_AMPERSAND";
  case TOKEN_DOUBLE_BAR:
    return "TOKEN_DOUBLE_BAR";
  case TOKEN_BANG:
    return "TOKEN_BANG";
  case TOKEN_BANG_EQUAL:
    return "TOKEN_BANG_EQUAL";
  case TOKEN_EQUAL_EQUAL:
    return "TOKEN_EQUAL_EQUAL";
  case TOKEN_GREATER_EQUAL:
    return "TOKEN_GREATER_EQUAL";
  case TOKEN_LESS_EQUAL:
    return "TOKEN_LESS_EQUAL";
  case TOKEN_NEWLINE:
    return "TOKEN_NEWLINE";
  default:
    die("TokenType_str: unknown token type");
  }
}

unsigned long Token_toInt(Token* tok) {
  assert(tok);
  char* end = NULL;
  long value = 0;

  switch (tok->type) {
  case TOKEN_HEXADECIMAL:
    value = strtol(tok->value, &end, 16);
    break;
  case TOKEN_DECIMAL:
    value = strtol(tok->value, &end, 10);
    break;
  case TOKEN_OCTAL:
    value = strtol(tok->value, &end, 8);
    break;
  case TOKEN_BINARY:
    value = strtol(tok->value, &end, 2);
    break;
  case TOKEN_CHAR:
    value = escToInt(tok->value);
    break;
  default:
    die("Token_toInt(): could not convert to integer");
  }

  assert(value >= 0);
  if (end && tok->value + tok->len != end)
    die("Token_toInt(): incorrect integer");
  return (unsigned)value;
}

Lexer Lexer_make(char const* buf) {
  assert(buf);
  Lexer lex = {.buf = buf};
  return lex;
}

Token Lexer_next(Lexer* lex) {
  assert(lex);

  if (isAtEnd(lex))
    return makeToken(lex, TOKEN_END);

  while (eatWhitespace(lex))
    ;
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
    if (matchChar(lex, '|'))
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
    return makeToken(lex, TOKEN_LEFT_BRACE);
  case '>':
    if (matchChar(lex, '='))
      return makeToken(lex, TOKEN_GREATER_EQUAL);
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
  case '\n':
    return makeToken(lex, TOKEN_NEWLINE);
  }

  return (Token){0};
}

char* Lexer_line(Lexer* lex, size_t line) {
  assert(lex);
  assert(line > 0);

  size_t start = 0, n_line = 1;
  for (; lex->buf[start] != '\0'; ++start) {
    if (lex->buf[start] == '\n')
      n_line += 1;
    if (n_line == line)
      break;
  }

  if (n_line < line)
    return NULL;

  size_t end = start + 1;
  while (lex->buf[end] != '\n' && lex->buf[end] != '\0')
    end += 1;

  size_t len = end - start;
  char* buf = malloc(len + 1);
  if (!buf) {
    perror("malloc() failed");
    return NULL;
  }

  strncpy(buf, lex->buf, len);
  buf[len] = '\0';

  return buf;
}

static bool isAtEnd(Lexer* lex) { return lex->buf[lex->cur] == '\0'; }

static char peek(Lexer* lex) { return lex->buf[lex->cur]; }

static char advance(Lexer* lex) {
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
static bool eatWhitespace(Lexer* lex) {
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

static bool matchChar(Lexer* lex, char c) {
  if (peek(lex) == c) {
    advance(lex);
    return true;
  }
  return false;
}

static bool matchLiteral(Lexer* lex, char const* lit) {
  size_t save = lex->cur;
  for (size_t i = 0; i < strlen(lit); ++i) {
    if (advance(lex) != lit[i]) {
      lex->cur = save;
      return false;
    }
  }
  return true;
}

static bool matchRange(Lexer* lex, char start, char end) {
  assert(start <= end);
  if (peek(lex) >= start && peek(lex) <= end) {
    advance(lex);
    return true;
  }
  return false;
}

static bool matchRanges(Lexer* lex, size_t n, ...) {
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

static Token parseNumber(Lexer* lex) {
  bool ok = false;
  Token tok = {0};

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

  // ("0b" / '%') [01]+
  if (matchLiteral(lex, "0b") || matchLiteral(lex, "0B") || matchChar(lex, '%')) {
    lex->start = lex->cur;
    while (matchRange(lex, '0', '1'))
      ok = true;
    if (!ok)
      tok = makeErrorToken(lex, "incorrect binary number");
    else
      tok = makeToken(lex, TOKEN_BINARY);
    return tok;
  }

  // "0q" [0-7]+
  if (matchLiteral(lex, "0q") || matchLiteral(lex, "0Q")) {
    lex->start = lex->cur;
    while (matchRange(lex, '0', '7'))
      ok = true;
    if (!ok)
      tok = makeErrorToken(lex, "incorrect octal number");
    else
      tok = makeToken(lex, TOKEN_BINARY);
    return tok;
  }

  return tok;
}

static Token parseLiteral(Lexer* lex) {
  // [a-zA-Z_][a-zA-Z0-9_]*
  if (matchRanges(lex, 3, 'a', 'z', 'A', 'Z', '_', '_')) {
    while (matchRanges(lex, 4, 'a', 'z', 'A', 'Z', '0', '9', '_', '_'))
      ;
    return makeToken(lex, TOKEN_ID);
  }
  return (Token){0};
}

static Token parseChar(Lexer* lex) {
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

static Token parseString(Lexer* lex) {
  Token tok;
  size_t col = lex->cur - lex->bol;
  while (true) {
    tok = parseChar(lex);
    if (tok.type == TOKEN_CHAR && tok.value[tok.len] == '"' || tok.type == TOKEN_ERROR)
      break;
  }
  tok.col = col;
  return tok;
}

static Token makeErrorToken(Lexer* lex, char const* msg) {
  return (Token){
      .type = TOKEN_ERROR,
      .col = lex->cur - lex->bol,
      .line = lex->line + 1,
      .len = lex->cur - lex->start,
      .value = msg,
  };
}

static Token makeToken(Lexer* lex, TokenType type) { return makeTokenIdx(lex, type, lex->start, lex->cur); }

static Token makeTokenIdx(Lexer* lex, TokenType type, size_t start, size_t end) {
  return (Token){
      .type = type,
      .col = end - lex->bol,
      .line = lex->line + 1,
      .len = end - start,
      .value = lex->buf + start,
  };
}

static int escToInt(char const* ch) {
  assert(ch);
  if (ch[0] == '\\') {
    switch (ch[1]) {
    case '\\':
      return 92;
    case '?':
      return 63;
    case '\'':
      return 39;
    case '"':
      return 34;
    case '0':
      return 0;
    case 'a':
    case 'A':
      return 7;
    case 'b':
    case 'B':
      return 8;
    case 'd':
    case 'D':
      return 127;
    case 'e':
    case 'E':
      return 27;
    case 'f':
    case 'F':
      return 12;
    case 'n':
    case 'N':
      return 10;
    case 'r':
    case 'R':
      return 13;
    case 't':
    case 'T':
      return 9;
    case 'v':
    case 'V':
      return 11;
    default:
      die("escToInt(): incorrect escape sequence");
    }
  }
  return ch[0];
}
