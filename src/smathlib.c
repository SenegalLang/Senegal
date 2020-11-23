#include "includes/smathlib.h"
#include "includes/sapi.h"

Constant initMathLib(VM* vm, int arity, Constant* args) {
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

  defineGlobal(vm, "pi", NUM_CONST(3.14159265358979323846));

  // Square root of 1/2
  defineGlobal(vm, "sqrt1_2", NUM_CONST(0.7071067811865476));

  // Square root of 2
  defineGlobal(vm, "sqrt2", NUM_CONST(1.4142135623730951));
}