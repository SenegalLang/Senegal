#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "includes/sutils.h"
#include "includes/sinstructions.h"
#include "includes/svm.h"
#include "includes/smemory.h"
#include "includes/stable_utils.h"
#include "includes/smathlib.h"

static void repl(VM* vm) {
  char line[1024];

  for (;;) {
    printf("> ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    interpret(vm, line);
  }
}

static void runFile(VM* vm, const char* path) {
  char* source = readFile(path);
  InterpretationResult result = interpret(vm, source);

  if (result == COMPILE_TIME_ERROR)
    exit(65);

  if (result == RUNTIME_ERROR)
    exit(70);
}

static void addPaths(VM* vm) {
  tableInsert(vm, &corePaths,
              copyString(vm, NULL, "sgl:math", 8),
              GC_OBJ_CONST(newNative(vm, initMathLib)));

}

int main(int argc, const char* argv[]) {

  VM vm;

//  setbuf(stdout, 0);
  initVM(&vm);

  initTable(&corePaths);

  addPaths(&vm);

  if (argc == 1) {
    repl(&vm);
  } else if (argc == 2) {
    runFile(&vm, argv[1]);
  } else {
    fprintf(stderr, "Usage: senegal [path]\n");
    exit(64);
  }

  freeVM(&vm);
  return 0;
}