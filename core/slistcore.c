#include "includes/slistcore.h"
#include "../src/includes/sparser.h"
#include "includes/sapi.h"
#include "../src/includes/smemory.h"
#include "../src/includes/scompiler.h"
#include "../src/includes/sgcobject_utils.h"

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

// Appends all elements from `other` to the end of `this` List.
static Constant listAddAll(VM* vm, int arity, Constant* args) {
  expect(1, arity, "addAll");

  GCList* list = AS_LIST(args[-1]);
  GCList* other = AS_LIST(args[0]);

  if (list->listCurrentCap > 0 && list->elementC == list->listCurrentCap) {
    printf("List cannot grow beyond %d elements", list->elementC);
    exit(1);
  }

  for (int i = other->elementC - 1; i >= 0; i--)
    addToList(vm, &list, other->elements[i]);

  return NULL_CONST;
}

static Constant listClear(VM* vm, int arity, Constant* args) {
  expect(0, arity, "clear");
  AS_LIST(args[-1])->elementC = 0;

  return NULL_CONST;
}

static Constant listContains(VM* vm, int arity, Constant* args) {
  expect(1, arity, "contains");

  GCList* list = AS_LIST(args[-1]);
  Constant match = args[0];

  for (int i = 0; i < list->elementC; i++) {
    Constant element = list->elements[i];

    if (areEqual(match, element))
      return BOOL_CONST(true);
  }

  return BOOL_CONST(false);
}

static Constant listIterate(VM* vm, int arity, Constant* args) {
  expect(1, arity, "iterate");

  GCList* list = AS_LIST(args[-1]);
  Constant iter = args[0];

  if (IS_NULL(iter)) {
    if (!list->elementC)
      return NULL_CONST;

    return NUM_CONST(0);
  }

  if (!IS_NUMBER(iter)) {
    fprintf(stderr, "Invalid argument passed to iterate, expected num or null.");
    exit(1);
  }

  double index = AS_NUMBER(iter);

  // Stop iterating
  if (index < 0 || index >= list->elementC - 1)
    return NULL_CONST;

  return NUM_CONST(index + 1);
}

static Constant listIteratorCurrent(VM* vm, int arity, Constant* args) {
  expect(1, arity, "iteratorCurrent");

  GCList* list = AS_LIST(args[-1]);
  Constant iter = args[0];

  if (!IS_NUMBER(iter) || AS_NUMBER(iter) >= list->elementC) {
    fprintf(stderr, "Invalid argument passed to iteratorCurrent, expected a num within range 0-%d.", list->elementC - 1);
    exit(1);
  }
  uint32_t index = AS_NUMBER(iter);

  return list->elements[list->elementC - 1 - index];
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
  double index = list->elementC - AS_NUMBER(args[0]) - 1;

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
  Constant element = args[1];

  GCList* list = newList(vm, (int)size);
  list->elementC = (int)size;

  for (int i = 0; i < size; i++)
    list->elements[i] = element;

  return GC_OBJ_CONST(list);
}

Constant listToString(VM* vm, int arity, Constant* args) {
  char* listString = constantToString(args[-1]);

  return GC_OBJ_CONST(copyString(vm, NULL, listString, strlen(listString)));
}

void initListClass(VM *vm) {
  vm->listClass = newClass(vm, copyString(vm, NULL, "List", 4), true);

  defineClassNativeStaticMethod(vm, "List", listNew, vm->listClass);
  defineClassNativeStaticMethod(vm, "filled", listFilled, vm->listClass);

  defineClassNativeMethod(vm, "add", listAdd, vm->listClass);
  defineClassNativeMethod(vm, "addAll", listAddAll, vm->listClass);
  defineClassNativeMethod(vm, "clear", listClear, vm->listClass);
  defineClassNativeMethod(vm, "contains", listContains, vm->listClass);
  defineClassNativeMethod(vm, "iterate", listIterate, vm->listClass);
  defineClassNativeMethod(vm, "iteratorCurrent", listIteratorCurrent, vm->listClass);
  defineClassNativeMethod(vm, "insertAt", listInsertAt, vm->listClass);
  defineClassNativeMethod(vm, "length", listLength, vm->listClass);
  defineClassNativeMethod(vm, "removeAt", listRemoveAt, vm->listClass);
  defineClassNativeMethod(vm, "toString", listToString, vm->listClass);
}
