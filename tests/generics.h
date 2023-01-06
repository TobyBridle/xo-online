#ifndef TEST_GENERIC_H
#define TEST_GENERIC_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  SUCCESS = 1,
  FAILURE = 0,
} TestResult;

typedef struct TEST_T {
  char *desc;
  uint8_t id;
  TestResult (*test)(void);
  int is_suite;
} Test;

typedef struct SUITE_T {
  char *name;
  uint16_t test_count;
  Test *tests;
} Suite;

#define EXPECT(e)                                                              \
  if (!(e)) {                                                                  \
    fprintf(stderr, "\x1b[31;1mFAILURE at %s:%d\x1b[0;0m\n", __FILE__,         \
            __LINE__);                                                         \
    return FAILURE;                                                            \
  }

#define EXPECT_EQ(a, b)                                                        \
  if ((a) != (b)) {                                                            \
    fprintf(stderr, "\x1b[31;1mFAILURE at %s:%d\x1b[0;0m\n", __FILE__,         \
            __LINE__);                                                         \
    return FAILURE;                                                            \
  }

#define EXPECT_NEQ(a, b)                                                       \
  if ((a) == (b)) {                                                            \
    return FAILURE;                                                            \
  }

Suite new_suite(char *test_name, Test *_tests);
Test new_test(char *test_desc, TestResult(_test)(void));

void run_suite(Suite suite);
void run_test(Test test);

#endif
