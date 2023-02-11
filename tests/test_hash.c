#include "../src/lib/utils.h"
#include "generics.h"

TestResult test_hash_string() {
  const char *string_1 = "hello";
  const char *string_2 = "bye";
  unsigned int mod = 67;
  EXPECT_NEQ(hash_string(string_1, mod), hash_string(string_2, mod));
  return SUCCESS;
}

TestResult test_hash_null() {
  const char *string_1 = "hello";
  const char *string_2 = NULL;
  unsigned int mod = 67;
  EXPECT_NEQ(hash_string(string_1, mod), hash_string(string_2, mod));
  return SUCCESS;
}

TestResult test_hash_string_order() {
  const char *string_1 = "hello";
  const char *string_2 = "elloh";
  unsigned int mod = 67;
  EXPECT_NEQ(hash_string(string_1, mod), hash_string(string_2, mod));
  return SUCCESS;
}

TestResult test_hash_string_reverse() {
  const char *string_1 = "hello";
  const char *string_2 = "olleh";
  unsigned int mod = 67;
  EXPECT_NEQ(hash_string(string_1, mod), hash_string(string_2, mod));
  return SUCCESS;
}

TestResult test_hash_string_uppercase() {
  const char *string_1 = "hello";
  const char *string_2 = "HELLO";
  unsigned int mod = 67;
  EXPECT_NEQ(hash_string(string_1, mod), hash_string(string_2, mod));
  return SUCCESS;
}

int main() {
  Test *tests = (Test[]){
      new_test("Test String Hash", &test_hash_string),
      new_test("Test String Hash (NULL)", &test_hash_null),
      new_test("Test String Hash (Order)", &test_hash_string_order),
      new_test("Test String Hash (Reversed)", &test_hash_string_reverse),
      new_test("Test String Hash (Uppercase)", &test_hash_string_uppercase),
  };
  Suite my_suite = new_suite("Hashing Tests", tests, 4);
  run_suite(my_suite);
  return 0;
}
