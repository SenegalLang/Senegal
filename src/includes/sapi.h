#ifndef SENEGAL_SAPI_H
#define SENEGAL_SAPI_H

#include <time.h>
#include "sconstant.h"

Constant assertApi(int arity, Constant* args);
Constant clockApi(int arity, Constant* args);
Constant printApi(int arity, Constant* args);
Constant printLnApi(int arity, Constant* args);

#endif //SENEGAL_SAPI_H
