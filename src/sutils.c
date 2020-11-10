#include <stdio.h>
#include <stdlib.h>
#include "includes/sutils.h"

char* readFile(const char* path) {
  FILE* file = fopen(path, "rb");

  if (file == NULL) {
    fprintf(stderr, "Senegal was unable to open file `%s`.\n", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(fileSize + 1);

  if (buffer == NULL) {
    fprintf(stderr, "Senegal was unable to allocate enough memory to read \"%s\".\n", path);
    exit(74);
  }

  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

  if (bytesRead < fileSize) {
    fprintf(stderr, "Senegal could not read file  \"%s\".\n", path);
    exit(74);
  }

  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}
