#ifndef SENEGAL_STABLE_UTILS_H
#define SENEGAL_STABLE_UTILS_H

#include "svm.h"

void freeTable(VM* vm, Table* table);

bool tableInsert(VM* vm, Table* table, Constant key, Constant c, bool isFinal);
void tableInsertAll(VM* vm, Table* from, Table* to);
bool tableContainsAll(Table* a, Table* b);
bool tableContainsAny(Table* a, Table* b);

void markTable(VM* vm, Table* table);

#endif //SENEGAL_STABLE_UTILS_H