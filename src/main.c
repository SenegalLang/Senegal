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
#include "../libs/includes/scorolib.h"
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

      interpret(vm, block, senegalPath, cwd);
    } else if (strcmp(line, ".exit\n") == 0) {
      break;
    } else {
      interpret(vm, line, senegalPath, cwd);
    }
  }
}

static inline char* getFileDir(VM* vm, const char* filePath) {
  char* pathDup = strdup(filePath);

  return dirname(pathDup);
}

static void runFile(VM* vm, const char* path, char* senegalPath) {
  char* source = readFileWithPath(path);
  char* dir = getFileDir(vm, path);

  InterpretationResult result = interpret(vm, source, senegalPath, dir);

  if (result == COMPILE_TIME_ERROR)
    exit(65);

  if (result == RUNTIME_ERROR)
    exit(70);
}

static void addPaths(VM* vm) {
  tableInsert(vm, &vm->corePaths,
              GC_OBJ_CONST(copyString(vm, NULL, "sgl:math", 8)),
              GC_OBJ_CONST(newNative(vm, initMathLib)));

  tableInsert(vm, &vm->corePaths,
              GC_OBJ_CONST(copyString(vm, NULL, "sgl:io", 6)),
              GC_OBJ_CONST(newNative(vm, initIoLib)));

  tableInsert(vm, &vm->corePaths,
              GC_OBJ_CONST(copyString(vm, NULL, "sgl:corolib", 11)),
              GC_OBJ_CONST(newNative(vm, initCoroLib)));

  tableInsert(vm, &vm->corePaths,
              GC_OBJ_CONST(copyString(vm, NULL, "sgl:file", 8)),
              GC_OBJ_CONST(newNative(vm, initFileLib)));

  tableInsert(vm, &vm->corePaths,
              GC_OBJ_CONST(copyString(vm, NULL, "sgl:sock", 8)),
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

  if (!senegalPath) {
    char cwd[260]; // PATH_MAX
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      senegalPath = cwd;
    }
  }

  if (argc == 1) {
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
      } else if(!strcmp(argv[i], "--args")) {
        argsStart = i + 1;
        break;
      } else {
        fprintf(stderr, "%s", SENEGAL_HELP);
        return 64;
      }
    }

    defineArgv(&vm, argsStart == 0 ? argsStart : argc - argsStart, argv + argsStart);

    runFile(&vm, argv[1], senegalPath);
  }

  freeVM(&vm);
  return 0;
}
