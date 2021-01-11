#ifndef SENEGAL_SLOADCLIB_H
#define SENEGAL_SLOADCLIB_H

#include "svm.h"

void sysUnloadLib(void* lib);
void* sysLoad(const char* path, int seeglb);
Constant sysSym(VM* vm, void* lib, const char* sym);

#endif //SENEGAL_SLOADCLIB_H
