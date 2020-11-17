#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "includes/sutils.h"
#include "includes/sinstructions.h"
#include "includes/svm.h"
#include "includes/smemory.h"
#include "includes/stable_utils.h"

#define REPL_HELP \
  "Usage: senegal [flags] | [senegal-file]\n\n" \
  "Global options:\n" \
  "-h, --help                 Print this usage information.\n" \
  "    --version              Print the Senegal version.\n" \
  "    --execute [code]       Executes the code provided as an argument with type of a string."

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
        printf(".. ");

        if (!fgets(line, sizeof(line), stdin)) {
          printf("\n");
          break;
        }

        for (int i = 0; line[i]; i++) {
          lBraceCount += (line[i] == '{');
          rBraceCount += (line[i] == '}');
        }

        strcat(block, line);

        if (lBraceCount == rBraceCount) {
          break;
        }
      }

      interpret(vm, block);
    } else if (strcmp(line, ".exit") == 1) {
      break;
    } else {
      interpret(vm, line);
    }
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

int main(int argc, const char* argv[]) {
  VM vm;

  // setbuf(stdout, 0);
  initVM(&vm);

  initTable(&corePaths);

  // == ADD PATHS TO CORE LIBRARIES ==
  char cwd[260];

  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    fprintf(stderr, "Failed to get CWD");
  }

  // While in development, cwd should be in the project root.
  // Once we push to stable, it will be within the bin/ directory
  char* mathPath = strcat(cwd, "/core/math/math.sgl");

  tableInsert(&vm, &corePaths,
              copyString(&vm, NULL, "sgl:math", 8),
              GC_OBJ_CONST(copyString(&vm, NULL,mathPath, strlen(mathPath))));

  if (argc == 1) {
    repl(&vm);
  } 
  
  else if (argc == 2) {
    if (0 == strcmp(argv[1], "-h") || 0 == strcmp(argv[1], "--help")) {
      printf("%s", REPL_HELP);

      exit(0);
    }

    else if (0 == strcmp(argv[1], "--version")) {
      printf("Senegal 0.0.1");

      exit(0);
    }

    else if (0 == strcmp(argv[1], "--execute")) {
      fprintf(stderr, "Senegal expected the code to execute as an argument with the type of a string.");

      exit(0);
    }

    runFile(&vm, argv[1]);
  } 
  
  else if (argc == 3) {
    if (0 == strcmp(argv[1], "--execute")) {
      interpret(&vm, strdup(argv[2]));

      exit(0);
    }

    fprintf(stderr, "%s", REPL_HELP);
    exit(64);
  }
  
  else {
    fprintf(stderr, "%s", REPL_HELP);
    exit(64);
  }

  freeVM(&vm);
  return 0;
}