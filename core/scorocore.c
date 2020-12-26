#include "includes/scorocore.h"
#include "../core/includes/sapi.h"
#include "../src/includes/sparser.h"

static Constant sglNewCoroutine(VM* vm, int arity, Constant* args) {
  expect(1, arity, "Coroutine");

  if (!IS_CLOSURE(args[0])) {
    printf("Coroutine's constructor requires a closure as an argument\n");
    exit(0);
  }

  GCClosure* closure = AS_CLOSURE(args[0]);

  if (closure->function->arity > 1) {
    printf("Coroutine functions can have a maximum of one argument\n");
    exit(0);
  }

  return GC_OBJ_CONST(newCoroutine(vm, OTHER, closure));
}

static bool runCoroutine(VM* vm, GCCoroutine* coroutine, Constant* args, bool isCall, bool hasConstant) {
  if (coroutine->error != NULL) {
    printf("Cannot run a halted coroutine\n");
    return true;
  }

  if (isCall) {
    if (coroutine->caller != NULL) {
      printf("Cannot run a coroutine that has already been called\n");
      return true;
    }

    if (coroutine->state == ROOT) {
      printf("Cannot run the root coroutine\n");
      return true;
    }

    coroutine->caller = vm->coroutine;
  }

  if (coroutine->frameCount == 0) {
    printf("Cannot run a completed coroutine\n");
    return true;
  }

  if (hasConstant)
    pop(vm);

  if (coroutine->frameCount == 1 && coroutine->frames[0].pc == coroutine->frames[0].closure->function->instructions.bytes) {
    if (coroutine->frames[0].closure->function->arity == 1) {
      push(vm, hasConstant ? args[1] : NULL_CONST);
    }
  } else {
    coroutine->stackTop[-1] = hasConstant ? args[1] : NULL_CONST;
  }

  vm->coroutine = coroutine;
  return false;
}

static Constant sglCurrentCoroutine(VM *vm, int arity, Constant *args) {
  return GC_OBJ_CONST(vm->coroutine);
}

static Constant sglCallCoroutine(VM* vm, int arity, Constant* args) {
  GCCoroutine* coroutine = AS_COROUTINE(args[-1]);
  return BOOL_CONST(runCoroutine(vm, coroutine, args, true, arity > 1));
}

static Constant sglTryCoroutine(VM* vm, int arity, Constant* args) {
  GCCoroutine* coroutine = AS_COROUTINE(args[-1]);
  (*(&coroutine))->state = TRY;

  return BOOL_CONST(runCoroutine(vm, coroutine, args, true, arity > 1));
}

static Constant sglTakeoverCoroutine(VM* vm, int arity, Constant* args) {
  GCCoroutine* coroutine = AS_COROUTINE(args[-1]);
  return BOOL_CONST(runCoroutine(vm, coroutine, args, false, arity > 1));
}

static Constant sglCoroutineTakeoverError(VM* vm, int arity, Constant* args) {
  runCoroutine(vm, AS_COROUTINE(args[-1]), args, false, true);
  vm->coroutine->error = &args[0];
  return NULL_CONST;
}

static Constant sglCoroutineIsComplete(VM* vm, int arity, Constant* args) {
  GCCoroutine* coroutine = AS_COROUTINE(args[-1]);

  return BOOL_CONST(coroutine->frameCount == 0 || !coroutine->error);
}

void initCoroutineClass(VM *vm) {
  vm->coroutineClass = newClass(vm, copyString(vm, NULL, "Coroutine", 9), true);

  defineClassNativeStaticMethod(vm, "Coroutine", sglNewCoroutine, vm->coroutineClass);
  defineClassNativeStaticMethod(vm, "current", sglCurrentCoroutine, vm->coroutineClass);

  defineClassNativeMethod(vm, "call", sglCallCoroutine, vm->coroutineClass);
  defineClassNativeMethod(vm, "isComplete", sglCoroutineIsComplete, vm->coroutineClass);
  defineClassNativeMethod(vm, "takeover", sglTakeoverCoroutine, vm->coroutineClass);
  defineClassNativeMethod(vm, "takeoverError", sglCoroutineTakeoverError, vm->coroutineClass);
  defineClassNativeMethod(vm, "try", sglTryCoroutine, vm->coroutineClass);
}
