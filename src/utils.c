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

char *serialize_int(int i) {
  // 0x01 0x04 0x00 0x00 0x00 0x00
  // ---- ---- ---- ---- ---- ----
  // 6 Bytes + NULL Terminator
  char *buf = malloc(7);
  buf[0] = 0x01;
  buf[1] = 0x04;
  buf[2] = (i >> 24) & 0xFF;
  buf[3] = (i >> 16) & 0xFF;
  buf[4] = (i >> 8) & 0xFF;
  buf[5] = i & 0xFF;
  buf[6] = '\0';
  return buf;
}

int deserialize_int(char *buf) {
  int i = 0;
  i |= buf[2] << 24;
  i |= buf[3] << 16;
  i |= buf[4] << 8;
  i |= buf[5];
  return i;
}

char *serialize_bool(BOOL b) {
  // 0x01 0x01 0x00
  // ---- ---- ----
  // 3 Bytes + NULL Terminator
  char *buf = malloc(4);
  buf[0] = 0x01;
  buf[1] = 0x01;
  buf[2] = b ? 0x01 : 0x00;
  buf[3] = '\0';
  return buf;
}

BOOL deserialize_bool(char *buf) { return buf[2] == 0x01; }

char *serialize_string(char *str) {
  // 0x03 0x02 0x00 0x00
  // ---- ---- ---- ----
  // 4 Bytes + NULL Terminator
  // The 2nd byte will be the length of characters + 1
  // as the last byte would be the NULL terminator

  int len = strlen(str) + 1;

  if (len > 255) {
    fprintf(stderr, "\x1b[31;1mString is too long to be serialized\x1b[0m\n");
    return NULL;
  } else if (len == 1) {
    fprintf(stderr, "\x1b[31;1mString is empty\x1b[0m\n");
    return NULL;
  }

  char *buf = calloc(len + 2, 1);
  buf[0] = 0x03;
  buf[1] = len;
  memcpy(buf + 2, str, len);
  return buf;
}

char *deserialize_string(char *buf) {

  if (buf[0] != 0x03) {
    fprintf(stderr, "\x1b[31;1mInvalid string\x1b[0m\n");
    return NULL;
  } else if (buf[1] == 0x00) {
    fprintf(stderr, "\x1b[31;1mString is empty\x1b[0m\n");
    return NULL;
  }

  int len = buf[1];

  char *str = calloc(len, 1);
  memcpy(str, buf + 2, len);
  return str;
}

char *serialize_enum(int e) {
  // 0x01 0x01 0x00
  // ---- ---- ----
  // 3 Bytes + NULL Terminator
  char *buf = malloc(4);
  buf[0] = 0x01;
  buf[1] = 0x01;
  buf[2] = e;
  buf[3] = '\0';
  return buf;
}

int deserialize_enum(char *buf) { return buf[2]; }

char *serialize_client(client_t *client) {
  // Fields of client_t:
  // int socket;
  // int client_id;
  // char *client_name;
  // struct sockaddr_in addr;
  // enum {
  //   NOUGHT,
  //   CROSS,
  //   SPECTATOR
  // } player_type;

  // The length of fields (when serialized) are:
  // Note: This excludes the NULL terminator
  // int socket: 6
  // int client_id: 6
  // char *client_name: strlen(client_name) + 2
  // struct sockaddr_in addr: 16
  // enum player_type: 3

  int len = 6 + 6 + strlen(client->client_name) + 2 + 16 + 9;
  char *buf = calloc(len + 2, 1);

  // Setting serialized bytes info
  buf[0] = 0x04;
  buf[1] = len;

  // Serializing fields
  memcpy(buf + 2, serialize_int(client->socket), 6);
  memcpy(buf + 8, serialize_int(client->client_id), 6);
  memcpy(buf + 14, serialize_string(client->client_name),
         strlen(client->client_name) + 2);

  // We need to serialize the struct sockaddr_in addr
  // Fields:
  // .sin_family (enum),
  // .sin_addr (struct in_addr),
  // .sin_len (unsigned char),
  // .sin_port (unsigned short),

  // Serializing the sin_family using serialize_enum
  memcpy(buf + 15 + strlen(client->client_name) + 2,
         serialize_enum(client->addr.sin_family), 3);

  // Serializing the sin_addr using serialize_int
  memcpy(buf + 18 + strlen(client->client_name) + 2,
         serialize_int(client->addr.sin_addr.s_addr), 6);

  // Serializing the sin_len using serialize_int
  memcpy(buf + 24 + strlen(client->client_name) + 2,
         serialize_int(client->addr.sin_len), 6);

  // Serializing the sin_port using serialize_int
  memcpy(buf + 30 + strlen(client->client_name) + 2,
         serialize_int(client->addr.sin_port), 6);

  // Serializing the player_type using serialize_enum
  memcpy(buf + 36 + strlen(client->client_name) + 2,
         serialize_enum(client->player_type), 3);
  return buf;
}

client_t *deserialize_client(char *buf) {
  // Fields of client_t:
  // int socket;
  // int client_id;
  // char *client_name;
  // struct sockaddr_in addr;
  // enum {
  //   NOUGHT,
  //   CROSS,
  //   SPECTATOR
  // } player_type;

  client_t *client = malloc(sizeof(client_t));
  client->socket = deserialize_int(buf + 2);
  client->client_id = deserialize_int(buf + 8);
  client->client_name = deserialize_string(buf + 14);
  client->addr.sin_family =
      deserialize_enum(buf + 15 + strlen(client->client_name) + 2);
  client->addr.sin_addr.s_addr =
      deserialize_int(buf + 18 + strlen(client->client_name) + 2);
  client->addr.sin_len =
      deserialize_int(buf + 24 + strlen(client->client_name) + 2);
  client->addr.sin_port =
      deserialize_int(buf + 30 + strlen(client->client_name) + 2);
  client->player_type =
      deserialize_enum(buf + 36 + strlen(client->client_name) + 2);

  return client;
}
