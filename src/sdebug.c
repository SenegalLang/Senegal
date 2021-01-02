#include <stdio.h>

#include "includes/sdebug.h"
#include "includes/svm.h"
#include "includes/sopcodes.h"

// Used to ensure a consistent formatting
#define TABS "\t\t"

void disassembleInstructions(Instructions *instructions, const char *name) {
  //        <<< name >>>
  printf("\t\t\t\t%s<<< %s >>>%s\n", YELLOW, name, DEFAULT);

  //  | PC |    (line)       OPCODE       Constant
  printf("%2$s| PC |%1$s%3$s(line)%1$s%4$s    OPCODE   %1$s%1$s%5$sConstant%6$s\n",
         TABS, GREEN, CYAN, PURPLE, BLUE, DEFAULT);

  for (int offset = 0; offset < instructions->bytesCount;) {
    offset = disassembleInstruction(instructions, offset);
  }

  printf("\n");
}

static int noOperandInstruction(const char* name, int offset) {
  printf("%s%s%s\n", PURPLE, name, DEFAULT);
  return offset + 1;
}

static int loadNInstruction(const char* name, Instructions *instructions, int n, int offset) {
  uint8_t constant = instructions->bytes[n + 1];
  printf("%s%-16s%s%s%4d ", PURPLE, name, TABS, BLUE, constant);
  printConstant(stdout, instructions->constants.constants[constant]);
  printf("\n");

  return offset + 1;
}

static int loadInstruction(const char* name, Instructions *instructions, int offset) {
  uint8_t constant = instructions->bytes[offset + 1];
  printf("%s%-16s%s%s%4d ", PURPLE, name, TABS, BLUE, constant);
  printConstant(stdout, instructions->constants.constants[constant]);
  printf("\n");

  return offset + 2;
}

static int lLoadInstruction(const char* name, Instructions *instructions, int offset) {
  uint8_t constant = instructions->bytes[offset + 1]
                     | (instructions->bytes[offset + 1] << 8)
                     | (instructions->bytes[offset + 1] << 16);

  printf("%s%-16s%s%s%4d ", PURPLE, name, TABS, BLUE, constant);
  printConstant(stdout, instructions->constants.constants[constant]);
  printf("\n");

  return offset + 4;
}

static int byteInstruction(const char* name, Instructions* instructions, int offset) {
  uint8_t slot = instructions->bytes[offset + 1];
  printf("%s%-16s%s%s%4d\n", PURPLE, name, TABS, BLUE, slot);

  return offset + 2;
}

static int byteNInstruction(const char* name, Instructions* instructions, int n, int offset) {
  uint8_t slot = instructions->bytes[n + 1];
  printf("%s%-16s%s%s%4d\n", PURPLE, name, TABS, BLUE, slot);

  return offset + 1;
}

static int jmpInstruction(const char* name, int sign, Instructions* instructions, int offset) {
  uint16_t jump = (uint16_t)(instructions->bytes[offset + 1] << 8) | instructions->bytes[offset + 2];
  printf("%s%-16s%s%s%4d -> %d\n", PURPLE, name, TABS, BLUE, offset, offset + 3 + sign * jump);

  return offset + 3;
}

static int invokeInstruction(const char* id, Instructions* i, int offset) {
  uint8_t constant = i->bytes[offset + 1];
  uint8_t arity = i->bytes[offset + 2];

  printf("%s%-16s%s%s(%d args) %4d '", PURPLE, id, TABS, BLUE, arity, constant);
  printConstant(stdout, i->constants.constants[constant]);
  printf("\n");

  return offset + 3;
}

int disassembleInstruction(Instructions *instructions, int offset) {
  printf("%s|%04X|%s", GREEN, offset, DEFAULT);

  int line = getLine(instructions, offset);

    printf("%s(%s%4d%s)%s", TABS, CYAN, line, DEFAULT, TABS);

  uint8_t opcode = instructions->bytes[offset];

  switch (opcode) {
    case OPCODE_INC:
      return noOperandInstruction("OPCODE_INC", offset);

    case OPCODE_DEC:
      return noOperandInstruction("OPCODE_DEC", offset);

    case OPCODE_POW:
      return noOperandInstruction("OPCODE_POW", offset);

    case OPCODE_ADD:
      return noOperandInstruction("OPCODE_ADD", offset);

    case OPCODE_SUB:
      return noOperandInstruction("OPCODE_SUB", offset);

    case OPCODE_MUL:
      return noOperandInstruction("OPCODE_MUL", offset);

    case OPCODE_DIV:
      return noOperandInstruction("OPCODE_DIV", offset);

    case OPCODE_NEG:
      return noOperandInstruction("OPCODE_NEG", offset);

    case OPCODE_NOT:
      return noOperandInstruction("OPCODE_NOT", offset);

    case OPCODE_LOAD:
      return loadInstruction("OPCODE_LOAD", instructions, offset);

    case OPCODE_LOADN1:
      return loadNInstruction("OPCODE_LOADN1", instructions, -1, offset);

    case OPCODE_LOAD0:
      return loadNInstruction("OPCODE_LOAD0", instructions, 0, offset);

    case OPCODE_LOAD1:
      return loadNInstruction("OPCODE_LOAD1", instructions, 1, offset);

    case OPCODE_LOAD2:
      return loadNInstruction("OPCODE_LOAD2", instructions, 2, offset);

    case OPCODE_LOAD3:
      return loadNInstruction("OPCODE_LOAD3", instructions, 3, offset);

    case OPCODE_LLOAD:
      return lLoadInstruction("OPCODE_LLOAD", instructions, offset);

    case OPCODE_TRUE:
      return noOperandInstruction("OPCODE_TRUE", offset);

    case OPCODE_FALSE:
      return noOperandInstruction("OPCODE_FALSE", offset);

    case OPCODE_EQUAL:
      return noOperandInstruction("OPCODE_EQUAL", offset);

    case OPCODE_NOTEQ:
      return noOperandInstruction("OPCODE_NOTEQ", offset);

    case OPCODE_GREATER:
      return noOperandInstruction("OPCODE_GREATER", offset);

    case OPCODE_LESSER:
      return noOperandInstruction("OPCODE_LESS", offset);

    case OPCODE_GE:
      return noOperandInstruction("OPCODE_GE", offset);

    case OPCODE_LE:
      return noOperandInstruction("OPCODE_LE", offset);

    case OPCODE_JMP:
      return jmpInstruction("OPCODE_JMP", 1, instructions, offset);

    case OPCODE_JF:
      return jmpInstruction("OPCODE_JF", 1, instructions, offset);

    case OPCODE_LOOP:
      return jmpInstruction("OPCODE_LOOP", 1, instructions, offset);

    case OPCODE_DUP:
      return noOperandInstruction("OPCODE_DUP", offset);

    case OPCODE_POP:
      return noOperandInstruction("OPCODE_POP", offset);

    case OPCODE_POPN:
      return loadInstruction("OPCODE_POPN", instructions, offset);

    case OPCODE_CALL:
      return byteInstruction("OPCODE_CALL", instructions, offset);

    case OPCODE_CALL0:
      return byteNInstruction("OPCODE_CALL0", instructions, 0, offset);

    case OPCODE_CALL1:
      return byteNInstruction("OPCODE_CALL1", instructions, 1, offset);

    case OPCODE_CALL2:
      return byteNInstruction("OPCODE_CALL2", instructions, 2, offset);

    case OPCODE_CALL3:
      return byteNInstruction("OPCODE_CALL3", instructions, 3, offset);

    case OPCODE_CALL4:
      return byteNInstruction("OPCODE_CALL4", instructions, 4, offset);

    case OPCODE_CALL5:
      return byteNInstruction("OPCODE_CALL5", instructions, 5, offset);

    case OPCODE_CALL6:
      return byteNInstruction("OPCODE_CALL6", instructions, 6, offset);

    case OPCODE_CALL7:
      return byteNInstruction("OPCODE_CALL7", instructions, 7, offset);

    case OPCODE_CALL8:
      return byteNInstruction("OPCODE_CALL8", instructions, 8, offset);

    case OPCODE_CLOSURE: {
      offset++;
      uint8_t constant = instructions->bytes[offset++];
      printf("%s%-16s%s%s%4d%s ", PURPLE, "OPCODE_CLOSURE", TABS, BLUE, constant, DEFAULT);
      printConstant(stdout, instructions->constants.constants[constant]);
      printf("\n");

      GCFunction* function = AS_FUNCTION(instructions->constants.constants[constant]);

      for (int j = 0; j < function->upvalueCount; j++) {
        int isLocal = instructions->bytes[offset++];
        int index = instructions->bytes[offset++];
        printf("%04d |   %s %d\n", offset - 2, isLocal ? "local" : "upvalue", index);
      }

      return offset;
    }

    case OPCODE_SUPERINVOKE:
    case OPCODE_INVOKE:
      return invokeInstruction("OPCODE_INVOKE", instructions, offset);

    case OPCODE_NEWMAP:
      return loadInstruction("OPCODE_NEWMAP", instructions, offset);

    case OPCODE_NEWLIST:
      return loadInstruction("OPCODE_NEWLIST", instructions, offset);

    case OPCODE_ACCESS:
      return loadInstruction("OPCODE_ACCESS", instructions, offset);

    case OPCODE_SETACCESS:
      return invokeInstruction("OPCODE_SETACCESS", instructions, offset);

    case OPCODE_NEWFINALCLASS:
    case OPCODE_NEWCLASS:
      return loadInstruction("OPCODE_NEWCLASS", instructions, offset);

    case OPCODE_INHERIT:
      return noOperandInstruction("OPCODE_INHERIT", offset);

    case OPCODE_GETSUPER:
      return loadInstruction("OPCODE_GETSUPER", instructions, offset);

    case OPCODE_GETFIELD:
      return loadInstruction("OPCODE_GETFIELD", instructions, offset);

    case OPCODE_SETFIELD:
      return loadInstruction("OPCODE_SETFIELD", instructions, offset);

    case OPCODE_NEWSTATICMETHOD:
    case OPCODE_NEWMETHOD:
      return loadInstruction("OPCODE_NEWMETHOD", instructions, offset);

    case OPCODE_NEWFIELD:
    case OPCODE_NEWSTATICFIELD:
      return loadInstruction("OPCODE_NEWFIELD", instructions, offset);

    case OPCODE_NEWGLOB:
      return loadInstruction("OPCODE_NEWGLOB", instructions, offset);

    case OPCODE_GETGLOB:
      return loadInstruction("OPCODE_GETGLOB", instructions, offset);

    case OPCODE_SETGLOB:
      return byteInstruction("OPCODE_SETGLOB", instructions, offset);

    case OPCODE_GETLOC:
      return byteInstruction("OPCODE_GETLOC", instructions, offset);

    case OPCODE_GETLOC0:
      return loadNInstruction("OPCODE_GETLOC0", instructions, 0, offset);

    case OPCODE_GETLOC1:
      return loadNInstruction("OPCODE_GETLOC1", instructions, 1, offset);

    case OPCODE_GETLOC2:
      return loadNInstruction("OPCODE_GETLOC2", instructions, 2, offset);

    case OPCODE_GETLOC3:
      return loadNInstruction("OPCODE_GETLOC3", instructions, 3, offset);

    case OPCODE_GETLOC4:
      return loadNInstruction("OPCODE_GETLOC4", instructions, 4, offset);

    case OPCODE_GETLOC5:
      return loadNInstruction("OPCODE_GETLOC5", instructions, 5, offset);

    case OPCODE_SETLOC:
      return loadInstruction("OPCODE_SETLOC", instructions, offset);

    case OPCODE_SETLOC0:
      return loadNInstruction("OPCODE_SETLOC0", instructions, 0, offset);

    case OPCODE_SETLOC1:
      return loadNInstruction("OPCODE_SETLOC1", instructions, 1, offset);

    case OPCODE_SETLOC2:
      return loadNInstruction("OPCODE_SETLOC2", instructions, 2, offset);

    case OPCODE_SETLOC3:
      return loadNInstruction("OPCODE_SETLOC3", instructions, 3, offset);

    case OPCODE_SETLOC4:
      return loadNInstruction("OPCODE_SETLOC4", instructions, 4, offset);

    case OPCODE_SETLOC5:
      return loadNInstruction("OPCODE_SETLOC5", instructions, 5, offset);

    case OPCODE_GETUPVAL:
      return byteInstruction("OPCODE_GETUPVAL", instructions, offset);

    case OPCODE_SETUPVAL:
      return loadInstruction("OPCODE_SETUPVAL", instructions, offset);

    case OPCODE_CLOSEUPVAL:
      return noOperandInstruction("OPCODE_CLOSEUPVAL", offset);

    case OPCODE_NULL:
      return noOperandInstruction("OPCODE_NULL", offset);

    case OPCODE_RET:
      return noOperandInstruction("OPCODE_RET", offset);

    default:
      printf("Unknown opcode %d\n", opcode);
      return offset + 1;
  }
}