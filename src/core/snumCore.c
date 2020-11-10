#include <stdio.h>
#include <float.h>
#include <math.h>
#include "snumCore.h"
#include "../includes/sparser.h"
#include "../includes/sapi.h"

//#define NAN (0.0 / 0.0)
//#define INFINITY (1.0 / 0.0)
#define NEGATIVE_INFINITY (-1.0 / 0.0)
#define MAX_FINITE 1.7976931348623157e+308
#define MIN_POSITIVE 5e-324

static Constant numIsFinite(VM* vm, int arity, Constant* args) {
  register double num = AS_NUMBER(args[0]);
  return BOOL_CONST(num != INFINITY && num != NEGATIVE_INFINITY && num != NAN);
}

static Constant numIsInfinite(VM* vm, int arity, Constant* args) {
  register double num = AS_NUMBER(args[0]);
  return BOOL_CONST(num == INFINITY ||num == NEGATIVE_INFINITY);
}

static Constant numToString(VM* vm, int arity, Constant* args) {

  char numString[3 + DBL_MANT_DIG - DBL_MIN_EXP];

  sprintf(numString, "%lf", AS_NUMBER(args[0]));

  return GC_OBJ_CONST(copyString(vm, NULL, numString, strlen(numString)));
}

static Constant numIsNan(VM* vm, int arity, Constant* args) {
  return BOOL_CONST(AS_NUMBER(args[0]) == NAN);
}

static Constant numIsNeg(VM* vm, int arity, Constant* args) {
  return BOOL_CONST(AS_NUMBER(args[0]) < 0);
}

static Constant numAbs(VM* vm, int arity, Constant* args) {
  return NUM_CONST(fabs(AS_NUMBER(args[0])));
}

static Constant numCeil(VM* vm, int arity, Constant* args) {
  return NUM_CONST(ceil(AS_NUMBER(args[0])));
}

static Constant numClamp(VM* vm, int arity, Constant* args) {
  double num = AS_NUMBER(args[-1]);
  double upperbound = AS_NUMBER(args[0]);
  double lowerbound = AS_NUMBER(args[1]);

  if (num < lowerbound)
    return NUM_CONST(lowerbound);
  else if (num > upperbound)
    return NUM_CONST(upperbound);

  return NUM_CONST(num);
}

static Constant numCompareTo(VM* vm, int arity, Constant* args) {
  double num = AS_NUMBER(args[-1]);
  double other = AS_NUMBER(args[0]);

  if (num < other)
    return -1;
  else if (num > other)
    return 1;

  return 0;
}

static Constant numFloor(VM* vm, int arity, Constant* args) {
  return NUM_CONST(floor(AS_NUMBER(args[0])));
}

static Constant numRemainder(VM* vm, int arity, Constant* args) {
  double num = AS_NUMBER(args[-1]);
  double other = AS_NUMBER(args[0]);

  double quotient = other / num;

  double remainder = other - num * floor(quotient);

  return NUM_CONST(remainder);
}

void initNumClass(VM *vm) {
  vm->numClass = newClass(vm, copyString(vm, NULL, "num", 3), false, false);
  defineClassNativeFunc(vm, "isFinite", numIsFinite, vm->numClass);
  defineClassNativeFunc(vm, "isInfinite", numIsInfinite, vm->numClass);
  defineClassNativeFunc(vm, "toString", numToString, vm->numClass);
  defineClassNativeFunc(vm, "isNaN", numIsNan, vm->numClass);
  defineClassNativeFunc(vm, "isNeg", numIsNeg, vm->numClass);

  defineClassNativeFunc(vm, "abs", numAbs, vm->numClass);
  defineClassNativeFunc(vm, "ceil", numCeil, vm->numClass);
  defineClassNativeFunc(vm, "clamp", numClamp, vm->numClass);
  defineClassNativeFunc(vm, "compareTo", numCompareTo, vm->numClass);
  defineClassNativeFunc(vm, "floor", numFloor, vm->numClass);
  defineClassNativeFunc(vm, "remainder", numRemainder, vm->numClass);

  defineClassNativeField(vm, "type", GC_OBJ_CONST(copyString(vm, NULL, "num", 3)), vm->numClass);
  defineClassNativeField(vm, "nan", NUM_CONST(NAN), vm->numClass);
  defineClassNativeField(vm, "infinity", NUM_CONST(INFINITY), vm->numClass);
  defineClassNativeField(vm, "negInfinity", NUM_CONST(NEGATIVE_INFINITY), vm->numClass);
  defineClassNativeField(vm, "maxFinite", NUM_CONST(MAX_FINITE), vm->numClass);
  defineClassNativeField(vm, "minPositive", NUM_CONST(MIN_POSITIVE), vm->numClass);
}
