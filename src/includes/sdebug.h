#include "sinstructions.h"

#ifndef SENEGAL_DEBUG_H
#define SENEGAL_DEBUG_H

void disassembleInstructions(Instructions* instructions, const char* name);
int disassembleInstruction(Instructions* instructions, int offset);

#endif //SENEGAL_DEBUG_H
