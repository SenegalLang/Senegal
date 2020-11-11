#include "includes/smapCore.h"
#include "includes/sparser.h"
#include "includes/sapi.h"

static Constant mapIsEmpty(VM* vm, int arity, Constant* args) {
  return BOOL_CONST(AS_MAP(args[0])->table.count == 0);
}

static Constant mapIsNotEmpty(VM* vm, int arity, Constant* args) {
  return BOOL_CONST(AS_MAP(args[0])->table.count != 0);
}

static Constant mapLength(VM* vm, int arity, Constant* args) {
  return NUM_CONST(AS_MAP(args[0])->table.count / 2);
}

void initMapClass(VM *vm) {
  vm->mapClass = newClass(vm, copyString(vm, NULL, "Map", 3), false, false);

  defineClassNativeField(vm, "type", GC_OBJ_CONST(copyString(vm, NULL, "Map", 3)), vm->mapClass);
  defineClassNativeFunc(vm, "isEmpty", mapIsEmpty, vm->mapClass);
  defineClassNativeFunc(vm, "isNotEmpty", mapIsNotEmpty, vm->mapClass);
  defineClassNativeFunc(vm, "length", mapLength, vm->mapClass);
}
