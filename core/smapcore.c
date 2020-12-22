#include "includes/smapcore.h"
#include "../src/includes/sparser.h"
#include "includes/sapi.h"
#include "../src/includes/stable_utils.h"

static Constant mapNew(VM* vm, int arity, Constant* args) {
  expect(0, arity, "Map constructor");
  return GC_OBJ_CONST(newMap(vm));
}

static Constant mapAdd(VM* vm, int arity, Constant* args) {
  expect(2, arity, "add");

  tableInsert(vm, &AS_MAP(args[-1])->table, args[0], args[1]);

  return NULL_CONST;
}

static Constant mapClear(VM* vm, int arity, Constant* args) {
  initTable(&AS_MAP(args[-1])->table);
  return NULL_CONST;
}

static Constant mapContains(VM* vm, int arity, Constant* args) {
  expect(1, arity, "contains");

  Constant constant;
  return BOOL_CONST(tableGetEntry(&AS_MAP(args[-1])->table, args[0], &constant));
}

static Constant mapRemove(VM* vm, int arity, Constant* args) {
  expect(1, arity, "remove");

  tableRemove(&AS_MAP(args[-1])->table, args[0]);
  AS_MAP(args[-1])->table.count--;
  return NULL_CONST;
}

static Constant mapIsEmpty(VM* vm, int arity, Constant* args) {
  return BOOL_CONST(AS_MAP(args[-1])->table.count == 0);
}

static Constant mapIsNotEmpty(VM* vm, int arity, Constant* args) {
  return BOOL_CONST(AS_MAP(args[-1])->table.count != 0);
}

static Constant mapLength(VM* vm, int arity, Constant* args) {
  return NUM_CONST(AS_MAP(args[-1])->table.count / 2);
}

void initMapClass(VM *vm) {
  vm->mapClass = newClass(vm, copyString(vm, NULL, "Map", 3), true);

  defineClassNativeStaticFunc(vm, "Map", mapNew, vm->mapClass);
  defineClassNativeFunc(vm, "add", mapAdd, vm->mapClass);
  defineClassNativeFunc(vm, "clear", mapClear, vm->mapClass);
  defineClassNativeFunc(vm, "contains", mapContains, vm->mapClass);
  defineClassNativeFunc(vm, "isEmpty", mapIsEmpty, vm->mapClass);
  defineClassNativeFunc(vm, "isNotEmpty", mapIsNotEmpty, vm->mapClass);
  defineClassNativeFunc(vm, "length", mapLength, vm->mapClass);
  defineClassNativeFunc(vm, "remove", mapRemove, vm->mapClass);
}
