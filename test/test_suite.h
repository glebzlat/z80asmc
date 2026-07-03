#ifndef TEST_SUITE_H
#define TEST_SUITE_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  int tests_run;
  int tests_failed;
  int current_test_failed;
} TestSuite;

#define TEST_CLEANUP test_cleanup
#define TEST_NOP NULL

#define STRINGIZE(EXP) #EXP

#define TEST(TEST_NAME) static void TEST_NAME(TestSuite* _ts)

#define TC(TEST_SUITE, TEST_FN)                                                                                        \
  do {                                                                                                                 \
    TEST_SUITE.current_test_failed = false;                                                                            \
    TEST_FN(&TEST_SUITE);                                                                                              \
    TEST_SUITE.tests_run += 1;                                                                                         \
    if (TEST_SUITE.current_test_failed) {                                                                              \
      fprintf(stderr, "Test %i: " #TEST_FN " failed\n", TEST_SUITE.tests_run);                                         \
      TEST_SUITE.tests_failed += 1;                                                                                    \
    }                                                                                                                  \
  } while (0)

#define _PRINTERR(MSG) fprintf(stderr, "%s:%i: " MSG, __FILE__, __LINE__)
#define _PRINTERR_VA(FMT, ...) fprintf(stderr, "%s:%i: " FMT, __FILE__, __LINE__, __VA_ARGS__)

#define CHECK(ASSERTION)                                                                                               \
  do {                                                                                                                 \
    if (!(ASSERTION)) {                                                                                                \
      _PRINTERR("Check " STRINGIZE(#ASSERTION) " failed\n");                                                           \
      _ts->current_test_failed = true;                                                                                 \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
  } while (0)

#define CHECK_INT_EQUAL(A, B)                                                                                          \
  do {                                                                                                                 \
    if (A != B) {                                                                                                      \
      _ts->current_test_failed = true;                                                                                 \
      _PRINTERR_VA("Check " #A " (%lli) == " #B " (%lli) failed\n", (long long int)A, (long long int)B);               \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
  } while (0)

#define CHECK_INT_NEQUAL(A, B)                                                                                         \
  do {                                                                                                                 \
    if (A == B) {                                                                                                      \
      _ts->current_test_failed = true;                                                                                 \
      _PRINTERR_VA("Check " #A " (%lli) != " #B " (%lli) failed\n", (long long int)A, (long long int)B);               \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
  } while (0)

#define CHECK_UINT_EQUAL(A, B)                                                                                         \
  do {                                                                                                                 \
    if (A != B) {                                                                                                      \
      _ts->current_test_failed = true;                                                                                 \
      _PRINTERR_VA("Check " #A " (%llu) == " #B " (%llu) failed\n", (long long unsigned int)A,                         \
                   (long long unsigned int)B);                                                                         \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
  } while (0)

#define CHECK_UINT_NEQUAL(A, B)                                                                                         \
  do {                                                                                                                 \
    if (A == B) {                                                                                                      \
      _ts->current_test_failed = true;                                                                                 \
      _PRINTERR_VA("Check " #A " (%llu) != " #B " (%llu) failed\n", (long long unsigned int)A,                         \
                   (long long unsigned int)B);                                                                         \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
  } while (0)


#define CHECK_STR_EQUAL(STR1, STR2)                                                                                    \
  do {                                                                                                                 \
    if (strcmp(STR1, STR2) != 0) {                                                                                     \
      _ts->current_test_failed = true;                                                                                 \
      _PRINTERR_VA("Check " #STR1 "(\"%s\") == " #STR2 "(\"%s\")  failed\n", STR1, STR2);                              \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
  } while (0)

#define CHECK_STR_NEQUAL(STR1, STR2)                                                                                    \
  do {                                                                                                                 \
    if (strcmp(STR1, STR2) == 0) {                                                                                     \
      _ts->current_test_failed = true;                                                                                 \
      _PRINTERR_VA("Check " #STR1 "(\"%s\") != " #STR2 "(\"%s\")  failed\n", STR1, STR2);                              \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
  } while (0)

#define CHECK_STR_EQUAL_LEN(STR1, STR2, LEN)                                                                           \
  do {                                                                                                                 \
    if (strncasecmp(STR1, STR2, (LEN)) != 0) {                                                                         \
      _ts->current_test_failed = true;                                                                                 \
      _PRINTERR_VA("Check " #STR1 " == " #STR2 " (\"%s\" == \"%.*s\") failed\n", STR1, (int)(LEN), STR2);              \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
    while (0)

#define CHECK_STR_NEQUAL_LEN(STR1, STR2, LEN)                                                                           \
  do {                                                                                                                 \
    if (strncasecmp(STR1, STR2, (LEN)) == 0) {                                                                         \
      _ts->current_test_failed = true;                                                                                 \
      _PRINTERR_VA("Check " #STR1 " != " #STR2 " (\"%s\" == \"%.*s\") failed\n", STR1, (int)(LEN), STR2);              \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
    while (0)

#define CHECK_PTR_EQUAL(A, B)                                                                                          \
  do {                                                                                                                 \
    if (A != B) {                                                                                                      \
      _ts->current_test_failed = true;                                                                                 \
      _PRINTERR_VA("Check " #A " (%p) == " #B " (%p) failed\n", (void const*)A, (void const*)B);                                   \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
  } while (0)

#define CHECK_PTR_NEQUAL(A, B)                                                                                         \
  do {                                                                                                                 \
    if (A == B) {                                                                                                      \
      _ts->current_test_failed = true;                                                                                 \
      _PRINTERR_VA("Check " #A " (%p) != " #B " (%p) failed\n", (void const*)A, (void const*)B);                                   \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
  } while (0)

#define CHECK_BOOL_EQUAL(A, B)                                                                                         \
  do {                                                                                                                 \
    if (A != B) {                                                                                                      \
      _ts->current_test_failed = true;                                                                                 \
      _PRINTERR_VA("Check " #A " (%s) == " #B " (%s) failed\n", (A ? "true" : "false"), (B ? "true" : "false"));       \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
  } while (0)

#define CHECK_BOOL_NEQUAL(A, B)                                                                                         \
  do {                                                                                                                 \
    if (A == B) {                                                                                                      \
      _ts->current_test_failed = true;                                                                                 \
      _PRINTERR_VA("Check " #A " (%s) != " #B " (%s) failed\n", (A ? "true" : "false"), (B ? "true" : "false"));       \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
  } while (0)

#define CHECK_TOKEN_TYPE_EQUAL(A, B)                                                                                   \
  do {                                                                                                                 \
    if (A != B) {                                                                                                      \
      _ts->current_test_failed = true;                                                                                 \
      char const *_A_str_ = TokenType_str(A), *_B_str_ = TokenType_str(B);                                             \
      _PRINTERR_VA("Check " #A " (%s) == " #B " (%s) failed\n", _A_str_, _B_str_);                                     \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
  } while (0)

#define CHECK_TOKEN_TYPE_NEQUAL(A, B)                                                                                   \
  do {                                                                                                                 \
    if (A == B) {                                                                                                      \
      _ts->current_test_failed = true;                                                                                 \
      char const *_A_str_ = TokenType_str(A), *_B_str_ = TokenType_str(B);                                             \
      _PRINTERR_VA("Check " #A " (%s) != " #B " (%s) failed\n", _A_str_, _B_str_);                                     \
      goto TEST_CLEANUP;                                                                                               \
    }                                                                                                                  \
  } while (0)

#endif // TEST_SUITE_H
