cmake_minimum_required(VERSION 3.17)
project(Senegal C)

set(CMAKE_C_STANDARD 99)

add_executable(
    Senegal 
    src/main.c 
    src/includes/sutils.h 
    src/sinstructions.c 
    src/includes/sinstructions.h 
    src/includes/smemory.h 
    src/smemory.c 
    src/includes/sdebug.h 
    src/sdebug.c 
    src/includes/sconstant.h 
    src/sconstant.c
    src/includes/svm.h 
    src/svm.c 
    src/includes/scompiler.h 
    src/scompiler.c 
    src/includes/slexer.h 
    src/slexer.c 
    src/includes/sparser.h 
    src/sparser.c 
    src/includes/stable.h 
    src/stable.c 
    src/includes/sgcobject_utils.h 
    src/sgcobject_utils.c 
    src/includes/sapi.h 
    src/sapi.c 
    src/includes/sinstruction_utils.h 
    src/sinstruction_utils.c 
    src/includes/sconstant_utils.h 
    src/sconstant_utils.c 
    src/includes/stable_utils.h 
    src/stable_utils.c 
    src/includes/sopcodes.h 
    src/core/sboolCore.h 
    src/core/sboolCore.c 
    src/core/sstringCore.h 
    src/core/sstringCore.c 
    src/core/snumCore.h 
    src/core/snumCore.c
)