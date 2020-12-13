#ifndef SENEGAL_SFILELIB_H
#define SENEGAL_SFILELIB_H

#include "svm.h"

typedef struct {
    GCObject gc;

    GCClass* class;
    FILE* file;
} GCFile;

GCClass* fileClass;

#define AS_FILE(c) ((GCFile*)AS_GC_OBJ(c))

Constant initFileLib(VM* vm, int arity, Constant* args);
GCFile* newFile(VM* vm, GCClass* class, FILE* file);

#endif //SENEGAL_SFILELIB_H
