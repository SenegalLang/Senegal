---
id: cimport
title: CImport
sidebar_label: CImport
---

One of the rudimentary ways for extending Senegal is by defining new C function.

While this doesn't allow you to call *any* c function, Senegal exposes c functions to define functions/variables for senegal
within `core/sapi.c`. When you build senegal with CMake, it generates a shared library and copies it to the bin folder which
can be used to access necessary files.

## Example
Here's a *very* general example to help you get the gist of it.

We first create a new c file. Lets call it `addNums.c`.
```c
// Correct these paths as needed
#include 'svm.h'
#include 'sapi.h'

// This is the actual implementation of addNums.
Constant addNums(VM* vm, int arity, Constant* args) {
  // Expects a number of arguments to be passed.
  expect(2, arity, "addNums");
  
  // The first given argument is at args[0].
  // If this was to be a class method, the class instance would be at args[-1].
  double a = AS_NUMBER(args[0]);
  double b = AS_NUMBER(args[1]);
  
  return NUM_CONST(a + b);
}

// When senegal encounters a `cimport`, it searches for a `initLib` symbol within the library.
// This function will always have the same structure.
Constant initLib(VM* vm, int arity, Constant* args) {
  // defineGlobalFunc is imported from sapi.h.
  defineGlobalFunc(vm, "addNums", addNums);
  
  return NULL_CONST;
}
```

Now generate a shared library from this. For simplicity's sake, we'll call it `addNums.so`.

To use this within senegal, import the library as such:
```js
// This assumes that addNums.so is in the same directory as this file.
cimport 'addNums'

println(addNums(10, 5)); // Should print 15.
```

## C API Commonly Used Functions
| Function | Header | Description |
| ----------- | ----------- | ----------- |
| expect() | core/includes/sapi.h | Checks to see if the correct number of arguments are given | 
| defineGlobal() | core/includes/sapi.h | Defines a global variable | 
| defineGlobalFunc() | core/includes/sapi.h | Defines a function | 
| defineClassNativeStaticField() | core/includes/sapi.h | Defines a static field for a given class | 
| defineClassNativeStaticMethod() | core/includes/sapi.h | Defines a static method for a given class | 
| defineClassNativeField() | core/includes/sapi.h | Defines a field for a given class | 
| defineClassNativeMethod() | core/includes/sapi.h | Defines a method for a given class |
| newClass | src/includes/svm.h | Creates a new Senegal Class |
| copyString | src/includes/sparser.h | Creates a new Senegal String |

## C API Commonly Used Macros
| Macros | Header | Description |
| ----------- | ----------- | ----------- |
| AS_NUMBER | src/includes/sconstant.h | Returns a Senegal constant as a C double |
| IS_NUMBER | src/includes/sconstant.h | Checks if Senegal constant is a Senegal num type |
| NUM_CONST | src/includes/sconstant.h | Returns a C double as a Senegal constant |
| IS_NULL | src/includes/sconstant.h | Checks if Senegal constant is a Senegal null const |
| NULL_CONST | src/includes/sconstant.h | Returns a Senegal null const |
| AS_BOOL | src/includes/sconstant.h | Returns a Senegal constant as a Senegal Bool |
| IS_BOOL | src/includes/sconstant.h | Checks if Senegal constant is a Senegal Bool |
| BOOL_CONST | src/includes/sconstant.h | Creates a new Senegal bool from a c boolean literal |
| AS_STRING | src/includes/sconstant.h | Returns a Senegal constant as a Senegal String |
| IS_STRING | src/includes/sconstant.h | Checks if Senegal constant is a Senegal String |
| AS_CLASS | src/includes/svm.h | Returns a Senegal constant as a Senegal Class |
| IS_CLASS | src/includes/svm.h | Checks if Senegal constant is a Senegal Class |
| AS_CLOSURE | src/includes/svm.h | Returns a Senegal constant as a Senegal closure |
| IS_CLOSURE | src/includes/svm.h | Checks if Senegal constant is a Senegal closure |
| AS_LIST | src/includes/svm.h | Returns a Senegal constant as a Senegal List |
| IS_LIST | src/includes/svm.h | Checks if Senegal constant is a Senegal List |
| AS_MAP | src/includes/svm.h | Returns a Senegal constant as a Senegal Map |
| IS_MAP | src/includes/svm.h | Checks if Senegal constant is a Senegal Map |
