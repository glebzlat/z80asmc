#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"

#define MAP_INITIAL_CAPACITY 16

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

typedef enum {
  ENTRY_UNINITIALIZED = 0,
  ENTRY_DELETED,
  ENTRY_VALUE,
} MapEntryType;

typedef struct {
  char* key;
  void* value;
  uint64_t hash;
  MapEntryType type;
} MapEntry;

struct Map {
  MapEntry* entries;
  size_t capacity;
  size_t len;
  size_t key_size;
  size_t value_size;
};

static int Map_expand(Map* m);
static int Map_shrink(Map* m);
static void* Map_setEntry(Map* m, void const* key, void const* value);

static int MapEntry_set(MapEntry* e, char const* key, void const* value, size_t value_size, uint64_t hash);
static void MapEntry_del(MapEntry* e);

static uint64_t fnv1a(char const* s);

Map* Map_new(size_t value_size) {
  Map* m = malloc(sizeof(*m));
  if (!m) {
    perror("malloc() failed");
    return NULL;
  }

  m->len = 0;
  m->capacity = MAP_INITIAL_CAPACITY;
  m->value_size = value_size;

  m->entries = calloc(m->capacity, sizeof(MapEntry));
  if (!m->entries) {
    perror("calloc() failed");
    free(m);
    return NULL;
  }

  return m;
}

void Map_destroy(Map* m) {
  assert(m);

  for (size_t i = 0; i < m->capacity; ++i) {
    free(m->entries[i].key);
    free(m->entries[i].value);
  }

  free(m->entries);
  free(m);
}

void* Map_set(Map* m, char const* key, void const* value) {
  assert(m);
  assert(key);
  assert(value);

  if (m->len >= m->capacity / 2)
    if (Map_expand(m) == -1)
      return NULL;

  return Map_setEntry(m, key, value);
}

void* Map_get(Map const* m, char const* key) {
  assert(m);
  assert(key);

  uint64_t hash = fnv1a(key);
  size_t idx = (size_t)(hash & (uint64_t)(m->capacity - 1));

  /* Iterate through entries with deleted values and entries that are currently
   * holding a value. Deleted entry is a special type allowing to traverse
   * through gaps left by Map_del. */
  while (m->entries[idx].type != ENTRY_UNINITIALIZED) {
    if (m->entries[idx].type == ENTRY_VALUE && strcmp(m->entries[idx].key, key) == 0)
      return m->entries[idx].value;

    idx += 1;
    if (idx >= m->capacity)
      idx = 0;
  }

  return NULL;
}

int Map_del(Map* m, char const* key) {
  assert(m);
  assert(key);

  uint64_t hash = fnv1a(key);
  size_t idx = (size_t)(hash & (uint64_t)(m->capacity - 1));

  while (m->entries[idx].type == ENTRY_VALUE) {
    if (strcmp(m->entries[idx].key, key) == 0) {
      MapEntry_del(&m->entries[idx]);
      return 0;
    }
  }

  if (m->len < m->capacity / 2)
    if (Map_shrink(m) == -1)
      return -1;

  return -1;
}

size_t Map_len(Map const* m) {
  assert(m);
  return m->len;
}

MapIter MapIter_init(Map* m) {
  assert(m);
  return (MapIter){.m = m};
}

bool MapIter_next(MapIter* it) {
  assert(it);

  while (it->idx < it->m->capacity) {
    size_t i = it->idx;
    it->idx += 1;
    if (it->m->entries[i].type == ENTRY_VALUE) {
      MapEntry e = it->m->entries[i];
      it->key = e.key;
      it->value = e.value;
      return true;
    }
  }

  return false;
}

static int Map_expand(Map* m) {
  size_t new_capacity = m->capacity * 2;

  m->entries = realloc(m->entries, new_capacity);
  if (!m->entries) {
    perror("realloc() failed");
    return -1;
  }

  return 0;
}

static int Map_shrink(Map* m) {
  size_t new_capacity = m->capacity / 2;
  if (new_capacity < MAP_INITIAL_CAPACITY)
    return 0;

  MapEntry* new_entries = calloc(new_capacity, sizeof(MapEntry));
  if (!new_entries) {
    perror("calloc() failed");
    return -1;
  }

  for (size_t i = 0; i < m->capacity; ++i) {
    if (m->entries[i].type != ENTRY_VALUE)
      continue;

    size_t new_idx = (size_t)(m->entries[i].hash & (uint64_t)(new_capacity - 1));
    while (m->entries[new_idx].type == ENTRY_VALUE) {
      new_idx += 1;

      if (new_idx >= new_capacity)
        new_idx = 0;
    }
    new_entries[new_idx] = m->entries[i];
  }

  free(m->entries);
  m->entries = new_entries;

  return 0;
}

static void* Map_setEntry(Map* m, void const* key, void const* value) {
  uint64_t hash = fnv1a(key);
  size_t idx = (size_t)(hash & (m->capacity - 1));

  while (m->entries[idx].type != ENTRY_UNINITIALIZED) {
    if (m->entries[idx].type == ENTRY_DELETED) {
      if (MapEntry_set(&m->entries[idx], key, value, m->value_size, hash) == -1)
        return NULL;
      break;
    }

    if (m->entries[idx].type == ENTRY_VALUE && strcmp(m->entries[idx].key, key) == 0) {
      free(m->entries[idx].value);
      memcpy(m->entries[idx].value, value, m->value_size);
      break;
    }

    idx += 1;
    if (idx >= m->capacity)
      idx = 0;
  }

  if (MapEntry_set(&m->entries[idx], key, value, m->value_size, hash) == -1)
    return NULL;

  m->len += 1;

  return m->entries[idx].value;
}

static int MapEntry_set(MapEntry* e, char const* key, void const* value, size_t value_size, uint64_t hash) {
  size_t key_len = strlen(key);
  e->key = malloc(key_len + 1);
  if (!e->key) {
    perror("malloc() failed");
    return -1;
  }
  strcpy(e->key, key);
  e->key[key_len] = '\0';

  e->value = malloc(value_size);
  if (!e->value) {
    perror("malloc() failed");
    free(e->key);
    return -1;
  }
  memcpy(e->value, value, value_size);

  e->hash = hash;
  e->type = ENTRY_VALUE;

  return 0;
}

static void MapEntry_del(MapEntry* e) {
  free(e->key);
  free(e->value);
  e->key = NULL;
  e->value = NULL;
  e->type = ENTRY_DELETED;
}

static uint64_t fnv1a(char const* s) {
  uint64_t hash = FNV_OFFSET;
  for (const char* p = s; *p; p++) {
    hash ^= (uint64_t)(unsigned char)(*p);
    hash *= FNV_PRIME;
  }
  return hash;
}
