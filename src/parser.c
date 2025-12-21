#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "instruction.h"
#include "lexer.h"
#include "map.h"
#include "parser.h"
#include "utility.h"
#include "vector.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wparentheses"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wparentheses"
#endif

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
static void parseLabel(Parser* p);
static void parseInstruction(Parser* p);

static void error(Parser* p, const char* fmt, ...);

static void map_destroy_ir_label(void* value);

Parser Parser_make(Lexer* lex) {
  assert(lex);

  Vector* errors = Vector_new(sizeof(ParserError));
  if (!errors)
    die("Vector_new() failed");

  Map* labels = Map_new(sizeof(int32_t), map_destroy_ir_label);
  if (!labels)
    die("Map_new() failed");

  Vector* nodes = Vector_new(sizeof(IRNode));
  if (!nodes)
    die("Vector_new() failed");

  return (Parser){
      .lex = lex,
      .errors = errors,
      .labels = labels,
      .nodes = nodes,
  };
}

void Parser_deinit(Parser* p) {
  assert(p);

  for (size_t i = 0; i < Vector_len(p->errors); ++i) {
    ParserError* err = Vector_at(p->errors, i);
    free(err->reason);
    if (err->line)
      free(err->line);
  }

  MapIter it = MapIter_init(p->labels);
  while (MapIter_next(&it)) {
    printf("Label %s\n", it.key);
  }

  Vector_destroy(p->errors);
  Map_destroy(p->labels);

  for (size_t i = 0; i < Vector_len(p->nodes); ++i) {
    IRNode* n = Vector_at(p->nodes, i);
    if (n->kind == IR_INSTRUCTION)
      Vector_destroy(n->data.instruction.encoded_items);
    else if (n->kind == IR_LABEL)
      free(n->data.label.name);
  }
  Vector_destroy(p->nodes);
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

  while (true) {
    parseLabel(p);
    parseInstruction(p);
    if (p->buf[p->ptr].type == TOKEN_END)
      break;

    p->ptr = 0;
    for (size_t i = 1; i < TOKEN_BUF_LEN && p->buf[i].type != TOKEN_UNINITIALIZED; ++i)
      p->buf[i].type = TOKEN_UNINITIALIZED;
  }
}

bool Parser_hasErrors(Parser const* p) {
  assert(p);

  return !Vector_isEmpty(p->errors);
}

void ParserError_print(ParserError const* err, FILE* fout) {
  assert(err);
  assert(fout);

  fprintf(fout, "%zu:%zu: error: %s\n", err->lineno, err->col, err->reason);
  if (err->line)
    fprintf(fout, "%s\n%*s", err->line, (int)err->col, "^\n");
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
  /* Check whether the newline or END is already consumed. It is required due
   * to parseLabel function: it tries to consume two tokens: an ID and a colon,
   * and backtracks if the tokens don't match. */
  for (size_t i = p->ptr; i < TOKEN_BUF_LEN; ++i)
    if (p->buf[i].type == TOKEN_NEWLINE || p->buf[i].type == TOKEN_END)
      return;
  Token tok = p->buf[p->ptr];
  while (tok.type != TOKEN_NEWLINE && tok.type != TOKEN_END)
    tok = Lexer_next(p->lex);
}

static void parseLabel(Parser* p) {
  advance(p);
  if (p->buf[p->ptr].type != TOKEN_ID) {
    p->ptr -= 1;
    return;
  }
  advance(p);
  if (p->buf[p->ptr].type != TOKEN_COLON) {
    p->ptr -= 2;
    return;
  }

  Token* label_tok = &p->buf[p->ptr - 1];

  char* label_name = Token_str(label_tok);
  if (!label_name)
    die("Token_str() failed");

  IRNode node = {
      .kind = IR_LABEL,
      .data = {.label = {.name = label_name, .line = label_tok->line}},
  };
  Vector_push(p->nodes, &node);
  Map_set(p->labels, node.data.label.name, &node.data.label);
}

void parseInstruction(Parser* p) {
  size_t n_results = 0, n_results_save = 0, ptr_save = 0;
  Result results[SAVED_RESULTS_LEN] = {0};

#define ALT(COND, BODY)                                                                                                \
  do {                                                                                                                 \
    ptr_save = p->ptr;                                                                                                 \
    if (COND) {                                                                                                        \
      BODY;                                                                                                            \
      if (!tokenType(p, TOKEN_NEWLINE).success && !tokenType(p, TOKEN_END).success) {                                  \
        error(p, "excessive characters at the end of an instruction: %.*s", (int)p->buf[p->ptr].len,                   \
              p->buf[p->ptr].value);                                                                                   \
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
    return;

  if (p->buf[p->ptr].type == TOKEN_NEWLINE)
    return;

  if (tokenId(p, "ld").success) {
    advance(p);
    ALT(MATCH_SAVE(reg8Bit(p)) && MATCH(comma(p)) && MATCH_SAVE(reg8Bit(p)), {
      IRNode node = IRNode_createInstruction("bb", results[0].value.val, results[1].value.val);
      Vector_push(p->nodes, &node);
    });
    ALT(MATCH_SAVE(reg8Bit(p)) && MATCH(comma(p)) && MATCH_SAVE(int8Bit(p)), {
      IRNode node = IRNode_createInstruction("bb", results[0].value.val, results[1].value.val);
      Vector_push(p->nodes, &node);
    });
    error(p, "wrong operands to instruction");
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
    error(p, "expected instruction name");
    skip(p);
  } else {
    error(p, "unknown instruction: %.*s", (int)p->buf[p->ptr].len, p->buf[p->ptr].value);
    skip(p);
  }

error:
  return;

success:
  printf("success\n");

#undef ALT
#undef MATCH_SAVE
#undef MATCH
  return;
}

static void error(Parser* p, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char* str = vdsprintf(fmt, ap);
  va_end(ap);

  if (!str)
    die("vdsprintf() failed");

  Token* last = &p->buf[p->ptr];
  char* source_line = Lexer_line(p->lex, last->line);

  ParserError e = {
      .reason = str,
      .line = source_line,
      .col = last->col,
      .lineno = last->line,
  };

  Vector_push(p->errors, &e);
}

static void map_destroy_ir_label(void* value) {}
