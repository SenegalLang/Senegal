---
id: tour
title: Senegal Tour
sidebar_label: Tour
slug: /
---

```js
final class Birb {
    Birb(name, species) {
        this.name = name;
        this.species = species;
    }
}

var senegal = Birb("Mel", "Senegal");

println(senegal.name);
println(senegal.species);
```

### Notes
- Senegal classes can be final, once you've set its fields within the constructor, they cannot change.
- The Birb() function is a constructor
- Senegal is dynamically typed, which is why we dont specify types for the constructors arguments.
- Notice how fields were accessed before being declared, Senegal classes do not allow variable declarations, but rather automatically adds fields where needed.
- Variable declarations follow the syntax `var name = expression`.

## Data Types
- bool - true or false.
- number - Senegal numbers are 64-bit floating-point numbers as specified in the IEEE 754 standard. 1 bit for the sign, 11 for exponents and 52 for the value itself.
- List - A collection of dynamic objects.
- Map - Key / Value pairs, key must be a String.
- Strings - Literals (char array), surrounded by either " or 's.

Functions, classes, and instances, are also first-class values.

### Example
```js
// bool
var bool = true;

// number
var number = 0;

// List
var list = [1, 2, 3, 4, 5];

// Map
var map = {"key": number};

var string = "This is a string";
```

## Comments
- `//` for a single-line comment.
- `/* */` for a multi-line comment.

```js
// This is a single-line comment.
/*
This is
a multi-line
comment.
*/
```

## Loops

```js
var i = 0;

while (i <= 10) {
    i++

    if (i == 1)
        continue;

    println(i);
}

// OR

for (var i = 0; i <= 10; i++) {
    if (i == 9)
        break;

    println(i);
}
```