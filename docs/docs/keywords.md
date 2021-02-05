---
id: keywords
title: Keywords
sidebar_label: Keywords
---

## Break and continue
`break` is used to stop looping, whereas `continue` skips to the next iteration of the loop.

```js
while (i < 10) {
    i++;
    
    if (i == 6)
        break; // loop stops here, i never reaches 7
    if (i == 3)
        continue; // Skip the rest of the loop and go to the next iteration
    
    println(i);
}
```

## Switch, case and default
Switch statements compare constants for equality. `default` is used to execute code when no `case` clause matches.

```js
switch (10) {
    case 2:
        println(2);
    case 10:
        println(10);
    default:
        println('neither 10 nor 2');
}
```

## Cimport
Take a look at the cimport document.

## Class
### Constructors
Constructors are implicitly called when creating a class instance, a constructor follows the syntax `ClassName(...) {...}`.
Constructors may not return any object.

```js
class ClassWithConstructor {
    // Constructor
    ClassWithConstructor() {
        println("Constructor was called");
    }
}

ClassWithConstructor();
```

### Instance Variables
```js
class ClassWithInstanceVariables {
    var variable1; // Defaults to null
    var variable2 = 10; // Defaults to 10
}

println(ClassWithInstanceVariables().variable2); // 10
```

### Methods
```js
class ClassWithMethods {
    function ClassWithMethods() {
        println('ClassWithMethods');
    }
}

ClassWithMethods().printClassName; // ClassWithMethods
```



