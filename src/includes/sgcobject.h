#ifndef SENEGAL_SGCOBJECT_H
#define SENEGAL_SGCOBJECT_H

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

#define IS_CLASS(c) isGCType(c, GC_CLASS)
#define IS_FUNCTION(c) isGCType(c, GC_FUNCTION)
#define IS_INSTANCE(c) isGCType(c, GC_INSTANCE)
#define IS_INSTANCE_METHOD(c) isGCType(c, GC_INSTANCE_METHOD)
#define IS_LIST(c) isGCType(c, GC_LIST)
#define IS_MAP(c) isGCType(c, GC_MAP)
#define IS_NATIVE(c) isGCType(c, GC_NATIVE)
#define IS_CLOSURE(c) isGCType(c, GC_CLOSURE)
#define IS_COROUTINE(c) isGCType(c, GC_COROUTINE)

#define AS_CLASS(c) ((GCClass*)AS_GC_OBJ(c))
#define AS_FUNCTION(c) ((GCFunction*)AS_GC_OBJ(c))
#define AS_INSTANCE(c) ((GCInstance*)AS_GC_OBJ(c))
#define AS_INSTANCE_METHOD(c) ((GCInstanceMethod*)AS_GC_OBJ(c))
#define AS_LIST(c) ((GCList*)AS_GC_OBJ(c))
#define AS_MAP(c) ((GCMap*)AS_GC_OBJ(c))
#define AS_NATIVE(c) (((GCNative*)AS_GC_OBJ(c))->function)
#define AS_CLOSURE(c) (((GCClosure*)AS_GC_OBJ(c)))
#define AS_COROUTINE(c) ((GCCoroutine*)AS_GC_OBJ(c))

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

    GCString* id;
    Instructions instructions;
    int arity;
    int upvalueCount;

    bool isExternal;
} GCFunction;

typedef struct {
    uint8_t index;
    bool isLocal;
} Upvalue;

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
    GCClosure* closure;
    uint8_t* pc;
    Constant* constants;
} CallFrame;

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

#endif //SENEGAL_SGCOBJECT_H
