#include <stdio.h>
#include <malloc.h>
#include "includes/sconstant.h"
#include "includes/smemory.h"

void initConstantPool(ConstantPool* cp) {
  cp->constants = NULL;
  cp->capacity = 0;
  cp->count = 0;
}

static void printFunction(GCFunction* function) {
  if (function == NULL)
    return;

  if (function->id == NULL) {
    printf("<Senegal Program>");
    return;
  }

  printf("<Senegal Function %s>", function->id->chars);
}

void printConstant(Constant constant) {

#if NAN_TAGGING
  if (IS_BOOL(constant)) {
    printf(AS_BOOL(constant) ? "true" : "false");
  } else if (IS_LIST(constant)) {
    GCList* list = AS_LIST(constant);
    printf("[");

    for (int i = 0; i < list->elementC; i++) {
      printConstant(list->elements[(list->elementC - 1) - i]);

      if (i != list->elementC - 1)
        printf(",");
    }

    printf("]");

  } else if (IS_NULL(constant)) {
    printf("null");
  } else if (IS_NUMBER(constant)) {
    printf("%.16g", AS_NUMBER(constant));
  } else if (IS_GC_OBJ(constant)) {

    switch (GC_OBJ_TYPE(constant)) {
      case GC_CLASS:
        printf("%s", AS_CLASS(constant)->id->chars);
        break;

      case GC_CLOSURE:
        printFunction(AS_CLOSURE(constant)->function);
        break;

      case GC_FUNCTION:
        printFunction(AS_FUNCTION(constant));
        break;

      case GC_INSTANCE:
        printf("Instance of %s", AS_INSTANCE(constant)->class->id->chars);
        break;

      case GC_INSTANCE_METHOD:
        printFunction(AS_INSTANCE_METHOD(constant)->method->function);
        break;

      case GC_NATIVE:
        printf("<Senegal Native Function>");
        break;

      case GC_STRING:
        printf("%s", AS_CSTRING(constant));
        break;

      case GC_UPVALUE:
        printf("Upvalue");
        break;
    }
  }
#else

  switch (constant.type) {
    case TYPE_NULL:
      printf("null");
      break;

    case TYPE_BOOL:
      printf(AS_BOOL(constant) ? "true" : "false");
      break;

    case TYPE_NUM:
      printf("%g", AS_NUMBER(constant));
      break;

    case TYPE_GC_OBJ: {
      switch (GC_OBJ_TYPE(constant)) {

        case GC_CLASS:
          printf("%s", AS_CLASS(constant)->id->chars);
          break;

        case GC_CLOSURE:
          printFunction(AS_CLOSURE(constant)->function);
          break;

        case GC_FUNCTION:
          printFunction(AS_FUNCTION(constant));
          break;

        case GC_INSTANCE:
          printf("Instance of %s", AS_INSTANCE(constant)->class->id->chars);
          break;

        case GC_INSTANCE_METHOD:
          printFunction(AS_INSTANCE_METHOD(constant)->method->function);
          break;

        case GC_NATIVE:
          printf("<Senegal Native Function>");
          break;

        case GC_STRING:
          printf("%s", AS_CSTRING(constant));
          break;

        case GC_UPVALUE:
          printf("Upvalue");
          break;
      }
    }

    default:
      return;
  }

#endif
}

// TODO(Calamity210): Correct equality check ifn NAN_TAGGING
bool areEqual(Constant a, Constant b) {
#if NAN_TAGGING
  if(a == b)
    return true;
#else
  if (a.type != b.type)
    return false;

  if (!IS_NUMBER(a) && AS_NUMBER(a) == AS_NUMBER(b))
    return true;

  if (AS_GC_OBJ(a) == AS_GC_OBJ(b))
    return true;
#endif

  if(!IS_GC_OBJ(a) || !IS_GC_OBJ(b))
    return false;

  GCObject* aGCOBJ = AS_GC_OBJ(a);
  GCObject* bGCOBJ = AS_GC_OBJ(b);

  if (aGCOBJ->type != bGCOBJ->type)
    return false;


  switch (aGCOBJ->type) {

    case GC_STRING: {
      GCString* stringOne = (GCString*)aGCOBJ;
      GCString* stringTwo = (GCString*)bGCOBJ;
      return stringOne->hash == stringTwo->hash &&
             stringOne->length == stringTwo->length &&
             memcmp(stringOne->chars, stringTwo->chars, stringOne->length) == 0;
    }

    default:
      return false;
  }
}