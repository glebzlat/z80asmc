#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "instruction.h"
#include "utility.h"
#include "vector.h"

static char const* EncodedItemKind_toStr(EncodedItemKind kind);
static char const* IRNodeKind_toStr(IRNodeKind kind);

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
  for (size_t i = 0; i < fmt_len; ++i) {
    switch (fmt[i]) {
    case 'b':
      item.kind = EI_BYTE;
      item.data.byte = va_arg(ap, int);
      break;
    case 'w':
      item.kind = EI_WORD;
      item.data.word = va_arg(ap, int);
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
