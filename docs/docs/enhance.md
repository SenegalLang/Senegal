---
id: enhance
title: Enhance
sidebar_label: Enhance
---

While changing existing API is impractical, there are often cases where you want to add functionality. Take the following
as an example:

```js
function root(num) => num ** 0.5;

var root9 = root(9);
println(root9);
```

This would look better if `root` was a method on `num` types instead; this can be done by "enhancing" `num`:

```js
enhance num {
    function root() => this ** 0.5;
}
```

We can now call `root` on `num` types directly:

```js
var root9 = 9.root();
println(root9);
```

## Syntax
```js
enhance type {
    definition...
}
```
