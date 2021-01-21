---
id: pipeline
title: Pipeline Operator
sidebar_label: Pipeline Operator
---

The pipeline operator `<|` pipes the value from an expression into a function. This allows any single-argument function to be written as:

```js
function add2(num) => num + 2;

var result = add2 <| 6; // result = 8
```

This is equivalent to:

```js
function add2(num) => num + 2;

var result = add2(6); // result = 8
```

## Syntax
```js
function <| expression
```

A common case for such syntactic sugar would be when chaining multiple functions.

```js
function add2(n) => n + 2;
function mul2(n) => n * 2;

// without pipeline operator
var a = add2(mul2(mul2(add2(mul2(4)))));

var b = add2 <| mul2 <| mul2 <| add2 <| mul2 <| 4;

println(a); // 42
println(b); // 42
```
