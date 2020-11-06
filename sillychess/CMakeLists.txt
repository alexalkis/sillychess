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

if (CMAKE_C_COMPILER STREQUAL "/opt/amiga/gcc6/bin/m68k-amigaos-gcc")
    add_compile_options(-O3 -mcrt=nix13 -fbaserel -fomit-frame-pointer -msmall-code -Wall)
    set(CMAKE_EXE_LINKER_FLAGS "-s -mcrt=nix13 -fbaserel -msmall-code" CACHE INTERNAL "" FORCE)
#    FIND_PROGRAM(CRANKER cranker_generic PATHS "/usr/local/bin")
#    IF(CRANKER)
#        SET(CRANKER_COMMAND ${CRANKER} -f ${CMAKE_CURRENT_BINARY_DIR}/a68k -o ${CMAKE_CURRENT_BINARY_DIR}/a68kc)
#    ENDIF(CRANKER)
else()
    #set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -fsanitize=address -Wshadow")
    add_compile_options(
            "-Wall" "-Wpedantic" "-Wextra"
    #        "$<$<CONFIG:DEBUG>:-fsanitize=address;-Wshadow;>"
    )
    add_link_options(
            "-s"
    #        "$<$<CONFIG:DEBUG>:-fsanitize=address>"
    )
endif(CMAKE_C_COMPILER STREQUAL "/opt/amiga/gcc6/bin/m68k-amigaos-gcc")

