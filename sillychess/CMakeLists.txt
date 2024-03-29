cmake_minimum_required(VERSION 3.12)
project(sillychess C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

set(SRC_FILES
        src/board.c
        src/eval.c
        src/hash.c
        src/move.c
        src/move.h
        src/rkiss.c
        src/search.c
        src/sillychess.c
        src/sillychess.h
        src/time.c
        src/transpositiontable.c
        src/transpositiontable.h
        src/uci.c)

if (CMAKE_C_COMPILER STREQUAL "/opt/amiga/gcc6/bin/m68k-amigaos-gcc")
    set(CMAKE_C_FLAGS_MINSIZEREL "-O3")
    add_compile_options(-mcrt=nix13 -DNDEBUG -fomit-frame-pointer -Wall)
    set(CMAKE_EXE_LINKER_FLAGS "-s -mcrt=nix13" CACHE INTERNAL "" FORCE)
    add_executable(sillychess020 ${SRC_FILES})
    SET_TARGET_PROPERTIES(sillychess020 PROPERTIES
            COMPILE_FLAGS "-m68020")
endif ()


add_executable(sillychess ${SRC_FILES})

if (UNIX)
    target_link_libraries(sillychess readline)
endif (UNIX)