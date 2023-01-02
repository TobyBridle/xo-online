#include "../src/lib/utils.h"
#include "generics.h"
#include <stdio.h>

TestResult test_push() {
  LinkedList *list = init_list();
  push_node(list, 1);
  push_node(list, 2);
  EXPECT_EQ(list->head->data, 1);
  EXPECT_EQ(list->tail->data, 2);
  push_node(list, 3);
  EXPECT_EQ(list->tail->data, 3);
  EXPECT_EQ(list->head->next->data, 2);

  free_list(list);
  return SUCCESS;
}

TestResult test_pop() {
  LinkedList *list = init_list();
  push_node(list, 1);
  push_node(list, 2);
  push_node(list, 3);

  // NOTE: The LinkedList is a FIFO data structure
  EXPECT_EQ(pop_node(list), 1);
  EXPECT_EQ(pop_node(list), 2);
  EXPECT_EQ(pop_node(list), 3);
  EXPECT_EQ(pop_node(list), -1);

  free_list(list);
  return SUCCESS;
}

TestResult test_remove() {
  LinkedList *list = init_list();
  push_node(list, 1);
  push_node(list, 2);
  push_node(list, 3);

  // NOTE: The LinkedList is a FIFO data structure
  EXPECT_EQ(remove_node(list, 1), 0);
  EXPECT_EQ(remove_node(list, 2), 0);
  EXPECT_EQ(remove_node(list, 3), 0);
  EXPECT_EQ(remove_node(list, 4), -1);

  free_list(list);
  return SUCCESS;
}

int main() {
  Test *tests = (Test[]){
      new_test("Test Node Push", &test_push),
      new_test("Test Node Pop", &test_pop),
      new_test("Test Node Remove (By Value)", &test_remove),
  };
  Suite my_suite = new_suite("Linked List Tests", tests);
  run_suite(my_suite);
  return 0;
}
