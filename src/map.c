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
  Map_value_destructor_fn dtor;
  size_t capacity;
  size_t len;
  size_t key_size;
  size_t value_size;
  MapStatus status;
};

static int Map_expand(Map* m);
static int Map_shrink(Map* m);
static void* Map_setEntry(Map* m, void const* key, void* value);

static int MapEntry_set(MapEntry* e, char const* key, void* value, size_t value_size, uint64_t hash);
static void MapEntry_del(MapEntry* e, Map_value_destructor_fn dtor);

static uint64_t fnv1a(char const* s);

Map* Map_new(size_t value_size, Map_value_destructor_fn dtor) {
  assert(dtor);

  Map* m = malloc(sizeof(*m));
  if (!m) {
    return NULL;
  }

  m->len = 0;
  m->capacity = MAP_INITIAL_CAPACITY;
  m->value_size = value_size;
  m->dtor = dtor;
  m->status = MAP_OK;

  m->entries = calloc(m->capacity, sizeof(MapEntry));
  if (!m->entries) {
    free(m);
    return NULL;
  }

  return m;
}

void Map_destroy(Map* m) {
  assert(m);

  for (size_t i = 0; i < m->capacity; ++i) {
    if (m->entries[i].type == ENTRY_VALUE) {
      MapEntry_del(&m->entries[i], m->dtor);
    }
  }

  free(m->entries);
  free(m);
}

void* Map_set(Map* m, char const* key, void* value) {
  assert(m);
  assert(key);
  assert(value);

  if (m->len >= m->capacity / 2) {
    if (Map_expand(m) == -1) {
      return NULL;
    }
  }

  m->status = MAP_OK;
  return Map_setEntry(m, key, value);
}

void* Map_get(Map* m, char const* key) {
  assert(m);
  assert(key);

  uint64_t hash = fnv1a(key);
  size_t idx = (size_t)(hash & (uint64_t)(m->capacity - 1));

  /* Iterate through entries with deleted values and entries that are currently
   * holding a value. Deleted entry is a special type allowing to traverse
   * through gaps left by Map_del. */
  while (m->entries[idx].type != ENTRY_UNINITIALIZED) {
    if (m->entries[idx].type == ENTRY_VALUE && strcmp(m->entries[idx].key, key) == 0) {
      m->status = MAP_OK;
      return m->entries[idx].value;
    }

    idx = (idx + 1) % m->capacity;
  }

  m->status = MAP_NO_SUCH_KEY;
  return NULL;
}

int Map_del(Map* m, char const* key) {
  assert(m);
  assert(key);

  int ret = -1;
  uint64_t hash = fnv1a(key);
  size_t idx = (size_t)(hash & (uint64_t)(m->capacity - 1));

  while (m->entries[idx].type != ENTRY_UNINITIALIZED) {
    if (m->entries[idx].type == ENTRY_VALUE && strcmp(m->entries[idx].key, key) == 0) {
      MapEntry_del(&m->entries[idx], m->dtor);
      m->len -= 1;
      m->status = MAP_OK;
      ret = 0;
      break;
    }

    idx = (idx + 1) % m->capacity;
  }

  if (ret == -1)
    m->status = MAP_NO_SUCH_KEY;

  if (m->len < m->capacity / 2)
    if (Map_shrink(m) == -1)
      return -1;

  return ret;
}

size_t Map_len(Map const* m) {
  assert(m);
  return m->len;
}

MapStatus Map_getStatus(Map* m) {
  assert(m);
  return m->status;
}

MapIter MapIter_init(Map* m) {
  assert(m);
  return (MapIter){.m = m};
}

bool MapIter_next(MapIter* it) {
  assert(it);

  size_t start_idx = it->idx;
  MapEntry entry;
  while (it->idx < it->m->capacity) {
    entry = it->m->entries[it->idx];
    it->idx += 1;
    if (entry.type == ENTRY_VALUE) {
      it->key = entry.key;
      it->value = entry.value;
      return true;
    }
  }

  it->idx = start_idx;
  it->key = NULL;
  it->value = NULL;

  return false;
}

void* MapIter_getValue(MapIter* it) {
  assert(it);
  return it->value;
}

char const* MapIter_getKey(MapIter* it) {
  assert(it);
  return it->key;
}

static int Map_expand(Map* m) {
  size_t new_capacity = m->capacity * 2;

  MapEntry* entries = realloc(m->entries, new_capacity * sizeof(MapEntry));
  if (!entries) {
    m->status = MAP_ERROR;
    return -1;
  }

  memset(entries + m->capacity, 0, (new_capacity - m->capacity) * sizeof(MapEntry));

  m->capacity = new_capacity;
  m->entries = entries;

  return 0;
}

static int Map_shrink(Map* m) {
  size_t new_capacity = m->capacity / 2;
  if (new_capacity < MAP_INITIAL_CAPACITY)
    return 0;

  MapEntry* new_entries = calloc(new_capacity, sizeof(MapEntry));
  if (!new_entries) {
    m->status = MAP_ERROR;
    return -1;
  }

  for (size_t i = 0; i < m->capacity; ++i) {
    if (m->entries[i].type != ENTRY_VALUE)
      continue;

    size_t new_idx = (size_t)(m->entries[i].hash & (uint64_t)(new_capacity - 1));
    while (new_entries[new_idx].type == ENTRY_VALUE) {
      new_idx = (new_idx + 1) % new_capacity;
    }
    new_entries[new_idx] = m->entries[i];
  }

  free(m->entries);
  m->capacity = new_capacity;
  m->entries = new_entries;

  return 0;
}

static void* Map_setEntry(Map* m, void const* key, void* value) {
  uint64_t hash = fnv1a(key);
  size_t idx = (size_t)(hash & (uint64_t)(m->capacity - 1));
  bool init = true;

  while (m->entries[idx].type != ENTRY_UNINITIALIZED) {

    /* Reuse deleted entry */
    if (m->entries[idx].type == ENTRY_DELETED) {
      if (MapEntry_set(&m->entries[idx], key, value, m->value_size, hash)) {
        m->status = MAP_ERROR;
        return NULL;
      }
      break;
    }

    /* Replace the value of an existing entry */
    if (m->entries[idx].type == ENTRY_VALUE && strcmp(m->entries[idx].key, key) == 0) {
      m->dtor(m->entries[idx].value);
      memcpy(m->entries[idx].value, value, m->value_size);
      init = false;
      break;
    }

    idx = (idx + 1) % m->capacity;
  }

  /* Initialize a new entry */
  if (init) {
    if (MapEntry_set(&m->entries[idx], key, value, m->value_size, hash)) {
      m->status = MAP_ERROR;
      return NULL;
    }
    m->len += 1;
  }

  return m->entries[idx].value;
}

static int MapEntry_set(MapEntry* e, char const* key, void* value, size_t value_size, uint64_t hash) {
  char* tmp = malloc(value_size);
  if (!tmp) {
    return -1;
  }
  memcpy(tmp, value, value_size);

  char* str = malloc(strlen(key) + 1);
  if (!str) {
    return -1;
  }
  strcpy(str, key);

  e->key = str;
  e->value = tmp;
  e->hash = hash;
  e->type = ENTRY_VALUE;
  return 0;
}

static void MapEntry_del(MapEntry* e, Map_value_destructor_fn dtor) {
  dtor(e->value);
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
