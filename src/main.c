#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "includes/sutils.h"
#include "includes/sinstructions.h"
#include "includes/svm.h"
#include "includes/smemory.h"
#include "includes/stable_utils.h"
#include "includes/smathlib.h"
#include "includes/siolib.h"
#include "includes/scorolib.h"
#include "includes/sfilelib.h"

#ifdef _WIN32
#include "includes/swsocket.h"
#endif

#define SENEGAL_HELP \
  "Usage: senegal [flags] | [senegal-file]\n\n" \
  "Global options:\n" \
  "-h, --help                 Print this usage information.\n" \
  "    --version              Print the Senegal version.\n"

#define SENEGAL_VERSION "Senegal 0.0.1"

static void repl(VM* vm) {
  char line[1024];

  for (;;) {
    printf("> ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    int lBraceCount = 0;
    int rBraceCount = 0;

    for (int i = 0; line[i]; i++) {
      lBraceCount += (line[i] == '{');
      rBraceCount += (line[i] == '}');
    }

    if (lBraceCount > rBraceCount) {
      char block[1024];

      memcpy(block, line, sizeof(line));

      for (;;) {
        printf("%.*s ", (lBraceCount - rBraceCount) + 1, ">>>>>");

        if (!fgets(line, sizeof(line), stdin)) {
          printf("\n");
          break;
        }

        for (int i = 0; line[i]; i++) {
          lBraceCount += (line[i] == '{');
          rBraceCount += (line[i] == '}');
        }

        strcat(block, line);

        if (lBraceCount == rBraceCount)
          break;
        
      }

      interpret(vm, block);
    } else if (strcmp(line, ".exit\n") == 0) {
      break;
    } else {
      interpret(vm, line);
    }
  }
}

static void runFile(VM* vm, const char* path) {
  char* source = readFileWithPath(path);
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

  tableInsert(vm, &corePaths,
              copyString(vm, NULL, "sgl:io", 6),
              GC_OBJ_CONST(newNative(vm, initIoLib)));

  tableInsert(vm, &corePaths,
              copyString(vm, NULL, "sgl:corolib", 11),
              GC_OBJ_CONST(newNative(vm, initCoroLib)));

  tableInsert(vm, &corePaths,
              copyString(vm, NULL, "sgl:file", 8),
              GC_OBJ_CONST(newNative(vm, initFileLib)));

#ifdef _WIN32
  tableInsert(vm, &corePaths,
              copyString(vm, NULL, "sgl:sock", 8),
              GC_OBJ_CONST(newNative(vm, initSocketLib)));
#endif
}

int main(int argc, const char* argv[]) {
  VM vm;

  // setbuf(stdout, 0);
  initVM(&vm);

  initTable(&corePaths);

  addPaths(&vm);

  if (argc == 1) {
    repl(&vm);
  } 
  
  else if (argc == 2) {
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
      printf("%s", SENEGAL_HELP);

      return 0;
    }

    else if (strcmp(argv[1], "--version") == 0) {
      printf("%c", SENEGAL_VERSION);

      return 0;
    }

    runFile(&vm, argv[1]);
  }
  
  else {
    fprintf(stderr, "%s", SENEGAL_HELP);
    
    return 64;
  }

  freeVM(&vm);
  return 0;
}
