#ifndef UTILITY_H
#define UTILITY_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CALL_NON_NULL(obj, fn)                                                                                         \
  if ((obj) != NULL)                                                                                                   \
    fn(obj);

char* dsprintf(char const* format, ...) __attribute__((format(printf, 1, 2), __warn_unused_result__));

char* vdsprintf(char const* format, va_list ap) __attribute__((__warn_unused_result__));

int strncasecmp(char const* a, char const* b, size_t len);

char* ffullread(FILE* fin);

void die(char const* message) __attribute__((noreturn));

bool is_not_zero(void* ptr, size_t size);

/** Try to convert a string to a uint32_t integer
 *
 * Given a non-null string of the `len` length, tries to convert the string to
 * `uint32_t` according to the given base and sets the `correct` flag if the
 * conversion is successful.
 *
 * Sets `correct` to false in following cases:
 *
 * - The string length is zero - also sets `errno` to `EINVAL`.
 * - The resulting value is out of range - also sets `errno` to `ERANGE`.
 * - String contains unconvertible characters that are neither decimal nor
 *   hexadecimal digits.
 *
 * Does not modify `errno` on success.
 *
 * @param str Input string
 * @param len Input string length
 * @param[out] correct orrectness flag (must be non-null)
 * @param base Base
 */
uint32_t strtou32(char const* restrict str, size_t len, bool* restrict correct, uint8_t base)
    __attribute__((__warn_unused_result__));

#endif
