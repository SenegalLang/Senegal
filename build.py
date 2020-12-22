import argparse
import os
import platform
import shutil

def build(build_type: str, run: bool, test: bool):
    if not os.path.exists("build"):
        os.mkdir("build")

        if platform.system().lower() == "windows":
            os.system("cd build && cmake .. -G \"MinGW Makefiles\"")

        else:
            os.system("cd build && cmake ..")
    
    if build_type.lower() == "release":
        os.system("cd build && cmake --build . --config Release")

        shutil.copy(os.path.join("build", "senegal.exe"), os.path.join("test", "senegal.exe"))
    else:
        os.system("cd build && cmake --build . --config Debug")

        shutil.copy(os.path.join("build", "senegal.exe"), os.path.join("test", "senegal.exe"))


    if test:
        os.system("cd test && pub get && pub run test")

    if run:
        os.system(os.path.join("build", "senegal.exe"))

def main():
    parser = argparse.ArgumentParser(description="A small script that helps to build Senegal lang easily.")

    parser.add_argument("[RELEASE | DEBUG]", type=str, help="The build type,")
    parser.add_argument("--run", "-r", help="Run Senegal right after the build process.", action="store_true")
    parser.add_argument("--test", "-t", help="Run Senegal tests right after the build process.", action="store_true")

    args = vars(parser.parse_args())
    
    build(args["[RELEASE | DEBUG]"].lower(), args["run"], args["test"])

if __name__ == "__main__":
    main()