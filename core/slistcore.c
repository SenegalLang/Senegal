#include "includes/slistcore.h"
#include "../src/includes/sparser.h"
#include "includes/sapi.h"
#include "../src/includes/smemory.h"

static Constant listNew(VM* vm, int arity, Constant* args) {
  expect(1, arity, "List constructor");
  GCList* list = newList(vm, 0);
  int cap = AS_NUMBER(args[0]);

  if (cap > 0) {
    list->listCurrentCap = cap;
  }

  return GC_OBJ_CONST(list);
}

void addToList(VM* vm, GCList** list, Constant element) {

  Constant* elements = ALLOCATE(vm, NULL, Constant, (*list)->elementC + 1);
  elements[0] = element;

  for (int i = 1; i <= (*list)->elementC; i++) {
    elements[i] = (*list)->elements[i - 1];
  }

  (*list)->elements = elements;
  (*list)->elementC++;
}

static Constant listAdd(VM* vm, int arity, Constant* args) {
  expect(1, arity, "add");

  GCList* list = AS_LIST(args[-1]);

  if (list->listCurrentCap > 0 && list->elementC == list->listCurrentCap) {
    printf("List cannot grow beyond %d elements", list->elementC);
    exit(1);
  }

 addToList(vm, &list, args[0]);

  return NULL_CONST;
}

static Constant listClear(VM* vm, int arity, Constant* args) {
  expect(0, arity, "clear");
  AS_LIST(args[-1])->elementC = 0;

  return NULL_CONST;
}

// TODO: Shift elements rather than replacing at index
static Constant listInsertAt(VM* vm, int arity, Constant* args) {
  expect(2, arity, "insert");

  GCList* list = AS_LIST(args[-1]);
  double index = AS_NUMBER(args[0]);
  Constant element = args[1];

  if (index >= list->elementC) {
    printf("Out of range, maximum was %d but found %d", list->elementC - 1, (int)index);
    exit(1);
  }

  list->elements[(list->elementC - 1) - (int)index] = element;

  return NULL_CONST;
}

static Constant listRemoveAt(VM* vm, int arity, Constant* args) {
  expect(1, arity, "removeAt");

  GCList* list = AS_LIST(args[-1]);
  double index = AS_NUMBER(args[0]);

  if (index >= list->elementC) {
    printf("Out of range, maximum was %d but found %d", list->elementC - 1, (int)index);
    exit(1);
  }

  Constant* elements = ALLOCATE(vm, NULL, Constant, list->elementC - 1);

  for (int i = 0; i < list->elementC; i++) {
    if (i == (int)index)
      continue;

    elements[i > index ? i - 1 : i] = list->elements[i];
  }

  list->elements = elements;
  list->elementC--;

  return NULL_CONST;
}


static Constant listLength(VM* vm, int arity, Constant* args) {
  expect(0, arity, "length");
  return NUM_CONST(AS_LIST(args[-1])->elementC);
}

static Constant listFilled(VM* vm, int arity, Constant* args) {

  expect(2, arity, "filled");

  double size = AS_NUMBER(args[0]);
  double element = args[1];

  GCList* list = newList(vm, (int)size);
  list->elementC = (int)size;

  for (int i = 0; i < size; i++)
    list->elements[i] = element;

  return GC_OBJ_CONST(list);
}

void initListClass(VM *vm) {
  vm->listClass = newClass(vm, copyString(vm, NULL, "List", 4), true);

  defineClassNativeStaticFunc(vm, "List", listNew, vm->listClass);
  defineClassNativeStaticFunc(vm, "filled", listFilled, vm->listClass);

  defineClassNativeFunc(vm, "add", listAdd, vm->listClass);
  defineClassNativeFunc(vm, "clear", listClear, vm->listClass);
  defineClassNativeFunc(vm, "insertAt", listInsertAt, vm->listClass);
  defineClassNativeFunc(vm, "length", listLength, vm->listClass);
  defineClassNativeFunc(vm, "removeAt", listRemoveAt, vm->listClass);
}
