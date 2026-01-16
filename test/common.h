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

#define CHECK_TOKEN_TYPES_EQUAL(A, B, CLEANUP)                                                                         \
  do {                                                                                                                 \
    if (A != B) {                                                                                                      \
      char const *_A_str_ = TokenType_str(A), *_B_str_ = TokenType_str(B);                                             \
      fprintf(stderr, "Check " #A " (%s) == " #B " (%s) failed\n", _A_str_, _B_str_);                                  \
      CLEANUP;                                                                                                         \
      return 1;                                                                                                        \
    }                                                                                                                  \
  } while (0)

#define CHECK_STREQUALN(STR1, STR2, LEN, CLEANUP)                                                                      \
  do {                                                                                                                 \
    if (strncasecmp(STR1, STR2, (LEN)) != 0) {                                                                         \
      fprintf(stderr, "Check " #STR1 " == " #STR2 " (\"%s\" == \"%.*s\") failed\n", STR1, (int)(LEN), STR2);           \
      CLEANUP;                                                                                                         \
      return 1;                                                                                                        \
    }                                                                                                                  \
  } while (0)

#define TEST_CASE(CALL)                                                                                                \
  do {                                                                                                                 \
    bool failed = (CALL);                                                                                              \
    if (failed) {                                                                                                      \
      fprintf(stderr, "Test %s failed\nOn %s:%i\n", #CALL, __FILE__, __LINE__);                                             \
    }                                                                                                                  \
    tests_failed += failed;                                                                                            \
  } while (0)

#endif // TEST_COMMON_H
