#include <stdio.h>
#include <stdarg.h>

#include "includes/sutils.h"
#include "includes/scompiler.h"
#include "includes/sgcobject_utils.h"
#include "includes/sapi.h"
#include "includes/smemory.h"
#include "includes/stable_utils.h"
#include "core/sboolCore.h"

#if DEBUG_TRACE_EXECUTION
#include "includes/sdebug.h"
#include "includes/svm.h"
#include "includes/stable_utils.h"
#include "includes/sinstruction_utils.h"

#endif

static void resetStack(VM* vm) {
  vm->stackTop = vm->stack;
  vm->frameCount = 0;
  vm->openUpvalues = NULL;
}

static void throwRuntimeError(VM* vm, const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fputs("\n", stderr);

  for (int i = vm->frameCount - 1; i >= 0; i--) {
    CallFrame* frame = &vm->frames[i];
    GCFunction* function = frame->closure->function;

    size_t instruction = frame->pc - function->instructions.bytes - 1;

    fprintf(stderr, "<line %d> ",
            function->instructions.lines[instruction]);
    if (function->id == NULL) {
      fprintf(stderr, "Global Scope\n");
    } else {
      fprintf(stderr, "%s()\n", function->id->chars);
    }
  }

  CallFrame* frame = &vm->frames[vm->frameCount - 1];
  size_t instruction = frame->pc - frame->closure->function->instructions.bytes - 1;
  int line = getLine(&frame->closure->function->instructions, instruction);
  fprintf(stderr, "<Line %d> Global Scope\n", line);

  resetStack(vm);
}

static void defineNativeFunc(VM* vm, const char* id, NativeFunc function) {
  push(vm, GC_OBJ_CONST(copyString(vm, NULL, id, (int)strlen(id))));
  push(vm, GC_OBJ_CONST(newNative(vm, function)));
  tableInsert(vm, &vm->globals, AS_STRING(vm->stack[0]), vm->stack[1]);
  pop(vm);
  pop(vm);
}

static void defineClassNativeFunc(VM* vm, const char* id, NativeFunc function, GCClass* class) {
  push(vm, GC_OBJ_CONST(copyString(vm, NULL, id, (int)strlen(id))));
  push(vm, GC_OBJ_CONST(newNative(vm, function)));
  tableInsert(vm, &class->methods, AS_STRING(vm->stack[0]), vm->stack[1]);
  pop(vm);
  pop(vm);
}


static void initBoolClass(VM* vm) {
  vm->boolClass = newClass(vm, copyString(vm, NULL, "bool", 4), false, false);
  defineClassNativeFunc(vm, "toString", boolToString, vm->boolClass);
}

void initVM(VM* vm) {
  resetStack(vm);

  vm->gcObjects = NULL;
  vm->bytesAllocated = 0;
  vm->nextGC = 1024 * 1024;
  vm->grayCount = 0;
  vm->grayCapacity = 0;
  vm->grayStack = NULL;

  initTable(&vm->globals);
  initTable(&vm->strings);

  vm->constructString = NULL;
  vm->constructString = copyString(vm, NULL, "construct", 9);

  defineNativeFunc(vm, "assert", assertApi);
  defineNativeFunc(vm, "clock", clockApi);
  defineNativeFunc(vm, "print", printApi);
  defineNativeFunc(vm, "printLn", printLnApi);

  initBoolClass(vm);
}

static Constant peek(VM* vm, int topDelta) {
  return vm->stackTop[- 1 - topDelta];
}

static void concatenateStrings(VM* vm) {
  GCString* b = AS_STRING(peek(vm, 0));
  GCString* a = AS_STRING(peek(vm, 1));

  int length = a->length + b->length;
  char* chars = ALLOCATE(vm, NULL, char, length + 1);

  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);

  chars[length] = '\0';

  GCString* newString = getString(vm, chars, length);
  pop(vm);
  pop(vm);
  push(vm,GC_OBJ_CONST(newString));
}

static bool call(VM* vm, GCClosure* closure, int arity) {

  if (arity != closure->function->arity) {
    throwRuntimeError(vm, "Function %s expected %d arguments but found %d", closure->function->id->chars, closure->function->arity, arity);
    return false;
  }

  if (vm->frameCount == FRAMES_MAX) {
    throwRuntimeError(vm, "Senegal's stack overflowed: Stack overflow");
    return false;
  }

  CallFrame* frame = &vm->frames[vm->frameCount++];
  frame->closure = closure;
  frame->pc = closure->function->instructions.bytes;

  frame->constants = vm->stackTop - arity - 1;
  return true;
}

static bool callConstant(VM* vm,Constant callee, int arity) {
  if (IS_GC_OBJ(callee)) {
    switch (GC_OBJ_TYPE(callee)) {
      case GC_CLASS: {
        GCClass* class = AS_CLASS(callee);
        vm->stackTop[-arity - 1] = GC_OBJ_CONST(newInstance(vm, class));

        Constant constructor;
        if (tableGetEntry(&class->methods, vm->constructString, &constructor)) {
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
        vm->stackTop[-arity - 1] = im->receiver;
        return call(vm, im->method, arity);
      }

      case GC_NATIVE: {
        NativeFunc nativeFunc = AS_NATIVE(callee);
        Constant result = nativeFunc(vm, arity, vm->stackTop - arity);

        vm->stackTop -= arity + 1;
        push(vm, result);

        return true;
      }
      default:
        break;
    }
  }

  throwRuntimeError(vm, "Senegal can only call functions and constructors");
  return false;
}

static GCUpvalue* captureUpvalue(VM* vm, Constant* local) {
  GCUpvalue* previousUpvalue = NULL;
  GCUpvalue* upvalue = vm->openUpvalues;

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
    vm->openUpvalues = createdUpvalue;
  } else {
    previousUpvalue->next = createdUpvalue;
  }

  return createdUpvalue;
}

static void closeUpvalues(VM* vm, Constant* last) {
  while (vm->openUpvalues != NULL &&
         vm->openUpvalues->place >= last) {
    GCUpvalue* upvalue = vm->openUpvalues;
    upvalue->closed = *upvalue->place;
    upvalue->place = &upvalue->closed;
    vm->openUpvalues = upvalue->next;
  }
}

static void defineMethod(VM* vm, GCString* id) {
  Constant method = peek(vm, 0);
  GCClass* class = AS_CLASS(peek(vm, 1));
  tableInsert(vm, &class->methods, id, method);
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
  Constant receiver = peek(vm, arity);

  if (!IS_INSTANCE(receiver)) {

    if (IS_BOOL(receiver)) {
      GCInstance* boolInstance = newInstance(vm, vm->boolClass);

      Constant constant;
      if (tableGetEntry(&boolInstance->class->methods, id, &constant)) {
        vm->stackTop[-arity - 1] = constant;

        push(vm, receiver);
        NativeFunc nativeFunc = AS_NATIVE(constant);

        Constant result = nativeFunc(vm, 1, vm->stackTop - 1);

        vm->stackTop -= arity + 1;
        push(vm, result);

        return true;
      }

      return false;
    }
    throwRuntimeError(vm, "Senegal only allows methods on instances.");
    return false;
  }

  GCInstance* instance = AS_INSTANCE(receiver);

  Constant constant;
  if (tableGetEntry(&instance->fields, id, &constant)) {
    vm->stackTop[-arity - 1] = constant;
    return callConstant(vm, constant, arity);
  }

  return invokeFromClass(vm, instance->class, id, arity);
}

static InterpretationResult run(VM* vm) {
  register CallFrame* frame = &vm->frames[vm->frameCount - 1];

#define PEEK() (vm->stackTop[-1])
#define PEEK2() (vm->stackTop[-2])

#define PUSH(constant) *vm->stackTop++ = constant
#define POP() (*(--vm->stackTop))
#define POPN(count) (*(vm->stackTop -= (count)))

#define UPDATE_FRAME() (frame = &vm->frames[vm->frameCount - 1])

#define READ_BYTE() (*frame->pc++)
#define READ_SHORT() (frame->pc += 2, (uint16_t)((frame->pc[-2] << 8) | frame->pc[-1]))
#define READ_CONSTANT() (frame->closure->function->instructions.constants.constants[READ_BYTE()])
#define READ_CONSTANT_FROM_INDEX(i) (frame->closure->function->instructions.constants.constants[i])
#define READ_STRING() AS_STRING(READ_CONSTANT())

#define BINARY_OP(vm, pc, constantType, op) \
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

#define BITWISE_OP(vm, pc, constantType, op) \
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

#define CASE(code) case (code)
#define DISPATCH() goto run
#endif

  // === Labels ===
  RUN
  {
    CASE(OPCODE_TRUE):
    {
      Constant c = BOOL_CONST(true);
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_FALSE):
    {
      Constant c = BOOL_CONST(false);
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_AND):
    BITWISE_OP(vm, frame->pc, NUM_CONST, &);
    DISPATCH();

    CASE(OPCODE_OR):
    BITWISE_OP(vm, frame->pc, NUM_CONST, |);
    DISPATCH();

    CASE(OPCODE_XOR):
    BITWISE_OP(vm, frame->pc, NUM_CONST, ^);
    DISPATCH();

    CASE(OPCODE_LSHIFT):
    BITWISE_OP(vm, frame->pc, NUM_CONST, <<);
    DISPATCH();

    CASE(OPCODE_RSHIFT):
    BITWISE_OP(vm, frame->pc, NUM_CONST, >>);
    DISPATCH();

    CASE(OPCODE_BITNOT):
    if (!IS_NUMBER(PEEK())) {
      throwRuntimeError(vm, "Senegal encountered a non-number as an operand for OPCODE_NEG.");
      return RUNTIME_ERROR;
    }

    Constant c = NUM_CONST(~(int)AS_NUMBER(POP()));
    PUSH(c);
    DISPATCH();

    CASE(OPCODE_ADD):
    if (IS_STRING(PEEK()) && IS_STRING(PEEK2())) {
      concatenateStrings(vm);
    } else if (IS_NUMBER(PEEK()) && IS_NUMBER(PEEK2())) {
      BINARY_OP(vm, frame->pc, NUM_CONST, +);
    } else {
      throwRuntimeError(vm, "Senegal encountered an unexpected type while executing OPCODE_ADD.");
      return RUNTIME_ERROR;
    }
    DISPATCH();

    CASE(OPCODE_SUB):
    BINARY_OP(vm, frame->pc, NUM_CONST, -);
    DISPATCH();

    CASE(OPCODE_MUL):
    if (IS_STRING(PEEK2()) && IS_NUMBER(PEEK())) {

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
    } else if (IS_NUMBER(PEEK()) && IS_NUMBER(PEEK2())) {
      BINARY_OP(vm, frame->pc, NUM_CONST, *);
    } else {
      throwRuntimeError(vm, "Senegal encountered an unexpected type while executing OPCODE_MUL.");
      return RUNTIME_ERROR;
    }
    DISPATCH();

    CASE(OPCODE_DIV):
    BINARY_OP(vm, frame->pc, NUM_CONST, /);
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
    BINARY_OP(vm, frame->pc, BOOL_CONST, >);
    DISPATCH();

    CASE(OPCODE_LESSER):
    BINARY_OP(vm, frame->pc, BOOL_CONST, <);
    DISPATCH();

    CASE(OPCODE_GE):
    BINARY_OP(vm, frame->pc, BOOL_CONST, >=);
    DISPATCH();

    CASE(OPCODE_LE):
    BINARY_OP(vm, frame->pc, BOOL_CONST, <=);
    DISPATCH();

    CASE(OPCODE_NEG):
    {
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
    Constant c = BOOL_CONST(IS_NULL(constant) || (IS_BOOL(constant) && !AS_BOOL(constant)));
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_DUP):
    {
      Constant c = PEEK();
      PUSH(c);

      DISPATCH();
    }

    CASE(OPCODE_POP):
    POP();
    DISPATCH();

    CASE(OPCODE_POPN):
    {
      uint8_t count = READ_BYTE();
      POPN(count);
      DISPATCH();
    }

    CASE(OPCODE_LOAD):
    {
      Constant c = READ_CONSTANT();
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_LLOAD):
    {
      int index = READ_BYTE() | (READ_BYTE() << 8) | (READ_BYTE() << 16);
      Constant c = READ_CONSTANT_FROM_INDEX(index);
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_LOADN1):
    {
      Constant c = frame->closure->function->instructions.constants.constants[-1];
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_LOAD0):
    {
      Constant c = frame->closure->function->instructions.constants.constants[0];
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_LOAD1):
    {
      Constant c = frame->closure->function->instructions.constants.constants[1];
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_LOAD2):
    {
      Constant c = frame->closure->function->instructions.constants.constants[2];
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_LOAD3):
    {
      Constant c = frame->closure->function->instructions.constants.constants[3];
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_NEWCLASS):
    {
      Constant c = GC_OBJ_CONST(newClass(vm, READ_STRING(), false, false));
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_NEWFINALCLASS):
    {
      Constant c = GC_OBJ_CONST(newClass(vm, READ_STRING(), true, false));
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_NEWSTRICTCLASS):
    {
      Constant c = GC_OBJ_CONST(newClass(vm, READ_STRING(), false, true));
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_SETFIELD): {

    if (IS_CLASS(PEEK())) {
      GCClass *class = AS_CLASS(PEEK2());

      tableInsert(vm, &class->fields, READ_STRING(), PEEK());

      Constant constant = POP();

      POP();
      Constant c = constant;
      PUSH(c);

      DISPATCH();
    }

    if (!IS_INSTANCE(PEEK2())) {
      throwRuntimeError(vm, "Tried setting fields of non-class instance objects");
      return RUNTIME_ERROR;
    }

    GCInstance *instance = AS_INSTANCE(PEEK2());

    if (instance->class->isFinal) {
      throwRuntimeError(vm, "Senegal cannot mutate fields of a final class: %s", instance->class->id->chars);
      return RUNTIME_ERROR;
    }

    GCString *key = READ_STRING();

    Constant constant1;
    if (instance->class->isStrict && !tableGetEntry(&instance->fields, key, &constant1)) {
      throwRuntimeError(vm, "Senegal cannot insert fields in a strict class: %s", instance->class->id->chars);
      return RUNTIME_ERROR;
    }

    tableInsert(vm, &instance->fields, key, PEEK());

    Constant constant = POP();

    POP();
    Constant c = constant;
    PUSH(c);

    DISPATCH();
  }

    CASE(OPCODE_GETFIELD): {
    if (!IS_INSTANCE(PEEK())) {
      throwRuntimeError(vm, "Tried accessing fields of non-class instance objects");
      return RUNTIME_ERROR;
    }

    GCInstance *instance = AS_INSTANCE(PEEK());
    GCString *id = READ_STRING();

    Constant constant;
    if (tableGetEntry(&instance->fields, id, &constant)) {
      POP();
      Constant c = constant;
      PUSH(c);
      DISPATCH();
    }

    if (!bindMethod(vm, instance->class, id)) {
      return RUNTIME_ERROR;
    }

    DISPATCH();
  }

    CASE(OPCODE_METHOD):
    defineMethod(vm, READ_STRING());
    DISPATCH();

    CASE(OPCODE_INHERIT): {
    Constant super = PEEK2();

    if (!IS_CLASS(super)) {
      throwRuntimeError(vm, "Senegal classes can only inherit from another class");
      return RUNTIME_ERROR;
    }

    GCClass *sub = AS_CLASS(PEEK());
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

    Constant c = constant;
    PUSH(c);
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
    uint8_t slot = READ_BYTE();
    Constant c = frame->constants[slot];
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_GETLOC0): {
    Constant c = frame->constants[0];
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_GETLOC1): {
    Constant c = frame->constants[1];
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_GETLOC2): {
    Constant c = frame->constants[2];
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_GETLOC3): {
    Constant c = frame->constants[3];
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_GETLOC4): {
    Constant c = frame->constants[4];
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_GETLOC5): {
    Constant c = frame->constants[5];
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_SETLOC): {
    uint8_t slot = READ_BYTE();
    frame->constants[slot] = PEEK();
    DISPATCH();
  }

    CASE(OPCODE_SETLOC0): {
    frame->constants[0] = PEEK();
    DISPATCH();
  }

    CASE(OPCODE_SETLOC1): {
    frame->constants[1] = PEEK();
    DISPATCH();
  }

    CASE(OPCODE_SETLOC2): {
    frame->constants[2] = PEEK();
    DISPATCH();
  }

    CASE(OPCODE_SETLOC3): {
    frame->constants[3] = PEEK();
    DISPATCH();
  }

    CASE(OPCODE_SETLOC4): {
    frame->constants[4] = PEEK();
    DISPATCH();
  }

    CASE(OPCODE_SETLOC5): {
    frame->constants[5] = PEEK();
    DISPATCH();
  }

    CASE(OPCODE_GETUPVAL): {
    uint8_t slot = READ_BYTE();
    Constant c = *frame->closure->upvalues[slot]->place;
    PUSH(c);
    DISPATCH();
  }

    CASE(OPCODE_SETUPVAL): {
    uint8_t slot = READ_BYTE();
    *frame->closure->upvalues[slot]->place = PEEK();
    DISPATCH();
  }

    CASE(OPCODE_CLOSEUPVAL): {
    closeUpvalues(vm, vm->stackTop - 1);
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

    CASE(OPCODE_BREAK):
    {
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

    CASE(OPCODE_NULL):
    {
      Constant c = NULL_CONST;
      PUSH(c);
      DISPATCH();
    }

    CASE(OPCODE_RET): {
    Constant result = POP();

    closeUpvalues(vm, frame->constants);

    vm->frameCount--;

    if (vm->frameCount == 0) {
      POP();
      return OK;
    }

    vm->stackTop = frame->constants;
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


InterpretationResult interpret(VM* vm, const char* source) {

  Compiler compiler;
  GCFunction* function = compile(vm, &compiler, source);

  if (function == NULL)
    return COMPILE_TIME_ERROR;

  push(vm, GC_OBJ_CONST(function));

  GCClosure* closure = newClosure(vm, function);
  pop(vm);
  push(vm, GC_OBJ_CONST(closure));

  callConstant(vm, GC_OBJ_CONST(closure), 0);

  return run(vm);
}

void push(VM* vm, Constant constant) {
  *vm->stackTop++ = constant;
}

Constant pop(VM* vm) {
  return *(vm->stackTop--);
}

GCClass* newClass(VM *vm, GCString *id, bool isFinal, bool isStrict) {
  GCClass* class = ALLOCATE_GC_OBJ(vm, GCClass, GC_CLASS);
  class->id = id;
  class->isFinal = isFinal;
  class->isStrict = isStrict;
  initTable(&class->methods);

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
  initTable(&instance->fields);

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