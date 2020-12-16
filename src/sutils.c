#include <stdio.h>
#include <stdlib.h>
#include "includes/sutils.h"

char* readFileWithPath(const char* path) {
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

char* readFile(FILE* file) {

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(fileSize + 1);

  if (buffer == NULL) {
    fprintf(stderr, "Senegal was unable to allocate enough memory to read.\n");
    exit(74);
  }

  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

  if (bytesRead < fileSize) {
    fprintf(stderr, "Senegal could not read file.\n");
    exit(74);
  }

  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}

int removeChar(const char *src, char *dst, char c) {
  const char* s;
  char* d;

  if (src != NULL && dst != NULL) {

    for (d = dst, s = src; (*d = *s); s++) {
      if (c != *d)
        d++;
    }

    return 0;
  }
  return 1;
}

// TODO(calamity): improve
int removeCharFromIndex(const char* src, char* dst, int index) {
  if(!src || !dst)
    return -1;

  int s=0, d=0;

  while (*(src+s) != '\0') {
    if (s != index) {
      *(dst + d++) = *(src + s++);
    } else {
      s++;
    }
  }
  *(dst+d) = '\0';
  return 0;
}