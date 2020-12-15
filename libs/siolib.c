#include <unistd.h>
#include "includes/siolib.h"
#include "../core/includes/sapi.h"
#include "../src/includes/sparser.h"
#include "includes/sfilelib.h"
#include "../src/includes/smemory.h"

#ifdef _WIN32
size_t getline(char **lineptr, size_t *n, FILE *stream) {
  char *bufptr = NULL;
  char *p = bufptr;
  size_t size;
  int c;

  if (lineptr == NULL) {
    return -1;
  }
  if (stream == NULL) {
    return -1;
  }
  if (n == NULL) {
    return -1;
  }
  bufptr = *lineptr;
  size = *n;

  c = fgetc(stream);
  if (c == EOF) {
    return -1;
  }
  if (bufptr == NULL) {
    bufptr = malloc(128);
    if (bufptr == NULL) {
      return -1;
    }
    size = 128;
  }
  p = bufptr;
  while(c != EOF) {
    if ((p - bufptr) > (size - 1)) {
      size = size + 128;
      bufptr = realloc(bufptr, size);
      if (bufptr == NULL) {
        return -1;
      }
    }
    *p++ = c;
    if (c == '\n') {
      break;
    }
    c = fgetc(stream);
  }

  *p++ = '\0';
  *lineptr = bufptr;
  *n = size;

  return p - bufptr - 1;
}
#endif

GCClass* ioFileClass;

static Constant sglClock(VM* vm, int arity, Constant* args) {
  return NUM_CONST((double)clock());
}

static Constant sglReadLine(VM* vm, int arity, Constant* args) {
  char* line = NULL;
  size_t len;

  size_t lineLen = getline(&line, &len, stdin);

  return GC_OBJ_CONST(copyString(vm, NULL, line, lineLen));
}

static Constant sglSleep(VM* vm, int arity, Constant* args) {
  expect(1, arity, "seconds");

  int seconds = AS_NUMBER(args[0]);
  return NUM_CONST(sleep(seconds));
}

static Constant sglRunCmd(VM* vm, int arity, Constant* args) {
  expect(1, arity, "runCmd");

  GCString* commandStr = AS_STRING(args[0]);
  char* command = commandStr->chars;

  FILE *fp;

  char* output = "";
  int outputLength = 0;

  char line[1035];

  char* fullCommand = ALLOCATE(vm, NULL, char, commandStr->length + 6);
  memcpy(fullCommand, command, commandStr->length);
  memcpy(fullCommand + commandStr->length, " 2>&0", 5);
  fullCommand[commandStr->length + 5] = '\0';

  fp = popen(fullCommand, "r");

  if (fp == NULL)
    return NULL_CONST;

  while (fgets(line, sizeof(line), fp) != NULL) {
    int lineLength = strlen(line);
    int newLength = outputLength + lineLength;

    char* chars = ALLOCATE(vm, NULL, char, newLength + 1);

    memcpy(chars, output, outputLength);
    memcpy(chars + outputLength, line, lineLength);
    chars[newLength] = '\0';

    output = chars;
    outputLength = newLength;
  }

  pclose(fp);

  return GC_OBJ_CONST(copyString(vm, NULL, output, strlen(output)));
}

static Constant sglSystem(VM* vm, int arity, Constant* args) {
  expect(1, arity, "system");

  return NUM_CONST(system(AS_CSTRING(args[0])));
}

static Constant sglPOpen(VM* vm, int arity, Constant* args) {
  expect(2, arity, "popen");

  char* command = AS_CSTRING(args[0]);
  char* mode = AS_CSTRING(args[1]);

  return GC_OBJ_CONST(newFile(vm, ioFileClass, popen(command, mode)));
}

static Constant sglPClose(VM* vm, int arity, Constant* args) {
  expect(1, arity, "pclose");

  return NUM_CONST(pclose(AS_FILE(args[0])->file));
}

Constant initIoLib(VM* vm, int arity, Constant* args) {
  ioFileClass = newClass(vm, copyString(vm, NULL, "File", 4), true);

  defineGlobal(vm, "CLOCKS_PER_SEC", NUM_CONST(CLOCKS_PER_SEC));
  defineGlobalFunc(vm, "clock", sglClock);
  defineGlobalFunc(vm, "readln", sglReadLine);
  defineGlobalFunc(vm, "sleep", sglSleep);

  defineGlobalFunc(vm, "runCmd", sglRunCmd);
  defineGlobalFunc(vm, "system", sglSystem);
  defineGlobalFunc(vm, "popen", sglPOpen);
  defineGlobalFunc(vm, "pclose", sglPClose);
}