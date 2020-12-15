---
id: bool
title: Bool
sidebar_label: Bool
---

`true` and `false` are the only two instances of this class

```js
true
// OR
false
``` 

## Fields

### type
Returns the type of this object as a string.

`"foo".type // "bool"`

## Methods

### asNum()

Returns `this` as a number type. 0 for `false` and 1 for `true`.

`true.asNum() // 1` 

### toString()

Converts all characters in `this` to uppercase.

`true.toString() // "true"`
