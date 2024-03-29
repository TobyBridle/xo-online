#include "lib/utils.h"

#define _malloc malloc
#ifdef DEBUG
#define free(x)                                                                \
  fprintf(stderr, "Freeing Address %p at %s:%d\n", x, __FILE__, __LINE__);     \
  free(x);                                                                     \
  fprintf(stderr, "Freeing Successful\n");

#define _malloc(e) __malloc(e, __FILE__, __LINE__);
#endif

LinkedList *init_list() {
  // LinkedList uses a head for the first node and a tail for the next
  // node to be added.
  // Initially, the head and tail are NULL
  // We can say a list is empty if head == NULL
  LinkedList *list = calloc(1, sizeof(LinkedList));
  list->head = list->tail = NULL;
  return list;
}

int push_node(LinkedList *list, NodeValue value) {
  struct node *new_node = (struct node *)_malloc(sizeof(struct node));
  new_node->data = value;
  new_node->next = NULL;

  // If there are no nodes in the LinkedList
  if (list->head == NULL) {
    list->head = list->tail = new_node;
    return 0;
  } else {
    list->tail->next = new_node;
    list->tail = list->tail->next;
  }

  return 0;
}

// Push a node at a specific (0-based) index
int push_node_at(LinkedList *list, NodeValue value, int index) {
  struct node *new_node = (struct node *)_malloc(sizeof(struct node));
  new_node->data = value;
  new_node->next = NULL;

  // If there are no nodes in the LinkedList
  if (list->head == NULL) {
    list->head = list->tail = new_node;
    return 0;
  } else {
    struct node *curr = list->head;
    struct node *prev = NULL;
    uint iters = 0;
    while (curr != NULL && iters != index) {
      prev = curr;
      curr = curr->next;
      ++iters;
    }
    if (prev == NULL) {
      new_node->next = curr;
      list->head = new_node;
      return 0;
    } else if (index == iters + 1 || curr == NULL) {
      new_node->next = curr;
      prev->next = list->tail = new_node;
      return 0;
    } else {
      new_node->next = curr;
      prev->next = new_node;
      return 0;
    }
  }

  return -1;
}

NodeValue pop_node(LinkedList *list) {
  if (list->head == NULL) {
    return (NodeValue){.err = -1};
  }
  struct node *head_copy = list->head;
  NodeValue data = head_copy->data;
  list->head = list->head->next;
  free(head_copy);
  return data;
}

struct node *peek_node(LinkedList *list) {
  if (list->head == NULL)
    return NULL;
  return list->head;
}

NodeValue remove_node(LinkedList *list, NodeValue predicate) {
  if (list->head == NULL) {
    return (NodeValue){.err = -1};
  }
  struct node *head = list->head;
  struct node *prev = NULL;
  NodeValue ret;
  while (head != NULL) {
    if (head->data.pointer == predicate.pointer ||
        head->data.i_value == predicate.i_value) {
      if (prev == NULL) {
        list->head = head->next;
      } else {
        prev->next = head->next;
      }
      // We might not want to free the node here
      // We may just want to nullify the node
      if (head == list->tail) {
        list->tail = prev;
      }
      ret = head->data;
      free(head);
      head = NULL;
      return ret;
    }
    prev = head;
    head = head->next;
  }
  return (NodeValue){.err = -1};
}

void free_list(LinkedList *list) {
  struct node *head = list->head;
  struct node *next = NULL;
  while ((pop_node(list)).err != -1)
    ;
  free(list);
}

stck_t *init_stack(int max_size) {
  stck_t *stack = calloc(1, sizeof(stck_t));
  stack->is_full = stack->used = 0;
  stack->max = max_size;
  stack->top = NULL;
  return stack;
}

int push(stck_t *stack, NodeValue value) {
  if (stack->is_full)
    return -1;
  struct node *new_node = (struct node *)calloc(1, sizeof(struct node));
  new_node->data = value;
  new_node->next = stack->top;
  stack->top = new_node;
  stack->used++;
  if (stack->used == stack->max)
    stack->is_full = 1;
  return 0;
}

NodeValue pop(stck_t *stack) {
  // Check if the stack is empty
  if (peek(stack).err == -1)
    return (NodeValue){.err = -1};

  struct node *top = stack->top;
  NodeValue data = stack->top->data;
  stack->top = top->next;

  free(top);
  top = NULL;
  stack->used--;

  if (stack->used == stack->max - 1)
    stack->is_full = 0;
  return data;

  return (NodeValue){.err = -1};
}

NodeValue peek(stck_t *stack) {
  if (stack->top == NULL)
    return (NodeValue){.err = -1};
  return stack->top->data;
}

void free_stack(stck_t *stack) {
  while (pop(stack).err != -1)
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
  NodeValue entry_id;
  while ((entry_id = pop_node(map->entry_ids)).err != -1) {
    remove_value(map, entry_id.i_value);
  }

  free(map->buckets);
  free(map->entry_ids);
}

void put(HashMap *map, int key, BucketValue value) {
  int hash = HASH(key, map->bucket_count);
  Bucket *curr = &map->buckets[hash];
  if (curr->key == NO_VALUE) {
    *curr = (Bucket){.key = key, .value = value, .next = NULL};
    // We want to insert the key into the entry_ids list
    // but in an ordered fashion
    // The entry_ids list is a linked list
    // in ascending order
    // We need to get the node before the node that is greater than key
    // and insert the node after that node
    uint insert_pos = 0;
    struct node *curr = map->entry_ids->head;
    while (curr != NULL && curr->data.i_value < key) {
      curr = curr->next;
      insert_pos++;
    }

    // We have the position to insert the node
    push_node_at(map->entry_ids, (NodeValue){.i_value = key}, insert_pos);

    map->used_buckets++;
    return;
  }

  // NOTE: If we get here, we have a collision
  while (curr->next != NULL && curr->key != key)
    curr = curr->next;

  if (curr->key == key) {
    curr->value = value;
  } else {
    Bucket *new = malloc(sizeof(Bucket));
    *new = (Bucket){.key = key, .value = value, .next = NULL};
    curr->next = new;
    push_node(map->entry_ids, (NodeValue){.i_value = key});
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

void remove_value(HashMap *map, int key) {
  int hash = HASH(key, map->bucket_count);
  Bucket *curr = &map->buckets[hash];
  if (curr == NULL)
    return;

  // If the bucket is empty
  if (curr->key == NO_VALUE)
    return;

  // If the bucket is the only one in the chain
  if (curr->next == NULL) {
    *curr = (Bucket){
        .key = NO_VALUE, .value = (BucketValue){.err = -1}, .next = NULL};
    remove_node(map->entry_ids, (NodeValue){.i_value = key});
    map->used_buckets--;
    return;
  }

  // If the bucket is the first in the chain
  if (curr->key == key) {
    Bucket *next = curr->next;
    *curr = *next;
    free(next);
    remove_node(map->entry_ids, (NodeValue){.i_value = key});
    map->used_buckets--;
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
  map->used_buckets--;
  remove_node(map->entry_ids, (NodeValue){.i_value = key});
}

void handle_sock_error(int err) {
  char *error_message;
  switch (err) {
  case EACCES:
    error_message =
        "\x1b[41;30;1mSorry, you do not have permission to do "
        "that.\x1b[0;0m\r\n"
        "\x1b[43;30;1mIf you haven't already, try running with "
        "`sudo` permissions.\x1b[0;0m\n"
        "\x1b[;1mExample: sudo ./server\n"
        "\x1b[41;30;1mMaybe get in touch with one of your seniors!\x1b[0;0m";
    printf("%s\r\n\x1b[0;0m", error_message);
    perror("\x1b[41;30;1mPermission to create a socket of the specified type "
           "and/or to bind to the specified port is denied.\x1b[0m\n");
    break;
  case EADDRINUSE:
    error_message =
        "\x1b[41;30;1mSorry, you can't do that: the address is already in "
        "use\r\n\x1b[0;0m"
        "\x1b[43;30;1mIf you are trying to launch the server, make sure that "
        "you've not already got one running!";
    printf("%s\r\n\x1b[0;0m", error_message);
    perror("\x1b[41;30;1mThe given address is already in use.\x1b[0m\n");
    break;
  case EAFNOSUPPORT:
    error_message =
        "\x1b[41;30;1mSorry, you can't do that: the address family is not "
        "supported"
        "\x1b[43;30;1mIf this is happening to you, please contact the "
        "developer!";
    printf("%s\r\n\x1b[0;0m", error_message);
    perror("Addresses in the specified family cannot be used with this "
           "socket.\x1b[0m\n");
    break;
  case EINVAL:
    error_message =
        "\x1b[41;30;1mSorry, you can't do that: the address is already in "
        "use\r\n\x1b[0;0m"
        "\x1b[43;30;1mIf you are trying to launch the server, make sure that"
        "you've not already got one running!";
    printf("%s\r\n\x1b[0;0m", error_message);
    perror("\x1b[41;30;1mThe socket is already bound to an address.\x1b[0m\n");
    break;
  case EMFILE:
    error_message =
        "\x1b[41;30;1mSorry, you can't do that: the process already has the "
        "maximum number of files open\r\n\x1b[0;0m"
        "\x1b[43;30;1mIf this is happening to you, please contact the "
        "developer!";
    printf("%s\r\n\x1b[0;0m", error_message);
    perror("\x1b[41;30;1mThe process already has the maximum number of files "
           "open.\x1b[0m\n");
    break;
  case ENFILE:
    error_message =
        "\x1b[41;30;1mSorry, you can't do that: the system limit on the "
        "total number of open files has been reached\r\n\x1b[0;0m"
        "\x1b[43;30;1mYou may be able to fix this by closing some other "
        "applications."
        "\x1b[0;0m";
    printf("%s\r\n\x1b[0;0m", error_message);
    perror(
        "\x1b[41;30;1mThe system limit on the total number of open files has "
        "been reached.\x1b[0m\n");
    break;
  case ENOBUFS:
    error_message =
        "\x1b[41;30;1mSorry, you can't do that: insufficient resources were "
        "available in the system to perform the operation\r\n\x1b[0;0m"
        "\x1b[43;30;1mYou may be able to fix this by closing some other "
        "applications."
        "\x1b[0;0m";
    printf("%s\r\n\x1b[0;0m", error_message);
    perror(
        "\x1b[41;30;1mInsufficient resources were available in the system to "
        "perform the operation.\x1b[0m\n");
    break;
  case ENOMEM:
    error_message =
        "\x1b[41;30;1mSorry, you can't do that: insufficient memory was "
        "available to fulfill the request\r\n\x1b[0;0m"
        "\x1b[43;30;1mYou may be able to fix this by closing some other "
        "applications."
        "\x1b[0;0m";
    printf("%s\r\n\x1b[0;0m", error_message);
    perror("\x1b[41;30;1mInsufficient memory was available to fulfill the "
           "request.\x1b[0m\n");
    break;
  case EPROTONOSUPPORT:
    error_message =
        "\x1b[41;30;1mSorry, something went wrong!\r\n\x1b[0;0m"
        "\x1b[43;30;1mIf this is happening to you, please contact the "
        "developer!";
    printf("%s\r\n\x1b[0;0m", error_message);
    perror("\x1b[41;30;1mThe protocol type or the specified protocol is not "
           "supported within this domain.\x1b[0m\n");
    break;
  case EPROTOTYPE:
    error_message =
        "\x1b[41;30;1mSorry, something went wrong!\r\n\x1b[0;0m"
        "\x1b[43;30;1mIf this is happening to you, please contact the "
        "developer!";
    printf("%s\r\n\x1b[0;0m", error_message);
    perror("\x1b[41;30;1mThe protocol type is the wrong type for this "
           "socket.\x1b[0m\n");
    break;
  case ESOCKTNOSUPPORT:
    error_message =
        "\x1b[41;30;1mSorry, something went wrong!\r\n\x1b[0;0m"
        "\x1b[43;30;1mIf this is happening to you, please contact the "
        "developer!";
    printf("%s\r\n\x1b[0;0m", error_message);
    perror("\x1b[41;30;1mThe socket type is not supported in this address "
           "family.\x1b[0m\n");
    break;
  case ECONNREFUSED:
    error_message =
        "\x1b[41;30;1mSorry, you can't do that: no one is listening on the "
        "remote address\r\n\x1b[0;0m"
        "\x1b[43;30;1mIf you are trying to connect to the server, make sure "
        "that you've launched it!";
    printf("%s\r\n\x1b[0;0m", error_message);
    perror("\x1b[41;30;1mNo one listening on the remote address.\x1b[0m\n");
    break;
  case EADDRNOTAVAIL:
    error_message =
        "\x1b[41;30;1mSorry, you can't do that: the specified address is not "
        "available from the local machine\r\n\x1b[0;0m"
        "\x1b[43;30;1mIf you are trying to connect to the server, make sure "
        "that you've launched it or that it hasn't crashed!";
    printf("%s\r\n\x1b[0;0m", error_message);
    perror("\x1b[41;30;1mThe specified address is not available from the "
           "local machine.\x1b[0m\n");
    break;
  default:
    error_message =
        "\x1b[41;30;1mSorry, something went wrong!\r\n\x1b[0;0m"
        "\x1b[43;30;1mIf this is happening to you, please contact the "
        "developer and include the following error code:";
    printf("%s\r\n\x1b[0;0m", error_message);
    fprintf(stderr, "Unknown error: %d\x1b[0m\n", err);
    break;
  }
}

char *serialize_int(int i) {
  // 0x01 0x04 0x00 0x00 0x00 0x00
  // ---- ---- ---- ---- ---- ----
  // 6 Bytes + NULL Terminator
  char *buf = calloc(7, sizeof(char));
  buf[0] = INT_SERIALIZE_FLAG;
  buf[1] = 0x04;
  buf[2] = (i >> 24) & 0xFF;
  buf[3] = (i >> 16) & 0xFF;
  buf[4] = (i >> 8) & 0xFF;
  buf[5] = i & 0xFF;
  buf[6] = '\0';
  return buf;
}

int deserialize_int(const char *buf) {
  // The buffer is not a serialized int
  if (buf[0] != INT_SERIALIZE_FLAG)
    return -1;
  int i = 0;
  i |= buf[2] << 24;
  i |= buf[3] << 16;
  i |= buf[4] << 8;
  i |= buf[5];
  return i;
}

char *serialize_bool(BOOL b) {
  // 0x01 0x01 0x01 0x00
  // ---- ---- ----
  // 3 Bytes + NULL Terminator
  char *buf = calloc(4, sizeof(char));
  buf[0] = BOOL_SERIALIZE_FLAG;
  buf[1] = 0x01;
  buf[2] = b ? 0x01 : 0x00;
  buf[3] = '\0';
  return buf;
}

BOOL deserialize_bool(const char *buf) {
  if (buf[0] != BOOL_SERIALIZE_FLAG)
    return -1;
  return buf[2] == 0x01;
}

serialized_string serialize_string(char *str) {
  // 0x03 0x02 0x00 0x00
  // ---- ---- ---- ----
  // 4 Bytes + NULL Terminator
  // The 2nd byte will be the length of characters + 1
  // as the last byte would be the NULL terminator

  int len = strlen(str) + 1;

  if (len > 255) {
    fprintf(stderr, "\x1b[31;1mString is too long to be serialized\x1b[0m\n");
    return (serialized_string){NULL};
  } else if (len == 1) {
    fprintf(stderr, "\x1b[31;1mString is empty\x1b[0m\n");
    return (serialized_string){NULL};
  }

  char *buf = calloc(len + 2, 1);
  buf[0] = STRING_SERIALIZE_FLAG;
  buf[1] = len;
  memcpy(buf + 2, str, len);
  return (serialized_string){.str = buf, .len = len};
}

char *deserialize_string(const char *buf) {

  if (buf == NULL)
    return NULL;
  if (buf[0] != STRING_SERIALIZE_FLAG) {
    return NULL;
  } else if (buf[1] == 0x00) {
    return NULL;
  }

  int len = buf[1];

  char *str = calloc(len, 1);
  memcpy(str, buf + 2, len);
  return str;
}

char *serialize_enum(int e) {
  // 0x05 0x01 0x00
  // ---- ---- ----
  // 3 Bytes + NULL Terminator
  char *buf = calloc(4, sizeof(char));
  buf[0] = ENUM_SERIALIZE_FLAG;
  buf[1] = 0x01;
  buf[2] = e;
  buf[3] = '\0';
  return buf;
}

int deserialize_enum(const char *buf) {
  if (buf[0] != ENUM_SERIALIZE_FLAG)
    return -1;
  return buf[2];
}

char *serialize_client(client_t *client) {
  unsigned int buf_length =
      2 + 1; // 2 for the type & len, 1 for the null terminator.

  // Serialize the Socket Field
  char *client_socket_f = serialize_int(client->socket); // Adds 6 bytes.
  buf_length += 6;

  // Serialize the Client ID Field
  char *client_id_f = serialize_int(client->client_id); // Another 6 bytes.
  buf_length += 6;

  // Serialize the Client Name Field
  serialized_string client_name_f = serialize_string(client->client_name);
  // `.len` is the length of the contained string (including null byte.)
  // We want to ignore the null byte.
  buf_length +=
      client_name_f.len + 2; // +2 for the type & len, -1 to remove null byte.

  // Serialize the Addr Fields.
  // Serialize the Addr Port.
  char *client_sin_port_f = serialize_int(client->addr.sin_port);
  buf_length += 6;

  // Serialize the Addr Family.
  char *client_sin_family_f = serialize_int(client->addr.sin_family);
  buf_length += 6;

  // Serialize the Addr Address
  char *client_sin_addr_f = serialize_int(client->addr.sin_addr.s_addr);
  buf_length += 6;

  char *buf = calloc(buf_length, sizeof(char));
  buf[0] = CLIENT_SERIALIZE_FLAG;
  buf[1] =
      buf_length - 2 - 1; // Get rid of the length field and the null terminator
  // Move the Socket Field into the Buffer.
  unsigned int buf_offset = 2;
  memcpy(buf + buf_offset, client_socket_f, 6);
  buf_offset += 6;

  // Move the Client ID Field into the Buffer.
  memcpy(buf + buf_offset, client_id_f, 6);
  buf_offset += 6;

  // Move the Addr Fields into the Buffer.
  // Move the Addr Port field into the Buffer.
  memcpy(buf + buf_offset, client_sin_port_f, 6);
  buf_offset += 6;

  // Move the Addr Family field into the Buffer.
  memcpy(buf + buf_offset, client_sin_family_f, 6);
  buf_offset += 6;

  // Move the Addr Address field into the Buffer.
  memcpy(buf + buf_offset, client_sin_addr_f, 6);
  buf_offset += 6;

  // Move the Client Name Field into the Buffer.
  memcpy(buf + buf_offset, client_name_f.str, client_name_f.len + 2 - 1);
  buf_offset += client_name_f.len + 2;

  // Free the intermediate values.
  free(client_socket_f);
  free(client_id_f);
  free(client_sin_port_f);
  free(client_sin_family_f);
  free(client_sin_addr_f);
  free(client_name_f.str);

  return buf;
}

client_t *deserialize_client(const char *buf) {
  if (buf == NULL ||
      buf[0] !=
          CLIENT_SERIALIZE_FLAG) // Make sure that it is a serialized client_t
    return NULL;
  client_t *client = calloc(1, sizeof(client_t));
  unsigned int offset = 2; // We want to ignore the type & length data. Each
                           // call to `deserialize_int` will add 6 to this.

  // Deserialize the Socket Field
  client->socket = deserialize_int(buf + offset);
  offset += 6;

  // Deserialize the id
  client->client_id = deserialize_int(buf + offset);
  offset += 6;

  // Deserialize the ADDR Port
  client->addr.sin_port = deserialize_int(buf + offset);
  offset += 6;

  // Deserialize the ADDR Family
  client->addr.sin_family = deserialize_int(buf + offset);
  offset += 6;

  // Deserialize the ADDR Address
  client->addr.sin_addr.s_addr = deserialize_int(buf + offset);
  offset += 6;

  // Deserialize the Client Name
  client->client_name = deserialize_string(buf + offset);

  return client;
}

uint8_t trim_whitespace(char *str) {
  uint8_t spaces_trimmed = 0;
  uint str_len = strlen(str);

  // Trim trailing whitespaces
  while (str_len > 0 && isspace(str[str_len - 1])) {
    str[--str_len] = '\0';
    spaces_trimmed++;
  }

  // Trim preceding whitespaces
  uint start = 0;
  while (isspace(str[start])) {
    start++;
    spaces_trimmed++;
  }

  // Shift remaining characters to the left
  uint i = 0;
  for (int j = start; j < str_len; j++) {
    str[i++] = str[j];
  }
  str[i] = '\0';

  return spaces_trimmed;
}

BOOL is_valid_input_key(char c) {
  // Check if the key is arrows or CTRL+<char> (CTRL+a -> 1, CTRL+b -> 2, CTRL+c
  // -> 3 etc)
  if (c == 27) { // 27 is the ASCII value of the escape character
    return FALSE;
  } else if (c > 0 && c < 26) {
    // Handle CTRL+<char>
    return FALSE;
  }
  return TRUE;
}

int smart_send(int socket, const void *data, int data_length) {
  int len = data_length;
  int ret = send(socket, &len, sizeof(len), 0);
  if (ret < 0)
    return ret;
  return send(socket, data, len, 0);
}

int smart_recv(int socket, const void *buffer, int buffer_size) {
  int len;
  int ret = recv(socket, &len, sizeof(len), 0);
  if (ret < 0)
    return ret;
  if (len > buffer_size)
    return -1;
  return recv(socket, buffer, len, 0);
}

unsigned int hash_string(const char *buf, unsigned int mod) {
  if (buf == NULL)
    return 0;
  unsigned int hash = 0;
  unsigned int len = strlen(buf);
  for (int i = 0; i < len; i++) {
    hash = (hash * 31 + buf[i]) % mod;
  }

  return hash;
}

void *__malloc(size_t size, const char *file, int line) {
  printf("Attempting to allocate memory!\n");
  void *ptr = malloc(size);
  if (ptr == NULL) {
    fprintf(stderr, "\x1b[31;1mFailed to allocate memory at %s:%d\x1b[0m\n",
            file, line);
    exit(1);
  }
  printf("\x1b[32;1mAllocated %zu bytes at %p - %s:%d\x1b[0m\n", size, ptr,
         file, line);
  return ptr;
}
