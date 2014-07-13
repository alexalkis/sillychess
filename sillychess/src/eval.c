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
		0, 100, 20000, 300, 0, 330, 500, 900,
		0, -100, -20000, -300,0, -330, -500, -900 };

//int matValues[] = {
//		0,  100,  20000,  325, 0,  325,  550,  1000,
//		0, -100, -20000, -325, 0, -325, -550, -1000 };

/*
v0.3 has the "correct" values!!! And plays worse
In v0.3.1 values were rolled back to their "out of my head" values
Rank Name                  Elo    +    - games score oppo. draws
   1 Vice 1.1             1951   12   12  2724   78%  1730   14%
   2 sc v0.3.1            1759   16   16  1200   44%  1804   13%
   3 sc v0.2              1757   13   13  1800   46%  1796   22%
   4 Bremboce 0.6.2       1753   14   14  1524   49%  1758   12%
   5 sc (no draw detect)  1717   15   15  1200   38%  1805   40%
   6 tscp                 1706   11   11  2725   40%  1784   17%
   7 sc v0.3              1704   14   15  1573   38%  1803   13%

*/

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
				 0,  0,  0,  0,  0,  0,  0,  0 };

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

int Evaluate(void) {
	int i;
	int sq88;
	int mat = 0;
	board.obigCount[0]=board.obigCount[1]=0;
	board.opawnCount[0]=board.opawnCount[1]=0;
	for (i = 0; i < 64; ++i) {
		sq88=i + (i & ~7);
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
	if (board.obigCount[0]!=board.bigCount[0]) {
		printf("obigcount: %d/%d\nbigcount: %d/%d\n",board.obigCount[0],board.obigCount[1],board.bigCount[0],board.bigCount[1]);

	}
	if ((board.matValues[0]+board.matValues[1])!=mat) {
		printf("matValues: %d/%d mat: %d\n",board.matValues[0],board.matValues[1],mat);
	}

	ASSERT(board.obigCount[0]==board.bigCount[0]);
	ASSERT(board.obigCount[1]==board.bigCount[1]);
	ASSERT((board.matValues[0]+board.matValues[1])==mat);
	if ( board.sideToMove == BLACK )
	       return -mat;
	    else
	       return mat;
}
