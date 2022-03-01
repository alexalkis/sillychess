/*
 * sillychess.h
 *
 *  Created on: May 27, 2014
 *      Author: alex
 */

#ifndef SILLYCHESS_H_
#define SILLYCHESS_H_

//workarround for mingw on windows not printing %llX correctly
#ifdef __MINGW32__
#define INT64_FORMAT "I64"
#else
#define INT64_FORMAT "ll"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME    "sc v0.7.3f"
#define FULLNAME        NAME " " __DATE__ " " __TIME__

#define START_FEN    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

//#define NDEBUG
#ifdef NDEBUG
#define ASSERT(x)
#else
#define ASSERT(x) \
    if(!(x)) {fprintf(stdout,"%s assertion failed. File %s, at line %d.\n",#x,__FILE__,__LINE__);\
    exit(1);}
#endif

#define COL(x) ((x)&7)
#define ROW(x) ((x)>>4)

//#ifdef WIN32
#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif
#ifdef INFINITE
#undef INFINITE
#endif
//#endif

enum {
    FALSE = 0,
    NOMOVE = 0,
    TRUE = 1,
    MAXDEPTH = 200,
    MAX_PLY_IN_A_GAME = 1000,
    FIRST_KILLER_SCORE = 900000,
    SECOND_KILLER_SCORE = 800000,
    CAPTURE_SCORE = 1000000,
    PVMOVE_SCORE = 2000000
};
enum {
    WHITE = 0, BLACK = 0x8
};
enum {
    EMPTY = 0x00,
    ENPASSANTNULL = 127,
    PAWN = 0x01,
    KING = 0x02,
    KNIGHT = 0x03,
    BISHOP = 0x05,
    ROOK = 0x06,
    QUEEN = 0x07
};
// White pieces have their 4th bit = 0
enum {
    WHITE_PAWN = 0x01,
    WHITE_KING = 0x02,
    WHITE_KNIGHT = 0x03,
    WHITE_BISHOP = 0x05,
    WHITE_ROOK = 0x06,
    WHITE_QUEEN = 0x07
};
// Black pieces have their 4th bit = 1
enum {
    BLACK_PAWN = 0x09,
    BLACK_KING = 0x0A,
    BLACK_KNIGHT = 0x0B,
    BLACK_BISHOP = 0x0D,
    BLACK_ROOK = 0x0E,
    BLACK_QUEEN = 0x0F
};

enum esqare {
    A1 = 0, B1, C1, D1, E1, F1, G1, H1,
    A2 = 16, B2, C2, D2, E2, F2, G2, H2,
    A3 = 32, B3, C3, D3, E3, F3, G3, H3,
    A4 = 48, B4, C4, D4, E4, F4, G4, H4,
    A5 = 64, B5, C5, D5, E5, F5, G5, H5,
    A6 = 80, B6, C6, D6, E6, F6, G6, H6,
    A7 = 96, B7, C7, D7, E7, F7, G7, H7,
    A8 = 112, B8, C8, D8, E8, F8, G8, H8
};

#define CASTLE_WK    (1<<3)
#define CASTLE_WQ    (1<<2)
#define CASTLE_BK    (1<<1)
#define CASTLE_BQ    (1<<0)

typedef unsigned char u8;
typedef unsigned long long u64;


#define    valUNKNOWN   (~0)
#define    hashfEXACT   (1)
#define    hashfALPHA   (2)
#define    hashfBETA    (3)

typedef struct tagHASHE {
    u64 key;
    unsigned char depth;
    unsigned char flags;    // bits 0,1,2,3 store the type (exact,alpha,beta etc)
    int value;
    unsigned int bestMove;
} Hash_Entry;

struct aboard {
    u8 bs[128];
    u8 sideToMove;
    u8 castleRights;
    u8 enPassant;
    u8 fiftyCounter;
    short int ply;
    u64 posKey;

    short int gameply;
    u64 historyPosKey[MAX_PLY_IN_A_GAME];
    int historymatValue[2][MAX_PLY_IN_A_GAME];
    unsigned int searchKillers[2][MAX_PLY_IN_A_GAME];
    int searchHistory[16][128];
    Hash_Entry *ht;
    u64 htSize;

    int opawnCount[2];
    int obigCount[2];
    int matValues[2];
    int psqValues[2];
    int pawnCount[2];
    int bigCount[2];
#ifndef NDEBUG
    int historymoves[MAX_PLY_IN_A_GAME];
#endif
};


struct _smove {
    unsigned int move;
    u8 enPassantsq;
    u8 fiftycounter;
    u8 castleRights;
    int score;
};
typedef struct _smove smove;

//#define moveMAX	128
typedef struct LINE {
    int cmove;              // Number of moves in the line.
    unsigned int argmove[MAXDEPTH];  // The line.
} LINE;

enum {
    GAMEMODE_CONSOLE,
    GAMEMODE_UCI,
    GAMEMODE_SILLENT
};

typedef struct {
    int maxSearchPly;
    unsigned int starttime;
    unsigned int stoptime;
    int depth;
    int timeset;
    int movestogo;

    u64 nodes;
    u64 qnodes;

    int quit;
    int stopped;

    int failHigh;
    int failHighFirst;
    int nullCut;
    int lmr;
    int lmr2;
    int lmr3;

    int hthit;
    int htmiss;
    int htExact;
    int htAlpha;
    int htBeta;

    int displayCurrmove;

    int GAME_MODE;
    int POST_THINKING;

} S_SEARCHINFO;


extern struct aboard board;

extern int kingLoc[2];

extern int matValues[];
extern int psq_pawns[2][64];
extern int raw_psq_pawns[64];
extern int psq_knights[2][64];
extern int raw_psq_knights[64];
extern int psq_bishops[2][64];
extern int raw_psq_bishops[64];
extern int psq_rooks[2][64];
extern int raw_psq_rooks[64];
extern int psq_queens[2][64];
extern int raw_psq_queens[64];

extern u64 pieceKeys[16][128];
extern u64 castleKeys[16];
extern u64 side;

extern LINE pv;

enum {
    INFINITE = 32000,
    CHECKMATE_SCORE = 31000,
    ISMATE = CHECKMATE_SCORE - MAXDEPTH,
    STALEMATE_SCORE = 0,
    DDEPTH = 8
};

void InitMvvLva(void);

char *move_to_san(smove m, int mcount, smove *moves);

void printMoveList(void);

void printLine(LINE *line, S_SEARCHINFO *info);

int generateMoves(smove *moves);

int generateCaptureMoves(smove *moves);

void initBoard(void);

void printBoard(void);

int findOtherKing(void);

void fen2board(char *str);

char *board2fen(void);

char *sq2algebraic(u8 sq);

int isAttacked(int byColor, int sq);

u64 dummyPerft(u8 depth);

u64 Perft(u8 depth);

u64 Divide(u8 depth);

int moveExists(unsigned int move);

int generateLegalMoves(smove *m);

void printMove(smove m);

void pickMove(smove *m, int j, int n);

int move_make(smove *move);

int move_unmake(smove *move);

void move_makeNull(smove *move);

void move_unmakeNull(smove *move);

unsigned int get_ms(void);

int think(S_SEARCHINFO *info);

void thinkFen(char *fen, int depth);

int Evaluate(void);

int AlphaBeta(int depth, int alpha, int beta, LINE *pline, int doNull, S_SEARCHINFO *info);

int Quiescence(int alpha, int beta, S_SEARCHINFO *info);

int numOfLegalMoves(void);

void CheckUp(S_SEARCHINFO *info);

char *moveToUCI(unsigned int move);

int InputWaiting(void);

void ReadInput(S_SEARCHINFO *info);

void input_loop(S_SEARCHINFO *info);

u64 rand64(void);

void rkissSeed(int seed);

u64 generatePosKey(void);

void initHash(void);

void TT_set_size(unsigned int mbSize);

void TT_clear(void);

void TT_free(void);

Hash_Entry *TT_probe(unsigned int *move, int *score, int depth, int alpha, int beta);

void TT_RecordHash(int depth, int value, int hashf, unsigned int best);

void TT_fillPVLineFromTT(int deep, LINE *tLine);

char *getCPUModel(void);

void testEPD(char *filename, int miliseconds);

#endif /* SILLYCHESS_H_ */
