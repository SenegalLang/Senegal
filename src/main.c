#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <libgen.h>
#include <string.h>

#include "includes/sutils.h"
#include "includes/sinstructions.h"
#include "includes/svm.h"
#include "includes/smemory.h"
#include "includes/stable_utils.h"
#include "../libs/includes/smathlib.h"
#include "../libs/includes/siolib.h"
#include "../libs/includes/sfilelib.h"
#include "../core/includes/sapi.h"

#include "../libs/includes/ssocketlib.h"

static void repl(VM* vm, char* senegalPath) {
  printf(SENEGAL_REPL);

  char line[1024];

  char cwd[260]; // PATH_MAX
  if (!getcwd(cwd, sizeof(cwd))) {
    fprintf(stderr, "Failed to get current directory");
    exit(1);
  }

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

      interpret(vm, "REPL", block, senegalPath, cwd);
    } else if (strcmp(line, ".exit\n") == 0) {
      break;
    } else {
      interpret(vm, "REPL", line, senegalPath, cwd);
    }
  }
}

static char* getFileDir(const char* filePath) {
  char* pathDup = strdup(filePath);
  return dirname(pathDup);
}

static void runFile(VM* vm, char* path, char* senegalPath) {
  char* source = readFileWithPath(path);
  char* dir = getFileDir(path);

  InterpretationResult result = interpret(vm, path, source, senegalPath, dir);

  if (result == COMPILE_TIME_ERROR)
    exit(65);

  if (result == RUNTIME_ERROR)
    exit(70);
}

static GCString* newPathString(VM* vm, char* path) {
  return copyString(vm, NULL, path, (int)strlen(path));
}

static void addPaths(VM* vm) {
  tableInsert(vm, &vm->corePaths,
              GC_OBJ_CONST(copyString(vm, NULL, "sgl:math", 8)),
              GC_OBJ_CONST(newNative(vm, initMathLib)));

  tableInsert(vm, &vm->corePaths,
              GC_OBJ_CONST(copyString(vm, NULL, "sgl:io", 6)),
              GC_OBJ_CONST(newNative(vm, initIoLib)));

  tableInsert(vm, &vm->corePaths,
              GC_OBJ_CONST(copyString(vm, NULL, "sgl:file", 8)),
              GC_OBJ_CONST(newNative(vm, initFileLib)));

  tableInsert(vm, &vm->corePaths,
              GC_OBJ_CONST(copyString(vm, NULL, "sgl:sock", 8)),
              GC_OBJ_CONST(newNative(vm, initSocketLib)));
}

static void defineArgv(VM* vm, int argc, char* argv[]) {
  GCList* args = newList(vm, argc);

  for (int i = 0; i < argc; i++)
    args->elements[argc - 1 - i] = GC_OBJ_CONST(copyString(vm, NULL, argv[i], strlen(argv[i])));

  args->elementC = argc;

  defineGlobal(vm, "argv", GC_OBJ_CONST(args));
}

static void applyEnhancements(VM* vm, char* senegalPath) {
  char* path = concat(senegalPath, "/libs/list/list.sgl");
  interpret(vm, path, readFileWithPath(path), senegalPath, senegalPath);
}

int main(int argc, char* argv[]) {
  setlocale(LC_ALL, "");

  VM vm;

  // setbuf(stdout, 0);
  initVM(&vm);
  addPaths(&vm);

  // Get senegal directory from PATH
  char* tmpSenegalPath = getenv("SENEGAL_HOME");
  int pathLen = (int)strlen(tmpSenegalPath) - 4; // "/bin"

  char* senegalPath = malloc(pathLen);
  memcpy(senegalPath, tmpSenegalPath, pathLen);
  senegalPath[pathLen] = '\0';

  if (argc == 1) {
    if (tmpSenegalPath) {
      fprintf(stderr,
              "SENEGAL_HOME not found, please provide the directory to the senegal directory with the `--path` flag.");
      exit(1);
    }

    applyEnhancements(&vm, senegalPath);

    repl(&vm, senegalPath);
  } else {
    int argsStart = 0;

    for (int i = 2; i < argc; i++) {
      if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
        printf("%s", SENEGAL_HELP);
        return 0;
      } else if (!strcmp(argv[i], "--version")) {
        printf("%s", SENEGAL_VERSION);
        return 0;
      } else if (!strcmp(argv[i], "--args")) {
        argsStart = i + 1;
      } else if (!strcmp(argv[i], "--path")) {
        if (!tmpSenegalPath)
          senegalPath = argv[i+1];
        i++;
      } else {
        fprintf(stderr, "%s", SENEGAL_HELP);
        return 64;
      }
    }

    if (!tmpSenegalPath) {
      fprintf(stderr,
              "SENEGAL_HOME not found, please provide the directory to the senegal directory with the `--path` flag.");
      exit(1);
    }

    applyEnhancements(&vm, senegalPath);

    defineArgv(&vm, argsStart == 0 ? argsStart : argc - argsStart, argv + argsStart);
    runFile(&vm, argv[1], senegalPath);
  }

  freeVM(&vm);
  return 0;
}
