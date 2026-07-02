#include "expr_parser_builder.h"
#include "interfaces.h"

ExprParser ExprParserBuilder_build(ExprParserBuilder* builder) {
  CALL(builder, _m_build);
}

void ExprParserBuilder_deinit(ExprParserBuilder* builder) {
  CALL_VOID(builder, _m_deinit);
}
