#ifndef SENEGAL_SCONSTANT_H
#define SENEGAL_SCONSTANT_H

#include <string.h>
#include "sutils.h"

typedef struct sGCObject GCObject;
typedef struct sGCString GCString;

#if NAN_TAGGING

#define SIGN_BIT ((uint64_t)0x8000000000000000)
#define QNAN ((uint64_t)0x7ffc000000000000)

#define TAG_NULL 1
#define TAG_FALSE 2
#define TAG_TRUE 3

typedef uint64_t Constant;

#define AS_NUMBER(c) valueToNumber(c)
#define IS_NUMBER(c) (((c) & QNAN) != QNAN)
#define NUM_CONST(c) numToConstant(c)

#define IS_NULL(c) ((c) == NULL_CONST)
#define NULL_CONST ((Constant)(uint64_t)(QNAN | TAG_NULL))

#define AS_BOOL(c) ((c) == TRUE_CONST)
#define IS_BOOL(c) (((c) | 1) == TRUE_CONST)
#define FALSE_CONST ((Constant)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_CONST ((Constant)(uint64_t)(QNAN | TAG_TRUE))
#define BOOL_CONST(c) ((c) ? TRUE_CONST : FALSE_CONST)

#define AS_GC_OBJ(gc) ((GCObject*)(uintptr_t)((gc) & ~(SIGN_BIT | QNAN)))
#define IS_GC_OBJ(gc) (((gc) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))
#define GC_OBJ_CONST(gc) (Constant)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(gc))

typedef union
{
    uint64_t bits64;
    uint32_t bits32[2];
    double num;
} SenegalDoubleBits;

static inline double valueToNumber(uint64_t constant) {
  SenegalDoubleBits data;
  data.bits64 = constant;
  return data.num;
}

static inline Constant numToConstant(double num) {
  SenegalDoubleBits data;
  data.num = num;
  return data.bits64;
}
#else

typedef enum {
 TYPE_BOOL,
 TYPE_GC_OBJ,
 TYPE_NULL,
 TYPE_NUM
} ConstantType;

typedef struct {
  ConstantType type;

  union {
      bool boolean;
      double num;
      GCObject* gc;
  } as;
} Constant;

#define IS_BOOL(c)    ((c).type == TYPE_BOOL)
#define IS_GC_OBJ(c)  ((c).type == TYPE_GC_OBJ)
#define IS_NULL(c)    ((c).type == TYPE_NULL)
#define IS_NUMBER(c)  ((c).type == TYPE_NUM)

#define AS_BOOL(c) ((c).as.boolean)
#define AS_GC_OBJ(c) ((c).as.gc)
#define AS_NUMBER(c) ((c).as.num)

#define BOOL_CONST(c) ((Constant){TYPE_BOOL, {.boolean = (c)}})
#define GC_OBJ_CONST(c) ((Constant){TYPE_GC_OBJ, {.gc = (GCObject*)(c)}})
#define NULL_CONST ((Constant){TYPE_NULL, {.num = 0}})
#define NUM_CONST(c) ((Constant){TYPE_NUM, {.num = (c)}})

#endif

typedef struct {
    Constant* constants;
    int capacity;
    int count;
} ConstantPool;

bool areEqual(Constant a, Constant b);

void initConstantPool(ConstantPool* cp);

char* constantToString(Constant constant);
void printConstant(FILE* file, Constant constant);

typedef enum {
    GC_CLASS,
    GC_CLOSURE,
    GC_COROUTINE,
    GC_FUNCTION,
    GC_INSTANCE,
    GC_INSTANCE_METHOD,
    GC_LIST,
    GC_MAP,
    GC_NATIVE,
    GC_STRING,
    GC_UPVALUE
} GCObjectType;

struct sGCObject {
    GCObjectType type;
    struct sGCObject* next;

    bool isMarked;
};

struct sGCString {
    GCObject gc;
    int length;
    char* chars;

    uint32_t hash;
};

static bool isGCType(Constant c, GCObjectType type) {
  return IS_GC_OBJ(c) && AS_GC_OBJ(c)->type == type;
}

#define GC_OBJ_TYPE(c) (AS_GC_OBJ(c)->type)
#define IS_STRING(c) isGCType(c, GC_STRING)

#define AS_STRING(c) ((GCString*)AS_GC_OBJ(c))
#define AS_CSTRING(c) (((GCString*)AS_GC_OBJ(c))->chars)

#endif //SENEGAL_SCONSTANT_H