#include "includes/siolib.h"
#include "includes/sapi.h"
#include "includes/sparser.h"

#include <stdio.h>

static Constant sglExit(VM* vm, int arity, Constant* args) {
  int exitCode = NUM_CONST(args[0]);

  exit(exitCode);

  return NULL_CONST;
}

Constant initIoLib(VM* vm, int arity, Constant* args) {
  // Terminate the process immediately and the exit code is returned to the parent process.
  defineGlobalFunc(vm, "exit", sglExit);
}