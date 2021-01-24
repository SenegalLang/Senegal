#include <stdio.h>
#include <stdlib.h>
#include "includes/sapi.h"
#include "../src/includes/stable_utils.h"
#include "../src/includes/sparser.h"

static Constant peek(VM* vm) {
  return vm->coroutine->stackTop[-1];
}

static Constant peek2(VM* vm) {
  return vm->coroutine->stackTop[-2];
}

void expect(int expected, int actual, char *name) {
  if (actual != expected) {
    fprintf(stderr, "%s expected %d args, but found %d\n", name, expected, actual);
    exit(1);
  }
}

void defineClassNativeMethod(VM* vm, const char* id, NativeFunc function, GCClass* class) {
  push(vm, GC_OBJ_CONST(copyString(vm, NULL, id, (int)strlen(id))));
  push(vm, GC_OBJ_CONST(newNative(vm, function)));
  tableInsert(vm, &class->methods, peek2(vm), peek(vm), false);
  pop(vm);
  pop(vm);
}

void defineClassNativeField(VM* vm, const char* id, Constant field, GCClass* class) {
  push(vm, GC_OBJ_CONST(copyString(vm, NULL, id, (int)strlen(id))));
  tableInsert(vm, &class->fields, peek(vm), field, false);
  pop(vm);
}

void defineClassNativeStaticMethod(VM* vm, const char* id, NativeFunc function, GCClass* class) {
  push(vm, GC_OBJ_CONST(copyString(vm, NULL, id, (int)strlen(id))));
  push(vm, GC_OBJ_CONST(newNative(vm, function)));
  tableInsert(vm, &class->staticMethods, peek2(vm), peek(vm), false);
  pop(vm);
  pop(vm);
}

void defineClassNativeStaticField(VM* vm, const char* id, Constant field, GCClass* class) {
  push(vm, GC_OBJ_CONST(copyString(vm, NULL, id, (int)strlen(id))));
  tableInsert(vm, &class->staticFields, peek(vm), field, false);
  pop(vm);
}

void defineGlobal(VM* vm, const char* id, Constant field) {
  push(vm, GC_OBJ_CONST(copyString(vm, NULL, id, (int)strlen(id))));
  tableInsert(vm, &vm->globals, peek(vm), field, false);
  pop(vm);
}

void defineGlobalFunc(VM* vm, const char* id, NativeFunc function) {
  push(vm, GC_OBJ_CONST(copyString(vm, NULL, id, (int)strlen(id))));
  push(vm, GC_OBJ_CONST(newNative(vm, function)));
  tableInsert(vm, &vm->globals, peek2(vm), peek(vm), false);
  pop(vm);
  pop(vm);
}

Constant assertApi(VM* vm, int arity, Constant *args) {
  if (arity < 2) {
    printf("assert expected two arguments, but found  %d", arity);
    exit(1);
  }

  /// assert(arg1, arg2, "MSGs"...);
  if (!areEqual(args[0], args[1])) {
    if (arity >= 3) {
      for (int i = 1; i < arity; i++)
        printConstant(stdout, args[i]);

      printf("\n");
    } else {
      printf("ASSERT(");
      printConstant(stdout, args[0]);
      printf(" != ");
      printConstant(stdout, args[1]);
      printf(")");
    }

    exit(1);
  }

  return NULL_CONST;
}

Constant printApi(VM* vm, int arity, Constant* args) {
  for (int i = 0; i < arity; i++)
    printConstant(stdout, args[i]);

  return NULL_CONST;
}

Constant printlnApi(VM* vm, int arity, Constant* args) {

  for (int i = 0; i < arity; i++)
    printConstant(stdout, args[i]);

  printf("\n");

  return NULL_CONST;
}