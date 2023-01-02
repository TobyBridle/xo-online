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
#include "client.h"
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

/*
 * HashMap
 */
/* #define INITIAL_MAP_SIZE 1000 */
/* #define LOAD_FACTOR 0.75 */
/* #define GROWTH_FACTOR 2 */
#define NO_VALUE -1

/* #define SHOULD_MAP_EXPAND(used, total) (float)used / total >= LOAD_FACTOR */

#define HASH(key, size) (key % size)
#define ZERO_HASHMAP(map)                                                      \
  for (int i = 0; i < map.bucket_count; ++i)                                   \
    map.buckets[i] = (Bucket) {                                                \
      .key = NO_VALUE, .value = NO_VALUE, .next = NULL                         \
    }

typedef union {
  uint i_value;
  char c_value;
  client_t *client;
  int err;
} BucketValue;

typedef struct BUCKET_T {
  int key;
  BucketValue value;
  // pointer to next bucket in case of collision
  struct BUCKET_T *next;
} Bucket;

typedef struct HASHMAP_T {
  Bucket *buckets;
  int bucket_count;
  int used_buckets;
  struct node
      *entry_ids; // We will use this to loop over the entries in the map.
} HashMap;

HashMap new_hashmap(int map_size);
void free_hashmap(HashMap *map);

void put(HashMap *map, int key, BucketValue value);
BucketValue get(HashMap map, int key);
void remove_value(HashMap map, int key);

/**
 * @brief Handle the errno provided by the socket failures
 *
 * @param err
 */
void handle_sock_error(int err);
#endif
