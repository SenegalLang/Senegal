---
id: map
title: Map
sidebar_label: Map
---

A collection of key/value pairs.

```js
{'key': 'value'}
``` 

## Fields

### type
Returns the type of this object as a string.

`"foo".type // "Map"`

## Methods

### add(key, value)

Adds a new `key`/`value` pair entry to `this` map.

`{}.add('key', 'value') // {'key': 'value'}`

### clear()
Removes all entries from `this` map

`{'key': 'value'}.clear() // {}` 

### isEmpty()
Returns true is `this` map has no entries.

`{}.isEmpty() // true`

### isNotEmpty()
Returns true is `this` map has entries.

`{'key': 'value'}.isNotEmpty() // true`

### length()
Returns the length of `this` map.

`{'key': 'value'}.length() // 1`

### remove(key)
Removes the entry associated with `key` from `this` map.

`{'key': 'value'}.remove('key') // {}`
