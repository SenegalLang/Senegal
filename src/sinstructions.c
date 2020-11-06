#include <stdlib.h>

#include "includes/sinstructions.h"

void initInstructions(Instructions* instructions) {
  instructions->lines = NULL;
  instructions->bytesCount = 0;
  instructions->bytesCapacity = 0;
  instructions->bytes = NULL;
  instructions->lineC = 0;
  instructions->lineCap = 0;
  instructions->lines = NULL;
  initConstantPool(&instructions->constants);
}

int getLine(Instructions *instructions, int i) {
  int start = 0;
  int end = instructions->lineC - 1;

  for (;;) {
    int mid = (start + end) / 2;
    Line * line = &instructions->lines[mid];
    if (i < line->offset) {
      end = mid - 1;
    } else if (mid == instructions->lineC - 1 ||
               i < instructions->lines[mid + 1].offset) {
      return line->line;
    } else {
      start = mid + 1;
    }
  }
}