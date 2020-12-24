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

#define SENEGAL_REPL \
"\x1B[32m  _________                                 .__    \x1B[33m__________              .__   \n" \
"\x1B[32m /   _____/ ____   ____   ____   _________  |  |   \x1B[33m\\______   \\ ____ ______ |  |  \n" \
"\x1B[32m \\_____  \\_/ __ \\ /    \\_/ __ \\ / ___\\__  \\ |  |    \x1B[33m|       _// __ \\\\____ \\|  |  \n" \
"\x1B[32m /        \\  ___/|   |  \\  ___// /_/  > __ \\|  |__  \x1B[33m|    |   \\  ___/|  |_> >  |__\n" \
"\x1B[32m/_______  /\\___  >___|  /\\___  >___  (____  /____/  \x1B[33m|____|_  /\\___  >   __/|____/\n" \
"\x1B[32m        \\/     \\/     \\/     \\/_____/     \\/               \x1B[33m\\/     \\/|__|         \x1B[0m\n" \

#define SENEGAL_HELP \
  "Usage: senegal [flags] | [senegal-file]\n\n" \
  "Global options:\n" \
  "-h, --help                 Print this usage information.\n" \
  "    --version              Print the Senegal version.\n"

#define SENEGAL_VERSION "Senegal 0.0.1"

char* readFileWithPath(const char* path);
char* readFile(FILE* file);

int removeCharFromIndex(const char *src, char *dst, int index);

#endif //SENEGAL_SUTILS_H