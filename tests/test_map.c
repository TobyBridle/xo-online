#include "../src/lib/utils.h"
#include "generics.h"
#include <stdio.h>

TestResult test_types() {
  HashMap map = new_hashmap(3);
  put(&map, 1, (BucketValue){.i_value = 3});
  put(&map, 3, (BucketValue){.c_value = 'H'});

  BucketValue ret[] = {get(map, 1), get(map, 3)};
  if (ret[0].i_value != 3 || ret[1].c_value != 'H') {
    free_hashmap(&map);
    return FAILURE;
  }
  free_hashmap(&map);
  return SUCCESS;
}

TestResult test_collisions() {
  HashMap map = new_hashmap(2);
  // NOTE: This is a collision where we also have a node with different type!
  put(&map, 5, (BucketValue){.i_value = 5});
  put(&map, 9, (BucketValue){.i_value = 9});
  put(&map, 11, (BucketValue){.c_value = 72});
  int first = get(map, 5).i_value;
  int second = get(map, 9).i_value;
  int third = get(map, 11).c_value;

  if (first != 5 || second != 9 || (char)third != 'H') {
    printf("%d %d %d\n", first, second, (char)third);
    free_hashmap(&map);
    return FAILURE;
  }
  free_hashmap(&map);
  return SUCCESS;
}

TestResult test_remove_without_collisions() {
  HashMap map = new_hashmap(5);
  put(&map, 1, (BucketValue){.i_value = 1});
  put(&map, 2, (BucketValue){.i_value = 2});
  put(&map, 3, (BucketValue){.i_value = 3});
  put(&map, 4, (BucketValue){.i_value = 4});
  put(&map, 5, (BucketValue){.i_value = 5});
  remove_value(map, 3);

  EXPECT_EQ(get(map, 3).err, -1);
  EXPECT_EQ(get(map, 5).i_value, 5);

  remove_value(map, 1);

  EXPECT_EQ(get(map, 1).err, -1);
  EXPECT_EQ(get(map, 6).err, -1);

  free_hashmap(&map);
  return SUCCESS;
}

TestResult test_remove_with_collisions() {
  HashMap map = new_hashmap(5);
  put(&map, 1, (BucketValue){.i_value = 1});
  put(&map, 2, (BucketValue){.i_value = 2});
  put(&map, 3, (BucketValue){.i_value = 3});
  put(&map, 4, (BucketValue){.i_value = 4});
  put(&map, 5, (BucketValue){.i_value = 5});
  put(&map, 6, (BucketValue){.i_value = 6});
  remove_value(map, 3);

  EXPECT_EQ(get(map, 3).err, -1);
  EXPECT_EQ(get(map, 5).i_value, 5);

  remove_value(map, 1);

  EXPECT_EQ(get(map, 1).err, -1);
  EXPECT_EQ(get(map, 6).i_value, 6);

  free_hashmap(&map);
  return SUCCESS;
}

int main() {
  Test *tests = (Test[]){
      new_test("Types", &test_types),
      new_test("Buckets == Elements", &test_collisions),
      new_test("Removing Values (NO COLLISIONS)",
               &test_remove_without_collisions),
      new_test("Removing Values (COLLISIONS)", &test_remove_with_collisions),
  };
  Suite my_suite = new_suite("HashMap Tests", tests);
  run_suite(my_suite);
  return 0;
}
