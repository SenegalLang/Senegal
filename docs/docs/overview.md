---
id: overview
title: Overview
sidebar_label: Overview
slug: /
---

Senegal is a powerful, small-but-fast, concurrent, class-based, and dynamically-typed programming language with a modern syntax.

## Sample Senegal Program
```typescript
class Senegal {
    var name;

    Senegal(name) {
        this.name = name;
    }
    
    function talk(msg) {
        println(this.name + " says: " + msg);
    }
}

var mel = Senegal("Mel");
mel.talk("Hello world!"); // Mel says: Hello World
```

## Features
- Fast single-pass compiler
- Dynamically typed
- Coroutines

## Get Started
Have I piqued your interest? Great! Head over to our [docs](https://lang-senegal.web.app) to get started.

## Setup
Clone the repo:
```
git clone https://github.com/SenegalLang/Senegal.git
```
Add the bin folder within senegal's root directory to your PATH.

### Windows

1. Press the Windows key, then search for and select "System (Control Panel)".
2. Click "Advanced system settings".
3. Click "Environment Variables".
4. Under "System Variables", find the `PATH` variable, select it, and click
   "Edit".
5. Add directory. For example, if the value was `C:\Windows\System32`, change it to
   `path-to-senegal-bin;C:\Windows\System32`.
6. Click "OK".
7. Restart your terminal.

### Mac OS X

1. Open the `.bash_profile` file in your home directory (for example,
   `/Users/your-user-name/.bash_profile`) in a text editor.
2. Add `export PATH="path-to-senegal-bin:$PATH"` to the last line of the file.
3. Save the `.bash_profile` file.
4. Restart your terminal.

### Linux

1. Open the `.bashrc` file in your home directory (for example,
   `/home/your-user-name/.bashrc`) in a text editor.
2. Add `export PATH="path-to-senegal-bin:$PATH"` to the last line of the file, where
   *your-dir* is the directory you want to add.
3. Save the `.bashrc` file.
4. Restart your terminal.

## Contributing
Feel free to join in and help out with Senegal. You can start by
- Joining the discord - https://discord.gg/9dq6YB2
- and reading the style guide - https://lang-senegal.web.app/docs/Contribute/style