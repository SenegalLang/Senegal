#include <stdio.h>
#include "includes/stable.h"
#include "includes/smemory.h"
#include "includes/stable_utils.h"

#define MAX_TABLE_LOAD 0.75

void freeTable(VM* vm, Table *table) {
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

bool tableInsert(VM* vm, Table* table, Constant key, Constant c, bool isFinal) {
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
  entry->isFinal = isFinal;

  return isNew;
}

void tableInsertAll(VM* vm, Table* from, Table* to) {
  for (int i = 0; i <= from->cap; i++) {
    Entry* entry = &from->entries[i];

    if (!IS_NULL(entry->key))
      tableInsert(vm, to, entry->key, entry->constant, false);
  }
}

bool tableContainsAll(Table* a, Table* b) {
  for (int i = 0; i <= a->cap; i++) {
    Entry* entry = &a->entries[i];

    if (!IS_NULL(entry->key)) {
      if (!tableFind(b->entries, b->cap, entry->key))
        return false;
    }
  }

  return true;
}

bool tableContainsAny(Table* a, Table* b) {
  for (int i = 0; i <= a->cap; i++) {
    Entry* entry = &a->entries[i];

    if (!IS_NULL(entry->key)) {
      if (tableFind(b->entries, b->cap, entry->key))
        return true;
    }
  }

  return false;
}

void markTable(VM* vm, Table *table) {
  for (int i = 0; i <= table->cap; i++) {
    Entry* entry = &table->entries[i];

    markConstant(vm, entry->key);
    markConstant(vm, entry->constant);
  }
}