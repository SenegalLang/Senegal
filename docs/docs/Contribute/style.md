---
id: style
title: Senegal Styling Guide
sidebar_label: Style Guide
---

## Indentation

Keep indentations 2 characters deep. Do not use spaces to indent.

## Line Length

Each line of text in your code should be around 80 characters long.

We say 'around' intentionally, while we do prefer 80 characters, we are lenient and will overlook exceeding it by small amounts, especially for:

- An include statement
- A comment or raw-string that cannot be split onto multiple lines without reducing readability

## Casing
Use lowerCamelCase for variable and function identifiers,<br/>
UpperCamelCase for class identifiers,<br/>
and UPPERCASE for constants (the define preprocessor also defines a constant)

## Naming
Keep variable and function names as relevant as possible while also keeping them concise

```
uint8_t thisIsATemporaryVariable // ❌
uint8_t tmp // ✅
```

## Function Declarations

The return type, function identifier, and open parenthesis must always be on the same line. Parameters should be on the same line but may be split onto a new line where needed.

A typical function should look like:

```c
returnType identifier(type param1, type param2) {

}
```

If the parameters do not fit on a single line, they can be split as:

```c
returnType identifier(type param1,
                      type param2) {

}
```

If the first parameter cannot fit on a single line, give each parameter its own line:
```c
returnType identifier(
type param1,
type param2) {

}
```

Functions should have a clear objective, keep them short and "to the point".

## Braces, Spaces, and Newlines
- Do not place braces on a newline
- Do not place braces where a single statement will do, unless only one branch is a single statement

```
if (true) // no brace
    printf(. . .); // newline

// OR

if (true) { // braces
    ...
    return ...;
} else { // braces, although it is a single statement 
    printf(. . .); 
}
```

- Leave a space before every opening brace
- Leave a space after the following: `if, else, switch, case for, do, while, ","`
- Leave a space before and after any binary or ternary operator, but none after a unary operator
- Place single statements on a newline

## Trailing Whitespace
DONT, please...

## Comments
You cannot over-comment, well... maybe you can, but comment where it may be needed. Do not split lines in between of a sentence, unless it doesnt affect readablility.
