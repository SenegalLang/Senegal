#include "includes/slistCore.h"
#include "includes/sparser.h"
#include "includes/sapi.h"

// TODO(Calamity): Correct
static Constant listForEach(VM* vm, int arity, Constant* args) {

  GCClosure* closure = AS_CLOSURE(args[0]);
  GCList* list = AS_LIST(args[-1]);

  pop(vm);
  for (int i = 0; i < list->elementC; i++) {
    if (closure->function->arity != 1) {
      fprintf(stderr, "Invalid forEach callback function");
    }

    push(vm, GC_OBJ_CONST(closure));
    push(vm, list->elements[i]);
    call(vm, closure, arity);
  }
}

static Constant listLength(VM* vm, int arity, Constant* args) {
  return NUM_CONST(AS_LIST(args[-1])->elementC);
}

void initListClass(VM *vm) {
  vm->listClass = newClass(vm, copyString(vm, NULL, "List", 4), true, false);
  defineClassNativeField(vm, "type", GC_OBJ_CONST(copyString(vm, NULL, "List", 4)), vm->listClass);

  defineClassNativeFunc(vm, "forEach", listForEach, vm->listClass);
  defineClassNativeFunc(vm, "length", listLength, vm->listClass);
}
