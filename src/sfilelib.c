#include "includes/sfilelib.h"
#include "includes/sparser.h"
#include "includes/sapi.h"
#include "includes/sgcobject_utils.h"

#include <unistd.h>

typedef struct {
    GCObject gc;

    GCClass* class;
    FILE* file;
} GCFile;

GCClass* fileClass;

#define AS_FILE(c) ((GCFile*)AS_GC_OBJ(c))

static GCFile* newFile(VM* vm, GCClass* class, FILE* file) {
  GCFile* instance = ALLOCATE_GC_OBJ(vm, GCFile, GC_INSTANCE);

  instance->class = class;
  instance->file = file;

  return instance;
}

static Constant sglOpenFile(VM* vm, int arity, Constant* args) {
  // var file = fOpen('path', 'mode');
  expect(2, arity, "fOpen");

  char* filePath = AS_CSTRING(args[0]);

  // r/rb: open for read
  // w/wb: Truncate to 0 or create for write
  // a/ab: append, write to EOF
  // r+ or rb+ or r+b: Open file for read/write.
  // w+ or wb+ or w+b: Truncate to 0 or create file for read/write.
  // a+ or ab+ or a+b: Append; open or create file for read/write, writing at end-of-file.
  char* mode = AS_CSTRING(args[1]);

  FILE* file = fopen(filePath, mode);

  if (file == NULL) {
    fprintf(stderr, "Senegal was unable to open file `%s`.\n", filePath);
    exit(74);
  }

  return GC_OBJ_CONST(newFile(vm, fileClass, file));
}

static Constant sglReadFile(VM* vm, int arity, Constant* args) {
  expect(1, arity, "fRead(file)");

  GCFile* file = AS_FILE(args[0]);

  char* content = readFile(file->file);

  return GC_OBJ_CONST(copyString(vm, NULL, content, strlen(content)));
}

Constant initFileLib(VM* vm, int arity, Constant* args) {
  GCClass* directoryClass = newClass(vm, copyString(vm, NULL, "Directory", 9), true);
  fileClass = newClass(vm, copyString(vm, NULL, "File", 4), true);

  defineGlobalFunc(vm, "fOpen", sglOpenFile);
  defineGlobalFunc(vm, "fRead", sglReadFile);

  char cwd[260]; // PATH_MAX
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    defineClassNativeStaticField(vm, "current", GC_OBJ_CONST(copyString(vm, NULL, cwd, strlen(cwd))), directoryClass);
  }

  defineGlobal(vm, "Directory", GC_OBJ_CONST(directoryClass));
}
