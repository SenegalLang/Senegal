#include "includes/siolib.h"
#include "includes/sapi.h"
#include "includes/sparser.h"

#include <stdio.h>

static Constant sglExit(VM* vm, int arity, Constant* args) {
  int exitCode = NUM_CONST(args[0]);

  exit(exitCode);

  return NULL_CONST;
}

static Constant sglClock(VM* vm, int arity, Constant* args) {
  return NUM_CONST((double)clock());
}

static Constant sglReadLine(VM* vm, int arity, Constant* args) {
  char* line = NULL;
  size_t len;

  size_t lineLen = getline(&line, &len, stdin);

  return GC_OBJ_CONST(copyString(vm, NULL, line, lineLen));
}

Constant initIoLib(VM* vm, int arity, Constant* args) {
  // Terminate the process immediately and the exit code is returned to the parent process.
  defineGlobalFunc(vm, "exit", sglExit);

  defineGlobal(vm, "CLOCKS_PER_SEC", NUM_CONST(CLOCKS_PER_SEC));
  defineGlobalFunc(vm, "clock", sglClock);
  defineGlobalFunc(vm, "readln", sglReadLine);
}