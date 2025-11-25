#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

struct Vector {
  uint8_t* data;
  size_t el_size;
  size_t len;
  size_t capacity;
};

Vector* Vector_new(size_t elem_size) {
  assert(elem_size != 0);

  Vector* v = malloc(sizeof(*v));
  if (!v) {
    perror("malloc() failed");
    return NULL;
  }

  v->el_size = elem_size;
  v->capacity = 16;
  v->len = 0;
  v->data = malloc(v->el_size * v->capacity);

  return v;
}

void Vector_destroy(Vector* v) {
  assert(v);
  free(v->data);
  free(v);
}

int Vector_push(Vector* v, void const* item) {
  assert(v);
  assert(item);

  assert(v->len <= v->capacity);
  if (v->len == v->capacity)
    if (Vector_resize(v, v->capacity * 2) == -1)
      return -1;

  if (v->len == SIZE_MAX)
    return -1;

  uint8_t* data = v->data + v->len * v->el_size;
  memcpy(data, item, v->el_size);

  v->len += 1;

  return 0;
}

int Vector_pop(Vector* v, void* dest) {
  assert(v);
  assert(dest);

  if (v->len == 0)
    return -1;

  v->len -= 1;
  uint8_t const* last = v->data + v->len * v->el_size;
  memcpy(dest, last, v->el_size);

  if (v->len == v->capacity / 2) {
    if (Vector_resize(v, v->capacity / 2) == -1) {
      return -1;
    }
  }

  return 0;
}

int Vector_pushFront(Vector* v, void const* item) {
  assert(v);
  assert(item);
  assert(v->len <= v->capacity);

  if (v->len == v->capacity) {
    if (Vector_resize(v, v->capacity * 2) == -1) {
      return -1;
    }
  }

  if (v->len == SIZE_MAX)
    return -1;

  for (size_t i = v->len + 1; i != 0; --i) {
    uint8_t* dest = v->data + i * v->el_size;
    uint8_t const* src = v->data + (i - 1) * v->el_size;
    memcpy(dest, src, v->el_size);
  }

  memcpy(v->data, item, v->el_size);

  v->len += 1;

  return 0;
}

int Vector_popFront(Vector* v, void* dest) {
  assert(v);
  assert(dest);

  if (v->len == 0)
    return -1;

  memcpy(dest, v->data, v->el_size);

  for (size_t i = 0; i < v->len - 1; ++i) {
    uint8_t* d = v->data + i * v->el_size;
    uint8_t const* src = v->data + (i + 1) * v->el_size;
    memcpy(d, src, v->el_size);
  }

  v->len -= 1;
  if (v->len == v->capacity / 2) {
    if (Vector_resize(v, v->capacity / 2) == -1) {
      return -1;
    }
  }

  return 0;
}

int Vector_resize(Vector* v, size_t size) {
  assert(v);

  if (size < v->len)
    return -1;

  v->data = realloc(v->data, size * v->el_size);
  if (!v->data) {
    perror("realloc() failed");
    return -1;
  }

  v->capacity = size;

  return 0;
}

void* Vector_at(Vector* v, size_t idx) {
  assert(v);

  if (idx >= v->len)
    return NULL;

  uint8_t* src = v->data + idx * v->el_size;

  return src;
}

size_t Vector_len(Vector const* v) {
  assert(v);
  return v->len;
}

size_t Vector_capacity(Vector const* v) {
  assert(v);
  return v->capacity;
}

bool Vector_isEmpty(Vector const* v) {
  assert(v);
  return v->len == 0;
}
