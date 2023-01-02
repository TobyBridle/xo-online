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

HashMap new_hashmap(int map_size) {
  HashMap map = {.buckets = calloc(map_size, sizeof(Bucket)),
                 .bucket_count = map_size,
                 .used_buckets = 0,
                 .entry_ids = NULL};
  ZERO_HASHMAP(map);
  return map;
}

void free_hashmap(HashMap *map) {
  // Free all linked lists inside buckets
  // Free the buckets themselves
  for (int i = 0; i < map->bucket_count; ++i) {
    if (map->buckets[i].next != NULL) {
      Bucket *bucket, *next;
      bucket = (Bucket *)map->buckets[i].next;
      while (bucket != NULL) {
        next = (Bucket *)bucket->next;
        free(bucket);
        bucket = next;
      }
    }
  }
  free(map->buckets);
}

void put(HashMap *map, int key, BucketValue value) {
  int hash = HASH(key, map->bucket_count);
  if (map->buckets[hash].key == NO_VALUE) {
    map->buckets[hash] = (Bucket){.key = key, .value = value, .next = NULL};
    map->used_buckets++;
  } else {
    // TODO: HANDLE COLLISIONS BETTER!
    Bucket *current = &map->buckets[hash];
    while (current->next != NULL) {
      current = current->next;
    }
    Bucket *new_bucket = malloc(sizeof(Bucket));
    new_bucket->key = key;
    new_bucket->value = value;
    new_bucket->next = NULL;
    current->next = new_bucket;
  }
}

BucketValue get(HashMap map, int key) {
  int hash = HASH(key, map.bucket_count);
  if (map.buckets[hash].key == NO_VALUE) {
    return (BucketValue){.err = -1};
  } else {
    Bucket *current = &map.buckets[hash];
    while (current != NULL) {
      if (current->key == key) {
        return current->value;
      }
      current = (Bucket *)current->next;
    }
    return (BucketValue){.err = -1};
  }
}

void remove_value(HashMap map, int key) {
  int hash = HASH(key, map.bucket_count);
  if (map.buckets[hash].key == NO_VALUE) {
    return;
  } else {
    // Find the bucket to remove
    Bucket *current = &map.buckets[hash];
    Bucket *prev = NULL;
    while (current != NULL) {
      if (current->key == key) {
        if (prev == NULL && current->next == NULL) {
          // Remove the first bucket
          map.buckets[hash] = (Bucket){.key = NO_VALUE, .value = {.err = -1}};
        } else if (prev == NULL && current->next != NULL) {
          map.buckets[hash] = *current->next;
          return;
        } else {
          prev->next = current->next;
          free(current);
        }
        return;
      }
      prev = current;
      current = (Bucket *)current->next;
    }
    // free(current);
    // free(prev);
  }
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
