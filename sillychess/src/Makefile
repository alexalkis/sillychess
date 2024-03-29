WCC=x86_64-w64-mingw32-gcc
LCC=gcc
ACC=m68k-amigaos-gcc
CFLAGS = -Wall -O3 -DNDEBUG -std=gnu99 -fomit-frame-pointer 

DEPS = sillychess.h

OBJS=board.o hash.o rkiss.o sillychess.o  transpositiontable.o\
	eval.o move.o search.o time.o uci.o

linux: CFLAGS+=-march=native
linux: LDFLAGS+=-lreadline

linuxclang: CC=clang-15
linuxclang: CFLAGS=-fsanitize=address -Wall -ggdb -DNDEBUG -std=gnu99 -fomit-frame-pointer
linuxclang: LDFLAGS+=-lreadline

windows: CFLAGS+=-DWIN32
windows: CC=$(WCC)
windows: OBJS+=sc_res.o

amiga: CC=$(ACC)
amiga: CFLAGS+=-m68020

a500: CC=$(ACC)
a500: CFLAGS+=-lm -mcrt=nix13

.SUFFIXES:
.SUFFIXES: .c.o .rc.o


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

linux: $(OBJS)
	$(LCC) $(CFLAGS) -s -o sillychess $(OBJS) $(LDFLAGS)
	@ls -l sillychess

linuxclang: $(OBJS)
	$(CC) $(CFLAGS) -s -o sillychessa $(OBJS) $(LDFLAGS)
	@ls -l sillychessa


windows: $(OBJS) sc_res.o
	$(WCC) $(WFLAGS) -s -o sillychess.exe $(OBJS)
	@ls -l sillychess.exe

amiga: $(OBJS)
	$(ACC) $(CFLAGS) -s -o sillychess $(OBJS)
	@ls -l sillychess

a500: $(OBJS)
	$(ACC) $(CFLAGS) -s -o sillychess $(OBJS)
	@ls -l sillychess


sc_res.o: windows_res/sc.rc
	x86_64-w64-mingw32-windres -i windows_res/sc.rc -o sc_res.o


clean:
	rm -f sillychess sillychess.exe sillychessa *.o
	@echo "all cleaned up!"
