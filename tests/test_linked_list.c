#include "../src/lib/utils.h"
#include "generics.h"
#include <stdio.h>

TestResult test_push() {
  LinkedList *list = init_list();
  push_node(list, (NodeValue){.i_value = 1});
  push_node(list, (NodeValue){.i_value = 2});
  EXPECT_EQ(list->head->data.i_value, 1);
  EXPECT_EQ(list->head->next->data.i_value, 2);
  EXPECT_EQ(list->tail->data.i_value, 2);
  push_node(list, (NodeValue){.i_value = 3});
  EXPECT_EQ(list->head->next->next->data.i_value, 3);
  EXPECT_EQ(list->tail->data.i_value, 3);
  EXPECT_EQ(list->head->next->data.i_value, 2);

  free_list(list);
  return SUCCESS;
}

TestResult test_pop() {
  LinkedList *list = init_list();
  push_node(list, (NodeValue){.i_value = 1});
  push_node(list, (NodeValue){.i_value = 2});
  push_node(list, (NodeValue){.i_value = 3});

  // NOTE: The LinkedList is a FIFO data structure
  EXPECT_EQ(pop_node(list).i_value, 1);
  EXPECT_EQ(pop_node(list).i_value, 2);
  EXPECT_EQ(pop_node(list).i_value, 3);
  EXPECT_EQ(pop_node(list).err, -1);

  free_list(list);
  return SUCCESS;
}

TestResult test_remove() {
  LinkedList *list = init_list();
  push_node(list, (NodeValue){.i_value = 1});
  push_node(list, (NodeValue){.i_value = 2});
  push_node(list, (NodeValue){.i_value = 3});

  // NOTE: The LinkedList is a FIFO data structure
  EXPECT_EQ(remove_node(list, (NodeValue){.i_value = 1}), 0);
  EXPECT_EQ(remove_node(list, (NodeValue){.i_value = 2}), 0);
  EXPECT_EQ(remove_node(list, (NodeValue){.i_value = 3}), 0);
  EXPECT_EQ(remove_node(list, (NodeValue){.i_value = 4}), -1);

  free_list(list);
  return SUCCESS;
}

TestResult test_peek() {
  LinkedList *list = init_list();
  for (int i = 0; i < 1000; ++i) {
    push_node(list, (NodeValue){.i_value = i});
  }
  for (int i = 0; i < 1000; ++i) {
    EXPECT_EQ(peek_node(list)->data.i_value, i);
    pop_node(list);
  }
  free_list(list);
  return SUCCESS;
}

TestResult test_push_pop() {
  LinkedList *list = init_list();

  push_node(list, (NodeValue){.i_value = 1});
  EXPECT_EQ(peek_node(list)->data.i_value, 1);

  push_node(list, (NodeValue){.i_value = 2});
  EXPECT_EQ(peek_node(list)->data.i_value, 1);

  EXPECT_EQ(pop_node(list).i_value, 1);

  push_node(list, (NodeValue){.i_value = 1});
  EXPECT_EQ(peek_node(list)->data.i_value, 2);

  EXPECT_EQ(pop_node(list).i_value, 2);

  free_list(list);
  return SUCCESS;
}

TestResult test_push_pop_remove() {
  LinkedList *list = init_list();

  push_node(list, (NodeValue){.i_value = 1});
  EXPECT_EQ(peek_node(list)->data.i_value, 1);
  EXPECT_EQ(list->head->data.i_value, 1);
  EXPECT_EQ(list->tail->data.i_value, 1);

  push_node(list, (NodeValue){.i_value = 2});
  EXPECT_EQ(peek_node(list)->data.i_value, 1);
  EXPECT_EQ(list->head->data.i_value, 1);
  EXPECT_EQ(list->head->next->data.i_value, 2);
  EXPECT_EQ(list->tail->data.i_value, 2);

  remove_node(list, (NodeValue){.i_value = 1});
  EXPECT_EQ(peek_node(list)->data.i_value, 2);
  EXPECT_EQ(list->head->data.i_value, 2);
  EXPECT_EQ(list->tail->data.i_value, 2);

  push_node(list, (NodeValue){.i_value = 1});
  EXPECT_EQ(peek_node(list)->data.i_value, 2);
  EXPECT_EQ(list->head->data.i_value, 2);
  EXPECT_EQ(list->tail->data.i_value, 1);
  EXPECT_EQ(list->head->next->data.i_value, 1);

  remove_node(list, (NodeValue){.i_value = 1});
  EXPECT_EQ(peek_node(list)->data.i_value, 2);
  EXPECT_EQ(list->head->data.i_value, 2);
  EXPECT_EQ(list->tail->data.i_value, 2);

  push_node(list, (NodeValue){.i_value = 1});
  EXPECT_EQ(peek_node(list)->data.i_value, 2);
  EXPECT_EQ(list->head->data.i_value, 2);
  EXPECT_EQ(list->tail->data.i_value, 1);
  EXPECT_EQ(list->head->next->data.i_value, 1);

  free_list(list);
  return SUCCESS;
}

TestResult test_push_at() {
  LinkedList *list = init_list();
  push_node(list, (NodeValue){.i_value = 1});
  push_node(list, (NodeValue){.i_value = 2});
  push_node_at(list, (NodeValue){.i_value = 3}, 1);
  EXPECT_EQ(list->head->data.i_value, 1);
  EXPECT_EQ(list->head->next->data.i_value, 3);
  EXPECT_EQ(list->tail->data.i_value, 2);
  push_node_at(list, (NodeValue){.i_value = 4}, 2);
  EXPECT_EQ(list->head->next->next->data.i_value, 4);
  EXPECT_EQ(list->tail->data.i_value, 2);
  EXPECT_EQ(list->head->next->data.i_value, 3);

  return SUCCESS;
}

int main() {
  Test tests[] = {
      new_test("Test Push", test_push),
      new_test("Test Pop", test_pop),
      new_test("Test Remove", test_remove),
      new_test("Test Peek", test_peek),
      new_test("Test Push Pop", test_push_pop),
      new_test("Test Push Pop Remove", test_push_pop_remove),
      new_test("Test Push (At Index)", test_push_at),
  };
  Suite my_suite = new_suite("Linked List Tests", tests, 7);
  run_suite(my_suite);
  return 0;
}
