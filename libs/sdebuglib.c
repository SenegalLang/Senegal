#include "includes/sdebuglib.h"
#include "../core/includes/sapi.h"
#include "../src/includes/sdebug.h"

static Constant sglDisassembleFunction(VM* vm, int arity, Constant* args) {
  expect(1, arity, "disassemble");

  GCFunction* function = AS_CLOSURE(args[0])->function;
  disassembleInstructions(&function->instructions, function->id->chars);

  return NULL_CONST;
}

Constant initDebugLib(VM* vm, int arity, Constant* args) {
  defineGlobalFunc(vm, "disassemble", sglDisassembleFunction);
}