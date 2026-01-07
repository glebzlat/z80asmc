#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>
#include <stdio.h>

#include "vector.h"

typedef enum {
  EI_BYTE,
  EI_EXPR,
} EncodedItemKind;

typedef enum {
  IR_INSTRUCTION,
  IR_LABEL,
} IRNodeKind;

typedef struct {
  EncodedItemKind kind;
  union {
    uint8_t byte;
    Vector* expr;
  } data;
} EncodedItem;

typedef struct {
  Vector* encoded_items;
} IRInstruction;

typedef struct {
  char* name;
  size_t line;
  uint16_t addr;
  bool has_addr;
} IRLabel;

typedef struct {
  IRNodeKind kind;
  union {
    IRInstruction instruction;
    IRLabel label;
  } data;
} IRNode;

/* Create an instruction node
 *
 * Creates a node with kind = IR_INSTRUCTION, holding IRInstruction instance.
 * IRInstruction instance takes variable arguments, specified by the given
 * format.
 *
 * Format string consists of letters, denoting the type of the corresponding
 * argument:
 *
 *   - b: byte (uint8_t)
 *   - w: word (uint16_t)
 *   - e: expression (Vector[Token])
 */
IRNode IRNode_createInstruction(char const* fmt, ...);

void IRNode_print(FILE* fout, IRNode* n);

#endif // INSTRUCTION_H
