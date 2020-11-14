#include "includes/sboolCore.h"
#include "includes/sparser.h"
#include "includes/sapi.h"

static Constant boolAsNum(VM* vm, int arity, Constant *args) {
  return NUM_CONST(AS_BOOL(args[0]) ? 1 : 0);
}

static Constant boolToString(VM* vm, int arity, Constant *args) {
  bool isTrue = AS_BOOL(args[0]);

  return GC_OBJ_CONST(copyString(vm, NULL, isTrue ? "true" : "false", isTrue ? 4 : 5));
}

static Constant boolType(VM* vm, int arity, Constant *args) {
  return GC_OBJ_CONST(copyString(vm, NULL, "bool", 4));
}

void initBoolClass(VM* vm) {
  vm->boolClass = newClass(vm, copyString(vm, NULL, "bool", 4), true, false);
  defineClassNativeField(vm, "type", GC_OBJ_CONST(copyString(vm, NULL, "bool", 4)), vm->boolClass);

  defineClassNativeFunc(vm, "asNum", boolAsNum, vm->boolClass);
  defineClassNativeFunc(vm, "toString", boolToString, vm->boolClass);
}