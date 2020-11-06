#include <stdio.h>
#include <stdlib.h>
#include "includes/sapi.h"

Constant assertApi(int arity, Constant *args) {
  if (arity < 2) {
    printf("assert expected two arguments, but found  %d", arity);
    exit(1);
  }

  /// assert(arg1, arg2, "MSGs"...);
  if (!areEqual(args[0], args[1])) {
    if (arity >= 3) {
      for (int i = 1; i < arity; i++)
        printConstant(args[i]);

      printf("\n");
    } else {
      printf("ASSERT(");
      printConstant(args[0]);
      printf(" != ");
      printConstant(args[1]);
      printf(")");
    }

    exit(1);
  }
}

Constant clockApi(int arity, Constant* args) {
  return NUM_CONST((double)clock() / CLOCKS_PER_SEC);
}

Constant printApi(int arity, Constant* args) {
  for (int i = 0; i < arity; i++)
    printConstant(args[i]);
}

Constant printLnApi(int arity, Constant* args) {
  for (int i = 0; i < arity; i++)
    printConstant(args[i]);

  printf("\n");
}