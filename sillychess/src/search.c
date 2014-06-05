/*
 * source.c
 *
 *  Created on: May 27, 2014
 *      Author: alex
 */


#include "sillychess.h"
#include "move.h"
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

void CheckUp(S_SEARCHINFO *info) {
	// .. check if time up, or interrupt from GUI
	//printf("Checked: %d\n",info->stoptime-get_ms());
	if(info->timeset == TRUE && get_ms() > info->stoptime) {

		info->stopped = TRUE;
	}
	ReadInput(info);
}

void pickMove(smove *m,int j,int n)
{
	int i;
	smove temp;
	int bestScore = -CHECKMATE_SCORE;
	int bestNum = j;

	for(i=j; i<n; ++i) {
		if (m[i].score>bestScore) {
			bestScore = m[i].score;
			bestNum = i;
		}
	}

	temp = m[j];
	m[j] = m[bestNum];
	m[bestNum] = temp;
}

int OldAlphaBeta(int ply,int depth, int alpha, int beta, LINE * pline, int doNull,S_SEARCHINFO *info) {
	int i;
	int val=alpha;
	LINE line;

	if (depth == 0) {
		pline->cmove = 0;
		return Quiesce(0,alpha,beta,info);
		//return Evaluate();
	}
	++info->nodes;

	if ((info->nodes & 0xFFF) == 0) {
		CheckUp(info);
	}

	//is the square of _our_ king attacked by the other side?  i.e. are _we_ in check?
	int inCheck = isAttacked(board.sideToMove ^ BLACK,	kingLoc[board.sideToMove >> 3]);
	if (inCheck) ++depth;
#ifdef NULLMOVE
	if (doNull && !inCheck && ply /*&& (pos->bigPce[pos->side] > 0)*/ && depth >= 4) {
		smove nm;
		move_makeNull(&nm);
		int Score = -AlphaBeta(ply + 1, depth - 4, -beta, -beta + 1, &line, FALSE,info);
		move_unmakeNull(&nm);
		if (Score >= beta && abs(Score) < 28000) {
			++info->nullCut;
			return beta;
		}
	}
#endif

	smove m[256];
	int mcount = generateMoves(m);  //    GenerateLegalMoves();
	if( ply < pv.cmove) {
			for(i = 0; i < mcount; ++i) {
				if( m[i].move == pv.argmove[ply]) {
					m[i].score = 2000000;
					//if (ply==0) printf("Pv move found (ply %d)\n",ply);
					break;
				}
			}
		}


	int legalMoves = 0;
	for (i = 0; i < mcount; ++i) {
		pickMove(m,i,mcount);
		move_make(&m[i]);
		if (!isAttacked(board.sideToMove, kingLoc[1 - (board.sideToMove >> 3)])) {
			val = -AlphaBeta(ply+1,depth - 1, -beta, -alpha, &line,TRUE,info);
			++legalMoves;
		}
		move_unmake(&m[i]);
		if (info->stopped == TRUE) {
			return 0;
		}

		if (val > alpha) {
			if (val >= beta) {
				if (legalMoves==1)
					++info->fhf;
				else
					++info->fh;
				if(!(ISCAPTURE(m[i].move))) {
					if (board.searchKillers[0][board.ply] != m[i].move) {
						board.searchKillers[1][board.ply] = board.searchKillers[0][board.ply];
						board.searchKillers[0][board.ply] = m[i].move;
					}
				}
				return beta;
			}
			alpha = val;
			pline->argmove[0] = m[i].move;
			memcpy(pline->argmove + 1, line.argmove, line.cmove * sizeof(int));
			pline->cmove = line.cmove + 1;
//			if (depth==DDEPTH) {
//				printf("nodes %lld cp %d ",inodes,val);
//				printLine(pline);
//			}
		}
	}

	if (!legalMoves) {
		if (inCheck) {
			return -CHECKMATE_SCORE+ply;
		} else {
			return STALEMATE_SCORE;
		}
	}
	return alpha;
}


//http://www.glaurungchess.com/lmr.html
const int FullDepthMoves = 4;
const int ReductionLimit = 3;
int AlphaBeta(int ply,int depth, int alpha, int beta, LINE * pline, int doNull,S_SEARCHINFO *info) {
	int i;
	int val=alpha;
	LINE line;

	if (depth == 0) {
		pline->cmove = 0;
		return Quiesce(0,alpha,beta,info);
		//return Evaluate();
	}
	++info->nodes;

	if ((info->nodes & 0xFFF) == 0) {
		CheckUp(info);
	}

	//is the square of _our_ king attacked by the other side?  i.e. are _we_ in check?
	int inCheck = isAttacked(board.sideToMove ^ BLACK,	kingLoc[board.sideToMove >> 3]);
	if (inCheck) ++depth;
#ifdef NULLMOVE
	if (doNull && !inCheck && ply /*&& (pos->bigPce[pos->side] > 0)*/ && depth >= 4) {
		smove nm;
		move_makeNull(&nm);
		int Score = -AlphaBeta(ply + 1, depth - 4, -beta, -beta + 1, &line, FALSE,info);
		move_unmakeNull(&nm);
		if (Score >= beta && abs(Score) < 28000) {
			++info->nullCut;
			return beta;
		}
	}
#endif

	smove m[256];
	int mcount = generateMoves(m);  //    GenerateLegalMoves();
	if( ply < pv.cmove) {
			for(i = 0; i < mcount; ++i) {
				if( m[i].move == pv.argmove[ply]) {
					m[i].score = 2000000;
					//if (ply==0) printf("Pv move found (ply %d)\n",ply);
					break;
				}
			}
		}


	int legalMoves = 0;
	int moves_searched=0;
	for (i = 0; i < mcount; ++i) {
		pickMove(m,i,mcount);
		move_make(&m[i]);
		if (!isAttacked(board.sideToMove, kingLoc[1 - (board.sideToMove >> 3)])) {
			if (!moves_searched)
				val = -AlphaBeta(ply+1,depth - 1, -beta, -alpha, &line,TRUE,info);
			else {
				if(moves_searched >= FullDepthMoves && depth >= ReductionLimit &&
				         !inCheck &&
				         !ISCAPTURE(m[i].move)&&
				         !ISPROMOTION(m[i].move) &&
				         m[i].move!=board.searchKillers[0][ply] &&
				         m[i].move!=board.searchKillers[1][ply])
				{
					// Search this move with reduced depth:
					val = -AlphaBeta(ply+1,depth-2,-(alpha+1), -alpha, &line,TRUE,info);

				} else
					val = alpha+1;
				if(val > alpha) {
				        val = -AlphaBeta(ply+1,depth-1,-(alpha+1), -alpha, &line,TRUE,info);
				        if(val > alpha && val < beta)
				          val = -AlphaBeta(ply+1,depth - 1, -beta, -alpha, &line,TRUE,info);
				      }
			}
			++legalMoves;
		}
		move_unmake(&m[i]);
		++moves_searched;
		if (info->stopped == TRUE) {
			return 0;
		}

		if (val > alpha) {
			if (val >= beta) {
				if (legalMoves==1)
					++info->fhf;
				else
					++info->fh;
				if(!(ISCAPTURE(m[i].move))) {
					if (board.searchKillers[0][board.ply] != m[i].move) {
						board.searchKillers[1][board.ply] = board.searchKillers[0][board.ply];
						board.searchKillers[0][board.ply] = m[i].move;
					}
				}
				return beta;
			}
			alpha = val;
			pline->argmove[0] = m[i].move;
			memcpy(pline->argmove + 1, line.argmove, line.cmove * sizeof(int));
			pline->cmove = line.cmove + 1;
//			if (depth==DDEPTH) {
//				printf("nodes %lld cp %d ",inodes,val);
//				printLine(pline);
//			}
		}
	}

	if (!legalMoves) {
		if (inCheck) {
			return -CHECKMATE_SCORE+ply;
		} else {
			return STALEMATE_SCORE;
		}
	}
	return alpha;
}

int Quiesce(int qply, int alpha, int beta, S_SEARCHINFO *info)
{
	int i;
	int stand_pat = Evaluate();
	int score = stand_pat;

	++info->nodes;
	if ((info->nodes & 0xFFF) == 0) {
		CheckUp(info);

	}
	if (stand_pat >= beta)
		return beta;
	if (alpha < stand_pat)
		alpha = stand_pat;
	smove m[256];
	int mcount = generateCaptureMoves(m);
	for (i = 0; i < mcount; ++i) {
		pickMove(m, i, mcount);
		move_make(&m[i]);
		if (!isAttacked(board.sideToMove, kingLoc[1 - (board.sideToMove >> 3)]))
			score = -Quiesce(qply + 1, -beta, -alpha, info);
		move_unmake(&m[i]);
		if (info->stopped == TRUE) {
			return 0;
		}
		if (score >= beta)
			return beta;
		if (score > alpha)
			alpha = score;
	}
	return alpha;
}
