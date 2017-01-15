/*
 * eval.c
 *
 *  Created on: Jul 2, 2014
 *      Author: alex
 */
#include "sillychess.h"


/*
 PAWN = 0x01,
 KING = 0x02,
 KNIGHT = 0x03,
 BISHOP = 0x05,
 ROOK = 0x06,
 QUEEN = 0x07
 BLACK_PAWN = 0x09,
 BLACK_KING = 0x0A,
 BLACK_KNIGHT = 0x0B,
 BLACK_BISHOP = 0x0D,
 BLACK_ROOK = 0x0E,
 BLACK_QUEEN = 0x0F
 */

int matValues[] = {
		0,  100,  20000,  350, 0,  350,  525,  1000,
		0, -100, -20000, -350, 0, -350, -525, -1000 };

int psq_pawns[2][64];
int psq_knights[2][64];
int psq_bishops[2][64];
int psq_rooks[2][64];
int psq_queens[2][64];

int raw_psq_pawns[] = {
		 0,   0, 0,  0,  0,  0,  0,  0,
		50, 50, 50, 50, 50, 50, 50, 50,
		10, 10, 20,	30, 30, 20, 10, 10,
		 5,  5, 10, 25, 25, 10,  5,  5,
		 0,  0,  0, 20, 20,  0,  0,  0,
		 5, -5, -10, 0,  0,-10, -5,  5,
		 5, 10, 10,-20,-20, 10, 10,  5,
		 0,  0,  0,  0,  0,  0,  0,  0
};

int raw_psq_knights[] = {
		-50,-40,-30,-30,-30,-30,-40,-50,
		-40,-20,  0,  0,  0,  0,-20,-40,
		-30,  0, 10, 15, 15, 10,  0,-30,
		-30,  5, 15, 20, 20, 15,  5,-30,
		-30,  0, 15, 20, 20, 15,  0,-30,
		-30,  5, 10, 15, 15, 10,  5,-30,
		-40,-20,  0,  5,  5,  0,-20,-40,
		-50,-40,-30,-30,-30,-30,-40,-50
};

int raw_psq_bishops[] = {
		-20,-10,-10,-10,-10,-10,-10,-20,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-10,  0,  5, 10, 10,  5,  0,-10,
		-10,  5,  5, 10, 10,  5,  5,-10,
		-10,  0, 10, 10, 10, 10,  0,-10,
		-10, 10, 10, 10, 10, 10, 10,-10,
		-10,  5,  0,  0,  0,  0,  5,-10,
		-20,-10,-10,-10,-10,-10,-10,-20
};

int raw_psq_rooks[] = {
		 0,  0,  0,  0,  0,  0,  0,  0,
		 5, 10, 10, 10, 10, 10, 10,  5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		 0,  0,  0,  5,  5,  0,  0,  0
};

int raw_psq_queens[] = {
		-20,-10,-10, -5, -5,-10,-10,-20,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-10,  0,  5,  5,  5,  5,  0,-10,
		 -5,  0,  5,  5,  5,  5,  0, -5,
		  0,  0,  5,  5,  5,  5,  0, -5,
		-10,  5,  5,  5,  5,  5,  0,-10,
		-10,  0,  5,  0,  0,  0,  0,-10,
		-20,-10,-10, -5, -5,-10,-10,-20
};

int bishopPairBonus[] = {0,0};//{50, -50};

int Evaluate(void) {

#ifndef NDEBUG
	int mat = 0;
	int bishopPair[2];
	bishopPair[0]=bishopPair[1]=0;
	board.obigCount[0]=board.obigCount[1]=0;
	board.opawnCount[0]=board.opawnCount[1]=0;
	for (int i = 0; i < 64; ++i) {
		int sq88=i + (i & ~7);
		if ((board.bs[sq88] != EMPTY)) {
			mat += matValues[board.bs[sq88]];
			int piece=board.bs[sq88]&0x07;
			int color=(board.bs[sq88]&BLACK)>>3;
			switch(piece) {
			case KING:
				continue;	// so we don't increment bigCount
				break;
			case PAWN:
				mat+=psq_pawns[color][i];
				++board.opawnCount[color];
				continue;	// so we don't increment bigCount
				break;
			case BISHOP:
				if (bishopPair[color]++)
					mat+=bishopPairBonus[color];
				mat+=psq_bishops[color][i];
				break;
			case KNIGHT:
				mat+=psq_knights[color][i];
				break;
			case ROOK:
				mat+=psq_rooks[color][i];
				break;
			case QUEEN:
				mat+=psq_queens[color][i];
				break;
			}
			++board.obigCount[color];
		}
	}

//	/* Checks to see if the updated on capture/uncapture material values match the calculated ones */
//	if (board.obigCount[0]!=board.bigCount[0]) {
//		printf("obigcount: %d/%d\nbigcount: %d/%d\n",board.obigCount[0],board.obigCount[1],board.bigCount[0],board.bigCount[1]);
//
//	}

	if ((board.matValues[0]+board.matValues[1])!=mat) {
		printBoard();
		printf("matValues: %d/%d = %d mat: %d\n",
				board.matValues[0],
				board.matValues[1],
				board.matValues[0]+board.matValues[1],
				mat);
	}
//	else {
//		printf("Success\n");
//	}
#else
	int mat = board.matValues[0]+board.matValues[1];
#endif
	ASSERT(board.obigCount[0]==board.bigCount[0]);
	ASSERT(board.obigCount[1]==board.bigCount[1]);
	ASSERT((board.matValues[0]+board.matValues[1])==mat);

	if ( board.sideToMove == BLACK )
	       return -mat;
	    else
	       return mat;
}
