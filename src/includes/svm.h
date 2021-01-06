#ifndef SENEGAL_SVM_H
#define SENEGAL_SVM_H

#include "sinstructions.h"
#include "stable.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef enum {
    OK,
    COMPILE_TIME_ERROR,
    RUNTIME_ERROR
} InterpretationResult;

typedef enum {
    CONSTRUCTOR,
    TYPE_FUNCTION,
    METHOD,
    PROGRAM
} FunctionType;

typedef enum {
    ROOT,
    TRY,
    OTHER
} CoroutineState;

typedef struct {
    GCObject gc;
    int arity;
    Instructions instructions;
    GCString* id;

    int upvalueCount;
} GCFunction;

typedef struct sVM VM;

typedef Constant (*NativeFunc)(VM* vm, int arity, Constant* args);

typedef struct {
    uint8_t index;
    bool isLocal;
} Upvalue;

typedef struct {
    GCObject gc;
    NativeFunc function;
} GCNative;

typedef struct GCUpvalue {
    GCObject gc;
    Constant closed;

    Constant* place;
    struct GCUpvalue* next;
} GCUpvalue;

typedef struct {
    GCObject gc;
    GCFunction* function;

    int upvalueCount;
    GCUpvalue** upvalues;
} GCClosure;

typedef struct {
    GCObject gc;
    GCString* id;

    bool isFinal;

    Table fields;
    Table methods;
    Table staticFields;
    Table staticMethods;
} GCClass;

typedef struct {
    GCObject gc;
    GCClass* class;
} GCInstance;

typedef struct {
    GCObject gc;
    Constant receiver;
    GCClosure* method;
} GCInstanceMethod;

typedef struct {
    GCObject gc;
    Table table;
} GCMap;

typedef struct {
    GCObject gc;
    Constant* elements;
    int elementC;
    int listCurrentCap;
} GCList;

#define IS_CLASS(c) isGCType(c, GC_CLASS)
#define IS_FUNCTION(c) isGCType(c, GC_FUNCTION)
#define IS_INSTANCE(c) isGCType(c, GC_INSTANCE)
#define IS_INSTANCE_METHOD(c) isGCType(c, GC_INSTANCE_METHOD)
#define IS_LIST(c) isGCType(c, GC_LIST)
#define IS_MAP(c) isGCType(c, GC_MAP)
#define IS_NATIVE(c) isGCType(c, GC_NATIVE)
#define IS_CLOSURE(c) isGCType(c, GC_CLOSURE)

#define AS_CLASS(c) ((GCClass*)AS_GC_OBJ(c))
#define AS_FUNCTION(c) ((GCFunction*)AS_GC_OBJ(c))
#define AS_INSTANCE(c) ((GCInstance*)AS_GC_OBJ(c))
#define AS_INSTANCE_METHOD(c) ((GCInstanceMethod*)AS_GC_OBJ(c))
#define AS_LIST(c) ((GCList*)AS_GC_OBJ(c))
#define AS_MAP(c) ((GCMap*)AS_GC_OBJ(c))
#define AS_NATIVE(c) (((GCNative*)AS_GC_OBJ(c))->function)
#define AS_CLOSURE(c) (((GCClosure*)AS_GC_OBJ(c)))

typedef struct {
    GCClosure* closure;
    uint8_t* pc;
    Constant* constants;
} CallFrame;

typedef struct sCoroutine {
    GCObject gc;

    Constant stack[STACK_MAX];
    Constant* stackTop;

    CallFrame frames[FRAMES_MAX];
    int frameCount;

    GCUpvalue* openUpvalues;

    struct sCoroutine* caller;

    Constant* error;

    CoroutineState state;
} GCCoroutine;

#define IS_COROUTINE(c) isGCType(c, GC_COROUTINE)
#define AS_COROUTINE(c) ((GCCoroutine*)AS_GC_OBJ(c))

struct sVM {
    GCCoroutine* coroutine;

    size_t bytesAllocated;
    size_t nextGC;

    GCObject* gcObjects;

    Table globals;
    Table strings;
    Table corePaths;

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


void initVM(VM* vm, char* senegalPath);
GCCoroutine* newCoroutine(VM* vm, CoroutineState state, GCClosure* closure);
InterpretationResult run(register VM* vm);

bool call(VM* vm, GCClosure* closure, int arity);

InterpretationResult interpret(VM* vm, char* source, const char* senegalPath, char* dir);

void push(VM* vm, Constant constant);
Constant pop(VM* vm);

GCClass* newClass(VM* vm, GCString* id, bool isFinal);
GCFunction* newFunction(VM* vm);
GCInstance* newInstance(VM* vm, GCClass* class);
GCInstanceMethod* newInstanceMethod(VM* vm, Constant receiver, GCClosure* method);
GCList* newList(VM *vm, int length);
GCMap* newMap(VM *vm);
GCNative* newNative(VM* vm, NativeFunc function);
GCClosure* newClosure(VM* vm, GCFunction* function);
GCUpvalue* newUpvalue(VM* vm, Constant* constant);

#endif //SENEGAL_SVM_H