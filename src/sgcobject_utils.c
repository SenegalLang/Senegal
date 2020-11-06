#include <stdio.h>
#include "includes/sgcobject_utils.h"
#include "includes/smemory.h"

GCObject* allocateGCObject(VM* vm, size_t size, GCObjectType type) {
  GCObject* gc = (GCObject*)reallocate(vm, NULL, NULL, 0, size);
  gc->type = type;
  gc->next= vm->gcObjects;
  gc->isMarked = false;

  vm->gcObjects = gc;

#if DEBUG_LOG_GC
  printf("%p allocate %ld for %d\n", (void*)gc, size, type);
#endif

  return gc;
}