---
id: list
title: List
sidebar_label: List
---

A collection of objects of any type;

```js
[0, 1, 2, "3", false]
``` 

## Constructors

## List(capacity)
Returns a list of with `capacity` number of elements allowed.

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
Removes all elements from `this` list.

`[0].clear() // []`

### contains(match)
Returns true if `this` contains an element equivalent to `match`.

`[1, 2, 7].contains(7) // true`

### insertAt(index, element)
Inserts `element` at `index`.

`[0].insertAt(0, 1) // [1]`

### length()
Returns the length of `this` list.

`[0].length() // 1`

### removeAt(index)
Removes the element at `index`, reduces the length and shifts all trailing elements over by one

## Extraneous Methods (Methods that require `sgl:list` to be imported)

### addAll(other)

Appends all elements from `other` to the end of `this` List.

`[0].addAll([1, 2]) // [0, 1, 2]`

### asMap()
Returns `this` as a map where the index is the key and the element is the value.
`["foo"].asMap() // {0: "foo"}`

### at()
Returns the element at `index`.

### checkEvery(function test(e))
Returns true if `test` returns true for each element.

### expand()
Expands `this` by extracting the elements of nested lists out to the parent list.

### fillRange(start, end, fill)
Fills from `start` (inclusive) to `end` (exclusive) with `fill`.

### firstWhere(function test(e))
Returns the first element that satisfies `test`.

### forEach(function func(e))
Calls `func` for each element in `this`.

```js
function printElement(e) => println(e);

[1, 2].forEach(printElement); // Prints 1 and 2
```

### getRange(start, end)
Returns a sublist from the given range.

### indexOf(element, start)
Returns the index of the `element` from `start`.

### indexWhere(function test(e), start)
Returns the index of the first element that satisfies `test` from `start`.

### join(separator)
Returns a string of all elements joined by `separator`.

### reduce(initial, function combine(value, element))
Reduces `this` to a single value by iteratively combining its elements using the given function.
