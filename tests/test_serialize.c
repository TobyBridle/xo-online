#include "../src/lib/utils.h"
#include "generics.h"
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
  char *serialized = serialize_string("Hello World");
  char *deserialized = deserialize_string(serialized);
  EXPECT_EQ(strcmp(deserialized, "Hello World"), 0);
  free(serialized);
  free(deserialized);
  return SUCCESS;
}

TestResult test_client_serialize() {
  client_t *client = malloc(sizeof(client_t));
  memset(client, 0, sizeof(client_t));
  client->socket = 1;
  client->client_id = 2;
  client->client_name = "Toby Bridle";
  struct sockaddr_in addr = {.sin_family = AF_INET,
                             .sin_addr = INADDR_ANY,
                             .sin_len = sizeof(addr),
                             .sin_port = htons(5000)};
  client->addr = addr;
  char *serialized = serialize_client(client);
  client_t *deserialized = deserialize_client(serialized);
  EXPECT_EQ(deserialized->socket, 1);
  EXPECT_EQ(deserialized->client_id, 2);
  EXPECT_EQ(strcmp(deserialized->client_name, "Toby Bridle"), 0);
  EXPECT_EQ(deserialized->addr.sin_family, AF_INET);
  EXPECT_EQ(deserialized->addr.sin_port, htons(5000));

  free(serialized);
  free(deserialized->client_name);
  free(deserialized);
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
  Suite my_suite = new_suite("HashMap Tests", tests);
  run_suite(my_suite);
  return 0;
}
