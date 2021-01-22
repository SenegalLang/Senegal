---
id: num
title: Num
sidebar_label: Num
---

Any integer/floating-point number.

```js
10
10.0
```

## Static Fields
### infinity
`1/0`

### maxFinite
`1.7976931348623157e+308`

### minPositive
`5e-324`

### nan
`0/0`

### negInfinity
`-1/0`

## Fields

### type
Returns the type of this object as a string.

`"foo".type // "Num"`

## Methods

### abs()
Returns the absolute value of `this`.

### asBool()
Returns `this` as the boolean `true` or `false` value.

### ceil()
Returns the smallest integer `>=` to `this`.

### clamp(lower, upper)
Returns `this` clamped between the range `lower to upper`.

### compareTo(other)
Compares `this` to `other`.

`-1` = num < other
`1` = num > other
`0` = num = other

### floor()
Returns the greatest integer `<=` `this`.

### isFinite()
Returns true if `this` is not `nan` or `(+/-)infinity`.

### isInfinite()
Returns true if `this` is `(+/-)infinity`.

### isNan()
Returns true if `this` is `nan`.

### isNeg()
Returns true if `this` is less than zero.

### remainder(other)
Returns the remainder after dividing `this` by `other`.

### toString()
Returns the string representation of `this`.
