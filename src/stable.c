#include "includes/stable.h"
#include "includes/sparser.h"

void initTable(Table *table) {
  table->count = 0;
  table->cap = -1;
  table->entries = NULL;
}

Entry* tableFind(Entry* entries, int cap, Constant key) {
  uint32_t index = hashConstant(key) & cap; // hash % cap
  Entry* tombstone = NULL;

  for (;;) {
    Entry* entry = &entries[index];

    if (IS_NULL(entry->key)) {
      if (IS_NULL(entry->constant)) {
        return tombstone != NULL ? tombstone : entry;
      } else {
        if (tombstone == NULL)
          tombstone = entry;
      }
    }
    else if (areEqual(key, entry->key))
      return entry;

    index = (index + 1) & cap; // (index + 1) % cap
  }
}

bool tableRemove(Table* table, Constant key) {
  if (table->count == 0)
    return false;

  Entry* entry = tableFind(table->entries, table->cap, key);
  if (IS_NULL(entry->key))
    return false;

  entry->key = NULL_CONST;
  entry->constant = BOOL_CONST(true);

  return true;
}

bool tableGetEntry(Table* table, Constant key, Constant* c) {
  if (table->count == 0)
    return false;

  Entry* entry = tableFind(table->entries, table->cap, key);

  if (IS_NULL(entry->key))
    return false;

  *c = entry->constant;
  return true;
}

GCString *tableFindString(Table *table, const char *chars, int length, uint32_t hash) {
   if (table->count == 0)
    return NULL;

  uint32_t index = hash & table->cap; // hash % cap

  for (;;) {
    Entry* entry = &table->entries[index];

    if (IS_NULL(entry->key)) {
      if (IS_NULL(entry->constant))
        return NULL;
    } else {
      GCString* string = AS_STRING(entry->key);
      if (string->length == length && string->hash == hash && memcmp(string->chars, chars, length) == 0)
      return string;
    }

    index = (index + 1) & table->cap; // (index + 1) % cap
  }
}

void tableRemoveWhite(Table *table) {
  for (int i = 0; i <= table->cap; i++) {
    Entry* entry = &table->entries[i];

    if (IS_NULL(entry->key) && !AS_GC_OBJ(entry->key)->isMarked) {
      tableRemove(table, entry->key);
    }
  }
}