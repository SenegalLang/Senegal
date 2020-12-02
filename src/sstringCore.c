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

void initStringClass(VM *vm) {
  vm->stringClass = newClass(vm, copyString(vm, NULL, "String", 6), true);
  defineClassNativeField(vm, "type", GC_OBJ_CONST(copyString(vm, NULL, "String", 6)), vm->stringClass);

  defineClassNativeStaticFunc(vm, "fromByte", stringFromByte, vm->stringClass);

  defineClassNativeFunc(vm, "at", stringByteAt, vm->stringClass);
  defineClassNativeFunc(vm, "contains", stringContains, vm->stringClass);
  defineClassNativeFunc(vm, "endsWith", stringEndsWith, vm->stringClass);
  defineClassNativeFunc(vm, "indexOf", stringIndexOf, vm->stringClass);
  defineClassNativeFunc(vm, "startsWith", stringStartsWith, vm->stringClass);
  defineClassNativeFunc(vm, "isEmpty", stringIsEmpty, vm->stringClass);
  defineClassNativeFunc(vm, "isNotEmpty", stringIsNotEmpty, vm->stringClass);
  defineClassNativeFunc(vm, "length", stringLength, vm->stringClass);

  defineClassNativeFunc(vm, "toNum", stringToNum, vm->stringClass);

}
