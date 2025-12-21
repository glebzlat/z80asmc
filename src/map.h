#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct Map Map;
typedef struct MapIter MapIter;

typedef void(*Map_value_destructor_fn)(void*);

struct MapIter {
  Map* m;
  char const* key;
  void* value;
  size_t idx;
};

Map* Map_new(size_t value_size, Map_value_destructor_fn dtor);
void Map_destroy(Map* m);

void* Map_set(Map* m, char const* key, void* value);
void* Map_setCopy(Map* m, char const* key, void const* value);
void* Map_get(Map const* m, char const* key);
int Map_del(Map* m, char const* key);
size_t Map_len(Map const* m);

MapIter MapIter_init(Map* m);
bool MapIter_next(MapIter* it);

#endif // MAP_H

