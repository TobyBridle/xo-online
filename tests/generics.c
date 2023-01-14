#include "./generics.h"

Suite new_suite(char *test_name, Test *tests, u_int16_t test_count) {
  Suite suite;
  char *name = calloc(strlen(test_name) + 1, sizeof(char));
  strcpy(name, test_name);

  for (int i = 0; i < test_count; ++i) {
    tests[i].id = i + 1;
    tests[i].is_suite = 1;
  }

  suite = (Suite){name, test_count, tests};
  return suite;
}

Test new_test(char *test_desc, TestResult(_test)()) {
  char *desc = calloc(strlen(test_desc) + 1, sizeof(char));
  strcpy(desc, test_desc);

  Test test = (Test){.desc = desc, .id = 0, .test = _test, .is_suite = 0};
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
  if (!test.is_suite) {
    if (result == FAILURE) {
      fprintf(stderr, "\x1b[31;1mTest `%s` failed!\x1b[0;0m\n", test.desc);
    } else {
      printf("\x1b[32;1mTest `%s` succeeded!\x1b[0;0m\n", test.desc);
    }
  } else {
    if (result == FAILURE) {
      fprintf(stderr, "\x1b[31;1mTest `%s` with ID (%d) failed!\x1b[0;0m\n",
              test.desc, test.id);
    } else {
      fprintf(stderr, "\x1b[32;1mTest `%s` with ID (%d) succeeded!\x1b[0;0m\n",
              test.desc, test.id);
    }
    free(test.desc);
  }
}
