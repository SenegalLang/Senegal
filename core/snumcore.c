#include <stdio.h>
#include <float.h>
#include <math.h>
#include "includes/snumcore.h"
#include "../src/includes/sparser.h"
#include "includes/sapi.h"

//#define NAN (0.0 / 0.0)
#define INF (1.0 / 0.0)
#define NEGATIVE_INF (-1.0 / 0.0)
#define MAX_FINITE 1.7976931348623157e+308
#define MIN_POSITIVE 5e-324

static Constant numAsBool(VM* vm, int arity, Constant* args) {
  register double num = AS_NUMBER(args[-1]);
  return BOOL_CONST(num != 0);
}

static Constant numIsFinite(VM* vm, int arity, Constant* args) {
  register double num = AS_NUMBER(args[-1]);
  return BOOL_CONST(num != INF && num != NEGATIVE_INF && num != NAN);
}

static Constant numIsInfinite(VM* vm, int arity, Constant* args) {
  register double num = AS_NUMBER(args[-1]);
  return BOOL_CONST(num == INF || num == NEGATIVE_INF);
}

static Constant numToString(VM* vm, int arity, Constant* args) {

  char* numString = constantToString(args[-1]);

  return GC_OBJ_CONST(copyString(vm, NULL, numString, strlen(numString)));
}

static Constant numIsNan(VM* vm, int arity, Constant* args) {
  return BOOL_CONST(AS_NUMBER(args[-1]) == (0.0 / 0.0));
}

static Constant numIsNeg(VM* vm, int arity, Constant* args) {
  return BOOL_CONST(AS_NUMBER(args[-1]) < 0);
}

static Constant numAbs(VM* vm, int arity, Constant* args) {
  return NUM_CONST(fabs(AS_NUMBER(args[-1])));
}

static Constant numCeil(VM* vm, int arity, Constant* args) {
  return NUM_CONST(ceil(AS_NUMBER(args[-1])));
}

static Constant numClamp(VM* vm, int arity, Constant* args) {
  expect(2, arity, "clamp");

  double num = AS_NUMBER(args[-1]);
  double lowerbound = AS_NUMBER(args[0]);
  double upperbound = AS_NUMBER(args[1]);

  if (num < lowerbound)
    return NUM_CONST(lowerbound);
  else if (num > upperbound)
    return NUM_CONST(upperbound);

  return NUM_CONST(num);
}

static Constant numCompareTo(VM* vm, int arity, Constant* args) {
  expect(1, arity, "compareTo");

  double num = AS_NUMBER(args[-1]);
  double other = AS_NUMBER(args[0]);

  if (num < other)
    return NUM_CONST(-1);

  if (num > other)
    return NUM_CONST(1);

  return NUM_CONST(0);
}

static Constant numFloor(VM* vm, int arity, Constant* args) {
  return NUM_CONST(floor(AS_NUMBER(args[-1])));
}

static Constant numRemainder(VM* vm, int arity, Constant* args) {
  expect(1, arity, "remainder");

  double num = AS_NUMBER(args[-1]);
  double other = AS_NUMBER(args[0]);

  double quotient = other / num;

  double remainder = other - num * floor(quotient);

  return NUM_CONST(remainder);
}

void initNumClass(VM *vm) {
  vm->numClass = newClass(vm, copyString(vm, NULL, "num", 3), true);

  defineClassNativeMethod(vm, "abs", numAbs, vm->numClass);
  defineClassNativeMethod(vm, "asBool", numAsBool, vm->numClass);
  defineClassNativeMethod(vm, "ceil", numCeil, vm->numClass);
  defineClassNativeMethod(vm, "clamp", numClamp, vm->numClass);
  defineClassNativeMethod(vm, "compareTo", numCompareTo, vm->numClass);
  defineClassNativeMethod(vm, "floor", numFloor, vm->numClass);
  defineClassNativeMethod(vm, "isFinite", numIsFinite, vm->numClass);
  defineClassNativeMethod(vm, "isInfinite", numIsInfinite, vm->numClass);
  defineClassNativeMethod(vm, "isNaN", numIsNan, vm->numClass);
  defineClassNativeMethod(vm, "isNeg", numIsNeg, vm->numClass);
  defineClassNativeMethod(vm, "remainder", numRemainder, vm->numClass);
  defineClassNativeMethod(vm, "toString", numToString, vm->numClass);

  defineClassNativeStaticField(vm, "nan", NUM_CONST(NAN), vm->numClass);
  defineClassNativeStaticField(vm, "infinity", NUM_CONST(INF), vm->numClass);
  defineClassNativeStaticField(vm, "negInfinity", NUM_CONST(NEGATIVE_INF), vm->numClass);
  defineClassNativeStaticField(vm, "maxFinite", NUM_CONST(MAX_FINITE), vm->numClass);
  defineClassNativeStaticField(vm, "minPositive", NUM_CONST(MIN_POSITIVE), vm->numClass);
}
