#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>

#include "includes/sutils.h"
#include "includes/sinstructions.h"
#include "includes/svm.h"
#include "includes/smemory.h"
#include "includes/stable_utils.h"
#include "../core/includes/smathlib.h"
#include "../core/includes/siolib.h"
#include "../core/includes/scorolib.h"
#include "../core/includes/sfilelib.h"
#include "../core/includes/sapi.h"

#include "../core/includes/ssocketlib.h"

#define SENEGAL_HELP \
  "Usage: senegal [flags] | [senegal-file]\n\n" \
  "Global options:\n" \
  "-h, --help                 Print this usage information.\n" \
  "    --version              Print the Senegal version.\n"

#define SENEGAL_VERSION "Senegal 0.0.1"

static void repl(VM* vm, char* senegalPath) {
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

      interpret(vm, block, senegalPath);
    } else if (strcmp(line, ".exit\n") == 0) {
      break;
    } else {
      interpret(vm, line, senegalPath);
    }
  }
}

static void runFile(VM* vm, const char* path, char* senegalPath) {
  char* source = readFileWithPath(path);

  InterpretationResult result = interpret(vm, source, senegalPath);

  if (result == COMPILE_TIME_ERROR)
    exit(65);

  if (result == RUNTIME_ERROR)
    exit(70);
}

static void addPaths(VM* vm) {
  tableInsert(vm, &vm->corePaths,
              copyString(vm, NULL, "sgl:math", 8),
              GC_OBJ_CONST(newNative(vm, initMathLib)));

  tableInsert(vm, &vm->corePaths,
              copyString(vm, NULL, "sgl:io", 6),
              GC_OBJ_CONST(newNative(vm, initIoLib)));

  tableInsert(vm, &vm->corePaths,
              copyString(vm, NULL, "sgl:corolib", 11),
              GC_OBJ_CONST(newNative(vm, initCoroLib)));

  tableInsert(vm, &vm->corePaths,
              copyString(vm, NULL, "sgl:file", 8),
              GC_OBJ_CONST(newNative(vm, initFileLib)));

  tableInsert(vm, &vm->corePaths,
              copyString(vm, NULL, "sgl:sock", 8),
              GC_OBJ_CONST(newNative(vm, initSocketLib)));
}

static void defineArgv(VM* vm, int argc, const char* argv[]) {
  GCList* args = newList(vm, argc);

  for (int i = 0; i < argc; i++)
    args->elements[argc - 1 - i] = GC_OBJ_CONST(copyString(vm, NULL, argv[i], strlen(argv[i])));

  args->elementC = argc;

  defineGlobal(vm, "argv", GC_OBJ_CONST(args));
}

int main(int argc, const char* argv[]) {
  setlocale(LC_ALL, "");
  VM vm;

  // setbuf(stdout, 0);
  initVM(&vm);

  addPaths(&vm);

  defineArgv(&vm, argc, argv);

  // Get senegal directory from PATH
  char* senegalPath = NULL;
  char *sysPath = getenv("PATH");
  char *dir;

#ifdef _WIN32
  char *delim = ";";
  int execLen = 12;
  char *exec = "\\senegal.exe";
#else
  char* delim = ":";
    int execLen = 8;
    char* exec = "/senegal";
#endif

  for (dir = strtok(sysPath, delim); dir; dir = strtok(NULL, delim)) {
    int dirLen = strlen(dir);
    char execPath[dirLen + execLen + 1];

    memcpy(execPath, dir, dirLen);
    memcpy(execPath + dirLen, exec, execLen);
    execPath[dirLen + execLen] = '\0';

    if (!access(execPath, F_OK)) {
      senegalPath = dir;
      break;
    }
  }

  if (senegalPath == NULL) {
    char cwd[260]; // PATH_MAX
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      senegalPath = cwd;
    }
  }

  if (argc == 1) {
    repl(&vm, senegalPath);
  }

  else if (argc == 2) {
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
      printf("%s", SENEGAL_HELP);

      return 0;
    }

    else if (strcmp(argv[1], "--version") == 0) {
      printf("%s", SENEGAL_VERSION);

      return 0;
    }

    runFile(&vm, argv[1], senegalPath);
  }

  else {
    fprintf(stderr, "%s", SENEGAL_HELP);

    return 64;
  }

  freeVM(&vm);
  return 0;
}
