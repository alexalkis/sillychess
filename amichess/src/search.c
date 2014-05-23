/*
 * search.c
 *
 *  Created on: May 22, 2014
 *      Author: alex
 */
#include "amichess.h"
#include <string.h>

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

int inodes;

int matValues[] = {
		0, 100, 20000, 300, 0, 330, 500, 900,
		0, -100, -20000, -300,0, -330, -500, -900 };

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
	for (i = 0; i < 64; ++i) {
		sq88=i + (i & ~7);
		if ((board.bs[sq88] != EMPTY)) {
			mat += matValues[board.bs[sq88]];
			int piece=board.bs[sq88]&0x07;
			switch(piece) {
			case PAWN:
				mat+=psq_pawns[(board.bs[sq88]&BLACK)>>3][i];
				break;
			case BISHOP:
				mat+=psq_bishops[(board.bs[sq88]&BLACK)>>3][i];
				break;
			case KNIGHT:
				mat+=psq_knights[(board.bs[sq88]&BLACK)>>3][i];
				break;
			case ROOK:
				mat+=psq_rooks[(board.bs[sq88]&BLACK)>>3][i];
				break;
			case QUEEN:
				mat+=psq_queens[(board.bs[sq88]&BLACK)>>3][i];
				break;
			}
		}
	}
	if ( board.sideToMove == BLACK )
	       return -mat;
	    else
	       return mat;
}

int numOfLegalMoves(void){
	int i;
	int legalMoves = 0;
	smove m[256];
	int mcount = generateMoves(m);
	for (i = 0; i < mcount; ++i) {
			move_make(&m[i]);
			if (!isAttacked(board.sideToMove, kingLoc[1 - (board.sideToMove >> 3)])) {
				++legalMoves;
			}
			move_unmake(&m[i]);
	}
	return legalMoves;
}
void thinkFen(char *fen,int depth)
{
	LINE line;

	int i;
	fen2board(fen);
	inodes=0;
	int sc=AlphaBeta(0,depth, -CHECKMATE_SCORE-1000, +CHECKMATE_SCORE+1000, &line);
	smove m;
	printf("res=%d number of moves in pv: %d\n", sc, line.cmove);

	for (i = 0; i < line.cmove; ++i) {
		m.move = line.argmove[i];
		move_make(&m);
		printMove(m);
		if (isAttacked(board.sideToMove^BLACK,kingLoc[1 - ((board.sideToMove^BLACK) >> 3)])) {
			if (numOfLegalMoves())
				printf("+");
			else
				printf("#");
		}
		printf(" ");
	}
	for(i=line.cmove-1; i>=0; --i) {
		m.move = line.argmove[i];
		move_unmake(&m);
	}
	m.move = line.argmove[0];
	move_make(&m);
	printBoard();
	printf("Nodes: %d\n",inodes);
}



int AlphaBeta(int ply,int depth, int alpha, int beta, LINE * pline) {

	int i;
	int val=alpha;
	LINE line;
	if (depth == 0) {
		pline->cmove = 0;
		return Evaluate();
	}
	++inodes;
	smove m[256];
	int mcount = generateMoves(m);  //    GenerateLegalMoves();
	int legalMoves = 0;
	for (i = 0; i < mcount; ++i) {
		move_make(&m[i]);
		if (!isAttacked(board.sideToMove, kingLoc[1 - (board.sideToMove >> 3)])) {
			val = -AlphaBeta(ply+1,depth - 1, -beta, -alpha, &line);
			if (depth==DDEPTH)  {
				printMove(m[i]);printf(" score: %d\n",val);
			}
			++legalMoves;
		}
		move_unmake(&m[i]);
		if (val >= beta)
			return beta;
		if (val > alpha) {
			alpha = val;
			pline->argmove[0] = m[i].move;
			memcpy(pline->argmove + 1, line.argmove, line.cmove * sizeof(int));
			pline->cmove = line.cmove + 1;
		}
	}

	if (!legalMoves) {
		if (isAttacked(board.sideToMove^BLACK,
				kingLoc[1 - ((board.sideToMove^BLACK) >> 3)])) {

//			printBoard();
//			printf("king at : %s\n",sq2algebraic(kingLoc[1 - (board.sideToMove >> 3)]));
//			printf("Checkmate seen?! (%d moves checked and all illegal)\n",mcount);
//			printMoveList();
//			exit(1);
			return -CHECKMATE_SCORE+ply;
		} else {
			//printBoard();
			//printf("king at : %s\n",sq2algebraic(kingLoc[1 - (board.sideToMove >> 3)]));
			//printf("Stalemate seen?!(%d moves checked and all illegal)\n",mcount);
			//exit(1);
			return STALEMATE_SCORE;
		}
	}
	return alpha;
}
