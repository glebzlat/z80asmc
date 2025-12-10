#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct Map Map;
typedef struct MapIter MapIter;

struct MapIter {
  Map* m;
  char* key;
  void* value;
  size_t idx;
};

Map* Map_new(size_t value_size);
void Map_destroy(Map* m);

void* Map_set(Map* m, char const* key, void const* value);
void* Map_setCopy(Map* m, char const* key, void const* value);
void* Map_get(Map const* m, char const* key);
int Map_del(Map* m, char const* key);
size_t Map_len(Map const* m);

MapIter MapIter_init(Map* m);
bool MapIter_next(MapIter* it);

#endif // MAP_H

