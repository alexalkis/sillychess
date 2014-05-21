/*
 * amichess.h
 *
 *  Created on: May 18, 2014
 *      Author: alex
 */

#ifndef AMICHESS_H_
#define AMICHESS_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef NDEBUG
#define ASSERT(x)
#else
#define ASSERT(x) \
	if(!(x)) {fprintf(stderr,"%s assertion failed. File %s, at line %d.\n",#x,__FILE__,__LINE__);\
	exit(1);}
#endif

#define COL(x) (x&7)
#define ROW(x) (x>>4)

enum {
	WHITE = 0, BLACK = 0x8
};
enum {
	EMPTY = 0x00,
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
    A1=0  , B1, C1, D1, E1, F1, G1, H1,
    A2=16 , B2, C2, D2, E2, F2, G2, H2,
    A3=32 , B3, C3, D3, E3, F3, G3, H3,
    A4=48 , B4, C4, D4, E4, F4, G4, H4,
    A5=64 , B5, C5, D5, E5, F5, G5, H5,
    A6=80 , B6, C6, D6, E6, F6, G6, H6,
    A7=96 , B7, C7, D7, E7, F7, G7, H7,
    A8=112, B8, C8, D8, E8, F8, G8, H8
};

#define CASTLE_WK	(1<<3)
#define CASTLE_WQ	(1<<2)
#define CASTLE_BK	(1<<1)
#define CASTLE_BQ	(1<<0)

typedef unsigned char u8;
typedef unsigned long long u64;

struct aboard {
	u8 bs[128];
	u8 sideToMove;
	u8 castleRights;
	u8 enPassant;
	u8 fiftyCounter;
	short int ply;

};

struct _smove {
    unsigned int move;
    u8	enPassantsq;
    u8	fiftycounter;
    u8  castleRights;
    int score;
};
typedef struct _smove smove;


extern struct aboard board;

extern int kingLoc[2];

void printMoveList(void);
int generateMoves(smove *moves);
void initBoard(void);
void printBoard(void);
int findOtherKing(void);
void fen2board(char *str);
char *sq2algebraic(u8 sq);
int isAttacked(int byColor, int sq);
u64 dummyPerft(u8 depth);
u64 Perft(u8 depth);
u64 Divide(u8 depth);
int move_make(smove *move);
int move_unmake(smove *move);
int get_ms(void);
#endif /* AMICHESS_H_ */
