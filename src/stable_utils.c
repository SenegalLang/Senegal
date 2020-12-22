#include <stdio.h>
#include "includes/stable.h"
#include "includes/smemory.h"
#include "includes/stable_utils.h"

#define MAX_TABLE_LOAD 0.75

void freeTable(VM* vm, Table *table) {
  FREE_ARRAY(vm, NULL, Entry, table->entries, table->cap + 1);
  initTable(table);
}

static void adjustCap(VM* vm, Table* table, int cap) {
  Entry* entries = ALLOCATE(vm, NULL, Entry, cap + 1);

  for (int i = 0; i <= cap; i++) {
    entries[i].key = NULL_CONST;
    entries[i].constant = NULL_CONST;
  }

  table->count = 0;

  for (int i = 0; i <= table->cap; i++) {
    Entry* entry = &table->entries[i];

    if (IS_NULL(entry->key))
      continue;

    Entry* dest = tableFind(entries, cap, entry->key);
    dest->key = entry->key;
    dest->constant = entry->constant;
    table->count++;
  }

  FREE_ARRAY(vm, NULL, Entry, table->entries, table->cap + 1);
  table->entries = entries;
  table->cap = cap;
}

bool tableInsert(VM* vm, Table* table, Constant key, Constant c) {
  if (table->count >= (table->cap + 1) * MAX_TABLE_LOAD) {
    int cap = GROW_CAP(table->cap + 1) - 1;
    adjustCap(vm, table, cap);
  }

  Entry* entry = tableFind(table->entries, table->cap, key);

  bool isNew = IS_NULL(entry->key);

  if (isNew && IS_NULL(entry->constant))
    table->count++;

  if (isNew)
    table->count++;

  entry->key = key;
  entry->constant = c;

  return isNew;
}

void tableInsertAll(VM* vm, Table* from, Table* to) {
  for (int i = 0; i <= from->cap; i++) {
    Entry* entry = &from->entries[i];

    if (!IS_NULL(entry->key))
      tableInsert(vm, to, entry->key, entry->constant);
  }

}

void markTable(VM* vm, Table *table) {
  for (int i = 0; i <= table->cap; i++) {
    Entry* entry = &table->entries[i];
    markGCObject(vm, (GCObject*)&entry->key);
    markConstant(vm, entry->constant);
  }
}