#ifndef CLOXEXP_MEMORY_H
#define CLOXEXP_MEMORY_H
#include "common.h"
#include "object.h"
#define GROW_CAPACITY(capacity) capacity < 8 ? 16 : capacity*2
#define GROW_ARRAY(type, prev, oldCapacity, newCapacity) (type*)reallocate(prev, oldCapacity, newCapacity)
#define FREE_ARRAY(type, prev, oldCapacity) reallocate(prev, sizeof(type)*(oldCapacity), 0)
#define GROW_LINEARRAY_S(prev, oldCapacity, newCapacity) growLineArray(prev, oldCapacity, newCapacity)
#define ALLOCATE(type, length) reallocate(NULL, 0, sizeof(type) * length)
#define FREE(objType, obj) reallocate(obj, sizeof(objType), 0)

void* reallocate(void* prev, size_t oldC, size_t newC);
void* growLineArray(int* prev, size_t oldC, size_t newC);
void freeObjects();
void freeObj(Obj* obj);

#endif
