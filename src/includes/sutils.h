#ifndef SENEGAL_SUTILS_H
#define SENEGAL_SUTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <mem.h>

#define NAN_TAGGING 1
#define COMPUTED_GOTO 1

#define DEBUG_LOG_GC 0
#define DEBUG_PRINT_CODE 0
#define DEBUG_TRACE_EXECUTION 0

#define UINT8_COUNT (UINT8_MAX + 1)

char* readFile(const char* path);

static inline char* substring(const char* input, int offset, int len, char* dest)
{
  int input_len = strlen(input);

  if (offset + len > input_len)
  {
    return NULL;
  }

  strncpy(dest, input + offset, len);
  return dest;
}


#endif //SENEGAL_SUTILS_H