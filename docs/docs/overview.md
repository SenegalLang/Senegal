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

## Contributing
Feel free to join in and help out with Senegal. You can start by
- Joining the discord - https://discord.gg/9dq6YB2
- and reading the style guide - https://lang-senegal.web.app/docs/Contribute/style