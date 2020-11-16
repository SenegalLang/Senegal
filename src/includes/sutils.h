#ifndef SENEGAL_SUTILS_H
#define SENEGAL_SUTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define NAN_TAGGING 1
#define COMPUTED_GOTO 1

#define DEBUG_LOG_GC 0
#define DEBUG_PRINT_CODE 0

#define UINT8_COUNT (UINT8_MAX + 1)

char* readFile(const char* path);

int removeCharFromIndex(const char *src, char *dst, int index);

static inline int power(int x, unsigned int y) {
  int temp;
  if (y == 0)
    return 1;

  temp = power(x, y / 2);
  if ((y % 2) == 0)
    return temp * temp;
  else
    return x * temp * temp;
}

#endif //SENEGAL_SUTILS_H