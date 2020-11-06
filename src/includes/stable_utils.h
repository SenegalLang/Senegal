#ifndef SENEGAL_STABLE_UTILS_H
#define SENEGAL_STABLE_UTILS_H

#include "svm.h"

void freeTable(VM* vm, Table* table);

bool tableInsert(VM* vm, Table* table, GCString* key, Constant c);
void tableInsertAll(VM* vm, Table* from, Table* to);
void markTable(VM* vm, Table* table);

#endif //SENEGAL_STABLE_UTILS_H