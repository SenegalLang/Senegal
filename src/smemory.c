#include <stdlib.h>

#include "includes/smemory.h"
#include "includes/stable_utils.h"
#include "includes/scompiler.h"
#include "includes/sinstruction_utils.h"

#if DEBUG_LOG_GC
#include "stdio.h"
#include "includes/sdebug.h"
#include "includes/stable_utils.h"
#include "includes/scompiler.h"
#include "includes/sinstruction_utils.h"

#endif

#define GC_GROW_FACTOR 2

void* reallocate(VM* vm, Compiler* compiler, void* pointer, size_t oldSize, size_t newSize) {
  vm->bytesAllocated += newSize - oldSize;

  if (newSize > oldSize && vm->bytesAllocated > vm->nextGC) {
    collectGarbage(vm, compiler);
  }

  if (newSize == 0) {
    free(pointer);
    return NULL;
  }

  void* result = realloc(pointer, newSize);

  if (result == NULL) {
    exit(1);
  }

  return result;
}

static void freeGCObject(VM* vm, Compiler* compiler, GCObject* gc) {
#if DEBUG_LOG_GC
  printf("%p freeing %d\n", (void*)gc, gc->type);
#endif

  switch (gc->type) {

    case GC_CLASS: {
      GCClass* class = (GCClass*)gc;
      freeTable(vm, &class->methods);
      FREE(vm, compiler, GCClass, gc);
      break;
    }

    case GC_CLOSURE: {
      GCClosure* closure = (GCClosure*)gc;
      FREE_ARRAY(vm, compiler, GCUpvalue*, closure->upvalues, closure->upvalueCount);
      FREE(vm, compiler, GCClosure, gc);
      break;
    }

    case GC_FUNCTION: {
      GCFunction* function = (GCFunction*)gc;
      freeInstructions(vm, &function->instructions);
      FREE(vm, compiler, GCFunction, gc);
      break;
    }

    case GC_INSTANCE: {
      GCInstance* instance = (GCInstance*)gc;
      freeTable(vm, &instance->fields);
      FREE(vm, compiler, GCInstance, gc);
      break;
    }

    case GC_INSTANCE_METHOD: {
      FREE(vm, compiler, GCInstanceMethod, gc);
      break;
    }

    case GC_NATIVE:
      FREE(vm, compiler, GCNative, gc);
      break;

    case GC_STRING: {
      GCString *string = (GCString *) gc;
      FREE_ARRAY(vm, compiler, char, string->chars, string->length);
      FREE(vm, compiler, GCString, gc);
      break;
    }

    case GC_UPVALUE:
      FREE(vm, compiler, GCUpvalue, gc);
      break;
  }
}

static void freeGCObjects(VM *vm, Compiler* compiler) {
  GCObject* gc = vm->gcObjects;

  while (gc != NULL) {
    GCObject* next = gc->next;
    freeGCObject(vm, compiler, gc);
    gc = next;
  }

  free(vm->grayStack);
}

void freeVM(VM* vm) {
  freeTable(vm, &vm->globals);
  freeTable(vm, &vm->strings);

  vm->constructString = NULL;

  freeGCObjects(vm, NULL);
}

void markGCObject(VM* vm, GCObject* gc) {
  if (gc == NULL || gc->isMarked
      || gc->type == GC_STRING || gc->type == GC_NATIVE)
    return;

#if DEBUG_LOG_GC
  printf("%p mark ", (void*)gc);
  printConstant(GC_OBJ_CONST(gc));
  printf("\n");
#endif

  gc->isMarked = true;

  if (vm->grayCapacity <= vm->grayCount) {
    vm->grayCapacity = GROW_CAP(vm->grayCapacity);
    vm->grayStack = realloc(vm->grayStack, sizeof(GCObject*) * vm->grayCapacity);

    if (vm->grayStack == NULL)
      exit(1);
  }

  vm->grayStack[vm->grayCount++] = gc;
}

void markConstant(VM* vm, Constant constant) {
  if (!IS_GC_OBJ(constant))
    return;

  markGCObject(vm, AS_GC_OBJ(constant));
}

static void markRoots(VM* vm, Compiler* compiler) {
  for (Constant* constant = vm->stack; constant < vm->stackTop; constant++ ) {
    markConstant(vm, *constant);
  }

  for (int i = 0; i < vm->frameCount; i++) {
    markGCObject(vm, (GCObject*)vm->frames[i].closure);
  }

  for (GCUpvalue* upvalue = vm->openUpvalues; upvalue != NULL; upvalue = upvalue->next) {
    markGCObject(vm, (GCObject*)upvalue);
  }

  markTable(vm, &vm->globals);
  markCompilerRoots(vm, compiler);
  markGCObject(vm, (GCObject*)vm->constructString);
  markGCObject(vm, (GCObject*)vm->boolClass);
}

static void markArray(VM* vm, ConstantPool* cp) {
  for (int i = 0; i < cp->count; i++) {
    markConstant(vm, cp->constants[i]);
  }
}

static void blackenGCObject(VM* vm, GCObject* gc) {
#if DEBUG_LOG_GC
  printf("%p Blacken ", (void*)gc);
  printConstant(GC_OBJ_CONST(gc));
  printf("\n");
#endif

  switch (gc->type) {

    case GC_CLASS: {
      GCClass* class = (GCClass*)gc;
      markGCObject(vm, (GCObject*)class->id);
      markTable(vm, &class->methods);
      break;
    }

    case GC_CLOSURE: {
      GCClosure* closure = (GCClosure*)gc;
      markGCObject(vm, (GCObject*)closure->function);

      for (int i = 0; i < closure->upvalueCount; i++) {
        markGCObject(vm, (GCObject*)closure->upvalues[i]);
      }

      break;
    }

    case GC_FUNCTION: {
      GCFunction* function = (GCFunction*)gc;
      markGCObject(vm, (GCObject*)function->id);
      markArray(vm, &function->instructions.constants);
    }

    case GC_INSTANCE: {
      GCInstance* instance = (GCInstance*)gc;
      markGCObject(vm, (GCObject*)instance->class);
      markTable(vm, &instance->fields);
      break;
    }

    case GC_INSTANCE_METHOD: {
      GCInstanceMethod* im = (GCInstanceMethod*)gc;
      markConstant(vm, im->receiver);
      markGCObject(vm, (GCObject*)im->method);
      break;
    }

      // REDUNDANT, MIGHT BE ABLE TO REMOVE
    case GC_NATIVE:
    case GC_STRING:
      break;

    case GC_UPVALUE:
      markConstant(vm, ((GCUpvalue*)gc)->closed);
      break;
  }
}

static void traceReferences(VM* vm) {
  while (vm->grayCount > 0) {
    GCObject* gc = vm->grayStack[--vm->grayCount];
    blackenGCObject(vm, gc);
  }
}

static void sweep(VM* vm, Compiler* compiler) {
  GCObject* previous = NULL;
  GCObject* gc = vm->gcObjects;

  while (gc != NULL) {
    if (gc->isMarked) {
      gc->isMarked = false;
      previous = gc;
      gc = gc->next;
    } else {
      GCObject* unreached = gc;

      gc = gc->next;
      if (previous != NULL) {
        previous->next = gc;
      } else {
        vm->gcObjects = gc;
      }

      freeGCObject(vm, compiler, unreached);
    }
  }
}

void collectGarbage(VM* vm, Compiler* compiler) {
#if DEBUG_LOG_GC
  printf("< GC Starting >\n");
  size_t before = vm->bytesAllocated;
#endif

  markRoots(vm, compiler);
  traceReferences(vm);
  tableRemoveWhite(&vm->strings);

  sweep(vm, compiler);

  vm->nextGC = vm->bytesAllocated * GC_GROW_FACTOR;

#if DEBUG_LOG_GC
  printf("< GC Ending >\n");
  printf(" Collected %ld bytes (from %ld to %ld) next at %ld\n",
         before - vm->bytesAllocated, before, vm->bytesAllocated,
         vm->nextGC);
#endif
}