#include "includes/siolib.h"
#include "includes/sapi.h"
#include "includes/sparser.h"

#include <stdio.h>

static Constant sglExit(VM* vm, int arity, Constant* args) {
  int exitCode = NUM_CONST(args[0]);

  exit(exitCode);

  return NULL_CONST;
}

static Constant sglSystem(VM* vm, int arity, Constant* args) {
  char* command = AS_CSTRING(args[0]);

  return NUM_CONST(system(command));
}

Constant initIoLib(VM* vm, int arity, Constant* args) {
  // Terminate the process immediately and the exit code is returned to the parent process.
  defineGlobalFunc(vm, "exit", sglExit);

  // Excutes the command passed as a string in the command processor or the terminal of the operating system.
  defineGlobalFunc(vm, "system", sglSystem);
}