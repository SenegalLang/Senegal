---
id: list
title: List
sidebar_label: List
---

A collection of objects of any type;

```js
[0, 1, 2, "3", false]
``` 

## Fields

### type
Returns the type of this object as a string.

`"foo".type // "List"`

## Static Methods

### filled(length, element)
Returns a list of size `length` filled with `element` at each index.

`List.filled(5, 0) // [0,0,0,0,0]`

## Methods

### add(element)

Appends `element` to `this` list.

`[0].add(1) // [0, 1]`

### clear()
Removes all elements from `this` list

`[0].clear() // []` 

### insertAt(index, element)
Inserts `element` at `index`.

`[0].insertAt(0, 1) // [1]`

### length()
Returns the length of `this` list.

`[0].length() // 1`

### removeAt(index)
Removes the element at `index`, reduces the length and shifts all trailing elements over by one
