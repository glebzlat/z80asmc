#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "utility.h"

char* dsprintf(char const* format, ...) {
  assert(format);

  va_list ap;
  va_start(ap, format);
  char* buf = vdsprintf(format, ap);
  va_end(ap);

  return buf;
}

char* vdsprintf(char const* format, va_list ap) {
  assert(format);

  char* buf;
  long len;

  va_list aq;
  va_copy(aq, ap);

  len = vsnprintf(NULL, 0, format, ap);
  if (len < 0) {
    perror("vsnprintf failed");
    return NULL;
  }

  buf = malloc(sizeof(*buf) * ((size_t)len + 1));
  if (!buf) {
    perror("malloc failed");
    return NULL;
  }

  len = vsnprintf(buf, (size_t)len + 1, format, aq);
  if (len < 0) {
    perror("vsnprintf failed");
    free(buf);
    return NULL;
  }
  va_end(aq);

  return buf;
}

int strncasecmp(char const* a, char const* b, size_t len) {
  assert(a);
  assert(b);
  char lower_a, lower_b;
  for (size_t i = 0; i < len; ++i) {
    lower_a = (a[i] >= 'A' && a[i] <= 'Z') ? a[i] + 0x20 : a[i];
    lower_b = (b[i] >= 'A' && b[i] <= 'Z') ? b[i] + 0x20 : b[i];
    if (lower_a == '\0' && lower_a == lower_b)
      return 0;
    else if (lower_a < lower_b)
      return -1;
    else if (lower_a > lower_b)
      return 1;
  }
  return 0;
}

void die(char const* message) {
  assert(message);

  if (errno) {
    fprintf(stderr, "%s (errno: %i: %s)\n", message, errno, strerror(errno));
  } else {
    fprintf(stderr, "%s\n", message);
  }

  exit(EX_SOFTWARE);
}
