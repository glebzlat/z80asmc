#ifndef EXPR_PARSER_H
#define EXPR_PARSER_H

#include "token.h"
#include "vector.h"

typedef enum {
  EXPR_NO_ERROR = 0,
  EXPR_ERROR_WRONG_UNARY_OP,
  EXPR_ERROR_UNBALANCED_LEFT_PAREN,
  EXPR_ERROR_UNBALANCED_RIGHT_PAREN,
  EXPR_ERROR_UNEXPECTED_TOKEN,
} ExprErrorType;

typedef struct {
  Token tok;
  ExprErrorType type;
} ExprError;

typedef struct ExprParser ExprParser;

/** Expression Parser interface
 *
 * `ExprParser` implementations must implement its functions, assign them to
 * function pointers, and save the implementer struct instance as `_m_impl`.
 *
 * An implementation must provide a constructor function.
 *
 * ```
 * ExprParser ImplExprParser_make(...);
 * ```
 */
struct ExprParser {
  int (*_m_feed)(void* impl, Token tok);
  void (*_m_deinit)(void* impl);
  Vector* (*_m_getOperations)(void* impl);
  Vector* (*_m_getExpressions)(void* impl);
  ExprError (*_m_getError)(void* impl);

  void* _m_impl;
};

/** Destroy `ExprParser` instance
 *
 * `ExprParser` does not destroy its operations vector if
 * `ExprParser_getOperations` is called at least once. If the caller retrieves
 * operations vector, it becomes responsible for its destruction.
 *
 * @param p `ExprParser`
 */
void ExprParser_deinit(ExprParser* p);

/** Feed the token to the parser
 *
 * Expression parser classifies the token and updates its operations or
 * expressions vector. If the token causes parsing error, the parser returns
 * a non-zero value.
 *
 * @param p `ExprParser`
 * @param tok Next token
 * @returns 0 on successful parsing
 */
int ExprParser_feed(ExprParser* p, Token tok);

/** Retrieve operations vector
 *
 * `ExprParser` does not destroy operations vector if this function is called
 * at least once. See the `ExprParser_deinit` function for more details.
 *
 * @param p `ExprParser`
 * @returns Operations vector
 */
Vector* ExprParser_getOperations(ExprParser* p);

/** Retrieve expressions vector
 *
 * @param p `ExprParser`
 * @returns Expressions vector
 */
Vector* ExprParser_getExpressions(ExprParser* p);

/** Retrieve parsing error
 *
 * @param p `ExprParser`
 * @returns Expressions vector
 */
ExprError ExprParser_getError(ExprParser* p);

/** Get a string representation of `ExprErrorType`
 *
 * @param type `ExprErrorType`
 * @returns Error string
 */
char const* ExprErrorType_toStr(ExprErrorType type);

#endif // EXPR_PARSER_H
