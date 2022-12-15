#include "lib/utils.h"

int push(stck_t *stack, int data) {
  // Check if the stack is full
  if (stack->is_full)
    return -1;

  struct node *new_node = (struct node *)malloc(sizeof(struct node));
  new_node->data = data;
  new_node->next = stack->head;
  stack->head = new_node;
  stack->used--;
  return 0;
}

int pop(stck_t *stack) {
  // Check if the stack is empty
  if (peek(stack) == -1) {
    return -1;
  }
  // Clone the data
  int data = stack->head->data;
  // Clone the head pointer
  struct node *head = (struct node *)malloc(sizeof(struct node));
  head->data = stack->head->data;
  head->next = stack->head->next;

  free(stack->head);
  stack->head = head->next;
  free(head);

  stack->used++;
  return data;
}
int peek(stck_t *stack) {
  if (stack->head == NULL)
    return -1;
  return stack->head->data;
}
