#include <stdio.h>
#include <float.h>
#include "includes/sconstant.h"
#include "includes/smemory.h"

void initConstantPool(ConstantPool* cp) {
  cp->constants = NULL;
  cp->capacity = 0;
  cp->count = 0;
}

static char* functionToString(GCFunction* function) {
  if (!function)
    return "";

  if (!function->id)
    return "<Senegal Program>";

  return function->id->chars;
}

static char* listToString(GCList* list) {

  char* string = "[";
  int length = 1;

  for (int i = 0; i < list->elementC; i++) {
    if (i != list->elementC - 1) {
      char *elementStr = constantToString(list->elements[(list->elementC - 1) - i]);
      int elementLen = (int) strlen(elementStr);
      int newLen = length + elementLen;

      char *newChars = malloc(newLen + 2);
      memcpy(newChars, string, length);
      memcpy(newChars + length, elementStr, elementLen);
      newChars[newLen] = ',';
      newChars[newLen + 1] = '\0';

      string = newChars;
      length = newLen + 1;
    } else {
      char *elementStr = constantToString(list->elements[(list->elementC - 1) - i]);
      int elementLen = (int) strlen(elementStr);
      int newLen = length + elementLen;

      char *newChars = malloc(newLen + 1);
      memcpy(newChars, string, length);
      memcpy(newChars + length, elementStr, elementLen);
      newChars[newLen] = '\0';

      string = newChars;
      length = newLen;
    }
  }

  length++;
  char *newChars = malloc(length + 1);
  memcpy(newChars, string, length - 1);
  memcpy(newChars + (length - 1), "]", 1);
  newChars[length] = '\0';
  string = newChars;

  return string;
}

static char* mapToString(GCMap* map) {
  char* string = "{";
  int length = 1;

  for (int i = 0; i < map->table.cap; i++) {
      if (IS_NULL(map->table.entries[i].key))
        continue;

    char *keyStr = constantToString(map->table.entries[i].key);
    int keyLen = (int) strlen(keyStr);
    int newKeyLen = length + keyLen;

    char *newKeyChars = malloc(newKeyLen + 3);
    memcpy(newKeyChars, string, length);
    memcpy(newKeyChars + length, keyStr, keyLen);
    newKeyChars[newKeyLen] = ':';
    newKeyChars[newKeyLen + 1] = ' ';
    newKeyChars[newKeyLen + 2] = '\0';

    string = newKeyChars;
    length = newKeyLen + 2;

    char *elementStr = constantToString(map->table.entries[i].constant);
    int elementLen = (int) strlen(elementStr);
    int newLen = length + elementLen;

    char *newChars = malloc(newLen + 2);
    memcpy(newChars, string, length);
    memcpy(newChars + length, elementStr, elementLen);
    newChars[newLen] = ',';
    newChars[newLen + 1] = '\0';

    string = newChars;
    length = newLen + 1;
  }

  string[length - 1] = '}';

  return string;
}

char* constantToString(Constant constant) {
#if NAN_TAGGING
  if (IS_BOOL(constant)) {
    return AS_BOOL(constant) ? "true" : "false";
  } else if (IS_NULL(constant)) {
    return "null";
  } else if (IS_NUMBER(constant)) {
    char* numString = malloc(3 + DBL_MANT_DIG - DBL_MIN_EXP);
    sprintf(numString, "%.16g", AS_NUMBER(constant));

    return numString;
  } else if (IS_GC_OBJ(constant)) {
    switch (GC_OBJ_TYPE(constant)) {
      case GC_CLASS:
        return AS_CLASS(constant)->id->chars;
      case GC_COROUTINE:
        return "Coroutine";
      case GC_CLOSURE:
        return functionToString(AS_CLOSURE(constant)->function);
      case GC_FUNCTION:
        return functionToString(AS_FUNCTION(constant));
      case GC_INSTANCE: {
        char* origId = AS_INSTANCE(constant)->class->id->chars;
        int idLen = (int)strlen(AS_INSTANCE(constant)->class->id->chars);

        char* id = malloc(13 + idLen);
        memcpy(id, "Instance of ", 12);
        memcpy(id + 12, origId, idLen);
        id[idLen + 13] = '\0';

        return id;
      }
      case GC_INSTANCE_METHOD:
        return functionToString(AS_INSTANCE_METHOD(constant)->method->function);
      case GC_NATIVE:
        return "<Senegal Native Function>";
      case GC_STRING:
        return AS_CSTRING(constant);
      case GC_UPVALUE:
        return "Upvalue";
      case GC_LIST:
        return listToString(AS_LIST(constant));
      case GC_MAP:
        return mapToString(AS_MAP(constant));
    }
  }
#else
  switch (constant.type) {
    case TYPE_NULL:
      return "null";

    case TYPE_BOOL:
      return AS_BOOL(constant) ? "true" : "false";

    case TYPE_NUM: {
      char* numString = malloc(3 + DBL_MANT_DIG - DBL_MIN_EXP);
      sprintf(numString, "%.16g", AS_NUMBER(constant));
      return numString;
    }

    case TYPE_GC_OBJ: {
      switch (GC_OBJ_TYPE(constant)) {

        case GC_CLASS:
          return AS_CLASS(constant)->id->chars;

        case GC_CLOSURE:
          return functionToString(AS_CLOSURE(constant)->function);

        case GC_COROUTINE:
          return "Coroutine";

        case GC_FUNCTION:
          return functionToString(AS_FUNCTION(constant));

        case GC_INSTANCE: {
          char* origId = AS_INSTANCE(constant)->class->id->chars;
          int idLen = (int)strlen(AS_INSTANCE(constant)->class->id->chars);

          char* id = malloc(13 + idLen);
          memcpy(id, "Instance of ", 12);
          memcpy(id + 12, origId, idLen);
          id[idLen + 13] = '\0';

          return id;
        }

        case GC_INSTANCE_METHOD:
          return functionToString(AS_INSTANCE_METHOD(constant)->method->function);

        case GC_LIST:
          return listToString(AS_LIST(constant));

        case GC_MAP:
          return mapToString(AS_MAP(constant));

        case GC_NATIVE:
          return "<Senegal Native Function>";

        case GC_STRING:
          return AS_CSTRING(constant);

        case GC_UPVALUE:
          return "Upvalue";
      }
    }

    default:
      return;
  }
#endif
}


void printConstant(FILE* file, Constant constant) {
  fprintf(file, "%s", constantToString(constant));
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