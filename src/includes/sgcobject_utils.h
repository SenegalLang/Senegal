#ifndef SENEGAL_SGCOBJECT_UTILS_H
#define SENEGAL_SGCOBJECT_UTILS_H

#include "sconstant.h"
#include "svm.h"

GCObject* allocateGCObject(VM* vm, size_t size, GCObjectType type);

#define ALLOCATE_GC_OBJ(vm, type, gcType) \
    (type*)allocateGCObject(vm, sizeof(type), gcType)

void resetStack(GCCoroutine* coroutine);

GCClass* newClass(VM* vm, GCString* id, bool isFinal);
GCClosure* newClosure(VM* vm, GCFunction* function);
GCCoroutine* newCoroutine(VM* vm, CoroutineState state, GCClosure* closure);
GCFunction* newFunction(VM* vm);
GCInstance* newInstance(VM* vm, GCClass* class);
GCInstanceMethod* newInstanceMethod(VM* vm, Constant receiver, GCClosure* method);
GCList* newList(VM *vm, int length);
GCMap* newMap(VM *vm);
GCUpvalue* newUpvalue(VM* vm, Constant* constant);

#endif //SENEGAL_SGCOBJECT_UTILS_H