#ifndef SENEGAL_SVM_H
#define SENEGAL_SVM_H

#include "sinstructions.h"
#include "stable.h"
#include "sgcobject.h"

typedef enum {
    OK,
    COMPILE_TIME_ERROR,
    RUNTIME_ERROR
} InterpretationResult;

typedef struct sVM VM;

typedef Constant (*NativeFunc)(VM* vm, int arity, Constant* args);

typedef struct {
    GCObject gc;
    NativeFunc function;
} GCNative;

struct sVM {
    GCCoroutine* coroutine;

    size_t bytesAllocated;
    size_t nextGC;

    GCObject* gcObjects;

    Table globals;
    Table strings;
    Table corePaths;
    Table imports;

    GCClass* boolClass;
    GCClass* coroutineClass;
    GCClass* listClass;
    GCClass* mapClass;
    GCClass* numClass;
    GCClass* stringClass;

    int grayCount;
    int grayCapacity;
    GCObject** grayStack;
};

void initVM(VM* vm);
GCNative* newNative(VM* vm, NativeFunc function);

InterpretationResult run(register VM* vm);
InterpretationResult interpret(VM* vm, char* file, char* source, char* senegalPath, char* dir);

bool call(VM* vm, GCClosure* closure, int arity);

void push(VM* vm, Constant constant);
Constant pop(VM* vm);

#endif //SENEGAL_SVM_H