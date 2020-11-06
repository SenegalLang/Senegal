#include "includes/sconstant_utils.h"
#include "includes/smemory.h"

void writeConstantPool(VM* vm, Compiler* compiler, ConstantPool *cp, Constant constant) {
  if (cp->capacity <= cp->count) {
    int oldCap = cp->capacity;
    cp->capacity = GROW_CAP(oldCap);
    cp->constants = GROW_ARRAY(vm, compiler, Constant,cp->constants, oldCap, cp->capacity);
  }

  cp->constants[cp->count] = constant;
  cp->count++;
}

void freeConstantPool(VM* vm, Compiler* compiler, ConstantPool *cp) {
  FREE_ARRAY(vm, compiler, Constant, cp->constants, cp->capacity);
  initConstantPool(cp);
}