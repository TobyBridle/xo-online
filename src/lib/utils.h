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

#ifndef NOUGHTS_CROSSES_CLIENT_T
#define NOUGHTS_CROSSES_CLIENT_T

#ifndef CLAMP
#define CLAMP(val, min, max)                                                   \
  ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
#endif

#ifndef MAX
#define MAX(val, min) (val > min ? val : min)
#endif

#ifndef NEXT_ITER
#define NEXT_ITER(head) head = head->next != NULL ? head->next : NULL;
#endif


#include <ctype.h>
#include <netinet/in.h>
typedef struct {
  int socket;
  int client_id; // This corresponds to the index in the server->conns->clients
                 // array and is -1 if the client has not yet been registered by
                 // the server
  char *client_name; // This is the display name of the user, which we will show
                     // other players
  struct sockaddr_in addr;
  enum {
    NOUGHT,
    CROSS,
    SPECTATOR
  } player_type; // Spectator by default, so that the user cannot interact with
                 // any games
  enum {
    SETUP_PAGE, // This includes the initial connection page,
                // and name entering
    HOME_PAGE,
    GAME_VIEW_PAGE,
    IN_GAME_PAGE,
    /* SPECTATOR_PAGE, */
  } screen_state;

  void *game; // This will not be set at all by the client
              // but will be used by the server.
  unsigned long last_sent_game_hash;
} client_t;
#endif

#define NOUGHTS_CROSSES_UTILS_H
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef union {
  int i_value;
  int err;
  void *pointer;
} NodeValue;

struct node {
  NodeValue data;
  struct node *next;
};

typedef struct LINKEDLIST_T {
  struct node *head;
  struct node *tail;
} LinkedList;

LinkedList *init_list();
int push_node(LinkedList *list, NodeValue value);
int push_node_at(LinkedList *list, NodeValue value, int index);
NodeValue pop_node(LinkedList *list);
struct node *peek_node(LinkedList *list);
NodeValue remove_node(LinkedList *list, NodeValue predicate);
void free_list(LinkedList *list);

/*
 * STACK
 */
typedef struct {
  struct node *top;
  BOOL is_full;
  int used;
  int max;
} stck_t; // Unfortunately need to use stck_t instead of stack_t as it is
          // already defined

stck_t *init_stack(int max_size);
int push(stck_t *stack, NodeValue data);
NodeValue pop(stck_t *stack);
NodeValue peek(stck_t *stack);
void free_stack(stck_t *stack);

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
  unsigned int i_value;
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
  LinkedList
      *entry_ids; // We will use this to loop over the entries in the map.
} HashMap;

HashMap new_hashmap(int map_size);
void free_hashmap(HashMap *map);

void put(HashMap *map, int key, BucketValue value);
BucketValue get(HashMap map, int key);
void remove_value(HashMap *map, int key);

#ifndef HANDLE_SOCK_ERROR_FN
#define HANDLE_SOCK_ERROR_FN
/**
 * @brief Handle the errno provided by the socket failures
 *
 * @param err
 */
void handle_sock_error(int err);
#endif

// We need some way of serializing the data to send it over the socket.
// We'll use a generic function to do this which will call the appropriate
// function based on the type of data we want to serialize.

// We will support the following types:
// - int
// - bool (int)
// - char
// - client_t

// The first byte will be the type of data we are sending.
// e.g. 0x01 for int, 0x02 for bool, 0x03 for string, 0x04 for client_t

// The second byte will be the size of the data we are sending.
// This will be represented using hexadecimals.
// e.g. 0x01 for 1 byte, 0x02 for 2 bytes, 0x04 for 4 bytes, 0x08 for 8 bytes
// 0xFF for 255 bytes (maximum)
// It is important to note that serializing a char will always be 1 byte.
// This means that strings will be serialized with the null terminator.
// This will affect the size; however, we will not need an additional byte
// as the null terminator value will be assumed.

// The rest of the bytes will be the data itself.
// We will use a union to store the data and then cast it to a char array
// to send it over the socket.

// We will use a similar approach to deserialize the data.
// The serialization of a struct will be done by serializing each of its
// members in order.

// As a serialization example we will use the following data:
// 1. int i = 5;
// 2. bool b = true;
// 3. char *c = "a";
// 4. client_t client = { .name = "John", .id = 1 };

// 1 => 0x01 0x04 0x00 0x00 0x00 0x05 => 0x010400000005
// 2 => 0x02 0x01 0x01 => 0x020101
// 3 => 0x03 0x02 0x61 => 0x030161
// 4. We will serialize the client_t struct in order by field
// First we will serialize the name field
// 4.1 => 0x03 0x04 0x00 0x00 0x00 0x4A 0x6F 0x68 0x6E
// Then we will serialize the id field
// 4.2 => 0x01 0x04 0x00 0x00 0x00 0x01
// The final serialization will be the concatenation of the two,
// with the information for the entire struct prepended.
// (0x04 - client_t, 0x04 + 0x04 - 8 bytes)
// 4.3 => 0x04            0x08                        0x03          0x04 0x4A
// 0x6F 0x68 0x6E 0x01       0x04           0x00 0x00 0x00 0x01
//        type (client_t) size (0x04 name + 0x04 int) type (char)   size (4
//        bytes) data (John)         type (int) size (4 bytes) data (1)
// => 0x040803044A6F686E010400000001

// The deserialization of this data will be done in the same order.
// We will use a union to store the data and then cast it to the appropriate
// type.
// Using 4.3 as an example:
// 1. We will read the first byte to get the type of data we are deserializing.
// 2. We will read the second byte to get the size of the data we are
// deserializing.
// 3. We will read the rest of the bytes to get the data itself.
// 4. We will cast the data to the appropriate type.
// 5. We will repeat the process until we have deserialized all the data.

// As we know that the serialization of the struct is done in order,
// we know that the deserialization will be done in order as well.
// This means that the first thing we deserialize will be the first field in the
// struct (name).

// 1 - 0x04 => client_t
// 2 - 0x08 => 8 bytes
// 3 - 0x03 0x04 0x4A 0x6F 0x68 0x6E 0x01 0x04 0x00 0x00 0x00 0x01
// 4.1 - 0x03 => char
// 4.2 - 0x04 => 4 bytes
// 4.3 - 0x4A 0x6F 0x68 0x6E => John
// 4.4 - 0x01 => int
// 4.5 - 0x04 => 4 bytes
// 4.6 - 0x00 0x00 0x00 0x01 => 1
// 5. No need to repeat the process since we have deserialized all the data.

// We have obtained `client_t client = { .name = "John", .id = 1 };`

typedef struct {
  char *str;
  size_t len;
} serialized_string;

typedef union {
  serialized_string str;
  char *val;
} serialized;

#ifndef INT_SERIALIZE_FLAG
#define INT_SERIALIZE_FLAG 0x01
#endif
char *serialize_int(int i);

#ifndef BOOL_SERIALIZE_FLAG
#define BOOL_SERIALIZE_FLAG 0x02
#endif
char *serialize_bool(int b); // Where b = 0 or 1

#ifndef STRING_SERIALIZE_FLAG
#define STRING_SERIALIZE_FLAG 0x03
#endif
serialized_string serialize_string(char *str);

#ifndef CLIENT_SERIALIZE_FLAG
#define CLIENT_SERIALIZE_FLAG 0x04
#endif
char *serialize_client(client_t *client);

#ifndef ENUM_SERIALIZE_FLAG
#define ENUM_SERIALIZE_FLAG 0x05
#endif
char *serialize_enum(int e);

typedef union {
  char *string;
  int i;
  client_t client;
} deserialized;

int deserialize_int(char *buf);
BOOL deserialize_bool(char *buf);
char *deserialize_string(char *buf);
int deserialize_enum(char *buf);
client_t *deserialize_client(char *buf);

uint8_t trim_whitespace(char *str);
BOOL is_valid_input_key(char c);
int smart_send(int socket, const void *data, int data_length);
int smart_recv(int socket, void *buffer, int buffer_size);

unsigned int hash_string(char *buf, unsigned int mod);

void *__malloc(size_t size, const char *file, int line);
#endif
