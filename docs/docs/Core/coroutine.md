---
id: coroutine
title: Coroutine
sidebar_label: Coroutine
---

Coroutines in senegal both control the execution of a program and take care of error handling.

```js
function foo() => throw('bar');

var coroutine = Coroutine(foo);

var err = coroutine.try();
println(err); // bar
``` 

## Constructor
### Coroutine(closure)
Creates a new coroutine that calls the given `closure` **_when_** the coroutine is run. `closure`'s function must have a maximum of one argument.

## Static Methods

### current()
Returns the current running coroutine.

## Methods

### call([param])
Starts/resumes the coroutine, passes `param` to the coroutines function if it is passed. 

### isComplete()
Returns true if `this` has completed, a paused function is not completed.

### takeover()
Runs `this` without setting the current running coroutine as the caller. This means that execution will not go back to the current coroutine.

### takeoverError()
Runs `this` without setting the current running coroutine as the caller while keeping the current coroutine's error.
This means that execution will not go back to the current coroutine.
