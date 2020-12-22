#include "includes/scorolib.h"
#include "../core/includes/sapi.h"
#include "../src/includes/sparser.h"

static Constant sglNewCoroutine(VM* vm, int arity, Constant* args) {
  if (arity != 1 || !IS_CLOSURE(args[0])) {
    printf("Coroutine's constructor requires a closure as an argument\n");
    exit(0);
  }

  GCClosure* closure = AS_CLOSURE(args[0]);
  if (closure->function->arity > 1) {
    printf("Coroutine functions can have a maximum of one argument\n");
  }

  GCCoroutine* coroutine = newCoroutine(vm, OTHER, closure);

  return GC_OBJ_CONST(coroutine);
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
  GCCoroutine* coroutine = AS_COROUTINE(args[0]);
  return BOOL_CONST(runCoroutine(vm, coroutine, args, true, arity > 2));
}

static Constant sglTransferCoroutine(VM* vm, int arity, Constant* args) {
  GCCoroutine* coroutine = AS_COROUTINE(args[0]);
  return BOOL_CONST(runCoroutine(vm, coroutine, args, false, arity > 2));
}

static Constant sglCoroutineTransferError(VM* vm, int arity, Constant* args) {
  runCoroutine(vm, AS_COROUTINE(args[0]), args, false, true);
  vm->coroutine->error = &args[1];
  return NULL_CONST;
}

static Constant sglErrorCoroutine(VM* vm, int arity, Constant* args) {
  return *AS_COROUTINE(args[0])->error;
}

static Constant sglCoroutineYield(VM* vm, int arity, Constant* args) {
  GCCoroutine* current = vm->coroutine;
  vm->coroutine = current->caller;

  current->caller = NULL;
  current->state = OTHER;

  if (vm->coroutine != NULL) {
    if (arity > 0)
      return args[0];
    else
      return NULL_CONST;
  }

  return NULL_CONST;
}

static Constant sglCoroutineIsComplete(VM* vm, int arity, Constant* args) {
  GCCoroutine* coroutine = AS_COROUTINE(args[0]);

  return BOOL_CONST(coroutine->frameCount == 0 || !coroutine->error);
}

static Constant sglHaltCoroutine(VM* vm, int arity, Constant* args) {
  vm->coroutine->error = &args[0];

  return BOOL_CONST(IS_NULL(args[0]));
}

static Constant sglSuspendCoroutine(VM* vm, int arity, Constant* args) {
  vm->coroutine = NULL;

  return BOOL_CONST(false);
}

Constant initCoroLib(VM *vm, int arity, Constant *args) {

  defineGlobalFunc(vm, "newCoroutine", sglNewCoroutine);
  defineGlobalFunc(vm, "currentCoroutine", sglCurrentCoroutine);

  defineGlobalFunc(vm, "call", sglCallCoroutine);
  defineGlobalFunc(vm, "transfer", sglTransferCoroutine);
  defineGlobalFunc(vm, "transferError", sglCoroutineTransferError);

  defineGlobalFunc(vm, "throw", sglErrorCoroutine);
  defineGlobalFunc(vm, "yield", sglCoroutineYield);

  defineGlobalFunc(vm, "isComplete", sglCoroutineIsComplete);
  defineGlobalFunc(vm, "halt", sglHaltCoroutine);
  defineGlobalFunc(vm, "suspend", sglSuspendCoroutine);
}
