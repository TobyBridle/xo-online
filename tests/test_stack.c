#include "../src/lib/utils.h"
#include "generics.h"

TestResult test_push() {
  stck_t *stack = init_stack(5);
  push(stack, 1);
  EXPECT(peek(stack) == 1);
  for (int i = 2; i <= 5; ++i) {
    push(stack, i);
  }
  EXPECT(peek(stack) == 5);
  push(stack, 6);
  EXPECT(peek(stack) == 5);

  free_stack(stack);
  return SUCCESS;
}

TestResult test_pop() {
  stck_t *stack = init_stack(100);
  for (int i = 1; i <= 100; i++) {
    push(stack, i);
  }
  for (int i = 100; i >= 1; i--) {
    EXPECT(pop(stack) == i);
  }
  EXPECT(peek(stack) == -1);
  EXPECT(pop(stack) == -1);
  EXPECT(peek(stack) == -1);

  free_stack(stack);
  return SUCCESS;
}

TestResult test_peek() {
  stck_t *stack = init_stack(100);
  for (int i = 1; i <= 100; i++) {
    push(stack, i);
  }
  for (int i = 100; i >= 1; i--) {
    EXPECT(peek(stack) == 100);
  }
  EXPECT(pop(stack) == 100);

  free_stack(stack);
  return SUCCESS;
}

int main() {
  Test *tests = (Test[]){
      new_test("Pushing Values", &test_push),
      new_test("Popping Values", &test_pop),
      new_test("Peeking Values", &test_peek),
  };
  Suite my_suite = new_suite("Stack Tests", tests, 3);
  run_suite(my_suite);
  return 0;
}
