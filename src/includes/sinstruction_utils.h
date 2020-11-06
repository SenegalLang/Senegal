#ifndef SENEGAL_SINSTRUCTION_UTILS_H
#define SENEGAL_SINSTRUCTION_UTILS_H

#include "svm.h"
#include "sparser.h"

void writeInstructions(VM* vm, Instructions* instructions, uint8_t byte, int line);
void freeInstructions(VM* vm, Instructions* instructions);

int addConstant(VM* vm, Compiler* compiler, Instructions *instructions, Constant constant);

#endif //SENEGAL_SINSTRUCTION_UTILS_H