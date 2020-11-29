#include "includes/sfiblib.h"
#include "includes/sapi.h"
#include "includes/sparser.h"

static Constant sglNewFiber(VM* vm, int arity, Constant* args) {
  if (arity != 1 || !IS_CLOSURE(args[0])) {
    printf("Fiber's constructor requires a closure as an argument\n");
    exit(0);
  }

  GCClosure* closure = AS_CLOSURE(args[0]);
  if (closure->function->arity > 1) {
    printf("Fiber functions can have a maximum of one argument\n");
  }

  GCFiber* fiber = newFiber(vm, OTHER, closure);

  return GC_OBJ_CONST(fiber);
}

static bool runFiber(VM* vm, GCFiber* fiber, Constant* args, bool isCall, bool hasConstant) {
  if (fiber->error != NULL) {
    printf("Cannot run a halted fiber\n");
    return true;
  }

  if (isCall) {
    if (fiber->caller != NULL) {
      printf("Cannot run a fiber that has already been called\n");
      return true;
    }

    if (fiber->state == ROOT) {
      printf("Cannot run the root fiber\n");
      return true;
    }

    fiber->caller = vm->fiber;
  }

  if (fiber->frameCount == 0) {
    printf("Cannot run a completed fiber\n");
    return true;
  }

  if (hasConstant)
    vm->fiber->stackTop--;

  if (fiber->frameCount == 1 && fiber->frames[0].pc == fiber->frames[0].closure->function->instructions.bytes) {
    if (fiber->frames[0].closure->function->arity == 1) {
      fiber->stackTop[0] = hasConstant ? args[1] : NULL_CONST;
      fiber->stackTop++;
    }
  } else {
    fiber->stackTop[-1] = hasConstant ? args[1] : NULL_CONST;
  }

  vm->fiber = fiber;
  return false;
}

static Constant sglCallFiber(VM* vm, int arity, Constant* args) {
  GCFiber* fiber = AS_FIBER(args[0]);
  return BOOL_CONST(runFiber(vm, fiber, args, true, arity > 1));
}

static Constant sglTransferFiber(VM* vm, int arity, Constant* args) {
  GCFiber* fiber = AS_FIBER(args[0]);
  return BOOL_CONST(runFiber(vm, fiber, args, false, arity > 1));
}

static Constant sglErrorFiber(VM* vm, int arity, Constant* args) {
  return *AS_FIBER(args[0])->error;
}

static Constant sglHaltFiber(VM* vm, int arity, Constant* args) {
  vm->fiber->error = &args[0];

  return IS_NULL(args[0]);
}

static Constant sglSuspendFiber(VM* vm, int arity, Constant* args) {
  vm->fiber = NULL;

  return BOOL_CONST(false);
}

Constant initFibLib(VM *vm, int arity, Constant *args) {
  GCClass* fiberClass = newClass(vm, copyString(vm, NULL, "Fiber", 5), true);

  defineGlobal(vm, "Fiber", GC_OBJ_CONST(fiberClass));

  defineGlobalFunc(vm, "newFiber", sglNewFiber);
  defineGlobal(vm, "currentFiber", GC_OBJ_CONST(vm->fiber));

  defineGlobalFunc(vm, "callFiber", sglCallFiber);
  defineGlobalFunc(vm, "transferFiber", sglTransferFiber);

  defineGlobalFunc(vm, "throw", sglErrorFiber);

  defineGlobalFunc(vm, "halt", sglHaltFiber);
  defineGlobalFunc(vm, "suspend", sglSuspendFiber);
}
