#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stddef.h>

struct Vector;
typedef struct Vector Vector;

Vector* Vector_new(size_t elem_size);
void Vector_destroy(Vector* v);

int Vector_push(Vector* v, void const* item);
void* Vector_pop(Vector* v);
int Vector_pushFront(Vector* v, void const* item);
void* Vector_popFront(Vector* v);
int Vector_resize(Vector* v, size_t size);

void* Vector_at(Vector* v, size_t idx);

size_t Vector_len(Vector const* v);
size_t Vector_capacity(Vector const* v);
bool Vector_isEmpty(Vector const* v);

#endif
