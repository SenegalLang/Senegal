#include <stdio.h>
#include "includes/sgcobject_utils.h"
#include "includes/smemory.h"
#include "../core/includes/sapi.h"

void resetStack(GCCoroutine* coroutine) {
  coroutine->stackTop = coroutine->stack;
  coroutine->frameCount = 0;
  coroutine->openUpvalues = NULL;
}

GCObject* allocateGCObject(VM* vm, size_t size, GCObjectType type) {
  GCObject* gc = (GCObject*)reallocate(vm, NULL, NULL, 0, size);
  gc->type = type;
  gc->next = vm->gcObjects;
  gc->isMarked = false;

  vm->gcObjects = gc;

#if DEBUG_LOG_GC
  printf("%p allocate %ld for %d\n", (void*)gc, size, type);
#endif

  return gc;
}

GCClass* newClass(VM *vm, GCString *id, bool isFinal) {
  GCClass* class = ALLOCATE_GC_OBJ(vm, GCClass, GC_CLASS);
  class->id = id;
  class->isFinal = isFinal;

  initTable(&class->methods);
  initTable(&class->fields);
  initTable(&class->staticMethods);
  initTable(&class->staticFields);

  // Define type for class
  defineClassNativeField(vm, "type", GC_OBJ_CONST(id), class);

  return class;
}

GCClosure* newClosure(VM *vm, GCFunction *function) {
  GCUpvalue** upvalues = ALLOCATE(vm, NULL, GCUpvalue*, function->upvalueCount);

  for (int i = 0; i < function->upvalueCount; i++) {
    upvalues[i] = NULL;
  }

  GCClosure* closure = ALLOCATE_GC_OBJ(vm, GCClosure, GC_CLOSURE);
  closure->function = function;

  closure->upvalues = upvalues;
  closure->upvalueCount = function->upvalueCount;

  return closure;
}

GCCoroutine* newCoroutine(VM* vm, CoroutineState state, GCClosure* closure) {
  GCCoroutine* coroutine = ALLOCATE_GC_OBJ(vm, GCCoroutine, GC_COROUTINE);
  coroutine->state = state;
  coroutine->caller = NULL;
  coroutine->error = NULL;
  resetStack(coroutine);

  if (closure) {
    CallFrame* frame = &coroutine->frames[coroutine->frameCount++];
    frame->closure = closure;
    frame->pc = closure->function->instructions.bytes;
    push(vm, GC_OBJ_CONST(closure));
  }

  return coroutine;
}

GCFunction* newFunction(VM* vm) {
  GCFunction* function = ALLOCATE_GC_OBJ(vm, GCFunction, GC_FUNCTION);
  function->arity = 0;
  function->upvalueCount = 0;
  function->id = NULL;
  initInstructions(&function->instructions);
  return function;
}

GCInstance* newInstance(VM* vm, GCClass* class) {
  GCInstance* instance = ALLOCATE_GC_OBJ(vm, GCInstance, GC_INSTANCE);

  instance->class = class;

  return instance;
}

GCInstanceMethod* newInstanceMethod(VM* vm, Constant receiver, GCClosure* method) {
  GCInstanceMethod* im = ALLOCATE_GC_OBJ(vm, GCInstanceMethod, GC_INSTANCE_METHOD);
  im->receiver = receiver;
  im->method = method;
  return im;
}

GCUpvalue* newUpvalue(VM *vm, Constant *constant) {
  GCUpvalue* upvalue = ALLOCATE_GC_OBJ(vm, GCUpvalue, GC_UPVALUE);
  upvalue->place = constant;
  upvalue->closed = NULL_CONST;
  upvalue->next = NULL;

  return upvalue;
}

GCList* newList(VM *vm, int length) {
  GCList* list = ALLOCATE_GC_OBJ(vm, GCList, GC_LIST);

  list->elements = ALLOCATE(vm, NULL, Constant, length);
  list->elementC = 0;

  list->listCurrentCap = 0;
  GROW_CAP(list->listCurrentCap);

  return list;
}

GCMap* newMap(VM *vm) {
  GCMap* map = ALLOCATE_GC_OBJ(vm, GCMap, GC_MAP);
  initTable(&map->table);

  return map;
}


