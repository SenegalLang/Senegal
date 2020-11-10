#include <stdio.h>
#include <stdlib.h>
#include "includes/sutils.h"
#include "includes/sinstructions.h"
#include "includes/svm.h"
#include "includes/smemory.h"

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
  free(source);

  if (result == COMPILE_TIME_ERROR)
    exit(65);

  if (result == RUNTIME_ERROR)
    exit(70);
}

int main(int argc, const char* argv[]) {
  VM vm;

  setbuf(stdout, 0);

  initVM(&vm);
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