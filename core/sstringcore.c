#include <ctype.h>
#include "includes/sstringcore.h"
#include "../src/includes/sparser.h"
#include "includes/sapi.h"
#include "includes/slistcore.h"

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

static int indexOf(char* string, char* contain, int start) {
  int strLen = strlen(string);
  int containLen = strlen(contain);

  if (containLen > strLen)
    return -1;

  char* res = strstr(string + start, contain);

  if (!res)
    return -1;

  return res - string;
}

static char* substring(char* string, int start, int end) {
  int length = (int)(end - start) - 1;

  char *sub = malloc(length);
  memcpy(sub, &string[(int)start], length);
  sub[length] = '\0';

  return sub;
}

static Constant stringIndexOf(VM* vm, int arity, Constant *args) {
  expect(2, arity, "indexOf");

  char* string = AS_CSTRING(args[-1]);
  char* contain = AS_CSTRING(args[0]);
  int start = (int)AS_NUMBER(args[1]);

  return NUM_CONST(indexOf(string, contain, start));
}

static Constant stringSplit(VM* vm, int arity, Constant *args) {
  expect(1, arity, "split");

  GCString* delimStr = AS_STRING(args[0]);
  char* delim = delimStr->chars;
  int delimLen = delimStr->length;

  GCString* string = AS_STRING(args[-1]);
  char* str = string->chars;
  int strLen = string->length;

  GCList* result = newList(vm, 0);

  int index = 0;
  int last = 0;

  while (last < strLen && (index = indexOf(str, delim, last)) != -1) {
    addToList(vm, &result, GC_OBJ_CONST(copyString(vm, NULL, substring(str, last, index + 1), index - last)));
    last = index + delimLen;
  }

  if (last < strLen) {
    addToList(vm, &result, GC_OBJ_CONST(copyString(vm, NULL, substring(str, last, strLen + 1), strLen - last)));
  }

  return GC_OBJ_CONST(result);
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
  GCString* string = AS_STRING(args[-1]);
  bool isAlpha = true;

  for (int i = 0; i < string->length; i++) {
    if (!isAlpha)
      break;

    isAlpha = isalpha(string->chars[i]);
  }


  return BOOL_CONST(isAlpha);
}

static Constant stringIsAlphaNum(VM* vm, int arity, Constant *args) {
  GCString* string = AS_STRING(args[-1]);
  bool isAlphanum = true;

  for (int i = 0; i < string->length; i++) {
    if (!isAlphanum)
      break;

    isAlphanum = isalnum(string->chars[i]);
  }


  return BOOL_CONST(isAlphanum);
}

static Constant stringIsNum(VM* vm, int arity, Constant *args) {
  GCString* string = AS_STRING(args[-1]);
  bool isDigit = true;

  for (int i = 0; i < string->length; i++) {
    if (!isDigit)
      break;

    isDigit = isdigit(string->chars[i]);
  }


  return BOOL_CONST(isDigit);
}

static Constant stringIsHex(VM* vm, int arity, Constant *args) {
  GCString* string = AS_STRING(args[-1]);
  bool isHex = true;

  for (int i = 0; i < string->length; i++) {
    if (!isHex)
      break;

    isHex = isxdigit(string->chars[i]);
  }


  return BOOL_CONST(isHex);
}

static Constant stringReplace(VM* vm, int arity, Constant *args) {
  expect(2, arity, "replace");

  char* string = AS_CSTRING(args[-1]);
  char* search = AS_CSTRING(args[0]);
  char* replace = AS_CSTRING(args[1]);

  char  *p = NULL , *old = NULL , *newString = NULL ;
  int c = 0 , searchLen;
  searchLen = strlen(search);

  for(p = strstr(string , search); p != NULL; p = strstr(p + searchLen , search))
    c++;

  c = (strlen(replace) - searchLen ) * c + strlen(string);

  newString = (char*)malloc(c );
  strcpy(newString , "");
  old = string;

  for(p = strstr(string , search); p != NULL; p = strstr(p + searchLen , search)) {
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

  return GC_OBJ_CONST(copyString(vm, NULL, substring(string , start, end), (end - start) - 1));
}

static Constant stringToString(VM* vm, int arity, Constant *args) {
  return args[-1];
}

void initStringClass(VM *vm) {
  vm->stringClass = newClass(vm, copyString(vm, NULL, "String", 6), true);

  // Static methods
  defineClassNativeStaticFunc(vm, "fromByte", stringFromByte, vm->stringClass);

  // Instance methods
  defineClassNativeFunc(vm, "at", stringByteAt, vm->stringClass);
  defineClassNativeFunc(vm, "contains", stringContains, vm->stringClass);
  defineClassNativeFunc(vm, "endsWith", stringEndsWith, vm->stringClass);
  defineClassNativeFunc(vm, "indexOf", stringIndexOf, vm->stringClass);
  defineClassNativeFunc(vm, "isAlpha", stringIsAlpha, vm->stringClass);
  defineClassNativeFunc(vm, "isAlphaNum", stringIsAlphaNum, vm->stringClass);
  defineClassNativeFunc(vm, "isEmpty", stringIsEmpty, vm->stringClass);
  defineClassNativeFunc(vm, "isHex", stringIsHex, vm->stringClass);
  defineClassNativeFunc(vm, "isNotEmpty", stringIsNotEmpty, vm->stringClass);
  defineClassNativeFunc(vm, "isNum", stringIsNum, vm->stringClass);
  defineClassNativeFunc(vm, "length", stringLength, vm->stringClass);
  defineClassNativeFunc(vm, "replace", stringReplace, vm->stringClass);
  defineClassNativeFunc(vm, "split", stringSplit, vm->stringClass);
  defineClassNativeFunc(vm, "startsWith", stringStartsWith, vm->stringClass);
  defineClassNativeFunc(vm, "substr", stringSubstr, vm->stringClass);
  defineClassNativeFunc(vm, "toLower", stringToLower, vm->stringClass);
  defineClassNativeFunc(vm, "toNum", stringToNum, vm->stringClass);
  defineClassNativeFunc(vm, "toUpper", stringToUpper, vm->stringClass);
  defineClassNativeFunc(vm, "toString", stringToString, vm->stringClass);

}
