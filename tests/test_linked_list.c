#include "../src/lib/utils.h"
#include "generics.h"
#include <stdio.h>

TestResult test_push() {
  LinkedList *list = init_list();
  push_node(list, 1);
  push_node(list, 2);
  EXPECT_EQ(list->head->data, 1);
  EXPECT_EQ(list->head->next->data, 2);
  EXPECT_EQ(list->tail->data, 2);
  push_node(list, 3);
  EXPECT_EQ(list->head->next->next->data, 3);
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

TestResult test_peek() {
  LinkedList *list = init_list();
  for (int i = 0; i < 1000; ++i) {
    printf("Pushing %d\n", i);
    push_node(list, i);
  }
  for (int i = 0; i < 1000; ++i) {
    EXPECT_EQ(peek_node(list)->data, i);
    pop_node(list);
  }
  free_list(list);
  return SUCCESS;
}

TestResult test_push_pop() {
  LinkedList *list = init_list();

  push_node(list, 1);
  EXPECT_EQ(peek_node(list)->data, 1);

  push_node(list, 2);
  EXPECT_EQ(peek_node(list)->data, 1);

  EXPECT_EQ(pop_node(list), 1);

  push_node(list, 1);
  EXPECT_EQ(peek_node(list)->data, 2);

  EXPECT_EQ(pop_node(list), 2);

  free_list(list);
  return SUCCESS;
}

TestResult test_push_pop_remove() {
  LinkedList *list = init_list();

  push_node(list, 1);
  EXPECT_EQ(peek_node(list)->data, 1);
  EXPECT_EQ(list->head->data, 1);
  EXPECT_EQ(list->tail->data, 1);

  push_node(list, 2);
  EXPECT_EQ(peek_node(list)->data, 1);
  EXPECT_EQ(list->head->data, 1);
  EXPECT_EQ(list->head->next->data, 2);
  EXPECT_EQ(list->tail->data, 2);

  remove_node(list, 1);
  EXPECT_EQ(peek_node(list)->data, 2);
  EXPECT_EQ(list->head->data, 2);
  EXPECT_EQ(list->tail->data, 2);

  push_node(list, 1);
  EXPECT_EQ(peek_node(list)->data, 2);
  EXPECT_EQ(list->head->data, 2);
  EXPECT_EQ(list->tail->data, 1);
  EXPECT_EQ(list->head->next->data, 1);

  remove_node(list, 1);
  EXPECT_EQ(peek_node(list)->data, 2);
  EXPECT_EQ(list->head->data, 2);
  EXPECT_EQ(list->tail->data, 2);

  push_node(list, 1);
  EXPECT_EQ(peek_node(list)->data, 2);
  EXPECT_EQ(list->head->data, 2);
  EXPECT_EQ(list->tail->data, 1);
  EXPECT_EQ(list->head->next->data, 1);

  free_list(list);
  return SUCCESS;
}

int main() {
  Test *tests = (Test[]){
      new_test("Test Node Push", &test_push),
      new_test("Test Node Pop", &test_pop),
      new_test("Test Node Remove (By Value)", &test_remove),
  Test tests[] = {
      new_test("Test Push", test_push),
      new_test("Test Pop", test_pop),
      new_test("Test Remove", test_remove),
      new_test("Test Peek", test_peek),
      new_test("Test Push Pop", test_push_pop),
      new_test("Test Push Pop Remove", test_push_pop_remove),
  };
  Suite my_suite = new_suite("Linked List Tests", tests);
  run_suite(my_suite);
  return 0;
}
