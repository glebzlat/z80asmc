#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
  TOKEN_UNINITIALIZED = 0,
  TOKEN_END,
  TOKEN_ERROR,
  TOKEN_ID,
  TOKEN_CHAR,
  TOKEN_STRING,
  TOKEN_DECIMAL,
  TOKEN_HEXADECIMAL,
  TOKEN_OCTAL,
  TOKEN_BINARY,
  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_COMMA,
  TOKEN_MINUS,
  TOKEN_PLUS,
  TOKEN_SLASH,
  TOKEN_STAR,
  TOKEN_PERCENT,
  TOKEN_CAP,
  TOKEN_TILDE,
  TOKEN_AMPERSAND,
  TOKEN_BAR,
  TOKEN_LEFT_SHIFT,
  TOKEN_RIGHT_SHIFT,
  TOKEN_DOUBLE_AMPERSAND,
  TOKEN_DOUBLE_BAR,
  TOKEN_BANG,
  TOKEN_BANG_EQUAL,
  TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER_EQUAL,
  TOKEN_LESS_EQUAL,
  TOKEN_COLON,
  TOKEN_NEWLINE,
} TokenType;

typedef struct {
  /** Token value
   *
   * Value pointer points to the start of the substring in the input buffer.
   * It is not allocated dynamically and not null-terminated. Use `len`
   * property to get the length of the value.
   */
  char const* value;

  /** Value length
   *
   * Denotes how many useful characters are referenced by the value pointer.
   * Value is located at [value,value+len), and the user must not read at
   * value+len or further.
   */
  size_t len;

  /** Symbol length
   *
   * Length of the entire source span consumed by the lexer to produce
   * this token: [col, col+symlen) in the source. Always satisfies
   * symlen >= len. The value is a substring of this span with prefixes
   * and quoting stripped.
   */
  size_t symlen;

  /** Line of the symbol (one-indexed)
   *
   * Any token occupies exactly one line.
   */
  size_t line;

  /** Column of the first character of the symbol (zero-indexed)
   *
   * The symbol occupies columns [col, col+symlen) in the source line. For
   * tokens whose value omits a prefix or quoting (e.g. "0x123" -> "123", "'a'"
   * -> "a"), col points at the first character of the symbol — the '0' in
   *  "0x123", the opening quote in "'a'" — not at the value.
   */
  size_t col;

  /** Token type
   *
   * Token type is used to differentiate between not only tokens that are
   * naturally different like a paren and a variable name, but also between
   * different representations that have the same value.
   *
   * This is applicable to integers, which may be expressed in binary, octal,
   * decimal, and hexadecimal formats. Lexer omits the prefix or the suffix (if
   * one of them is present) leaving only the effective integer value ("0x123"
   * results in the value of "123"). Token type is used to preserve the fact
   * that "123" means 0x123, not 123.
   */
  TokenType type;

  /** Unary flag
   *
   * Unary flag is true if the token is a unary operator (such as in "-1"),
   * otherwise is false.
   */
  bool unary;
} Token;

/** Create a formatted token string
 *
 * Depending on the value of the `unary` flag converts the token into the
 * string of two possible formats:
 *
 * - `<line>:<col>:<type>:<value>` if `unary` is false
 * - `<line>:<col>:u:<type>:<value>` if `unary` is true
 *
 * String deallocation is the responsibility of the caller.
 *
 * @param tok Token
 * @returns Null-terminated token representation
 */
char* Token_format(Token* tok);

/** Make a null-terminated copy of token's value
 *
 * String deallocation is the responsibility of the caller.
 *
 * @param tok Token @returns Null-terminated copy of the token's value
 */
char* Token_str(Token* tok);

/** Try to convert token value to unsigned integer
 *
 * Returns the integer value and sets the `correct` pointer's value to true if
 * the value of the token is well-formatted integer or a character. Otherwise
 * sets the value of `correct` to false.
 *
 * This function terminates the program if the token type is unsupported.
 *
 * Supported token types:
 *
 * - `TOKEN_HEXADECIMAL`
 * - `TOKEN_DECIMAL`
 * - `TOKEN_OCTAL`
 * - `TOKEN_BINARY`
 * - `TOKEN_CHAR`
 *
 * @param tok Token
 * @param[out] correct Whether the convertion is successful
 * @returns Converted integer if the convertion is successful
 */
unsigned long Token_toInt(Token* tok, bool* correct);

/** Get the string representation of the given token type
 *
 * @param type Token type
 * @returns String representation
 */
char const* TokenType_str(TokenType type);

#endif // TOKEN_H
