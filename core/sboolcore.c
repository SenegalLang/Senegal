#include "includes/sboolcore.h"
#include "../src/includes/sparser.h"
#include "includes/sapi.h"

static Constant boolAsNum(VM* vm, int arity, Constant *args) {
  expect(0, arity, "asNum");
  return NUM_CONST(AS_BOOL(args[-1]) ? 1 : 0);
}

static Constant boolToString(VM* vm, int arity, Constant *args) {
  expect(0, arity, "toString");

  bool isTrue = AS_BOOL(args[-1]);
  return GC_OBJ_CONST(copyString(vm, NULL, isTrue ? "true" : "false", isTrue ? 4 : 5));
}


void initBoolClass(VM* vm) {
  vm->boolClass = newClass(vm, copyString(vm, NULL, "bool", 4), true);

  defineClassNativeMethod(vm, "asNum", boolAsNum, vm->boolClass);
  defineClassNativeMethod(vm, "toString", boolToString, vm->boolClass);
}