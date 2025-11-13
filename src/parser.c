#include <assert.h>
#include <stdint.h>

#include "lexer.h"
#include "parser.h"
#include "utility.h"

#define SAVED_RESULTS_LEN 8

#define SUCCESS(VAL)                                                                                                   \
  (Result) {                                                                                                           \
    .success = true, .value = {.val = (VAL) }                                                                          \
  }
#define FAILURE                                                                                                        \
  (Result) { .success = false }

typedef struct {
  uint8_t val;
} Value;

typedef struct {
  bool success;
  Value value;
} Result;

static Result tokenType(Parser* p, TokenType type);
static Result tokenTypeValue(Parser* p, TokenType type, char const* val);
static Result tokenId(Parser* p, char const* val);

static Result reg8Bit(Parser* p);
static Result int8Bit(Parser* p);
static Result comma(Parser* p);

static void advance(Parser* p);
static void skip(Parser* p);
static int parseInstruction(Parser* p);

Parser Parser_make(Lexer* lex) {
  assert(lex);
  return (Parser){.lex = lex};
}

static inline bool match_save(Parser* p, Result r, size_t* n_results, Result arr[]) {
  if (*n_results == SAVED_RESULTS_LEN)
    die("n_results == SAVED_RESULTS_LEN");
  if (!r.success)
    return false;
  arr[(*n_results)++] = r;
  advance(p);
  return true;
}

static inline bool match_discard(Parser* p, Result r) {
  if (!r.success)
    return false;
  advance(p);
  return true;
}

void Parser_parse(Parser* p) {
  assert(p);

  while (parseInstruction(p)) {
    p->ptr = 0;
    for (size_t i = 1; i < TOKEN_BUF_LEN && p->buf[i].type != TOKEN_UNINITIALIZED; ++i)
      p->buf[i].type = TOKEN_UNINITIALIZED;
  }
}

Result tokenType(Parser* p, TokenType type) { return (Result){.success = p->buf[p->ptr].type == type}; }

Result tokenTypeValue(Parser* p, TokenType type, char const* val) {
  Token cur = p->buf[p->ptr];
  bool success = cur.type == type && strncasecmp(cur.value, val, cur.len) == 0;
  return (Result){.success = success};
}

Result tokenId(Parser* p, char const* val) { return tokenTypeValue(p, TOKEN_ID, val); }

Result reg8Bit(Parser* p) {

  if (tokenId(p, "a").success)
    return SUCCESS(0x07);
  else if (tokenId(p, "b").success)
    return SUCCESS(0x00);
  else if (tokenId(p, "c").success)
    return SUCCESS(0x01);
  else if (tokenId(p, "d").success)
    return SUCCESS(0x02);
  else if (tokenId(p, "e").success)
    return SUCCESS(0x03);
  else if (tokenId(p, "h").success)
    return SUCCESS(0x08);
  else if (tokenId(p, "l").success)
    return SUCCESS(0x09);
  return FAILURE;
}

Result int8Bit(Parser* p) {
  // TODO: Add a vector of errors and an error flag to the Parser.
  // If the parsing function raises the error flag, parser skips the line
  // entirely. Modify the lexer to treat newlines not as a whitespace, but
  // rather than a delimiter. In this case I will be able to skip the line
  // on which the error is triggered entirely and start over.
  uint64_t result;
  Token cur = p->buf[p->ptr];
  if (cur.type == TOKEN_DECIMAL || cur.type == TOKEN_HEXADECIMAL || cur.type == TOKEN_OCTAL ||
      cur.type == TOKEN_BINARY || cur.type == TOKEN_CHAR)
    result = Token_toInt(&cur);
  else
    return FAILURE;

  if (result >= UINT8_MAX) {
    p->error = true;
    printf("int8Bit(): value is greater than 8 bit\n");
    return FAILURE;
  }

  return SUCCESS((uint8_t)result);
}

Result comma(Parser* p) { return tokenType(p, TOKEN_COMMA); }

void advance(Parser* p) {
  p->ptr += 1;
  if (p->ptr == TOKEN_BUF_LEN)
    die("advance(): token buffer is full");
  if (p->buf[p->ptr].type != TOKEN_UNINITIALIZED)
    return;
  p->buf[p->ptr] = Lexer_next(p->lex);
}

void skip(Parser* p) {
  Token tok = p->buf[p->ptr];
  while (tok.type != TOKEN_NEWLINE && tok.type != TOKEN_END)
    tok = Lexer_next(p->lex);
}

int parseInstruction(Parser* p) {
  size_t n_results = 0, n_results_save = 0, ptr_save = 0;
  Result results[SAVED_RESULTS_LEN] = {0};

#define ALT(COND, BODY)                                                                                                \
  do {                                                                                                                 \
    ptr_save = p->ptr;                                                                                                 \
    if (COND) {                                                                                                        \
      BODY;                                                                                                            \
      if (!tokenType(p, TOKEN_NEWLINE).success && !tokenType(p, TOKEN_END).success) {                                  \
        printf("parse(): excessive characters at the end of an instruction\n");                                        \
        goto error;                                                                                                    \
      }                                                                                                                \
      goto success;                                                                                                    \
    } else if (p->error) {                                                                                             \
      goto error;                                                                                                      \
    } else {                                                                                                           \
      p->ptr = ptr_save;                                                                                               \
      n_results = n_results_save;                                                                                      \
    }                                                                                                                  \
  } while (0)

#define MATCH_SAVE(CALL) (match_save(p, (CALL), &n_results, results))
#define MATCH(CALL) (match_discard(p, (CALL)))

  advance(p);
  if (p->buf[p->ptr].type == TOKEN_END)
    return 0;

  if (p->buf[p->ptr].type == TOKEN_NEWLINE)
    return 1;

  if (tokenId(p, "ld").success) {
    advance(p);
    ALT(MATCH_SAVE(reg8Bit(p)) && MATCH(comma(p)) && MATCH_SAVE(reg8Bit(p)),
        { printf("ld R R: r1=%i r2=%i\n", results[0].value.val, results[1].value.val); });
    ALT(MATCH_SAVE(reg8Bit(p)) && MATCH(comma(p)) && MATCH_SAVE(int8Bit(p)),
        { printf("ld R N: r=%i, n=%i\n", results[0].value.val, results[1].value.val); });
    printf("wrong operands to instruction\n");
    skip(p);
    goto error;
  } else if (tokenId(p, "push").success) {
    advance(p);
    ALT(MATCH(tokenId(p, "bc")), printf("push bc\n"));
    ALT(MATCH(tokenId(p, "de")), printf("push de\n"));
    ALT(MATCH(tokenId(p, "hl")), printf("push hl"));
  } else if (tokenId(p, "pop").success) {
    advance(p);
    ALT(MATCH(tokenId(p, "bc")), printf("pop bc\n"));
  } else if (p->buf[p->ptr].type != TOKEN_ID) {
    printf("error: expected instruction name, %s\n", TokenType_str(p->buf[p->ptr].type));
    skip(p);
  } else {
    printf("error: unknown instruction: %.*s\n", (int)p->buf[p->ptr].len, p->buf[p->ptr].value);
    skip(p);
  }

error:
  printf("error\n");
  return 1;

success:
  printf("success\n");

#undef ALT
#undef MATCH_SAVE
#undef MATCH
  return 1;
}
