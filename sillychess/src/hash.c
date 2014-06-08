/*
 * hash.c
 *
 *  Created on: Jun 3, 2014
 *      Author: alex
 */
#include "sillychess.h"

u64 pieceKeys[16][128];
u64 castleKeys[16];
u64 side;

void initHash(void)
{
	int i, j;

	rkissSeed(0);
	for (j = EMPTY; j <= BLACK_QUEEN; ++j) {
		castleKeys[j] = rand64();
		for (i = 0; i < 128; ++i)
			if (!(i&0x88))
				pieceKeys[j][i] = rand64();
	}
	side=rand64();
}

u64 generatePosKey(void)
{
	int i;
	int sq88;
	u64 posKey = 0;
	for (i = 0; i < 64; ++i) {
		sq88 = i + (i & ~7);
		if ((board.bs[sq88] != EMPTY))
			posKey ^= pieceKeys[board.bs[sq88]][sq88];
	}
	if (board.enPassant!=ENPASSANTNULL)
		posKey ^= pieceKeys[EMPTY][board.enPassant];

	posKey^= castleKeys[board.castleRights];
	if (board.sideToMove)
		posKey^= side;
	return posKey;
}

