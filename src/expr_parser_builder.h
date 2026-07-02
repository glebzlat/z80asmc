#ifndef EXPR_PARSER_BUILDER_H
#define EXPR_PARSER_BUILDER_H

#include "expr_parser.h"

typedef struct ExprParserBuilder ExprParserBuilder;

/** Expression Parser Builder interface
 *
 * The builder is used to create `ExprParser` instances. `ExprParserBuilder`
 * implementations must define its functions, assign them function pointers,
 * and save the implementer struct instance as `_m_impl`
 *
 * An implementation must provide a constructor function.
 *
 * ```
 * ExprParserBuilder ImplExprParserBuilder_make(...);
 * ```
 */
struct ExprParserBuilder {
  ExprParser (*_m_build)(void* impl);
  void(*_m_deinit)(void* impl);

  void* _m_impl;
};

ExprParser ExprParserBuilder_build(ExprParserBuilder* builder);
void ExprParserBuilder_deinit(ExprParserBuilder* builder);

#endif // EXPR_PARSER_BUILDER_H
