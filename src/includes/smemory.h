#ifndef SENEGAL_SMEMORY_H
#define SENEGAL_SMEMORY_H

#include "sutils.h"
#include "sparser.h"

#define ALLOCATE(vm, compiler, type, count) \
    (type*)reallocate(vm, compiler, NULL, 0, sizeof(type) * (count))

#define GROW_CAP(cap) \
  ((cap) < 8 ? 8 : (cap) * 2)

#define FREE(vm, compiler, type, pointer) reallocate(vm, compiler, pointer, sizeof(type), 0)

#define GROW_ARRAY(vm, compiler, type, pointer, oldCount, newCount) \
  (type*)reallocate(vm, compiler, pointer, sizeof(type) * (oldCount), sizeof(type) * (newCount))

#define FREE_ARRAY(vm, compiler, type, pointer, oldCount) \
  reallocate(vm, compiler, pointer, sizeof(type) * (oldCount), 0)

void* reallocate(VM* vm, Compiler* compiler, void* pointer, size_t oldSize, size_t newSize);

void freeVM(VM* vm);

void markGCObject(VM* vm, GCObject* gc);
void markConstant(VM* vm, Constant constant);

void collectGarbage(VM* vm, Compiler* compiler);

#endif //SENEGAL_SMEMORY_H