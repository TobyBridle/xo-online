#include "./generics.h"

Suite new_suite(char *test_name, Test *tests) {
  Suite suite;
  char *name = calloc(strlen(test_name) + 1, sizeof(char));
  strcpy(name, test_name);

  u_int16_t test_count = 0;
  while (tests[test_count].test != NULL) {
    tests[test_count].id = test_count + 1;
    tests[test_count].is_suite = 1;
    test_count++;
  }

  suite = (Suite){name, test_count, tests};
  return suite;
}

Test new_test(char *test_desc, TestResult(_test)()) {
  Test test;
  char *desc = calloc(strlen(test_desc) + 1, sizeof(char));
  strcpy(desc, test_desc);

  test = (Test){desc, .id = 0, .test = _test, .is_suite = 0};
  return test;
}

void run_suite(Suite suite) {
  for (int test = 0; test < suite.test_count; ++test) {
    run_test(suite.tests[test]);
  }
  printf("\x1b[33;1mTest Suite `%s` ran %d test%c!\n", suite.name,
         suite.test_count, suite.test_count == 1 ? '\0' : 's');

  free(suite.name);
}

void run_test(Test test) {
  TestResult result = test.test();
  if (result == FAILURE) {
    if (!test.is_suite) {
      fprintf(stderr, "\x1b[31;1mTest `%s` failed!\x1b[0;0m\n", test.desc);
    } else {
      fprintf(stderr, "\x1b[31;1mTest `%s` with ID (%d) failed!\x1b[0;0m\n",
              test.desc, test.id);
    }
  } else {
    printf("\x1b[32;1mTest `%s` succeeded!\x1b[0;0m\n", test.desc);
  }
  free(test.desc);
}