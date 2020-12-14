#ifndef SENEGAL_SAPI_H
#define SENEGAL_SAPI_H

#include <time.h>
#include "../../src/includes/sconstant.h"
#include "../../src/includes/svm.h"

void expect(int expected, int actual, char* name);

void defineClassNativeFunc(VM* vm, const char* id, NativeFunc function, GCClass* class);
void defineClassNativeField(VM* vm, const char* id, Constant field, GCClass* class);
void defineClassNativeStaticFunc(VM* vm, const char* id, NativeFunc function, GCClass* class);
void defineClassNativeStaticField(VM* vm, const char* id, Constant field, GCClass* class);

void defineGlobal(VM* vm, const char* id, Constant field);
void defineGlobalFunc(VM* vm, const char* id, NativeFunc function);

Constant assertApi(VM* vm, int arity, Constant* args);
Constant printApi(VM* vm, int arity, Constant* args);
Constant printlnApi(VM* vm, int arity, Constant* args);

#endif //SENEGAL_SAPI_H
