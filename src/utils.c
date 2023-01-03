#include "lib/utils.h"

LinkedList *init_list() {
  LinkedList *list = malloc(sizeof(LinkedList));
  list->head = list->tail = NULL;
  return list;
}

int push_node(LinkedList *list, int value) {
  struct node *new_node = (struct node *)malloc(sizeof(struct node));
  new_node->data = value;
  new_node->next = NULL;

  if (list->head == NULL) {
    list->head = list->tail = new_node;
    return 0;
  } else {
    list->tail->next = new_node;
    list->tail = new_node;
    return 0;
  }
}

int pop_node(LinkedList *list) {
  if (list->head == NULL) {
    return -1;
  }
  struct node *head = list->head;
  int data = head->data;
  list->head = head->next;
  free(head);
  return data;
}

int remove_node(LinkedList *list, int predicate) {
  if (list->head == NULL) {
    return -1;
  }
  struct node *head = list->head;
  struct node *prev = NULL;
  while (head != NULL) {
    if (head->data == predicate) {
      if (prev == NULL) {
        list->head = head->next;
      } else {
        prev->next = head->next;
      }
      free(head);
      return 0;
    }
    prev = head;
    head = head->next;
  }
  return -1;
}

void free_list(LinkedList *list) {
  struct node *head = list->head;
  struct node *next = NULL;
  while ((pop_node(list)) != -1)
    ;
  free(list);
}

stck_t *init_stack(int max_size) {
  stck_t *stack = malloc(sizeof(stck_t));
  memset(stack, 0, sizeof(stck_t));
  stack->head = NULL;
  stack->max = max_size;

  return stack;
}

int push(stck_t *stack, int data) {
  // Check if the stack is full
  if (stack->is_full)
    return -1;
  if (stack->used == stack->max) {
    stack->is_full = TRUE;
    return -1;
  }

  struct node *new_node = (struct node *)malloc(sizeof(struct node));
  new_node->data = data;
  new_node->next = stack->head;
  stack->head = new_node;
  stack->used++;
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

  stack->used--;
  return data;
}
int peek(stck_t *stack) {
  if (stack->head == NULL)
    return -1;
  return stack->head->data;
}

void free_stack(stck_t *stack) {
  while (pop(stack) != -1)
    ;
  free(stack);
}

HashMap new_hashmap(int map_size) {
  HashMap map = {.bucket_count = map_size,
                 .used_buckets = 0,
                 .buckets = calloc(map_size, sizeof(Bucket)),
                 .entry_ids = init_list()};
  ZERO_HASHMAP(map);
  return map;
}

void free_hashmap(HashMap *map) {
  int entry_id;
  while ((entry_id = pop_node(map->entry_ids)) != -1) {
    remove_value(*map, entry_id);
  }

  free(map->buckets);
  free(map->entry_ids);
}

void put(HashMap *map, int key, BucketValue value) {
  int hash = HASH(key, map->bucket_count);
  Bucket *curr = &map->buckets[hash];
  if (curr->key == NO_VALUE) {
    *curr = (Bucket){.key = key, .value = value, .next = NULL};
    push_node(map->entry_ids, key);
    map->used_buckets++;
    return;
  }

  while (curr->next != NULL && curr->key != key)
    curr = curr->next;

  if (curr->key == key) {
    curr->value = value;
  } else {
    Bucket *new = malloc(sizeof(Bucket));
    *new = (Bucket){.key = key, .value = value, .next = NULL};
    curr->next = new;
    push_node(map->entry_ids, key);
    return;
  }
}

BucketValue get(HashMap map, int key) {
  int hash = HASH(key, map.bucket_count);
  Bucket *curr = &map.buckets[hash];
  if (curr->key == NO_VALUE)
    return (BucketValue){.err = -1};

  while (curr != NULL && curr->key != key)
    curr = curr->next;

  if (curr == NULL) {
    return (BucketValue){.err = -1};
  } else {
    return curr->value;
  }
}

void remove_value(HashMap map, int key) {
  int hash = HASH(key, map.bucket_count);
  Bucket *curr = &map.buckets[hash];
  // If the bucket is empty
  if (curr->key == NO_VALUE)
    return;

  // If the bucket is the only one in the chain
  if (curr->next == NULL) {
    *curr = (Bucket){
        .key = NO_VALUE, .value = (BucketValue){.err = -1}, .next = NULL};
    remove_node(map.entry_ids, key);
    return;
  }

  // If the bucket is the first in the chain
  if (curr->key == key) {
    Bucket *next = curr->next;
    *curr = *next;
    free(next);
    remove_node(map.entry_ids, key);
    return;
  }

  // If the bucket is in the middle of the chain
  Bucket *prev = curr;
  curr = curr->next;
  while (curr != NULL && curr->key != key) {
    prev = curr;
    curr = curr->next;
  }

  if (curr == NULL)
    return;

  prev->next = curr->next;
  free(curr);
  remove_node(map.entry_ids, key);
}

void handle_sock_error(int err) {
  switch (err) {
  case EACCES:
    perror("\x1b[31;1mPermission to create a socket of the specified type "
           "and/or to bind to the specified port is denied.\x1b[0m\n");
    break;
  case EADDRINUSE:
    perror("\x1b[31;1mThe given address is already in use.\x1b[0m\n");
    break;
  case EADDRNOTAVAIL:
    perror("\x1b[31;1mThe specified address is not available from the local "
           "machine.\x1b[0m\n");
    break;
  case EAFNOSUPPORT:
    perror("Addresses in the specified family cannot be used with this "
           "socket.\x1b[0m\n");
    break;
  case EINVAL:
    perror("\x1b[31;1mThe socket is already bound to an address.\x1b[0m\n");
    break;
  case EMFILE:
    perror("\x1b[31;1mThe process already has the maximum number of files "
           "open.\x1b[0m\n");
    break;
  case ENFILE:
    perror("\x1b[31;1mThe system limit on the total number of open files has "
           "been reached.\x1b[0m\n");
    break;
  case ENOBUFS:
    perror("\x1b[31;1mInsufficient resources were available in the system to "
           "perform the operation.\x1b[0m\n");
    break;
  case ENOMEM:
    perror("\x1b[31;1mInsufficient memory was available to fulfill the "
           "request.\x1b[0m\n");
    break;
  case EPROTONOSUPPORT:
    perror("\x1b[31;1mThe protocol type or the specified protocol is not "
           "supported within this domain.\x1b[0m\n");
    break;
  case EPROTOTYPE:
    perror("\x1b[31;1mThe protocol type is the wrong type for this "
           "socket.\x1b[0m\n");
    break;
  case ESOCKTNOSUPPORT:
    perror("\x1b[31;1mThe socket type is not supported in this address "
           "family.\x1b[0m\n");
    break;
  case ECONNREFUSED:
    perror("\x1b[31;1mNo one listening on the remote address.\x1b[0m\n");
    break;
  default:
    fprintf(stderr, "Unknown error: %d\x1b[0m\n", err);
    break;
  }
}
