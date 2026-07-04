#include <stdio.h>
#include <stdlib.h>

#include <map.h>

#include "test_suite.h"

typedef struct {
  int count;
} DummyValue;

static int call_count_DummyValue_destroy = 0;

static void DummyValue_destroy(void* data);

TEST(map_get) {
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  DummyValue v = {0};
  DummyValue* ptr;

  Map_set(m, "key", &v);
  ptr = Map_get(m, "key");

  CHECK_PTR_NEQUAL(ptr, NULL);
  CHECK_INT_EQUAL(Map_getStatus(m), MAP_OK);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(map_del) {
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  DummyValue v = {0};

  Map_set(m, "key", &v);
  int ret = Map_del(m, "key");

  CHECK_INT_EQUAL(ret, 0);
  CHECK_INT_EQUAL(Map_getStatus(m), MAP_OK);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(map_len) {
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  DummyValue v = {0};

  CHECK_UINT_EQUAL(Map_len(m), 0);

  Map_set(m, "k1", &v);
  Map_set(m, "k2", &v);
  Map_set(m, "k3", &v);

  CHECK_UINT_EQUAL(Map_len(m), 3);

  Map_set(m, "k3", &v);

  CHECK_UINT_EQUAL(Map_len(m), 3);

  Map_set(m, "k4", &v);

  CHECK_UINT_EQUAL(Map_len(m), 4);

  Map_del(m, "k1");

  CHECK_UINT_EQUAL(Map_len(m), 3);

  Map_del(m, "k4");

  CHECK_UINT_EQUAL(Map_len(m), 2);

  Map_del(m, "k2");

  CHECK_UINT_EQUAL(Map_len(m), 1);

  Map_del(m, "k3");

  CHECK_UINT_EQUAL(Map_len(m), 0);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(map_stores_values_by_copy) {
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  DummyValue orig = {0};
  DummyValue* ptr;

  Map_set(m, "k1", &orig);
  Map_set(m, "k2", &orig);

  ptr = Map_get(m, "k1");
  ptr->count = 2;

  ptr = Map_get(m, "k2");

  CHECK_INT_EQUAL(ptr->count, 0);
  CHECK_INT_EQUAL(orig.count, 0);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(map_calls_dtor_for_added_entries) {
  call_count_DummyValue_destroy = 0;
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  DummyValue v = {0};

  Map_set(m, "k1", &v);
  Map_set(m, "k2", &v);
  Map_set(m, "k3", &v);
  Map_set(m, "k4", &v);

  Map_destroy(m);

  CHECK_INT_EQUAL(call_count_DummyValue_destroy, 4);

TEST_CLEANUP:
  TEST_NOP;
}

TEST(map_replaces_value_with_same_key) {
  call_count_DummyValue_destroy = 0;
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  DummyValue v1 = {.count = 1};
  DummyValue v2 = {.count = 2};
  DummyValue* ptr;

  Map_set(m, "key", &v1);
  ptr = Map_get(m, "key");

  CHECK_INT_EQUAL(ptr->count, 1);

  Map_set(m, "key", &v2);
  ptr = Map_get(m, "key");

  CHECK_INT_EQUAL(ptr->count, 2);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(dtor_is_called_on_each_deletion) {
  call_count_DummyValue_destroy = 0;
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  DummyValue v = {0};

  Map_set(m, "k1", &v);
  Map_set(m, "k2", &v);
  Map_set(m, "k3", &v);

  Map_del(m, "k1");

  CHECK_INT_EQUAL(call_count_DummyValue_destroy, 1);

  Map_del(m, "k2");

  CHECK_INT_EQUAL(call_count_DummyValue_destroy, 2);

  Map_del(m, "k3");

  CHECK_INT_EQUAL(call_count_DummyValue_destroy, 3);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(map_set_returns_pointer_to_copied_value) {
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  DummyValue v = {.count = 37};
  DummyValue* ptr;

  ptr = Map_set(m, "k", &v);

  CHECK_PTR_NEQUAL(&v, ptr);
  CHECK_INT_EQUAL(v.count, ptr->count);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(map_get_by_nonexisting_key) {
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  DummyValue* ptr = Map_get(m, "key");

  CHECK_PTR_EQUAL(ptr, NULL);
  CHECK_INT_EQUAL(Map_getStatus(m), MAP_NO_SUCH_KEY);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(map_del_by_nonexisting_key) {
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  int ret = Map_del(m, "key");

  CHECK_INT_NEQUAL(ret, 0);
  CHECK_INT_EQUAL(Map_getStatus(m), MAP_NO_SUCH_KEY);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(map_get_before_and_after_replacement) {
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  DummyValue v1 = {.count = 1};
  DummyValue v2 = {.count = 2};
  DummyValue *ptr1, *ptr2;

  Map_set(m, "key", &v1);
  ptr1 = Map_get(m, "key");

  CHECK_INT_EQUAL(ptr1->count, 1);

  Map_set(m, "key", &v2);
  ptr2 = Map_get(m, "key");

  CHECK_PTR_EQUAL(ptr1, ptr2);

  CHECK_INT_EQUAL(ptr2->count, 2);
  CHECK_INT_EQUAL(ptr1->count, 2);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(store_8_entries) {
  const size_t entries = 8;
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);
  char key[6];

  for (size_t i = 0; i < entries; ++i) {
    sprintf(key, "k%lu", i);
    DummyValue v = {.count = (int)i};
    CHECK_PTR_NEQUAL(Map_set(m, key, &v), NULL);
  }

  CHECK_UINT_EQUAL(Map_len(m), entries);

  for (int i = (int)entries - 1; i >= 0; --i) {
    sprintf(key, "k%i", i);
    Map_del(m, key);
  }

  CHECK_UINT_EQUAL(Map_len(m), 0);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(store_16_entries) {
  const size_t entries = 16;
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);
  char key[6];

  for (size_t i = 0; i < entries; ++i) {
    sprintf(key, "k%lu", i);
    DummyValue v = {.count = (int)i};
    CHECK_PTR_NEQUAL(Map_set(m, key, &v), NULL);
  }

  CHECK_UINT_EQUAL(Map_len(m), entries);

  for (int i = (int)entries - 1; i >= 0; --i) {
    sprintf(key, "k%i", i);
    Map_del(m, key);
  }

  CHECK_UINT_EQUAL(Map_len(m), 0);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(store_2000_entries) {
  const size_t entries = 2000;
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);
  char key[6];

  for (size_t i = 0; i < entries; ++i) {
    sprintf(key, "k%lu", i);
    DummyValue v = {.count = (int)i};
    CHECK_PTR_NEQUAL(Map_set(m, key, &v), NULL);
  }

  CHECK_UINT_EQUAL(Map_len(m), entries);

  for (int i = (int)entries - 1; i >= 0; --i) {
    sprintf(key, "k%i", i);
    Map_del(m, key);
  }

  CHECK_UINT_EQUAL(Map_len(m), 0);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(map_iterator) {
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  DummyValue v = {0};
  bool continue_iter = false;

  Map_set(m, "k1", &v);
  Map_set(m, "k2", &v);
  Map_set(m, "k3", &v);
  Map_set(m, "k4", &v);

  MapIter iter = MapIter_init(m);

  continue_iter = MapIter_next(&iter);
  CHECK_BOOL_EQUAL(continue_iter, true);
  continue_iter = MapIter_next(&iter);
  CHECK_BOOL_EQUAL(continue_iter, true);
  continue_iter = MapIter_next(&iter);
  CHECK_BOOL_EQUAL(continue_iter, true);
  continue_iter = MapIter_next(&iter);
  CHECK_BOOL_EQUAL(continue_iter, true);
  continue_iter = MapIter_next(&iter);
  CHECK_BOOL_EQUAL(continue_iter, false);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(map_is_unordered) {
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  DummyValue v1 = {0};
  DummyValue v2 = {1};
  DummyValue v3 = {2};
  DummyValue v4 = {3};
  DummyValue* ptr;
  char const* str;

  Map_set(m, "k1", &v1);
  Map_set(m, "k2", &v2);
  Map_set(m, "k3", &v3);
  Map_set(m, "k4", &v4);

  MapIter iter = MapIter_init(m);

  MapIter_next(&iter);
  str = MapIter_getKey(&iter);
  ptr = MapIter_getValue(&iter);

  CHECK_STR_EQUAL(str, "k1");
  CHECK_INT_EQUAL(ptr->count, 0);

  MapIter_next(&iter);
  str = MapIter_getKey(&iter);
  ptr = MapIter_getValue(&iter);

  CHECK_STR_EQUAL(str, "k2");
  CHECK_INT_EQUAL(ptr->count, 1);

  MapIter_next(&iter);
  str = MapIter_getKey(&iter);
  ptr = MapIter_getValue(&iter);

  CHECK_STR_EQUAL(str, "k4");
  CHECK_INT_EQUAL(ptr->count, 3);

  MapIter_next(&iter);
  str = MapIter_getKey(&iter);
  ptr = MapIter_getValue(&iter);

  CHECK_STR_EQUAL(str, "k3");
  CHECK_INT_EQUAL(ptr->count, 2);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(map_iterator_returns_null_when_exhausted) {
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  DummyValue v1 = {1};
  char const* str;
  DummyValue* ptr;

  Map_set(m, "k1", &v1);

  MapIter iter = MapIter_init(m);

  CHECK_BOOL_EQUAL(MapIter_next(&iter), true);
  str = MapIter_getKey(&iter);
  ptr = MapIter_getValue(&iter);

  CHECK_STR_EQUAL(str, "k1");
  CHECK_INT_EQUAL(ptr->count, 1);

  CHECK_BOOL_EQUAL(MapIter_next(&iter), false);
  str = MapIter_getKey(&iter);
  ptr = MapIter_getValue(&iter);

  CHECK_PTR_EQUAL(str, NULL);
  CHECK_PTR_EQUAL(ptr, NULL);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(map_iterator_reflects_map_changes_add_entries) {
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);
  MapIter iter = MapIter_init(m);

  DummyValue v1 = {0};
  DummyValue v2 = {1};
  DummyValue v3 = {2};
  DummyValue v4 = {3};

  Map_set(m, "k1", &v1);
  Map_set(m, "k2", &v2);
  Map_set(m, "k3", &v3);
  Map_set(m, "k4", &v4);

  int i = 0;
  while (MapIter_next(&iter)) {
    i++;
  }
  CHECK_INT_EQUAL(i, 4);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(map_iterator_reflects_map_changes_add_entries_mid_iteration) {
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);
  MapIter iter = MapIter_init(m);

  DummyValue v1 = {0};
  DummyValue v2 = {1};
  DummyValue v3 = {2};
  DummyValue v4 = {3};

  Map_set(m, "k1", &v1);

  CHECK_BOOL_EQUAL(MapIter_next(&iter), true);

  Map_set(m, "k2", &v2);

  CHECK_BOOL_EQUAL(MapIter_next(&iter), true);

  Map_set(m, "k3", &v3);
  Map_set(m, "k4", &v4);

  CHECK_BOOL_EQUAL(MapIter_next(&iter), true);
  CHECK_BOOL_EQUAL(MapIter_next(&iter), true);
  CHECK_BOOL_EQUAL(MapIter_next(&iter), false);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(map_iterator_with_some_entries_deleted) {
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  DummyValue v1 = {0};
  DummyValue v2 = {1};
  DummyValue v3 = {2};
  DummyValue v4 = {3};
  DummyValue v5 = {4};
  char const* str;

  Map_set(m, "k1", &v1);
  Map_set(m, "k2", &v2);
  Map_set(m, "k3", &v3);
  Map_set(m, "k4", &v4);
  Map_set(m, "k5", &v5);

  Map_del(m, "k2");
  Map_del(m, "k3");
  Map_del(m, "k4");

  MapIter iter = MapIter_init(m);

  CHECK_BOOL_EQUAL(MapIter_next(&iter), true);
  str = MapIter_getKey(&iter);
  CHECK_STR_EQUAL(str, "k1");

  CHECK_BOOL_EQUAL(MapIter_next(&iter), true);
  str = MapIter_getKey(&iter);
  CHECK_STR_EQUAL(str, "k5");

  CHECK_BOOL_EQUAL(MapIter_next(&iter), false);

TEST_CLEANUP:
  Map_destroy(m);
}

TEST(map_iterator_entries_added_after_iteration_end) {
  Map* m = Map_new(sizeof(DummyValue), DummyValue_destroy);

  DummyValue v1 = {0};
  DummyValue v2 = {1};

  MapIter iter = MapIter_init(m);

  Map_set(m, "k1", &v1);

  CHECK_BOOL_EQUAL(MapIter_next(&iter), true);
  CHECK_BOOL_EQUAL(MapIter_next(&iter), false);

  Map_set(m, "k2", &v2);

  CHECK_BOOL_EQUAL(MapIter_next(&iter), true);
  CHECK_BOOL_EQUAL(MapIter_next(&iter), false);

TEST_CLEANUP:
  Map_destroy(m);
}

int main(void) {
  TestSuite ts = {0};

  TC(ts, map_get);
  TC(ts, map_del);
  TC(ts, map_len);
  TC(ts, map_stores_values_by_copy);
  TC(ts, map_calls_dtor_for_added_entries);
  TC(ts, map_replaces_value_with_same_key);
  TC(ts, dtor_is_called_on_each_deletion);
  TC(ts, map_set_returns_pointer_to_copied_value);
  TC(ts, map_get_by_nonexisting_key);
  TC(ts, map_del_by_nonexisting_key);
  TC(ts, map_get_before_and_after_replacement);
  TC(ts, store_8_entries);
  TC(ts, store_16_entries);
  TC(ts, store_2000_entries);
  TC(ts, map_iterator);
  TC(ts, map_is_unordered);
  TC(ts, map_iterator_returns_null_when_exhausted);
  TC(ts, map_iterator_reflects_map_changes_add_entries);
  TC(ts, map_iterator_reflects_map_changes_add_entries_mid_iteration);
  TC(ts, map_iterator_with_some_entries_deleted);
  TC(ts, map_iterator_entries_added_after_iteration_end);

  return ts.tests_failed;
}

static void DummyValue_destroy(void* data) { call_count_DummyValue_destroy += 1; }
