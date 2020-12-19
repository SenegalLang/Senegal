# Contributing

When contributing to Senegal, kindly start by discussing any intended changes through creating a new issue, or starting a new discussion. This allows maintainers and other contributors
to have a better understanding of your work.

Take our [Citizen Code of Conduct](https://github.com/SenegalLang/Senegal/blob/master/CITIZEN_CODE_OF_CONDUCT.md) into consideration for any interaction you have.

## Creating Issues
To help us keep track of issues, please:

- Keep titles concise but informative.
- Use markdown to your advantage and format your issue in a clean manner.
- Provide steps to reproduce the issue.
- Use our [Discord](https://discord.gg/AacmA3W) or [Discussions](https://github.com/SenegalLang/Senegal/discussions) for code help.

## Reviewing PRs
To help contributors be more productive, be sure to:

- Read the [Contributor Code of Conduct](https://github.com/SenegalLang/Senegal/blob/master/CODE_OF_CONDUCT.md)
- Describe the issue, not the solution.

## Submitting PRs
To help your PRs get merged quicker as well as help us review them quicker, we ask that:

- Relavant changes are documented (See **Documenting Code** below).
- All changes (aside from documentation changes) are tested by creating tests in the `test/test_programs/` directory.
- You have a clear description of your work.
- You comment any changes made during the review period. 
- You respond to reviews quickly.
- Take suggestions positively.

## Creating a New Native Library
- Create a new header file in `libs/include/`, follow the format `s{libname}lib.h`
- Define an init function. `Constant init{libname}Lib(VM* vm, int arity, Constant* args)`
- Create a new source file in `libs/`, follow the format `s{libname}lib.c`
- Implement the init function (define functions/variables as need be).
- In `src/main.c` insert your library within the `addPaths` function.

## Creating a New Native Library
- Create a new directory for your library in `libs/`
- Create a new senegal file. **(The name of this file should be the same as the directory name)**.
- Implement the library.
- In `src/main.c` insert your library within the `addPaths` function, be sure to pass `NULL_CONST` as the value.


## Extending an Existing Library
- Start by navigating to the relavant library's source in `libs/s{libname}lib.c`.
- If you are creating a new function, write your native function (`static Constant someNativeFunc(VM *vm, int arity, Constant *args) {...}`).
- Scroll down to the `init{libname}Lib` function and define the function there.
- For fields skip step 2 and directly declare the field in the `init{libname}Lib` function.

## Helpful Functions
`sapi.h` provides a couple helpful functions for adding native functions/fields:
- `defineGlobal`: Defines a global variable.
- `defineFunc`: Defines a global function.
- `defineClassNativeStaticFunc`: Defines a static native method for a class.
- `defineClassNativeStaticField`: Defines a static field for a class.
- `defineClassNativeFunc`: Defines a native method for a class.
- `defineClassNativeField`: Defines a field for a class.

## Documenting Code

### Create a new document
To create a new document, add it within the relevant subdirectory within `docs/`.
At the top of the file, type: 
```
---
id: some id // prefer lowercase
title: Some Title // Capitalize first letter
---
```

Open `sidebars.js` and add its id in its respective place.

To test your changes, run:
```shell
$ yarn install
$ yarn start
```


Then browse to http://localhost:3000/ in your browser.
When you are confident with your changes, open a pull request and request a contributor to review it.

### Edit an older document
Find the doc that you want to edit in the `docs/` directory and make your changes.

To test your changes, run: 
```shell
$ yarn install
$ yarn start
```


Then browse to http://localhost:3000/ in your browser.
When you are confident with your changes, open a pull request and request a contributor to review it.

We look forward to your contribution!
