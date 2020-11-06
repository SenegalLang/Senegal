#include "includes/stable.h"

void initTable(Table *table) {
  table->count = 0;
  table->cap = -1;
  table->entries = NULL;
}

Entry* findEntry(Entry* entries, int cap, GCString* key) {
  uint32_t index = key->hash & cap; // hash % cap
  Entry* tombstone = NULL;

  for (;;) {
    Entry* entry = &entries[index];

    if (entry->key == NULL) {
      if (IS_NULL(entry->constant)) {
        return tombstone != NULL ? tombstone : entry;
      } else {
        if (tombstone == NULL)
          tombstone = entry;
      }
    }
    else if (entry->key == key)
      return entry;

    index = (index + 1) & cap; // (index + 1) % cap
  }
}

bool tableRemove(Table* table, GCString* key) {
  if (table->count == 0)
    return false;

  Entry* entry = findEntry(table->entries, table->cap, key);
  if (entry->key == NULL)
    return false;

  entry->key = NULL;
  entry->constant = BOOL_CONST(true);

  return true;
}

bool tableGetEntry(Table* table, GCString* key, Constant* c) {
  if (table->count == 0)
    return false;

  Entry* entry = findEntry(table->entries, table->cap, key);

  if (entry->key == NULL)
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

    if (entry->key == NULL) {
      if (IS_NULL(entry->constant))
        return NULL;
    } else if (entry->key->length == length && entry->key->hash == hash
               && memcmp(entry->key->chars, chars, length) == 0)
      return entry->key;

    index = (index + 1) & table->cap; // (index + 1) % cap
  }
}

void tableRemoveWhite(Table *table) {
  for (int i = 0; i <= table->cap; i++) {
    Entry* entry = &table->entries[i];

    if (entry->key != NULL && !entry->key->gc.isMarked) {
      tableRemove(table, entry->key);
    }
  }
}