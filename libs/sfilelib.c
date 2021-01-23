#include "includes/sfilelib.h"
#include "../src/includes/sparser.h"
#include "../core/includes/sapi.h"
#include "../src/includes/sgcobject_utils.h"

#include <unistd.h>
#include <wchar.h>

GCClass* fileClass;

GCFile* newFile(VM* vm, GCClass* class, FILE* file) {
  GCFile* instance = ALLOCATE_GC_OBJ(vm, GCFile, GC_INSTANCE);

  instance->class = class;
  instance->file = file;

  return instance;
}

// FILE ACCESS
static Constant sglOpenFile(VM* vm, int arity, Constant* args) {
  // var file = fOpen('path', 'mode');
  expect(2, arity, "open");

  char* filePath = AS_CSTRING(args[0]);

  // r/rb: open for read
  // w/wb: Truncate to 0 or create for write
  // a/ab: append, write to EOF
  // r+ or rb+ or r+b: Open file for read/write.
  // w+ or wb+ or w+b: Truncate to 0 or create file for read/write.
  // a+ or ab+ or a+b: Append; open or create file for read/write, writing at end-of-file.
  char* mode = AS_CSTRING(args[1]);

  FILE* file = fopen(filePath, mode);

  if (!file) {
    fprintf(stderr, "Senegal was unable to open file `%s`.\n", filePath);
    exit(74);
  }

  return GC_OBJ_CONST(newFile(vm, fileClass, file));
}

static Constant sglCloseFile(VM* vm, int arity, Constant* args) {
  expect(1, arity, "close(file)");

  return NUM_CONST(fclose(AS_FILE(args[0])->file));
}

static Constant sglExistsFile(VM* vm, int arity, Constant* args) {
  expect(1, arity, "exists(path)");

  return BOOL_CONST(!access(AS_CSTRING(args[0]), F_OK));
}

static Constant sglFlushFile(VM* vm, int arity, Constant* args) {
  expect(1, arity, "flush(file)");

  return NUM_CONST(fflush(AS_FILE(args[0])->file));
}

static Constant sglSizeFile(VM* vm, int arity, Constant* args) {
  expect(1, arity, "size(file)");

  FILE* file = AS_FILE(args[0])->file;
  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  return NUM_CONST(fileSize);
}

// FILE IO
static Constant sglReadFile(VM* vm, int arity, Constant* args) {
  expect(1, arity, "fRead(file)");

  GCFile* file = AS_FILE(args[0]);

  char* content = readFile(file->file);

  return GC_OBJ_CONST(copyString(vm, NULL, content, strlen(content)));
}

static Constant sglWriteBytesFile(VM* vm, int arity, Constant* args) {
  expect(2, arity, "writeBytes(file, List<num>)");

  GCFile* file = AS_FILE(args[0]);
  GCList* bytesList = AS_LIST(args[1]);

  double bytes[bytesList->elementC];

  for (int i = 0; i < bytesList->elementC; i ++) {
    bytes[i] = AS_NUMBER(bytesList->elements[bytesList->elementC - i]);
  }

  return NUM_CONST(fwrite(bytes, sizeof(bytes[0]), bytesList->elementC, file->file));
}

static Constant sglWriteStringFile(VM* vm, int arity, Constant* args) {
  expect(2, arity, "writeBytes(file, String)");

  GCFile* file = AS_FILE(args[0]);
  char* buffer = AS_CSTRING(args[1]);

  return NUM_CONST(fwrite(buffer, sizeof(buffer[0]), strlen(buffer), file->file));
}

Constant initFileLib(VM* vm, int arity, Constant* args) {
  GCClass* directoryClass = newClass(vm, copyString(vm, NULL, "Directory", 9), true);
  fileClass = newClass(vm, copyString(vm, NULL, "File", 4), true);

  // File access
  defineClassNativeStaticMethod(vm, "open", sglOpenFile, fileClass);
  defineClassNativeStaticMethod(vm, "close", sglCloseFile, fileClass);
  defineClassNativeStaticMethod(vm, "exists", sglExistsFile, fileClass);
  defineClassNativeStaticMethod(vm, "flush", sglFlushFile, fileClass);
  defineClassNativeStaticMethod(vm, "size", sglSizeFile, fileClass);

  // File IO
  defineClassNativeStaticMethod(vm, "read", sglReadFile, fileClass);
  defineClassNativeStaticMethod(vm, "writeBytes", sglWriteBytesFile, fileClass);
  defineClassNativeStaticMethod(vm, "writeString", sglWriteStringFile, fileClass);

  // Directory
  char cwd[260]; // PATH_MAX
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    defineClassNativeStaticField(vm, "current", GC_OBJ_CONST(copyString(vm, NULL, cwd, strlen(cwd))), directoryClass);
  }

  defineGlobal(vm, "File", GC_OBJ_CONST(fileClass));
  defineGlobal(vm, "Directory", GC_OBJ_CONST(directoryClass));
}
