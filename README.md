#<img src="https://raw.githubusercontent.com/alexalkis/sillychess/master/sillychess/src/windows_res/sc.ico" width="48"> sillychess - A simple chess engine.

Totally messy C code to deal with equally messy chess ideas.
Plays arround ~1800 ELO, don't expect much.

On the other hand it compiles for Linux, Windows **and** Amiga... ;-)


### Whereabouts...
Here in the long process of ELO measuring for version 0.7.3c

    Rank Name             Elo    +    - games score oppo. draws 
       1 robocide        2165    8    8 12928   87%  1829   11% 
       2 Vice 1.1        1961    7    6 12928   59%  1884   14% 
       3 sc v0.7.3a      1899   13   13  2400   50%  1899   15% 
       4 sc v0.7.3       1888   13   13  2400   49%  1899   14% 
       5 sc v0.7.3c      1864   20   20   999   41%  1944   14% 
       6 sc v0.7.2c      1862   12   12  2923   45%  1907   17% 
       7 sc v0.7.2d      1853   13   13  2408   45%  1899   17% 
       8 sc v0.7.3b      1824   15   15  1800   37%  1944   14% 
       9 Fairy-Max 4.8V  1763    7    7  9605   31%  1927   12% 
      10 tscp            1706    7    7 12931   23%  1953    9%

### Amiga
Due to libnix not having proper support for 64bit integers compiling with -noixemul won't do.  That means you'll need the ixemul.library.
