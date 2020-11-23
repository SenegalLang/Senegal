#include "includes/smathlib.h"
#include "includes/sapi.h"

#include <math.h>
#include <stdlib.h>
#include <time.h>

#define PI 3.14159265358979323846

static Constant sglMin(VM* vm, int arity, Constant* args) {
  int left = AS_NUMBER(args[0]);
  int right = AS_NUMBER(args[1]);

  if (right < left) 
    return NUM_CONST(right);

  return NUM_CONST(left);
}

static Constant sglMax(VM* vm, int arity, Constant* args) {
  int left = AS_NUMBER(args[0]);
  int right = AS_NUMBER(args[1]);

  if (right > left) 
    return NUM_CONST(right);

  return NUM_CONST(left);
}

static Constant sglRemap(VM* vm, int arity, Constant* args) {
  double value = AS_NUMBER(args[0]);

  double fromMin = AS_NUMBER(args[1]);
  double fromMax = AS_NUMBER(args[2]);

  double toMin = AS_NUMBER(args[3]);
  double toMax = AS_NUMBER(args[4]);

  return NUM_CONST((value - fromMin ) * (toMax - toMin) / (fromMax - fromMin) + toMin);
}

static Constant sglSin(VM* vm, int arity, Constant* args) { 
  double angle = AS_NUMBER(args[0]);

  return NUM_CONST(sin(angle));
}

static Constant sglCos(VM* vm, int arity, Constant* args) { 
  double angle = AS_NUMBER(args[0]);

  return NUM_CONST(cos(angle));
}

static Constant sglTan(VM* vm, int arity, Constant* args) { 
  double angle = AS_NUMBER(args[0]);

  return NUM_CONST(tan(angle));
}

static Constant sglAsin(VM* vm, int arity, Constant* args) { 
  double value = AS_NUMBER(args[0]);

  return NUM_CONST(asin(value));
}

static Constant sglAcos(VM* vm, int arity, Constant* args) { 
  double value = AS_NUMBER(args[0]);

  return NUM_CONST(acos(value));
}

static Constant sglAtan(VM* vm, int arity, Constant* args) { 
  double value = AS_NUMBER(args[0]);

  return NUM_CONST(atan(value));
}

static Constant sglAtan2(VM* vm, int arity, Constant* args) { 
  double y = AS_NUMBER(args[0]);
  double x = AS_NUMBER(args[1]);

  return NUM_CONST(atan2(y, x));
}

static Constant sglToDegrees(VM* vm, int arity, Constant* args) { 
  double x = AS_NUMBER(args[0]);

  return NUM_CONST(x * (180.0 / PI));
}

static Constant sglToRadians(VM* vm, int arity, Constant* args) { 
  double x = AS_NUMBER(args[0]);

  return NUM_CONST(x * (PI/ 180.0));
}

static Constant sglLog(VM* vm, int arity, Constant* args) { 
  double n = AS_NUMBER(args[0]);

  return NUM_CONST(log(n));
}

static Constant sglGcd(VM* vm, int arity, Constant* args) { 
  int n1 = AS_NUMBER(args[0]);
  int n2 = AS_NUMBER(args[1]);
  int gcd;

  n1 = ( n1 > 0) ? n1 : -n1;
  n2 = ( n2 > 0) ? n2 : -n2;

  for (int i = 1; i <= n1 && i <= n2; ++i) {
    if (n1 % i == 0 && n2 % i == 0) 
      gcd = i;
  }

  return NUM_CONST(gcd);
}

static Constant sglRound(VM* vm, int arity, Constant* args) { 
  double n = AS_NUMBER(args[0]);

  return NUM_CONST(round(n));
}

static Constant sglRandom(VM* vm, int arity, Constant* args) { 
  int minRng = AS_NUMBER(args[0]);
  int maxRng = AS_NUMBER(args[1]);
  
  return NUM_CONST((rand() % ( maxRng - minRng + 1 )) + minRng);
}

Constant initMathLib(VM* vm, int arity, Constant* args) {
  srand(time(NULL));

  // ============= VARIABLES =============

  // Base of natural logarithms, e.
  defineGlobal(vm, "e", NUM_CONST(2.718281828459045));

  // Closest representable value to the natural logarithm of base 10.
  defineGlobal(vm, "ln10", NUM_CONST(2.302585092994046));

  // Closest representable value to the natural logarithm of base 2.
  defineGlobal(vm, "ln2", NUM_CONST(0.6931471805599453));

  // 2nd base log of e.
  defineGlobal(vm, "log2e", NUM_CONST(1.4426950408889634));

  // 10th base logarithm of e.
  defineGlobal(vm, "log10e", NUM_CONST(0.4342944819032518));

  defineGlobal(vm, "pi", NUM_CONST(3.1415926535897932));

  // Square root of 1/2
  defineGlobal(vm, "sqrt1_2", NUM_CONST(0.7071067811865476));

  // Square root of 2
  defineGlobal(vm, "sqrt2", NUM_CONST(1.4142135623730951));

  // ============= FUNCTIONS =============

  // Determines the smallest value between 2 numbers, and then returns that value.
  defineGlobalFunc(vm, "min", sglMin);

  // Determines the largest value between 2 numbers, and then returns that value.
  defineGlobalFunc(vm, "max", sglMax);

  // Re-maps a number from one range to another.
  defineGlobalFunc(vm, "remap", sglRemap);

  // Calculates the sine of an angle. Values are returned in the range -1 to 1.
  defineGlobalFunc(vm, "sin", sglSin);

  // Calculates the cosine of an angle. Values are returned in the range -1 to 1.
  defineGlobalFunc(vm, "cos", sglCos);

  // Calculates the tangent of an angle. Values are returned in the range of all real numbers.
  defineGlobalFunc(vm, "tan", sglTan);

  // The inverse of sin(), returns the arc sine of a value.
  defineGlobalFunc(vm, "asin", sglAsin);

  // The inverse of cos(), returns the arc cosine of a value.
  defineGlobalFunc(vm, "acos", sglAcos);

  // The inverse of tan(), returns the arc tangent of a value.
  defineGlobalFunc(vm, "atan", sglAtan);

  // Calculates the angle (in radians) from a specified point to the coordinate origin as measured from the positive x-axis.
  // Note: The y-coordinate of the point is the first parameter, and the x-coordinate is the second parameter, due the the structure of calculating the tangent.
  defineGlobalFunc(vm, "atan2", sglAtan2);

  // Convert angle x from radians to degrees.
  defineGlobalFunc(vm, "toDegrees", sglToDegrees);

  // Convert angle x from degrees to radians.
  defineGlobalFunc(vm, "toRadians", sglToRadians);

  // Return the greatest common divisor of the specified integer arguments in a List.
  defineGlobalFunc(vm, "gcd", sglGcd);

  // Calculates the natural logarithm (the base-e logarithm) of a number. This function expects the n parameter to be a value greater than 0.0.
  defineGlobalFunc(vm, "log", sglLog);

  // Calculates the integer closest to the n parameter.
  defineGlobalFunc(vm, "round", sglRound);

  // Returns a random number in between the provided range.
  defineGlobalFunc(vm, "random", sglRandom);
}