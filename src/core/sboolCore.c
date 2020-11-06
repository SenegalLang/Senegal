#include "sboolCore.h"
#include "../includes/sparser.h"

Constant boolToString(VM* vm, int arity, Constant *args) {
  bool isTrue = AS_BOOL(args[0]);

  return GC_OBJ_CONST(getString(vm, isTrue ? "true" : "false", isTrue ? 4 : 5));
}
