#include "expr_parser_builder.h"
#include "expr_parser.h"

static ExprParser DefaultExprParserBuilder_build(void* impl);
static void DefaultExprParserBuilder_deinit(void* impl);

ExprParserBuilder DefaultExprParserBuilder_make(void) {
  ExprParserBuilder builder = {
      ._m_build = DefaultExprParserBuilder_build,
      ._m_deinit = DefaultExprParserBuilder_deinit,
      ._m_impl = NULL,
  };
  return builder;
}

static ExprParser DefaultExprParserBuilder_build(void* impl) {
  return DefaultExprParser_make();
}

static void DefaultExprParserBuilder_deinit(void* impl) {}
