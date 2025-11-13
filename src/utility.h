#ifndef UTILITY_H
#define UTILITY_H

#include <stdlib.h>

#define CALL_NON_NULL(obj, fn)                                                                                         \
  if ((obj) != NULL)                                                                                                   \
    fn(obj);

char* dsprintf(char const* format, ...) __attribute__((format(printf, 1, 2), __warn_unused_result__));

int strncasecmp(char const* a, char const* b, size_t len);

void die(char const* message) __attribute__((noreturn));

#endif
