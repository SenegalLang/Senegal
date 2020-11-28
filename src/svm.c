#include <stdio.h>
#include <stdarg.h>

#include "includes/sutils.h"
#include "includes/scompiler.h"
#include "includes/sgcobject_utils.h"
#include "includes/sapi.h"
#include "includes/smemory.h"
#include "includes/stable_utils.h"

#include "includes/sboolCore.h"
#include "includes/sstringCore.h"
#include "includes/snumCore.h"
#include "includes/smapCore.h"
#include "includes/slistCore.h"

#if DEBUG_TRACE_EXECUTION
#include "includes/sdebug.h"
#endif

static void resetStack(GCFiber* fiber) {
  fiber->stackTop = fiber->stack;
  fiber->frameCount = 0;
  fiber->openUpvalues = NULL;
}

static void throwRuntimeError(VM* vm, const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fputs("\n", stderr);

  for (int i = vm->fiber->frameCount - 1; i > 0; i--) {
    CallFrame* frame = &vm->fiber->frames[i];
    GCFunction* function = frame->closure->function;

    size_t instruction = frame->pc - function->instructions.bytes - 1;

    fprintf(stderr, "<Line %d> ",
            function->instructions.lines[instruction].line);
    if (function->id == NULL) {
      fprintf(stderr, "Global Scope\n");
    } else {
      fprintf(stderr, "%s()\n", function->id->chars);
    }
  }

  CallFrame* frame = &vm->fiber->frames[vm->fiber->frameCount - 1];
  size_t instruction = frame->pc - frame->closure->function->instructions.bytes - 1;
  int line = getLine(&frame->closure->function->instructions, instruction);
  fprintf(stderr, "<Line %d> Global Scope\n", line);

  resetStack(vm->fiber);
}

static void defineNativeFunc(VM* vm, const char* id, NativeFunc function) {
  push(vm, GC_OBJ_CONST(copyString(vm, NULL, id, (int)strlen(id))));
  push(vm, GC_OBJ_CONST(newNative(vm, function)));
  tableInsert(vm, &vm->globals, AS_STRING(vm->fiber->stack[0]), vm->fiber->stack[1]);
  pop(vm);
  pop(vm);
}

static void defineNativeInstance(VM* vm, const char* id, GCClass* class) {
  push(vm, GC_OBJ_CONST(copyString(vm, NULL, id, (int)strlen(id))));
  push(vm, GC_OBJ_CONST(newInstance(vm, class)));
  tableInsert(vm, &vm->globals, AS_STRING(vm->fiber->stack[0]), vm->fiber->stack[1]);
  pop(vm);
  pop(vm);
}

void initVM(VM* vm) {
  vm->fiber = NULL;
  vm->fiber = ALLOCATE_GC_OBJ(vm, GCFiber, GC_FIBER);
  initFiber(vm->fiber);

  vm->gcObjects = NULL;
  vm->bytesAllocated = 0;
  vm->nextGC = 1024 * 1024;
  vm->grayCount = 0;
  vm->grayCapacity = 0;
  vm->grayStack = NULL;

  initTable(&vm->globals);
  initTable(&vm->strings);

  defineNativeFunc(vm, "assert", assertApi);
  defineNativeFunc(vm, "print", printApi);
  defineNativeFunc(vm, "println", printlnApi);

  initBoolClass(vm);
  initListClass(vm);
  initMapClass(vm);
  initNumClass(vm);
  initStringClass(vm);

  defineNativeInstance(vm, "bool", vm->boolClass);
  defineNativeInstance(vm, "List", vm->listClass);
  defineNativeInstance(vm, "Map", vm->mapClass);
  defineNativeInstance(vm, "num", vm->numClass);
  defineNativeInstance(vm, "String", vm->stringClass);
}

void initFiber(GCFiber* fiber) {
  resetStack(fiber);
}

static Constant peek(VM* vm, int topDelta) {
  return vm->fiber->stackTop[- 1 - topDelta];
}

bool call(VM* vm, GCClosure* closure, int arity) {

  if (arity != closure->function->arity) {
    throwRuntimeError(vm, "Function %s expected %d arguments but found %d", closure->function->id->chars, closure->function->arity, arity);
    return false;
  }

  if (vm->fiber->frameCount == FRAMES_MAX) {
    throwRuntimeError(vm, "Senegal's stack overflowed: Stack overflow");
    return false;
  }

  CallFrame* frame = &vm->fiber->frames[vm->fiber->frameCount++];
  frame->closure = closure;
  frame->pc = closure->function->instructions.bytes;

  frame->constants = vm->fiber->stackTop - arity - 1;
  return true;
}

static bool callConstant(VM* vm, Constant callee, int arity) {
    switch (GC_OBJ_TYPE(callee)) {
      case GC_CLASS: {
        GCClass* class = AS_CLASS(callee);
        vm->fiber->stackTop[-arity - 1] = GC_OBJ_CONST(newInstance(vm, class));

        Constant constructor;
        if (tableGetEntry(&class->methods, class->id, &constructor)) {
          return call(vm, AS_CLOSURE(constructor), arity);
        } else if (arity != 0) {
          throwRuntimeError(vm, "%s's constructor takes no arguments", class->id->chars);
          return false;
        }

        return true;
      }

      case GC_CLOSURE:
        return call(vm, AS_CLOSURE(callee), arity);

      case GC_INSTANCE_METHOD: {
        GCInstanceMethod* im = AS_INSTANCE_METHOD(callee);
        vm->fiber->stackTop[-arity - 1] = im->receiver;
        return call(vm, im->method, arity);
      }

      case GC_NATIVE: {
        Constant result = AS_NATIVE(callee)(vm, arity, vm->fiber->stackTop - arity);
        vm->fiber->stackTop -= arity + 1;

        push(vm, result);

        return true;
      }
      default:
        break;
    }

  throwRuntimeError(vm, "Senegal can only call functions and constructors");
  return false;
}

static GCUpvalue* captureUpvalue(VM* vm, Constant* local) {
  GCUpvalue* previousUpvalue = NULL;
  GCUpvalue* upvalue = vm->fiber->openUpvalues;

  while (upvalue != NULL && upvalue->place > local) {
    previousUpvalue = upvalue;
    upvalue = upvalue->next;
  }

  if (upvalue != NULL && upvalue->place == local) {
    return upvalue;
  }

  GCUpvalue* createdUpvalue = newUpvalue(vm, local);
  createdUpvalue->next = upvalue;

  if (previousUpvalue == NULL) {
    vm->fiber->openUpvalues = createdUpvalue;
  } else {
    previousUpvalue->next = createdUpvalue;
  }

  return createdUpvalue;
}

static void closeUpvalues(VM* vm, const Constant* last) {
  while (vm->fiber->openUpvalues != NULL &&
         vm->fiber->openUpvalues->place >= last) {
    GCUpvalue* upvalue = vm->fiber->openUpvalues;
    upvalue->closed = *upvalue->place;
    upvalue->place = &upvalue->closed;
    vm->fiber->openUpvalues = upvalue->next;
  }
}

static void defineMethod(VM* vm, GCString* id) {
  Constant method = peek(vm, 0);
  GCClass* class = AS_CLASS(peek(vm, 1));
  tableInsert(vm, &class->methods, id, method);
  pop(vm);
}

static void defineField(VM* vm, GCString* id) {
  Constant field = peek(vm, 0);
  GCClass* class = AS_CLASS(peek(vm, 1));
  tableInsert(vm, &class->fields, id, field);
  pop(vm);
}

static bool bindMethod(VM* vm, GCClass* class, GCString* id) {

  Constant method;
  if (!tableGetEntry(&class->methods, id, &method)) {
    throwRuntimeError(vm, "Undefined class method: `%s`", id->chars);
    return false;
  }

  GCInstanceMethod* im = newInstanceMethod(vm, peek(vm, 0), AS_CLOSURE(method));

  pop(vm);
  push(vm, GC_OBJ_CONST(im));
  return true;
}

static bool invokeFromClass(VM* vm, GCClass* class, GCString* id, int arity) {
  Constant method;

  if (!tableGetEntry(&class->methods, id, &method)) {
    throwRuntimeError(vm, "Unknown property `%s` for class `%s`", id->chars, class->id->chars);
    return false;
  }

  return call(vm, AS_CLOSURE(method), arity);
}

static bool invoke(VM* vm, GCString* id, int arity) {
  register Constant receiver = peek(vm, arity);

  if (IS_BOOL(receiver)) {
    GCInstance* boolInstance = newInstance(vm, vm->boolClass);

    Constant constant;
    if (tableGetEntry(&boolInstance->class->methods, id, &constant)) {
      vm->fiber->stackTop[-arity - 1] = receiver;
      return callConstant(vm, constant, arity);
    }

    return false;
  }

  if (IS_STRING(receiver)) {
    GCInstance* stringInstance = newInstance(vm, vm->stringClass);

    Constant constant;
    if (tableGetEntry(&stringInstance->class->methods, id, &constant)) {
      vm->fiber->stackTop[-arity - 1] = receiver;
      return callConstant(vm, constant, arity);
    }

    return false;
  }

  if (IS_LIST(receiver)) {
    GCInstance* listInstance = newInstance(vm, vm->listClass);

    Constant constant;
    if (tableGetEntry(&listInstance->class->methods, id, &constant)) {
      vm->fiber->stackTop[-arity - 1] = receiver;
      return callConstant(vm, constant, arity);
    }

    return false;
  }

  if (IS_MAP(receiver)) {
    GCInstance* mapInstance = newInstance(vm, vm->mapClass);

    Constant constant;
    if (tableGetEntry(&mapInstance->class->methods, id, &constant)) {
      vm->fiber->stackTop[-arity - 1] = receiver;
      return callConstant(vm, constant, arity);
    }

    return false;
  }

  if (IS_NUMBER(receiver)) {
    GCInstance* numInstance = newInstance(vm, vm->numClass);

    Constant constant;
    if (tableGetEntry(&numInstance->class->methods, id, &constant)) {
      vm->fiber->stackTop[-arity - 1] = receiver;
      return callConstant(vm, constant, arity);
    }

    return false;
  }

  if (!IS_INSTANCE(receiver)) {
    throwRuntimeError(vm, "Senegal only allows methods on instances.");
    return false;
  }

  GCInstance* instance = AS_INSTANCE(receiver);

  Constant constant;
  if (tableGetEntry(&instance->class->fields, id, &constant)) {
    vm->fiber->stackTop[-arity - 1] = constant;
    return callConstant(vm, constant, arity);
  }

  return invokeFromClass(vm, instance->class, id, arity);
}

static InterpretationResult run(VM* vm) {
  register CallFrame* frame = &vm->fiber->frames[vm->fiber->frameCount - 1];

#define PEEK() (vm->fiber->stackTop[-1])
#define PEEK2() (vm->fiber->stackTop[-2])

#define PUSH(constant) *vm->fiber->stackTop++ = constant
#define POP() (*(--vm->fiber->stackTop))
#define POPN(count) (*(vm->fiber->stackTop -= (count)))

#define UPDATE_FRAME() (frame = &vm->fiber->frames[vm->fiber->frameCount - 1])

#define READ_BYTE() (*frame->pc++)
#define READ_SHORT() (frame->pc += 2, (uint16_t)((frame->pc[-2] << 8) | frame->pc[-1]))
#define READ_CONSTANT() (frame->closure->function->instructions.constants.constants[READ_BYTE()])
#define READ_CONSTANT_FROM_INDEX(i) (frame->closure->function->instructions.constants.constants[i])
#define READ_STRING() AS_STRING(READ_CONSTANT())

#define BINARY_OP(vm, constantType, op) \
  do {                    \
    if (!IS_NUMBER(PEEK()) || !IS_NUMBER(PEEK2())) { \
      throwRuntimeError(vm, "Senegal binary operations require numerical operands."); \
      return RUNTIME_ERROR;\
    }                      \
    double b = AS_NUMBER(POP()); \
    double a = AS_NUMBER(POP()); \
    Constant c = constantType(a op b); \
    PUSH(c); \
  } while(false)

  #define CONCAT_STRINGS() \
  do {                     \
    GCString* b = AS_STRING(peek(vm, 0)); \
    GCString* a = AS_STRING(peek(vm, 1));            \
                           \
    int length = a->length + b->length; \
    char* chars = ALLOCATE(vm, NULL, char, length + 1); \
    \
    memcpy(chars, a->chars, a->length); \
    memcpy(chars + a->length, b->chars, b->length); \
    \
    chars[length] = '\0'; \
    \
    GCString* newString = getString(vm, chars, length); \
    pop(vm); \
    pop(vm); \
    push(vm,GC_OBJ_CONST(newString)); \
  } while(false)

#define BITWISE_OP(vm, constantType, op) \
  do {                    \
    if (!IS_NUMBER(PEEK()) || !IS_NUMBER(PEEK2())) { \
      throwRuntimeError(vm, "Senegal binary operations require numerical operands."); \
      return RUNTIME_ERROR;\
    }                      \
    int b = AS_NUMBER(POP()); \
    int a = AS_NUMBER(POP()); \
    Constant c = constantType(a op b); \
    PUSH(c); \
  } while(false)

#if COMPUTED_GOTO

  static void* dispatchTable[] = {
#define OPCODE(code) &&code,
#include "includes/sopcodes.h"
#undef OPCODE
  };

#define DISPATCH() \
  do {             \
  goto *dispatchTable[READ_BYTE()]; \
  } while (false)

#define RUN DISPATCH();
#define CASE(code) code

#else

  #define RUN \
    run: \
      switch(READ_BYTE()) \

#define SENEGAL_CASE(code) case (code)
#define DISPATCH() goto run
#endif

  // === Labels ===
  RUN {
    CASE(OPCODE_TRUE): {
      Constant c = BOOL_CONST(true);
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_FALSE): {
      Constant c = BOOL_CONST(false);
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_AND):
    BITWISE_OP(vm, NUM_CONST, &);
    DISPATCH();

    CASE(OPCODE_OR):
    BITWISE_OP(vm, NUM_CONST, |);
    DISPATCH();

    CASE(OPCODE_XOR):
    BITWISE_OP(vm, NUM_CONST, ^);
    DISPATCH();

    CASE(OPCODE_LSHIFT):
    BITWISE_OP(vm, NUM_CONST, <<);
    DISPATCH();

    CASE(OPCODE_RSHIFT):
    BITWISE_OP(vm, NUM_CONST, >>);
    DISPATCH();

    CASE(OPCODE_BITNOT): {
      throwRuntimeError(vm, "Senegal encountered a non-number as an operand for OPCODE_NEG.");
      return RUNTIME_ERROR;
    }

    Constant c = NUM_CONST(~(int)AS_NUMBER(POP()));
    PUSH(c);
    DISPATCH();

    CASE(OPCODE_INC): {
    if (!IS_NUMBER(PEEK())) {
      throwRuntimeError(vm, "Senegal binary operations require numerical operands.");
      return RUNTIME_ERROR;
    }

    double num = AS_NUMBER(POP());

    Constant c = NUM_CONST( num + 1);
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_DEC): {
    if (!IS_NUMBER(PEEK())) {
      throwRuntimeError(vm, "Senegal binary operations require numerical operands.");
      return RUNTIME_ERROR;
    }

    double num = AS_NUMBER(POP());

    Constant c = NUM_CONST(num - 1);
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_POW): {
      if (!IS_NUMBER(PEEK()) || !IS_NUMBER(PEEK2())) {
        throwRuntimeError(vm, "Senegal binary operations require numerical operands.");
        return RUNTIME_ERROR;
      }

      double powerNum = AS_NUMBER(POP());
      double num = AS_NUMBER(POP());

      Constant c = NUM_CONST(power(num, powerNum));
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_ADD):
      if (IS_STRING(PEEK()) || IS_STRING(PEEK2()))
        CONCAT_STRINGS();
      else
        BINARY_OP(vm, NUM_CONST, +);

      DISPATCH();

    CASE(OPCODE_SUB):
      BINARY_OP(vm, NUM_CONST, -);
      DISPATCH();

    CASE(OPCODE_MUL):
      if (IS_NUMBER(PEEK2()) && IS_NUMBER(PEEK())) {
        BINARY_OP(vm, NUM_CONST, *);
      } else if (IS_STRING(PEEK2()) && IS_NUMBER(PEEK())) {

        int multiplier = (int) AS_NUMBER(POP());
        GCString *string = AS_STRING(POP());

        int length = string->length * multiplier;
        char *chars = ALLOCATE(vm, NULL, char, length + 1);

        for (int i = 0; i < multiplier; i++)
          memcpy(chars + i * string->length, string->chars, string->length);

        chars[length] = '\0';

        GCString *newString = getString(vm, chars, length);

        Constant c = GC_OBJ_CONST(newString);
        PUSH(c);
      } else {
        throwRuntimeError(vm, "Senegal encountered an unexpected type while executing OPCODE_MUL.");
        return RUNTIME_ERROR;
      }
      DISPATCH();

    CASE(OPCODE_DIV):
      BINARY_OP(vm, NUM_CONST, /);
      DISPATCH();

    CASE(OPCODE_EQUAL): {
    Constant b = POP();
    Constant c = BOOL_CONST(areEqual(POP(), b));
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_NOTEQ): {
    Constant b = POP();

    Constant c = BOOL_CONST(!areEqual(POP(), b));
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_GREATER):
      BINARY_OP(vm, BOOL_CONST, >);
      DISPATCH();

    CASE(OPCODE_LESSER):
      BINARY_OP(vm, BOOL_CONST, <);
      DISPATCH();

    CASE(OPCODE_GE):
      BINARY_OP(vm, BOOL_CONST, >=);
      DISPATCH();

    CASE(OPCODE_LE):
      BINARY_OP(vm, BOOL_CONST, <=);
      DISPATCH();

    CASE(OPCODE_NEG): {
      if (!IS_NUMBER(PEEK())) {
        throwRuntimeError(vm, "Senegal encountered a non-number as an operand for OPCODE_NEG.");
        return RUNTIME_ERROR;
      }

      Constant c = NUM_CONST(-AS_NUMBER(POP()));
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_NOT): {
    Constant constant = POP();
    Constant c = BOOL_CONST(IS_BOOL(constant) && !AS_BOOL(constant));
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_DUP):
      PUSH(PEEK());
      DISPATCH();

    CASE(OPCODE_POP):
      POP();
      DISPATCH();

    CASE(OPCODE_POPN):
      POPN(READ_BYTE());
      DISPATCH();

    CASE(OPCODE_LOAD):
      PUSH(READ_CONSTANT());
      DISPATCH();

    CASE(OPCODE_LLOAD): {
      int index = READ_BYTE() | (READ_BYTE() << 8) | (READ_BYTE() << 16);
      PUSH(READ_CONSTANT_FROM_INDEX(index));
      DISPATCH();
    }

    CASE(OPCODE_LOADN1):
      PUSH(frame->closure->function->instructions.constants.constants[-1]);
      DISPATCH();

    CASE(OPCODE_LOAD0):
      PUSH(frame->closure->function->instructions.constants.constants[0]);
      DISPATCH();

    CASE(OPCODE_LOAD1):
      PUSH(frame->closure->function->instructions.constants.constants[1]);
      DISPATCH();

    CASE(OPCODE_LOAD2):
      PUSH(frame->closure->function->instructions.constants.constants[2]);
      DISPATCH();

    CASE(OPCODE_LOAD3):
      PUSH(frame->closure->function->instructions.constants.constants[3]);
      DISPATCH();

    CASE(OPCODE_NEWMAP): {
      int arity = READ_BYTE();

      GCMap* map = newMap(vm);
      for (int i = 0; i < arity; i++) {
        Constant value = POP();
        GCString* key = AS_STRING(POP());

        tableInsert(vm, &map->table, key, value);
      }

      PUSH(GC_OBJ_CONST(map));
      DISPATCH();
    }

    CASE(OPCODE_NEWLIST): {
      int arity = READ_BYTE();

      GCList* list = newList(vm, arity);
      for (int i = 0; i < arity; i++) {
        Constant value = POP();

        list->elements[list->elementC++] = value;
      }

      PUSH(GC_OBJ_CONST(list));
      DISPATCH();
    }

    CASE(OPCODE_ACCESS): {
      if (IS_MAP(PEEK2())) {
        GCString* key = AS_STRING(POP());
        GCMap* map = AS_MAP(POP());

        Constant c;
        if (!tableGetEntry(&map->table, key, &c)) {
          throwRuntimeError(vm, "Map entry `%s` was not found", key->chars);
          return RUNTIME_ERROR;
        }

        PUSH(c);
      } else if (IS_LIST(PEEK2())) {
        int index = AS_NUMBER(POP());
        GCList* list = AS_LIST(POP());

        if (index >= list->elementC) {
          throwRuntimeError(vm, "Out of range: %d, valid range is %d", index, list->elementC - 1);
          return RUNTIME_ERROR;
        }

        push(vm, list->elements[(list->elementC - 1) - (int)index]);
      } else {
        throwRuntimeError(vm, "Tried accessing an invalid type.");
        return RUNTIME_ERROR;
      }

      DISPATCH();
    }

    CASE(OPCODE_SETACCESS): {
      Constant newValue = POP();

      if (IS_MAP(PEEK2())) {
        if (!IS_STRING(PEEK())) {
          throwRuntimeError(vm, "Senegal maps only support String keys");
          return RUNTIME_ERROR;
        }

        GCString* key = AS_STRING(POP());
        GCMap* map = AS_MAP(POP());

        tableInsert(vm, &map->table, key, newValue);
      } else if (IS_LIST(PEEK2())) {
        if (!IS_NUMBER(PEEK())) {
          throwRuntimeError(vm, "Index must be a numerical value");
          return RUNTIME_ERROR;
        }

        double index = AS_NUMBER(POP());
        GCList* list = AS_LIST(POP());

        if (index >= list->elementC) {
          throwRuntimeError(vm, "Out of range: %d, valid range is %d", index, list->elementC - 1);
          return RUNTIME_ERROR;
        }

        list->elements[(list->elementC - 1) - (int)index] = newValue;
      } else {
        throwRuntimeError(vm, "Tried accessing an invalid type.");
        return RUNTIME_ERROR;
      }

      DISPATCH();
    }

    CASE(OPCODE_NEWCLASS):
      PUSH(GC_OBJ_CONST(newClass(vm, READ_STRING(), false)));
      DISPATCH();

    CASE(OPCODE_NEWFINALCLASS):
      PUSH(GC_OBJ_CONST(newClass(vm, READ_STRING(), true)));
      DISPATCH();

    CASE(OPCODE_SETFIELD): {

    if (IS_CLASS(PEEK())) {
      GCClass *class = AS_CLASS(PEEK2());

      if (class->isFinal && frame->closure->function->id != class->id) {
        throwRuntimeError(vm, "Cannot mutate fields of a final class");
        return RUNTIME_ERROR;
      }

      tableInsert(vm, &class->fields, READ_STRING(), PEEK());

      Constant c = POP();

      POP();
      PUSH(c);

      DISPATCH();
    }

    if (!IS_INSTANCE(PEEK2())) {
      throwRuntimeError(vm, "Tried setting fields of non-class instance objects");
      return RUNTIME_ERROR;
    }

    GCInstance *instance = AS_INSTANCE(PEEK2());

    if (instance->class->isFinal && frame->closure->function->id != instance->class->id) {
      throwRuntimeError(vm, "Senegal cannot mutate fields of a final class: %s", instance->class->id->chars);
      return RUNTIME_ERROR;
    }

    GCString *key = READ_STRING();

    Constant constant1;
    if (!tableGetEntry(&instance->class->fields, key, &constant1)) {
      throwRuntimeError(vm, "Senegal cannot add fields to an instance", instance->class->id->chars);
      return RUNTIME_ERROR;
    }

    tableInsert(vm, &instance->class->fields, key, PEEK());

    Constant constant = POP();

    POP();
    Constant c = constant;
    PUSH(c);

    DISPATCH();
  }

    CASE(OPCODE_GETFIELD): {

    Constant left = PEEK();

    if (IS_BOOL(left) || (IS_INSTANCE(left) && memcmp(AS_INSTANCE(left)->class->id->chars, "bool", 4) == 0)) {
      GCString *id = READ_STRING();

      Constant constant;
      if (tableGetEntry(&vm->boolClass->fields, id, &constant)) {
        POP();
        Constant c = constant;
        PUSH(c);
        DISPATCH();
      }

      return RUNTIME_ERROR;
    }

    if (IS_STRING(left) || (IS_INSTANCE(left) && memcmp(AS_INSTANCE(left)->class->id->chars, "String", 6) == 0)) {
      GCString *id = READ_STRING();

      Constant constant;
      if (tableGetEntry(&vm->stringClass->fields, id, &constant)) {
        POP();
        Constant c = constant;
        PUSH(c);
        DISPATCH();
      }

      return RUNTIME_ERROR;
    }

    if (IS_LIST(left) || (IS_INSTANCE(left) && memcmp(AS_INSTANCE(left)->class->id->chars, "List", 4) == 0)) {
      GCString *id = READ_STRING();

      Constant constant;
      if (tableGetEntry(&vm->listClass->fields, id, &constant)) {
        POP();
        Constant c = constant;
        PUSH(c);
        DISPATCH();
      }

      return RUNTIME_ERROR;
    }

    if (IS_MAP(left) || (IS_INSTANCE(left) && memcmp(AS_INSTANCE(left)->class->id->chars, "Map", 3) == 0)) {
      GCString *id = READ_STRING();

      Constant constant;
      if (tableGetEntry(&vm->mapClass->fields, id, &constant)) {
        POP();
        Constant c = constant;
        PUSH(c);
        DISPATCH();
      }

      return RUNTIME_ERROR;
    }

    if (IS_NUMBER(left) || (IS_INSTANCE(left) && memcmp(AS_INSTANCE(left)->class->id->chars, "num", 3) == 0)) {
      GCString *id = READ_STRING();

      Constant constant;
      if (tableGetEntry(&vm->numClass->fields, id, &constant)) {
        POP();
        Constant c = constant;
        PUSH(c);
        DISPATCH();
      }

      return RUNTIME_ERROR;
    }

    if (!IS_INSTANCE(left)) {
      throwRuntimeError(vm, "Tried accessing fields of non-class or instance objects");
      return RUNTIME_ERROR;
    }

    GCInstance *instance = AS_INSTANCE(left);
    GCString *id = READ_STRING();

    Constant constant;
    if (tableGetEntry(&instance->class->fields, id, &constant)) {
      POP();
      PUSH(constant);
      DISPATCH();
    }

    if (!bindMethod(vm, instance->class, id)) {
      return RUNTIME_ERROR;
    }

    DISPATCH();
  }

    CASE(OPCODE_NEWMETHOD):
      defineMethod(vm, READ_STRING());
      DISPATCH();

    CASE(OPCODE_NEWFIELD):
      defineField(vm, READ_STRING());
      DISPATCH();

    CASE(OPCODE_INHERIT): {
    Constant super = PEEK2();

    if (!IS_CLASS(super)) {
      throwRuntimeError(vm, "Senegal classes can only inherit from another class");
      return RUNTIME_ERROR;
    }

    GCClass* sub = AS_CLASS(PEEK());

    tableInsertAll(vm, &AS_CLASS(super)->fields, &sub->fields);
    tableInsertAll(vm, &AS_CLASS(super)->methods, &sub->methods);

    POP();
    DISPATCH();
  }

    CASE(OPCODE_GETSUPER): {
    GCString *id = READ_STRING();
    GCClass *super = AS_CLASS(POP());

    if (!bindMethod(vm, super, id)) {
      return RUNTIME_ERROR;
    }

    DISPATCH();
  }

    CASE(OPCODE_SUPERINVOKE): {
    GCString *method = READ_STRING();
    int arity = READ_BYTE();
    GCClass *super = AS_CLASS(POP());

    if (!invokeFromClass(vm, super, method, arity)) {
      return RUNTIME_ERROR;
    }

    UPDATE_FRAME();
    DISPATCH();
  }

    CASE(OPCODE_NEWGLOB): {
    GCString *id = READ_STRING();
    Constant constant;

    if (tableGetEntry(&vm->globals, id, &constant)) {
      throwRuntimeError(vm, "Senegal attempted to define an existing global: %s", id);
      return RUNTIME_ERROR;
    }

    tableInsert(vm, &vm->globals, id, PEEK());
    POP();
    DISPATCH();
  }

    CASE(OPCODE_GETGLOB): {
    GCString *id = READ_STRING();
    Constant constant;

    if (!tableGetEntry(&vm->globals, id, &constant)) {
      throwRuntimeError(vm, "Senegal tried to get an undefined variable `%s`", id->chars);
      return RUNTIME_ERROR;
    }

    PUSH(constant);
    DISPATCH();
  }

    CASE(OPCODE_SETGLOB): {
    GCString *id = READ_STRING();

    if (tableInsert(vm, &vm->globals, id, PEEK())) {
      tableRemove(&vm->globals, id);
      throwRuntimeError(vm, "Senegal attempted to reassign an undefined variable: %s", id->chars);
      return RUNTIME_ERROR;
    }

    DISPATCH();
  }

    CASE(OPCODE_GETLOC): {
    Constant c = frame->constants[READ_BYTE()];
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_GETLOC0):
      PUSH(frame->constants[0]);
      DISPATCH();

    CASE(OPCODE_GETLOC1):
      PUSH(frame->constants[1]);
      DISPATCH();

    CASE(OPCODE_GETLOC2):
      PUSH(frame->constants[2]);
      DISPATCH();

    CASE(OPCODE_GETLOC3):
      PUSH(frame->constants[3]);
      DISPATCH();

    CASE(OPCODE_GETLOC4):
      PUSH(frame->constants[4]);
      DISPATCH();

    CASE(OPCODE_GETLOC5):
      PUSH(frame->constants[5]);
      DISPATCH();

    CASE(OPCODE_SETLOC):
      frame->constants[READ_BYTE()] = PEEK();
      DISPATCH();

    CASE(OPCODE_SETLOC0):
      frame->constants[0] = PEEK();
      DISPATCH();

    CASE(OPCODE_SETLOC1):
      frame->constants[1] = PEEK();
      DISPATCH();

    CASE(OPCODE_SETLOC2):
      frame->constants[2] = PEEK();
      DISPATCH();

    CASE(OPCODE_SETLOC3):
      frame->constants[3] = PEEK();
      DISPATCH();

    CASE(OPCODE_SETLOC4):
      frame->constants[4] = PEEK();
      DISPATCH();

    CASE(OPCODE_SETLOC5):
      frame->constants[5] = PEEK();
      DISPATCH();

    CASE(OPCODE_GETUPVAL): {
    Constant c = *frame->closure->upvalues[READ_BYTE()]->place;
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_SETUPVAL): {
    uint8_t slot = READ_BYTE();
    *frame->closure->upvalues[slot]->place = PEEK();
    DISPATCH();
  }

    CASE(OPCODE_CLOSEUPVAL): {
    closeUpvalues(vm, vm->fiber->stackTop - 1);
    POP();
    DISPATCH();
  }

    CASE(OPCODE_JMP):
    frame->pc += READ_SHORT();
    DISPATCH();

    CASE(OPCODE_JF): {
    uint16_t offset = READ_SHORT();

    Constant constant = PEEK();

    if (IS_NULL(constant) || (IS_BOOL(constant) && !AS_BOOL(constant)))
      frame->pc += offset;

    DISPATCH();
  }

    CASE(OPCODE_LOOP): {
    uint16_t offset = READ_SHORT();
    frame->pc -= offset;

    DISPATCH();
  }

    CASE(OPCODE_BREAK): {
      throwRuntimeError(vm, "Senegal encountered a badly parsed `break`");
      DISPATCH();
    }

    CASE(OPCODE_CLOSURE): {
    GCFunction *function = AS_FUNCTION(READ_CONSTANT());
    GCClosure *closure = newClosure(vm, function);
    Constant c = GC_OBJ_CONST(closure);
    PUSH(c);

    for (int i = 0; i < closure->upvalueCount; i++) {
      uint8_t isLocal = READ_BYTE();
      uint8_t index = READ_BYTE();
      if (isLocal) {
        closure->upvalues[i] = captureUpvalue(vm, frame->constants + index);
      } else {
        closure->upvalues[i] = frame->closure->upvalues[index];
      }
    }

    DISPATCH();
  }

    CASE(OPCODE_CALL): {
    int arity = READ_BYTE();
    if (!callConstant(vm, peek(vm, arity), arity)) {
      return RUNTIME_ERROR;
    }

    UPDATE_FRAME();
    DISPATCH();
  }

    CASE(OPCODE_CALL0): {
    if (!callConstant(vm, peek(vm, 0), 0)) {
      return RUNTIME_ERROR;
    }

    UPDATE_FRAME();
    DISPATCH();
  }

    CASE(OPCODE_CALL1): {
    if (!callConstant(vm, peek(vm, 1), 1)) {
      return RUNTIME_ERROR;
    }

    UPDATE_FRAME();
    DISPATCH();
  }

    CASE(OPCODE_CALL2): {
    if (!callConstant(vm, peek(vm, 2), 2)) {
      return RUNTIME_ERROR;
    }

    UPDATE_FRAME();
    DISPATCH();
  }

    CASE(OPCODE_CALL3): {
    if (!callConstant(vm, peek(vm, 3), 3)) {
      return RUNTIME_ERROR;
    }

    UPDATE_FRAME();
    DISPATCH();
  }

    CASE(OPCODE_CALL4): {
    if (!callConstant(vm, peek(vm, 4), 4)) {
      return RUNTIME_ERROR;
    }

    UPDATE_FRAME();
    DISPATCH();
  }

    CASE(OPCODE_CALL5): {
    if (!callConstant(vm, peek(vm, 5), 5)) {
      return RUNTIME_ERROR;
    }

    UPDATE_FRAME();
    DISPATCH();
  }

    CASE(OPCODE_CALL6): {
    if (!callConstant(vm, peek(vm, 6), 6)) {
      return RUNTIME_ERROR;
    }

    UPDATE_FRAME();
    DISPATCH();
  }

    CASE(OPCODE_CALL7): {
    if (!callConstant(vm, peek(vm, 7), 7)) {
      return RUNTIME_ERROR;
    }

    UPDATE_FRAME();
    DISPATCH();
  }

    CASE(OPCODE_CALL8): {
    if (!callConstant(vm, peek(vm, 8), 8)) {
      return RUNTIME_ERROR;
    }

    UPDATE_FRAME();
    DISPATCH();
  }

    CASE(OPCODE_INVOKE):{
    GCString *method = READ_STRING();
    int arity = READ_BYTE();

    if (!invoke(vm, method, arity)) {
      return RUNTIME_ERROR;
    }

    UPDATE_FRAME();
    DISPATCH();
  }

    CASE(OPCODE_NULL): {
      Constant c = NULL_CONST;
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_RET): {
    Constant result = POP();

    closeUpvalues(vm, frame->constants);

    vm->fiber->frameCount--;

    if (vm->fiber->frameCount == 0) {
      POP();
      return OK;
    }

    vm->fiber->stackTop = frame->constants;
    Constant c = result;
    PUSH(c);

    UPDATE_FRAME();
    DISPATCH();
  }

  }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

InterpretationResult interpret(VM* vm, char* source) {
  Compiler compiler;
  GCFunction* function = compile(vm, &compiler, source);

  if (function == NULL)
    return COMPILE_TIME_ERROR;

  push(vm, GC_OBJ_CONST(function));

  GCClosure* closure = newClosure(vm, function);
  pop(vm);
  push(vm, GC_OBJ_CONST(closure));

  call(vm, AS_CLOSURE(GC_OBJ_CONST(closure)), 0);

  return run(vm);
}

InterpretationResult interpretImport(VM *vm, char *source) {

  Compiler compiler;
  GCFunction* function = compileImport(vm, &compiler, source);

  if (function == NULL)
    return COMPILE_TIME_ERROR;

  push(vm, GC_OBJ_CONST(function));

  GCClosure* closure = newClosure(vm, function);
  pop(vm);
  push(vm, GC_OBJ_CONST(closure));

  call(vm, AS_CLOSURE(GC_OBJ_CONST(closure)), 0);

  return run(vm);
}

void push(VM* vm, Constant constant) {
  *vm->fiber->stackTop++ = constant;
}

Constant pop(VM* vm) {
  return *(vm->fiber->stackTop--);
}

GCClass* newClass(VM *vm, GCString *id, bool isFinal) {
  GCClass* class = ALLOCATE_GC_OBJ(vm, GCClass, GC_CLASS);
  class->id = id;
  class->isFinal = isFinal;
  initTable(&class->methods);
  initTable(&class->fields);

  return class;
}

GCClosure* newClosure(VM *vm, GCFunction *function) {
  GCUpvalue** upvalues = ALLOCATE(vm, NULL, GCUpvalue*, function->upvalueCount);

  for (int i = 0; i < function->upvalueCount; i++) {
    upvalues[i] = NULL;
  }

  GCClosure* closure = ALLOCATE_GC_OBJ(vm, GCClosure, GC_CLOSURE);
  closure->function = function;

  closure->upvalues = upvalues;
  closure->upvalueCount = function->upvalueCount;

  return closure;
}

GCFunction* newFunction(VM* vm) {
  GCFunction* function = ALLOCATE_GC_OBJ(vm, GCFunction, GC_FUNCTION);
  function->arity = 0;
  function->upvalueCount = 0;
  function->id = NULL;
  initInstructions(&function->instructions);
  return function;
}

GCInstance* newInstance(VM* vm, GCClass* class) {
  GCInstance* instance = ALLOCATE_GC_OBJ(vm, GCInstance, GC_INSTANCE);

  instance->class = class;

  return instance;
}

GCInstanceMethod* newInstanceMethod(VM* vm, Constant receiver, GCClosure* method) {
  GCInstanceMethod* im = ALLOCATE_GC_OBJ(vm, GCInstanceMethod, GC_INSTANCE_METHOD);
  im->receiver = receiver;
  im->method = method;
  return im;
}

GCNative* newNative(VM* vm, NativeFunc function) {
  GCNative* nativeFunc = ALLOCATE_GC_OBJ(vm, GCNative, GC_NATIVE);
  nativeFunc->function = function;
  return nativeFunc;
}

GCUpvalue* newUpvalue(VM *vm, Constant *constant) {
  GCUpvalue* upvalue = ALLOCATE_GC_OBJ(vm, GCUpvalue, GC_UPVALUE);
  upvalue->place = constant;
  upvalue->closed = NULL_CONST;
  upvalue->next = NULL;

  return upvalue;
}

GCList* newList(VM *vm, int length) {
  GCList* list = ALLOCATE_GC_OBJ(vm, GCList, GC_LIST);

  list->elements = ALLOCATE(vm, NULL, Constant, length);
  list->elementC = 0;

  list->listCurrentCap = 0;
  GROW_CAP(list->listCurrentCap);

  return list;
}

GCMap* newMap(VM *vm) {
  GCMap* map = ALLOCATE_GC_OBJ(vm, GCMap, GC_MAP);
  initTable(&map->table);

  return map;
}