#ifndef NOUGHTS_CROSSES_UTILS_H

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef BOOL
#define BOOL int
#endif

#define NOUGHTS_CROSSES_UTILS_H
#include <stdio.h>
#include <stdlib.h>

struct node {
  int data;
  struct node *next;
};

/*
 * STACK
 */
typedef struct {
  struct node *head;
  BOOL is_full;
  int used;
} stck_t; // Unfortunately need to use stck_t instead of stack_t as it is
          // already defined

int push(stck_t *stack, int data);
int pop(stck_t *stack);
int peek(stck_t *stack);

#endif
