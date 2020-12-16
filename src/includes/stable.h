#ifndef SENEGAL_STABLE_H
#define SENEGAL_STABLE_H

#include "sconstant.h"

typedef struct {
    Constant key;
    Constant constant;
} Entry;

typedef struct {
    int count;
    int cap;
    Entry* entries;
} Table;

void initTable(Table* table);

Entry* findEntry(Entry* entries, int cap, Constant key);

bool tableRemove(Table* table, Constant key);
void tableRemoveWhite(Table* table);

bool tableGetEntry(Table* table, Constant key, Constant* c);
GCString* tableFindString(Table* table, const char* chars, int length, uint32_t hash);

#endif //SENEGAL_STABLE_H