#include <stdio.h>
#include "includes/sconstant.h"
#include "includes/smemory.h"

void initConstantPool(ConstantPool* cp) {
  cp->constants = NULL;
  cp->capacity = 0;
  cp->count = 0;
}

static void printFunction(FILE* file, GCFunction* function) {
  if (!function)
    return;

  if (!function->id) {
    fprintf(file, "<Senegal Program>");
    return;
  }

  fprintf(file,"<Senegal Function %s>", function->id->chars);
}

void printConstant(FILE* file, Constant constant) {

#if NAN_TAGGING
  if (IS_BOOL(constant)) {
    fprintf(file, AS_BOOL(constant) ? "true" : "false");
  } else if (IS_LIST(constant)) {
    GCList* list = AS_LIST(constant);
    fprintf(file, "[");

    for (int i = 0; i < list->elementC; i++) {
      printConstant(file, list->elements[(list->elementC - 1) - i]);

      if (i != list->elementC - 1)
        fprintf(file, ",");
    }

    fprintf(file, "]");

  } else if (IS_NULL(constant)) {
    fprintf(file, "null");
  } else if (IS_NUMBER(constant)) {
    fprintf(file, "%.16g", AS_NUMBER(constant));
  } else if (IS_GC_OBJ(constant)) {

    switch (GC_OBJ_TYPE(constant)) {
      case GC_CLASS:
        fprintf(file, "%s", AS_CLASS(constant)->id->chars);
        break;

      case GC_COROUTINE:
        fprintf(file, "Coroutine");
        break;

      case GC_CLOSURE:
        printFunction(file, AS_CLOSURE(constant)->function);
        break;

      case GC_FUNCTION:
        printFunction(file, AS_FUNCTION(constant));
        break;

      case GC_INSTANCE:
        fprintf(file, "Instance of %s", AS_INSTANCE(constant)->class->id->chars);
        break;

      case GC_INSTANCE_METHOD:
        printFunction(file, AS_INSTANCE_METHOD(constant)->method->function);
        break;

      case GC_NATIVE:
        fprintf(file, "<Senegal Native Function>");
        break;

      case GC_STRING:
        fprintf(file, "%s", AS_CSTRING(constant));
        break;

      case GC_UPVALUE:
        fprintf(file, "Upvalue");
        break;
    }
  }
#else

  switch (constant.type) {
    case TYPE_NULL:
      fprintf(file, "null");
      break;

    case TYPE_BOOL:
      fprintf(file, AS_BOOL(constant) ? "true" : "false");
      break;

    case TYPE_NUM:
      fprintf(file, "%.16g", AS_NUMBER(constant));
      break;

    case TYPE_GC_OBJ: {
      switch (GC_OBJ_TYPE(constant)) {

        case GC_CLASS:
          fprintf(file, "%s", AS_CLASS(constant)->id->chars);
          break;

        case GC_CLOSURE:
          printFunction(file, AS_CLOSURE(constant)->function);
          break;

        case GC_COROUTINE:
          fprintf(file, "Coroutine");
          break;

        case GC_FUNCTION:
          printFunction(file, AS_FUNCTION(constant));
          break;

        case GC_INSTANCE:
          fprintf(file, "Instance of %s", AS_INSTANCE(constant)->class->id->chars);
          break;

        case GC_INSTANCE_METHOD:
          printFunction(file, AS_INSTANCE_METHOD(constant)->method->function);
          break;

        case GC_LIST: {
          GCList* list = AS_LIST(constant);
          fprintf(file, "[");

          for (int i = 0; i < list->elementC; i++) {
            printConstant(file, list->elements[(list->elementC - 1) - i]);

            if (i != list->elementC - 1)
              fprintf(file, ",");
          }

          fprintf(file, "]");
          break;
        }

        case GC_NATIVE:
          fprintf(file, "<Senegal Native Function>");
          break;

        case GC_STRING:
          fprintf(file, "%s", AS_CSTRING(constant));
          break;

        case GC_UPVALUE:
          fprintf(file, "Upvalue");
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