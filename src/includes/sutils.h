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

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define USE_DL_DLL
#else
#define USE_DLOPEN
#endif

#define DEBUG_LOG_GC 0
#define DEBUG_PRINT_CODE 0

#define UINT8_COUNT (UINT8_MAX + 1)

#define SENEGAL_HELP \
  "Usage: senegal [flags] | [senegal-file]\n\n" \
  "Global options:\n" \
  "-h, --help                 Print this usage information.\n" \
  "    --version              Print the Senegal version.\n"    \
  "    --args                 Arguments to pass to a Senegal Program\n" \
  "    --path                 Manually pass the path to the senegal executable's directory (used only if senegal isn't in PATH).\n"

#define SENEGAL_VERSION "Senegal 0.0.1"

#define DEFAULT "\x1B[0m"
#define RED     "\x1B[31m"
#define GREEN   "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE    "\x1B[34m"
#define PURPLE "\x1B[35m"
#define CYAN    "\x1B[36m"
#define WHITE   "\x1B[37m"

#define SENEGAL_REPL \
"\x1B[32m  _________                                 .__    \x1B[33m__________              .__   \n" \
"\x1B[32m /   _____/ ____   ____   ____   _________  |  |   \x1B[33m\\______   \\ ____ ______ |  |  \n" \
"\x1B[32m \\_____  \\_/ __ \\ /    \\_/ __ \\ / ___\\__  \\ |  |    \x1B[33m|       _// __ \\\\____ \\|  |  \n" \
"\x1B[32m /        \\  ___/|   |  \\  ___// /_/  > __ \\|  |__  \x1B[33m|    |   \\  ___/|  |_> >  |__\n" \
"\x1B[32m/_______  /\\___  >___|  /\\___  >___  (____  /____/  \x1B[33m|____|_  /\\___  >   __/|____/\n" \
"\x1B[32m        \\/     \\/     \\/     \\/_____/     \\/               \x1B[33m\\/     \\/|__|         \x1B[0m\n"

char* readFileWithPath(const char* path);
char* readFile(FILE* file);

int removeCharFromIndex(const char *src, char *dst, int index);

static inline char* concat(char* a, char* b) {
    int aLen = (int)strlen(a);
    int bLen = (int)strlen(b);
    int length = aLen + bLen;

    char* chars = malloc(length + 1);

    memcpy(chars, a, aLen);
    memcpy(chars + aLen, b, bLen);

    chars[length] = '\0';

    return chars;
}

#endif //SENEGAL_SUTILS_H
