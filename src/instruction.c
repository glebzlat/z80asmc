#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "instruction.h"
#include "lexer.h"
#include "utility.h"
#include "vector.h"

static void IRInstruction_print(FILE* fout, IRInstruction* iri);
static void IRLabel_print(FILE* fout, IRLabel* irl);
static void EncodedItem_print(FILE* fout, EncodedItem* item);
static void EncodedItem_printExpr(FILE* fout, Vector* expr);

IRNode IRNode_createInstruction(char const* fmt, ...) {
  assert(fmt);

  Vector* items = Vector_new(sizeof(EncodedItem));
  if (!items)
    die("Vector_new() failed");

  IRNode node = {.kind = IR_INSTRUCTION, .data.instruction.encoded_items = items};

  va_list ap;
  va_start(ap, fmt);
  size_t const fmt_len = strlen(fmt);
  EncodedItem item = {0};
  int tmp;
  for (size_t i = 0; i < fmt_len; ++i) {
    switch (fmt[i]) {
    case 'b':
      item.kind = EI_BYTE;
      tmp = va_arg(ap, int);
      if (tmp < 0 || tmp >= UINT8_MAX)
        die("IRNode_createInstruction(): invalid byte value");
      item.data.byte = (uint8_t)tmp;
      break;
    case 'e':
      item.kind = EI_EXPR;
      item.data.expr = va_arg(ap, Vector*);
      break;
    default:
      die("IRNode_createInstruction(): incorrect format specifier");
    }

    if (Vector_push(items, &item) == -1)
      die("Vector_push() failed");
  }
  va_end(ap);

  return node;
}

void IRNode_print(FILE* fout, IRNode* n) {
  assert(fout);
  assert(n);

  switch (n->kind) {
    case IR_INSTRUCTION:
      IRInstruction_print(fout, &n->data.instruction);
      break;
    case IR_LABEL:
      IRLabel_print(fout, &n->data.label);
      break;
    default:
      die("IRNode_print(): invalid kind");
  }

  fprintf(fout, "\n");
}

static void IRInstruction_print(FILE* fout, IRInstruction* iri) {
  fprintf(fout, "INSTRUCTION ");
  size_t const len = Vector_len(iri->encoded_items);
  for (size_t i = 0; i < len; ++i) {
    EncodedItem_print(fout, Vector_at(iri->encoded_items, i));
    if (i != len - 1)
      fprintf(fout, ", ");
  }
}

static void IRLabel_print(FILE* fout, IRLabel* irl) {
  fprintf(fout, "LABEL name=%s line=%zu addr=0x%04x", irl->name, irl->line, irl->addr);
}

static void EncodedItem_print(FILE* fout, EncodedItem* item) {
  switch (item->kind) {
    case EI_BYTE:
      fprintf(fout, "(BYTE %02x)", item->data.byte);
      break;
    case EI_EXPR:
      fprintf(fout, "(EXPR ");
      EncodedItem_printExpr(fout, item->data.expr);
      fprintf(fout, ")");
      break;
    default:
      die("EncodedItem_print(): invalid kind");
  }
}

static void EncodedItem_printExpr(FILE* fout, Vector* expr) {
  size_t const len = Vector_len(expr);
  for (size_t i = 0; i < len; ++i) {
    char* tok_str = Token_format(Vector_at(expr, i));
    fprintf(fout, "%s", tok_str);
    if (i != len - 1)
      fprintf(fout, ", ");
  }
}
