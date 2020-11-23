<p align="center"><img src="misc/logo.png" height="150px"></p>
<h1 align="center">Senegal</h1>

# Contribution
- Join the discord - https://discord.gg/9dq6YB2

# Installing from Source

## Building on a Unix-like system
### Using `cmake`:
1. Make sure you have installed the dependencies:

  	* `gcc`
	* `cmake` 3.17 or later
	* `git`

2. Clone the source code using `git`:

	```sh
	$ git clone https://github.com/Calamity210/Senegal
	$ cd Senegal
	```

3. Build and run:
	```sh
	$ mkdir build
	$ cd build
	$ cmake ..
	$ cmake --build . && ./Senegal
	```

### Using `make`
1. Make sure you have installed the dependencies:

  	* `gcc`
	* `GNU make`
	* `git`

2. Clone the source code using `git`:

	```sh
	$ git clone https://github.com/Calamity210/Senegal
	$ cd Senegal
	```

3. Build and run:
```sh
	$ make
	$ make debug
	$ cd build
	$ ./senegal
```

## Building on Windows
1. Make you have installed the dependencies:
	* `mingw` with the base selected
	* `cmake` 3.17 or later
	* `git`

2. Clone the source code using `git`:

	```sh
	$ git clone https://github.com/Calamity210/Senegal
	$ cd Senegal
	```

3. Build and run:
	```sh
	$ mkdir build
	$ cd build
	$ cmake ..

	# If you are using Powershell then:
	$ cmake --build . && ./Senegal.exe

	# If you are using Command Prompt then:
	$ cmake --build . && Senegal.exe
	```
# Docs
https://lang-senegal.web.app/
