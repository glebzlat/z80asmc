#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
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
    va_end(aq);
    return NULL;
  }

  buf = malloc(sizeof(*buf) * ((size_t)len + 1));
  if (!buf) {
    return NULL;
  }

  len = vsnprintf(buf, (size_t)len + 1, format, aq);
  if (len < 0) {
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

char* ffullread(FILE* fin) {
  enum {
    BUF_BASE_SIZE = 1024,
    READ_SIZE = BUF_BASE_SIZE - 1,
  };

  assert(fin);

  size_t buf_len = 0, buf_ptr = 0;
  char* buf = NULL;
  if (!(buf = malloc(sizeof(*buf) * BUF_BASE_SIZE))) {
    return NULL;
  }
  buf_len += BUF_BASE_SIZE;

  char read_tmp[READ_SIZE];
  size_t n_read = 0;
  do {
    n_read = fread(read_tmp, sizeof(*read_tmp), READ_SIZE, fin);
    if (n_read < READ_SIZE) {
      if (ferror(fin)) {
        free(buf);
        return NULL;
      }
    }

    // Do the newly read bytes overflow size_t?
    if (n_read > SIZE_MAX - buf_ptr - 1) {
      free(buf);
      errno = EFBIG;
      return NULL;
    }

    if (buf_ptr + n_read + 1 > buf_len) {
      // Can we reallocate the buffer without overflowing size_t?
      if (buf_len > SIZE_MAX - BUF_BASE_SIZE) {
        free(buf);
        errno = EFBIG;
        return NULL;
      }

      char* alloc_tmp = realloc(buf, buf_len + BUF_BASE_SIZE);
      if (!alloc_tmp) {
        free(buf);
        return NULL;
      }

      buf = alloc_tmp;
      buf_len += BUF_BASE_SIZE;
    }

    memcpy(buf + buf_ptr, read_tmp, n_read);
    buf_ptr += n_read;
  } while (READ_SIZE == n_read);

  buf[buf_ptr] = '\0';

  return buf;
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

bool is_not_zero(void* ptr, size_t size) {
  assert(ptr);

  char* bytes = ptr;
  for (size_t i = 0; i < size; ++i) {
    if (bytes[i] != 0) {
      return true;
    }
  }

  return false;
}
