WCC=x86_64-w64-mingw32-gcc
LCC=gcc
ACC=m68k-amigaos-gcc
PCC=arm-linux-gnueabihf-gcc
CFLAGS = -Wall -O2 -DNDEBUG -std=gnu99 -fomit-frame-pointer 

DEPS = sillychess.h

OBJS=board.o hash.o rkiss.o sillychess.o  transpositiontable.o\
	eval.o move.o search.o time.o uci.o

#linux: CFLAGS+=-march=native

windows: CFLAGS+=-DWIN32
windows: CC=$(WCC)
windows: OBJS+=sc_res.o

amiga: CC=$(ACC)
amiga: CFLAGS+=-m68020

amiga500: CC=$(ACC)
amiga500: CFLAGS+=-m68000


pi: CC=$(PCC)

.SUFFIXES:
.SUFFIXES: .c.o .rc.o


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

linux: $(OBJS)
	$(LCC) $(CFLAGS) -s -o sillychess $(OBJS)
	@ls -l sillychess

pi: $(OBJS)
	$(CC) $(CFLAGS) -s -o sillychess $(OBJS)
	@ls -l sillychess

windows: $(OBJS) sc_res.o
	$(WCC) $(WFLAGS) -s -o sillychess.exe $(OBJS)
	@ls -l sillychess.exe

amiga: $(OBJS)
	$(ACC) $(CFLAGS) -noixemul -s -o sillychess $(OBJS) -lm
	@ls -l sillychess

amiga500: $(OBJS)
	$(ACC) $(CFLAGS) -mcrt=nix13 -s -o sillychess $(OBJS) -lm
	cp sillychess "/home/alex/Documents/FS-UAE/Hard Drives/13hd/t/silly500" | true
	@ls -l sillychess


sc_res.o: windows_res/sc.rc
	x86_64-w64-mingw32-windres -i windows_res/sc.rc -o sc_res.o


clean:
	rm -f sillychess sillychess.exe *.o
	@echo "all cleaned up!"
