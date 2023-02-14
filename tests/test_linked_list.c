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

TestResult test_push_pointer() {
  LinkedList *list = init_list();
  char *word_one = "hello";
  char *word_two = "world";
  char *word_three = "!";

  push_node(list, (NodeValue){.pointer = word_one});
  push_node(list, (NodeValue){.pointer = word_two});
  push_node(list, (NodeValue){.pointer = word_three});

  EXPECT_EQ(strcmp(list->head->data.pointer, "hello"), 0);
  EXPECT_EQ(strcmp(list->head->next->data.pointer, "world"), 0);
  EXPECT_EQ(strcmp(list->head->next->next->data.pointer, "!"), 0);

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

TestResult test_pop_pointer() {
  LinkedList *list = init_list();
  char *word_one = "hello";
  char *word_two = "world";
  char *word_three = "!";

  push_node(list, (NodeValue){.pointer = word_one});
  push_node(list, (NodeValue){.pointer = word_two});
  push_node(list, (NodeValue){.pointer = word_three});

  EXPECT_EQ(strcmp(pop_node(list).pointer, "hello"), 0);
  EXPECT_EQ(strcmp(pop_node(list).pointer, "world"), 0);
  EXPECT_EQ(strcmp(pop_node(list).pointer, "!"), 0);
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
  EXPECT_EQ(remove_node(list, (NodeValue){.i_value = 1}).i_value, 1);
  EXPECT_EQ(remove_node(list, (NodeValue){.i_value = 2}).i_value, 2);
  EXPECT_EQ(remove_node(list, (NodeValue){.i_value = 3}).i_value, 3);
  EXPECT_EQ(remove_node(list, (NodeValue){.i_value = 4}).err, -1);

  free_list(list);
  return SUCCESS;
}

TestResult test_remove_pointer() {
  LinkedList *list = init_list();

  char *word_one = "hello";
  char *word_two = "world";
  char *word_three = "!";

  NodeValue val;

  push_node(list, (NodeValue){.pointer = word_one});
  push_node(list, (NodeValue){.pointer = word_two});
  push_node(list, (NodeValue){.pointer = word_three});

  // NOTE: The LinkedList is a FIFO data structure
  val.pointer = remove_node(list, (NodeValue){.pointer = word_one}).pointer;
  EXPECT_EQ(strcmp(val.pointer, "hello"), 0);

  val.err = remove_node(list, (NodeValue){.pointer = word_one}).err;
  EXPECT_EQ(val.err, -1);

  val.pointer = remove_node(list, (NodeValue){.pointer = word_three}).pointer;
  EXPECT_EQ(strcmp(val.pointer, "!"), 0);

  val.err = remove_node(list, (NodeValue){.pointer = word_three}).err;
  EXPECT_EQ(val.err, -1);

  val.pointer = remove_node(list, (NodeValue){.pointer = word_two}).pointer;
  EXPECT_EQ(strcmp(val.pointer, "world"), 0);

  EXPECT_EQ(remove_node(list, (NodeValue){.pointer = word_two}).err, -1);

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

  free_list(list);
  return SUCCESS;
}

TestResult test_push_at_pointer() {
  LinkedList *list = init_list();

  char *word_one = "hello";
  char *word_two = "world";
  char *word_three = "!";
  char *word_four = "\n";

  push_node(list, (NodeValue){.pointer = word_one});
  push_node(list, (NodeValue){.pointer = word_two});
  push_node_at(list, (NodeValue){.pointer = word_three}, 1);

  EXPECT_EQ(strcmp(list->head->data.pointer, "hello"), 0);
  EXPECT_EQ(strcmp(list->head->next->data.pointer, "!"), 0);
  EXPECT_EQ(strcmp(list->tail->data.pointer, "world"), 0);

  push_node_at(list, (NodeValue){.pointer = word_four}, 2);

  EXPECT_EQ(strcmp(list->head->next->next->data.pointer, "\n"), 0);
  EXPECT_EQ(strcmp(list->tail->data.pointer, "world"), 0);
  EXPECT_EQ(strcmp(list->head->next->data.pointer, "!"), 0);

  free_list(list);
  return SUCCESS;
}

int main() {
  Test tests[] = {
      new_test("Test Push", test_push),
      new_test("Test Push (Pointer)", test_push_pointer),
      new_test("Test Pop", test_pop),
      new_test("Test Pop (Pointer)", test_pop_pointer),
      new_test("Test Remove", test_remove),
      new_test("Test Remove (Pointer)", test_remove_pointer),
      new_test("Test Peek", test_peek),
      new_test("Test Push Pop", test_push_pop),
      new_test("Test Push Pop Remove", test_push_pop_remove),
      new_test("Test Push (At Index)", test_push_at),
      new_test("Test Push (At Index) (Pointer)", test_push_at_pointer),
  };
  Suite my_suite = new_suite("Linked List Tests", tests, 11);
  run_suite(my_suite);
  return 0;
}
