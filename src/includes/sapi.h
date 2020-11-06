#ifndef SENEGAL_SAPI_H
#define SENEGAL_SAPI_H

#include <time.h>
#include "sconstant.h"
#include "svm.h"

Constant assertApi(VM* vm, int arity, Constant* args);
Constant clockApi(VM* vm, int arity, Constant* args);
Constant printApi(VM* vm, int arity, Constant* args);
Constant printLnApi(VM* vm, int arity, Constant* args);

#endif //SENEGAL_SAPI_H
