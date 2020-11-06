#ifndef SENEGAL_SGCOBJECT_UTILS_H
#define SENEGAL_SGCOBJECT_UTILS_H

#include "sconstant.h"
#include "svm.h"


GCObject* allocateGCObject(VM* vm, size_t size, GCObjectType type);

#define ALLOCATE_GC_OBJ(vm, type, gcType) \
    (type*)allocateGCObject(vm, sizeof(type), gcType)

#endif //SENEGAL_SGCOBJECT_UTILS_H