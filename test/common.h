#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <string.h>

#define STRINGIZE(EXP) #EXP

#define CHECK(ASSERTION, CLEANUP)                                                                                      \
  do {                                                                                                                 \
    if (ASSERTION)                                                                                                     \
      ;                                                                                                                \
    else {                                                                                                             \
      fprintf(stderr, "%s:%i: Check " STRINGIZE(#ASSERTION) " failed\n", __FILE__, __LINE__);                          \
      CLEANUP;                                                                                                         \
      return 1;                                                                                                        \
    }                                                                                                                  \
  } while (0)

#define CHECK_STREQUAL(STR1, STR2, CLEANUP)                                                                            \
  do {                                                                                                                 \
    if (strcmp(STR1, STR2) != 0) {                                                                                     \
      fprintf(stderr, "%s:%i: Check " #STR1 " == " #STR2 " (\"%s\" == \"%s\") failed\n", __FILE__, __LINE__, STR1,     \
              STR2);                                                                                                   \
      CLEANUP;                                                                                                         \
      return 1;                                                                                                        \
    }                                                                                                                  \
  } while (0)

#define CHECK_EQUAL(A, B, CLEANUP)                                                                                     \
  do {                                                                                                                 \
    if (A != B) {                                                                                                      \
      fprintf(stderr, "%s:%i: Check " #A " == " #B " failed\n", __FILE__, __LINE__);                                   \
      CLEANUP;                                                                                                         \
      return 1;                                                                                                        \
    }                                                                                                                  \
  } while (0)

#endif // TEST_COMMON_H
