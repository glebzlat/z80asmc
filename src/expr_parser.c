#include "expr_parser.h"
#include "interfaces.h"

void ExprParser_deinit(ExprParser* p) {
  CALL_VOID(p, _m_deinit);
}

int ExprParser_feed(ExprParser* p, Token tok) {
  CALL_VA(p, _m_feed, tok);
}

Vector* ExprParser_getOperations(ExprParser* p) {
  CALL(p, _m_getOperations);
}

Vector* ExprParser_getExpressions(ExprParser* p) {
  CALL(p, _m_getExpressions);
}

ExprError ExprParser_getError(ExprParser* p) {
  CALL(p, _m_getError);
}

char const* ExprErrorType_toStr(ExprErrorType type) {
  switch (type) {
  case EXPR_NO_ERROR:
    return "EXPR_NO_ERROR";
  case EXPR_ERROR_WRONG_UNARY_OP:
    return "operator can't be used as unary";
  case EXPR_ERROR_UNBALANCED_LEFT_PAREN:
    return "unbalanced left parenthesis";
  case EXPR_ERROR_UNBALANCED_RIGHT_PAREN:
    return "unbalanced right parenthesis";
  case EXPR_ERROR_UNEXPECTED_TOKEN:
    return "unexpected token";
  default:
    return NULL;
  }
}
