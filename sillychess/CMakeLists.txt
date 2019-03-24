cmake_minimum_required(VERSION 3.12)
project(sillychess C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(sillychess
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
