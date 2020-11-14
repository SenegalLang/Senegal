#include "includes/sstringCore.h"
#include "includes/sparser.h"
#include "includes/sapi.h"

static Constant stringIsEmpty(VM* vm, int arity, Constant *args) {
  return BOOL_CONST(AS_STRING(args[0])->chars[0] == '\0');
}

static Constant stringIsNotEmpty(VM* vm, int arity, Constant *args) {
  return BOOL_CONST(AS_STRING(args[0])->chars[0] != '\0');
}

void initStringClass(VM *vm) {
  vm->stringClass = newClass(vm, copyString(vm, NULL, "String", 6), true, false);
  defineClassNativeField(vm, "type", GC_OBJ_CONST(copyString(vm, NULL, "String", 6)), vm->stringClass);

  defineClassNativeFunc(vm, "isEmpty", stringIsEmpty, vm->stringClass);
  defineClassNativeFunc(vm, "isNotEmpty", stringIsNotEmpty, vm->stringClass);
}
