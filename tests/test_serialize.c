#include "../src/lib/utils.h"
#include "generics.h"
#include <arpa/inet.h>
#include <string.h>

TestResult test_int_serialize() {
  char *serialized = serialize_int(5);
  char *serialized1 = serialize_int(-100);
  EXPECT_EQ(deserialize_int(serialized), 5);
  EXPECT_EQ(deserialize_int(serialized1), -100);
  free(serialized);
  free(serialized1);
  return SUCCESS;
}

TestResult test_bool_serialize() {
  char *serialized = serialize_bool(TRUE);
  char *serialized1 = serialize_bool(FALSE);
  EXPECT_EQ(deserialize_bool(serialized), TRUE);
  EXPECT_EQ(deserialize_bool(serialized1), FALSE);
  free(serialized);
  free(serialized1);
  return SUCCESS;
}

TestResult test_string_serialize() {
  serialized_string serialized = serialize_string("Hello World");
  char *deserialized = deserialize_string(serialized.str);
  EXPECT_EQ(strcmp(deserialized, "Hello World"), 0);
  free(serialized.str);
  free(deserialized);
  return SUCCESS;
}

TestResult test_client_serialize() {
  client_t *client = malloc(sizeof(client_t));
  memset(client, 0, sizeof(client_t));
  client->socket = 1;
  client->client_id = 2;
  client->client_name = "Toby Bridle";
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(5000);
  if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
    fprintf(stderr, "Could not initialise address!\n");
    exit(EXIT_FAILURE);
  }
  client->addr = addr;
  char *serialized = serialize_client(client);
  client_t *deserialized = deserialize_client(serialized);
  EXPECT_EQ(deserialized->socket, 1);
  EXPECT_EQ(deserialized->client_id, 2);
  EXPECT_EQ(strcmp(deserialized->client_name, "Toby Bridle"), 0);
  EXPECT_EQ(deserialized->addr.sin_family, AF_INET);
  EXPECT_EQ(deserialized->addr.sin_port, htons(5000));
  EXPECT_EQ(deserialized->addr.sin_addr.s_addr, client->addr.sin_addr.s_addr)

  free(deserialized->client_name);
  free(deserialized);
  free(serialized);
  free(client);
  return SUCCESS;
}

int main() {
  Test *tests = (Test[]){
      new_test("Integer Serialization", &test_int_serialize),
      new_test("Boolean Serialization", &test_bool_serialize),
      new_test("String Serialization", &test_string_serialize),
      new_test("Client Serialization", &test_client_serialize),
  };
  Suite my_suite = new_suite("Serialization Tests", tests, 4);
  run_suite(my_suite);
  return 0;
}
