---
id: concurrency
title: Concurrency
sidebar_label: Concurrency
---

Coroutines are light-weight thread-like objects allowing you to write asynchronous non-blocking code as well as handle errors in Senegal.
The key difference between threads and coroutines is that a program with threads runs several threads concurrently,
whereas coroutines run a single coroutine and cooperatively.
A coroutine is not paused so that control can be given to another unless and until you tell it to.

## Example
```js
function foo() {
    println('Before Yield');
    yield "Yield Value"; // Pass control back to calling coroutine, but remember where it was executing from
    println('After Yield');

    // Allow coroutine2 to take control, 
    // since control is taken rather than given, we should not return here
    coroutine2.takeover();
    
    println('Transfer should not return to this'); // Not called
}

function bar() {
    println("Transferred to coroutine2");
}

// Create new coroutines
var coroutine = Coroutine(foo);
var coroutine2 = Coroutine(bar);

println("Before Call");

var yielded = coroutine.call(); // Initial call, the value is given by the yield
println(yielded);

println("After Call");

coroutine.call(); // Resume executing from the yield

```

## Error Handling
```js
function foo() {
    throw "Caught Error"; // Throw an error
}

var coroutine = Coroutine(foo);

var caughtError = coroutine.try();
println(caughtError);
```

Notice that the error is caught and execution is not suspended, replace `.try()` with `.call()` and see what changes.