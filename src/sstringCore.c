#include <ctype.h>
#include "includes/sstringCore.h"
#include "includes/sparser.h"
#include "includes/sapi.h"

static Constant stringFromByte(VM* vm, int arity, Constant *args) {
  expect(1, arity, "fromByte");

  int byte = (int)AS_NUMBER(args[0]);

  if (byte < 0) {
    fprintf(stderr, "Byte must be non-negative\n");
    exit(1);
  }

  if (byte >= 0xff) {
    fprintf(stderr, "Byte must be less than 255\n");
    exit(1);
  }

  char string[1] = {(uint8_t)byte};

  return GC_OBJ_CONST(copyString(vm, NULL, string, 1));
}

static Constant stringByteAt(VM* vm, int arity, Constant *args) {
  expect(1, arity, "at");

  GCString* string = AS_STRING(args[-1]);
  int index = (int)AS_NUMBER(args[0]);

  if (index >= string->length || index < 0) {
    fprintf(stderr, "Index out of range (0-%d): %d\n", string->length, index);
    exit(1);
  }

  char at[1] = {string->chars[index]};

  return GC_OBJ_CONST(copyString(vm, NULL, at, 1));
}

static Constant stringContains(VM* vm, int arity, Constant *args) {
  expect(1, arity, "contains");

  GCString* string = AS_STRING(args[-1]);
  GCString* contain = AS_STRING(args[0]);

  if (contain->length > string->length)
    return BOOL_CONST(false);

  if (strstr(string->chars, contain->chars) != NULL)
    return BOOL_CONST(true);

  return BOOL_CONST(false);
}

static Constant stringEndsWith(VM* vm, int arity, Constant *args) {
  expect(1, arity, "endsWith");

  GCString* string = AS_STRING(args[-1]);
  GCString* contain = AS_STRING(args[0]);

  if (contain->length > string->length)
    return BOOL_CONST(false);

  int start = string->length - contain->length;

  return BOOL_CONST(!memcmp(string->chars + start, contain->chars, contain->length));
}

static Constant stringIndexOf(VM* vm, int arity, Constant *args) {
  expect(2, arity, "indexOf");

  GCString* string = AS_STRING(args[-1]);
  GCString* contain = AS_STRING(args[0]);
  int start = (int)AS_NUMBER(args[1]);

  if (contain->length > string->length)
    return NUM_CONST(-1);

  char* res = strstr(string->chars + start, contain->chars);

  if (res == NULL)
    return NUM_CONST(-1);

  return NUM_CONST(res - string->chars);
}

static Constant stringSplit(VM* vm, int arity, Constant *args) {
  expect(1, arity, "split");

  char* delim = AS_CSTRING(args[0]);
  char* str = AS_CSTRING(args[-1]);

  int count = 1;

  strtok(str, delim);
  while (strtok(NULL, delim) != NULL)
    count++;

  GCList* splitList = newList(vm, count);

  splitList->elementC = count;

  for (int i = 0; i < count; i++) {
    int length = strlen(str);
    splitList->elements[splitList->elementC - i - 1] = GC_OBJ_CONST(copyString(vm, NULL, str, length));
    str += length + 1;
  }

  return GC_OBJ_CONST(splitList);
}

static Constant stringStartsWith(VM* vm, int arity, Constant *args) {
  expect(1, arity, "startsWith");

  GCString* string = AS_STRING(args[-1]);
  GCString* contain = AS_STRING(args[0]);

  if (contain->length > string->length)
    return BOOL_CONST(false);

  return BOOL_CONST(!memcmp(string->chars, contain->chars, contain->length));
}

static Constant stringIsEmpty(VM* vm, int arity, Constant *args) {
  return BOOL_CONST(AS_STRING(args[-1])->chars[0] == '\0');
}

static Constant stringIsNotEmpty(VM* vm, int arity, Constant *args) {
  return BOOL_CONST(AS_STRING(args[-1])->chars[0] != '\0');
}

static Constant stringLength(VM* vm, int arity, Constant *args) {
  return NUM_CONST(AS_STRING(args[-1])->length);
}

static Constant stringToNum(VM* vm, int arity, Constant *args) {
  return NUM_CONST(strtol(AS_CSTRING(args[-1]), (char**)NULL, 10));
}

static Constant stringToLower(VM* vm, int arity, Constant *args) {
  char* string = AS_CSTRING(args[-1]);

  for ( ; *string; ++string) *string = tolower(*string);

  return args[-1];
}

static Constant stringToUpper(VM* vm, int arity, Constant *args) {
  char* string = AS_CSTRING(args[-1]);

  for ( ; *string; ++string) *string = toupper(*string);

  return args[-1];
}

static Constant stringIsAlpha(VM* vm, int arity, Constant *args) {
  return BOOL_CONST(isalpha(AS_CSTRING(args[-1])[0]));
}

static Constant stringIsAlphaNum(VM* vm, int arity, Constant *args) {
  return BOOL_CONST(isalnum(AS_CSTRING(args[-1])[0]));
}

static Constant stringIsDigit(VM* vm, int arity, Constant *args) {
  return BOOL_CONST(isdigit(AS_CSTRING(args[-1])[0]));
}

static Constant stringIsHex(VM* vm, int arity, Constant *args) {
  return BOOL_CONST(isxdigit(AS_CSTRING(args[-1])[0]));
}

static Constant stringReplace(VM* vm, int arity, Constant *args) {
  char* string = AS_CSTRING(args[-1]);
  char* search = AS_CSTRING(args[0]);
  char* replace = AS_CSTRING(args[1]);

  char  *p = NULL , *old = NULL , *newString = NULL ;
  int c = 0 , searchLen;
  searchLen = strlen(search);
  for(p = strstr(string , search) ; p != NULL ; p = strstr(p + searchLen , search))
  {
    c++;
  }
  c = (strlen(replace) - searchLen ) * c + strlen(string);
  newString = (char*)malloc(c );
  strcpy(newString , "");
  old = string;
  for(p = strstr(string , search) ; p != NULL ; p = strstr(p + searchLen , search))
  {
    strncpy(newString + strlen(newString) , old , p - old);
    strcpy(newString + strlen(newString) , replace);
    old = p + searchLen;
  }

  strcpy(newString + strlen(newString) , old);

  return GC_OBJ_CONST(copyString(vm, NULL, newString, strlen(newString)));
}

static Constant stringSubstr(VM* vm, int arity, Constant *args) {
  char* string = AS_CSTRING(args[-1]);

  double start = AS_NUMBER(args[0]);
  double end = AS_NUMBER(args[1]);

  int length = (int)(end - start) - 1;

  char sub[length];
  memcpy(sub, &string[(int)start], length);
  sub[length] = '\0';

  return GC_OBJ_CONST(copyString(vm, NULL, sub, length));
}

void initStringClass(VM *vm) {
  vm->stringClass = newClass(vm, copyString(vm, NULL, "String", 6), true);
  defineClassNativeField(vm, "type", GC_OBJ_CONST(copyString(vm, NULL, "String", 6)), vm->stringClass);

  defineClassNativeStaticFunc(vm, "fromByte", stringFromByte, vm->stringClass);

  defineClassNativeFunc(vm, "at", stringByteAt, vm->stringClass);
  defineClassNativeFunc(vm, "contains", stringContains, vm->stringClass);
  defineClassNativeFunc(vm, "endsWith", stringEndsWith, vm->stringClass);
  defineClassNativeFunc(vm, "indexOf", stringIndexOf, vm->stringClass);
  defineClassNativeFunc(vm, "split", stringSplit, vm->stringClass);
  defineClassNativeFunc(vm, "startsWith", stringStartsWith, vm->stringClass);
  defineClassNativeFunc(vm, "isEmpty", stringIsEmpty, vm->stringClass);
  defineClassNativeFunc(vm, "isNotEmpty", stringIsNotEmpty, vm->stringClass);
  defineClassNativeFunc(vm, "length", stringLength, vm->stringClass);

  defineClassNativeFunc(vm, "toNum", stringToNum, vm->stringClass);
  defineClassNativeFunc(vm, "toLower", stringToLower, vm->stringClass);
  defineClassNativeFunc(vm, "toUpper", stringToUpper, vm->stringClass);

  defineClassNativeFunc(vm, "isAlpha", stringIsAlpha, vm->stringClass);
  defineClassNativeFunc(vm, "isAlphaNum", stringIsAlphaNum, vm->stringClass);
  defineClassNativeFunc(vm, "isDigit", stringIsDigit, vm->stringClass);
  defineClassNativeFunc(vm, "isHex", stringIsHex, vm->stringClass);
  defineClassNativeFunc(vm, "replace", stringReplace, vm->stringClass);

  defineClassNativeFunc(vm, "substr", stringSubstr, vm->stringClass);

}
