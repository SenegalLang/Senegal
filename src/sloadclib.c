#include "includes/sloadclib.h"
#include "includes/sutils.h"

// Implementation based on lua loadlib.c
#ifdef USE_DLOPEN
#include <dlfcn.h>

void sysUnloadLib(void* lib) {
  dlclose(lib);
}

void* sysLoad(const char* path, int seeglb) {
  void* lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));

  if (!lib) {
    fprintf(stderr, "%s\n", dlerror());
    exit(1);
  }

  return lib;
}

Constant sysSym(VM* vm, void* lib, const char* sym) {

  void* func = dlsym(lib, sym);

  if (!func) {
    fprintf(stderr, "%s\n", dlerror());
    exit(1);
  }

  GCNative* c = newNative(vm, (NativeFunc)func);

  return GC_OBJ_CONST(c);
}

#else
#include <windows.h>

static void sglThrow() {
  int error = GetLastError();
  char buffer[128];

  if (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, error, 0, buffer, sizeof(buffer)/sizeof(char), NULL)) {
    fprintf(stderr, "%s\n", buffer);
    exit(1);
  }
}

void sysUnloadLib(void* lib) {
  FreeLibrary((HMODULE)lib);
}

void* sysLoad(const char* path, int seeglb) {
  HMODULE lib = LoadLibraryExA(path, NULL, 0);
  (void)(seeglb);
  if (!lib)
    sglThrow();

  return lib;
}

Constant sysSym(VM* vm, void* lib, const char* sym) {
   GCNative* c = newNative(vm, (NativeFunc)GetProcAddress((HMODULE)lib, sym));

  if (!c)
    sglThrow();

  return GC_OBJ_CONST(c);
}

#endif
