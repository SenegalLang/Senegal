#ifndef SENEGAL_SINSTRUCTIONS_H
#define SENEGAL_SINSTRUCTIONS_H

#include "sutils.h"
#include "sconstant.h"


typedef enum {
#define OPCODE(code) code,
#include "sopcodes.h"
#undef OPCODE
} Opcode;

typedef struct {
    int offset;
    int line;
} Line;

typedef struct {

    uint8_t* bytes;
    int bytesCapacity;
    int bytesCount;

    ConstantPool constants;

    int lineC;
    int lineCap;
    Line* lines;
} Instructions;

void initInstructions(Instructions* instructions);

int getLine(Instructions* instructions, int i);

#endif //SENEGAL_SINSTRUCTIONS_H