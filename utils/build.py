"""
build.py is a small python script that helps to build Senegal lang easily.

usage: python build.py [-h] [--run] [--test] [--verbose] [RELEASE | DEBUG]

positional arguments:
  [RELEASE | DEBUG]  The build type,

optional arguments:
  -h, --help         show this help message and exit
  --run, -r          Run Senegal right after the build process.
  --test, -t         Run Senegal tests right after the build process.
  --verbose, -v      Give the verbose output.

Note: While contributing to this script the applied changes should be compatible with both python 2 and python 3 
    as most of the operating systems come with python 2 by default.
"""

from __future__ import print_function
import argparse
import os
import sys
import subprocess
import platform
import shutil

verbose = False

def log(text):
    if verbose:
        print("\u001b[1m\u001b[33;1minfo:\u001b[0m " + text) if sys.stdout.isatty() else print("info: " + text)

def error(text):
    print("\u001b[1m\u001b[31;1merror:\u001b[0m " + text) if sys.stdout.isatty() else print("error: " + text)

def build(build_type, run, test):
    senegal_executable = "senegal.exe" if platform.system().lower() == "windows" else "senegal"

    if not os.path.exists("build"):
        log("Creating the build directory.")
        os.mkdir("build")

        if platform.system().lower() == "windows":
            log("Generating the MinGW Makefiles.")
            
            if os.system("cd build && cmake .. -G \"MinGW Makefiles\"") != 0:
                error("Failed running command `cd build && cmake .. -G \"MinGW Makefiles\"`")
                exit(1)

        else:
            log("Generating the make files.")
            
            if os.system("cd build && cmake ..") != 0:
                error("Failed running command `cd build && cmake ..`")
                exit(1)
    
    if build_type.lower() == "release":
        log("Building Senegal lang [optimized | release]")
        
        if os.system("cd build && cmake --build . --config Release") != 0:
            error("Failed building Senegal lang [optimized | release]")
            exit(1)

        log("Copying build/" + senegal_executable + " to test/" + senegal_executable)
        
        try:
            shutil.copy(os.path.join("build", senegal_executable), os.path.join("test", senegal_executable))
        except:
            error("Unable to copy build/" + senegal_executable + " to test/" + senegal_executable)
            exit(1)
    else:
        log("Building Senegal lang [unoptimized | debug]")
        
        if os.system("cd build && cmake --build . --config Debug") != 0:
            error("Failed building Senegal lang [unoptimized | debug]")
            exit(1)

        log("Copying build/" + senegal_executable + " to test/" + senegal_executable)

        try:
            shutil.copy(os.path.join("build", senegal_executable), os.path.join("test", senegal_executable))
        except:
            error("Unable to copy build/" + senegal_executable + " to test/" + senegal_executable)
            exit(1)

    if test:
        if os.system("cd test && dart pub get && dart pub run test") != 0:
            error("Failed running command `cd test && dart pub get && dart pub run test`")
            exit(1)

    if run:
        if os.system(os.path.join("build", "senegal.exe")) != 0:
            error("Failed executing command build/senegal.exe")
            exit(1)

def main():
    global verbose

    parser = argparse.ArgumentParser(description="A small python script that helps to build Senegal lang easily.")

    parser.add_argument("[RELEASE | DEBUG]", type=str, help="The build type,")
    parser.add_argument("--run", "-r", help="Run Senegal right after the build process.", action="store_true")
    parser.add_argument("--test", "-t", help="Run Senegal tests right after the build process.", action="store_true")
    parser.add_argument("--verbose", "-v", help="Give the verbose output.", action="store_true")

    args = vars(parser.parse_args())

    if args["verbose"]:
        verbose = True
    
    build(args["[RELEASE | DEBUG]"].lower(), args["run"], args["test"])

if __name__ == "__main__":
    main()