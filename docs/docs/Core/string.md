---
id: string
title: String
sidebar_label: String
---

Strings are arrays of immutable bytes, generally used to store text. Strings can be either single or multiline and are denoted by the single or double quotes surrounding them.

```js
"This is a string"
'This is also a string'
``` 

## Static Methods

### fromByte(byte)
Returns a new string with `byte` where byte is between 0 and 255.

`String.fromByte(244)`

## Fields

### type
Returns the type of this object as a string.

`"foo".type // "String"`

## Methods

### at(index)
Returns the character at `index`. Although this may not have much of an impact when working with `senegal < 2.0`, its important to note that `index` is a byte offset, not code point offset.

`"foo".at(0) // "f"`

### contains(other)
Returns true if `this` contains `other`, otherwise false.

`"foo".contains("f") // true`

### endsWith(other)
Returns true if `this` ends with `other`, otherwise false.

`"foo".endsWith("o") // true`

### indexOf(search, start)
Returns the index of the first match of `search` within `this` starting at the index `start`.

`"foo".indexOf("o", 0) // 1`

### isAlpha()
Returns true is the string contains only alphabetical characters.

`"foo".isAlpha() // true`

### isAlphaNum()
Returns true is the string contains only alphanumerical characters.

`"foo123".isAlphaNum() // true`

### isEmpty()
Returns true if `this` is empty, otherwise false.

`"".isEmpty() // true`

### isHex()
Returns true is the string contains only hex characters.

`"123A".isHex() // true`

### isNotEmpty()
Returns true if `this` is not empty, otherwise false.

`"foo".isNotEmpty() // true`

### isNum()
Returns true is the string contains only numeric characters.

`"123".isNum() // true`

### length()

Returns the length of `this`.

`"foo".length() // 3`

### replace(from, to)

`"boo".replace("b", "f") // foo`

### split(delim)
Returns a list of tokens after splitting the `this` against `delim`.

`"foo bar".split(" ") // ["foo", "bar"]`

### startsWith(other)
Returns true if `this` starts with `other`, otherwise false.

`"foo".startsWith("f") // true`

### substr(start, end)
Returns the substring of `this` from `start` (inclusive) to end (exclusive).

`"foo".substr(0, 2) // fo`

### toLower()

Converts all characters in `this` to lowercase.

`"FoO".toLower() // foo`

### toNum()

Returns `this` as a number type. It is a runtime error if `this` contains non-numerical characters.

`"10".toNum() // 10` 

### toUpper()

Converts all characters in `this` to uppercase.

`"fOo".toUpper() // FOO`
