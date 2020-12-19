<p align="center"><img src="misc/logo.png" height="150px"></p>
<h1 align="center">Senegal</h1>

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

## Documentation
The documentation is still a work in progress, but you can find it at: https://lang-senegal.web.app/
