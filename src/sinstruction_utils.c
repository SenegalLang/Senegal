#include "includes/sinstruction_utils.h"
#include "includes/smemory.h"
#include "includes/sconstant_utils.h"

void writeInstructions(VM* vm, Instructions *instructions, uint8_t byte, int line) {

  if (instructions->bytesCapacity <= instructions->bytesCount) {
    int oldCap = instructions->bytesCapacity;
    instructions->bytesCapacity = GROW_CAP(oldCap);
    instructions->bytes = GROW_ARRAY(vm, NULL, uint8_t, instructions->bytes, oldCap, instructions->bytesCapacity);
  }

  instructions->bytes[instructions->bytesCount] = byte;
  instructions->bytesCount++;

  if (instructions->lineC > 0 && instructions->lines[instructions->lineC - 1].line == line)
    return;

  if (instructions->lineCap <= instructions->lineC) {
    int oldCap = instructions->lineCap;
    instructions->lineCap = GROW_CAP(oldCap);
    instructions->lines = GROW_ARRAY(vm, NULL, Line, instructions->lines, oldCap, instructions->lineCap);
  }

  Line* newLine = &instructions->lines[instructions->lineC++];
  newLine->offset = instructions->bytesCount - 1;
  newLine->line = line;
}

void freeInstructions(VM* vm, Instructions *instructions) {
  FREE_ARRAY(vm, NULL, uint8_t, instructions->bytes, instructions->bytesCapacity);
  FREE_ARRAY(vm, NULL, Line, instructions->lines, instructions->lineCap);
  freeConstantPool(vm, NULL, &instructions->constants);
  initInstructions(instructions);
}

int addConstant(VM* vm, Compiler* compiler, Instructions *instructions, Constant constant) {
  push(vm, constant);
  writeConstantPool(vm, compiler, &instructions->constants, constant);
  pop(vm);
  return instructions->constants.count - 1;
}