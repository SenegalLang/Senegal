#ifndef SENEGAL_SIOLIB_H
#define SENEGAL_SIOLIB_H

#include "../../src/includes/svm.h"

static inline size_t sglGetLine(char **lineptr, size_t *n, FILE *stream) {
  char *bufptr = NULL;
  char *p = bufptr;
  size_t size;
  int c;

  if (!lineptr || !stream || !n)
    return -1;

  bufptr = *lineptr;
  size = *n;

  c = fgetc(stream);
  if (c == EOF)
    return -1;

  if (!bufptr) {
    bufptr = malloc(128);

    if (!bufptr)
      return -1;

    size = 128;
  }

  p = bufptr;

  while(c != EOF) {
    if ((p - bufptr) > (size - 1)) {
      size = size + 128;
      bufptr = realloc(bufptr, size);
      if (!bufptr)
        return -1;
    }

    *p++ = c;

    if (c == '\n')
      break;

    c = fgetc(stream);
  }

  *p++ = '\0';
  *lineptr = bufptr;
  *n = size;

  return p - bufptr - 1;
}

Constant initIoLib(VM* vm, int arity, Constant* args);

#endif //SENEGAL_SIOLIB_H
