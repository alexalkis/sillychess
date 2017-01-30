#<img src="https://raw.githubusercontent.com/alexalkis/sillychess/master/sillychess/src/windows_res/sc.ico" width="48"> sillychess - A simple chess engine.

Totally messy C code to deal with equally messy chess ideas.
Plays around ~1800 ELO, don't expect much.

On the other hand it compiles for Linux, Windows **and** Amiga... ;-)


### Whereabouts...
As of 0.7.3d the engine seems to break the 1900 elo barrier! Yay...

I think I am reaching the limits of "material-only" evaluation function though.

    Rank Name             Elo    +    - games score oppo. draws 
       1 robocide        2166    7    7 15530   86%  1834   11% 
       2 Vice 1.1        1961    6    6 15530   58%  1892   14% 
       3 sc v0.7.3d      1920   15   15  1800   46%  1944   13% 
       4 sc v0.7.3a      1900   13   13  2400   50%  1899   15% 
       5 sc v0.7.3       1889   13   13  2400   49%  1899   14% 
       6 sc v0.7.3c      1867   15   15  1800   41%  1944   13% 
       7 sc v0.7.2c      1863   12   12  2923   45%  1907   17% 
       8 sc v0.7.2d      1854   13   13  2408   45%  1899   17% 
       9 sc v0.7.3b      1825   15   16  1800   37%  1944   14% 
      10 Fairy-Max 4.8V  1763    7    7  9605   31%  1927   12% 
      11 tscp            1706    6    7 15532   22%  1964    9%
	  
### Amiga
Due to libnix not having proper support for 64bit integers compiling with -noixemul won't do.  That means you'll need the ixemul.library.

### Building
Although my main development platform for the project seems to be Eclipse, I wrote an almost adequate makefile.

  * make, will produce a linux executable ("make linux" will do the same thing)
  * make windows, will produce a windows executable
  * make amiga, will produce an amiga executable

If you happen to cycle between them always do "make clean" before.
