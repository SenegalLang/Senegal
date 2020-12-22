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
import platform
import shutil

verbose = False

def log(text):
    if verbose:
        print("\u001b[1m\u001b[33;1mInfo:\u001b[0m " + text)

def build(build_type, run, test):
    senegal_executable = "senegal.exe" if platform.system().lower() == "windows" else "senegal"

    if not os.path.exists("build"):
        log("Creating the build directory.")
        os.mkdir("build")

        if platform.system().lower() == "windows":
            log("Generating the MinGW Makefiles.")
            os.system("cd build && cmake .. -G \"MinGW Makefiles\"")

        else:
            log("Generating the make files.")
            os.system("cd build && cmake ..")
    
    if build_type.lower() == "release":
        log("Building Senegal lang [optimized | release]")
        os.system("cd build && cmake --build . --config Release")

        log("Copying build/" + senegal_executable + " to test/" + senegal_executable)
        shutil.copy(os.path.join("build", senegal_executable), os.path.join("test", senegal_executable))
    else:
        log("Building Senegal lang [unoptimized | debug]")
        os.system("cd build && cmake --build . --config Debug")

        log("Copying build/" + senegal_executable + " to test/" + senegal_executable)
        shutil.copy(os.path.join("build", senegal_executable), os.path.join("test", senegal_executable))

    if test:
        os.system("cd test && pub get && pub run test")

    if run:
        os.system(os.path.join("build", "senegal.exe"))

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