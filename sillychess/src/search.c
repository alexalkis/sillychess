/*
 * source.c
 *
 *  Created on: May 27, 2014
 *      Author: alex
 */


#include "sillychess.h"
#include "move.h"

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
	//printf("Checked: %d\n",info->stoptime-get_ms());
	unsigned int time=get_ms();
	if(info->timeset == TRUE && time > info->stoptime) {

		info->stopped = TRUE;
		info->displayCurrmove=FALSE;
	} else	if (info->GAME_MODE!=GAMEMODE_SILLENT && (time-info->starttime)>3000)
		info->displayCurrmove=TRUE;
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

	if (bestNum==j) return;

	temp = m[j];
	m[j] = m[bestNum];
	m[bestNum] = temp;
}



int isRepetition(void) {
	ASSERT(board.gameply-board.fiftyCounter>=0);

	int i;
	if (board.fiftyCounter>=100)
		return TRUE;
	//for(i=board.gameply-board.fiftyCounter; i<board.gameply-1; ++i)
	for(i=board.gameply-1-1; i>=board.gameply-board.fiftyCounter; i-=2)
		if (board.posKey==board.historyPosKey[i]) {
			//printf("Rep seen.\n");
			return TRUE;
		}
	return FALSE;
}

int Quiesce(int alpha, int beta, S_SEARCHINFO *info)
{
	int i;
	int stand_pat = Evaluate();
	int score = stand_pat;

	++info->nodes;
	if (isRepetition())
		return 0;
	if ((info->nodes & 0xFFF) == 0) {
		CheckUp(info);
	}
	if (stand_pat >= beta)
		return beta;
	if (alpha < stand_pat)
		alpha = stand_pat;
	smove m[256];
	int mcount = generateCaptureMoves(m);
	int legalMoves=0;
	for (i = 0; i < mcount; ++i) {
		pickMove(m, i, mcount);
		move_make(&m[i]);
		ASSERT(m[i].score>=CAPTURE_SCORE);
		if (!isAttacked(board.sideToMove, kingLoc[1 - (board.sideToMove >> 3)])) {
			++legalMoves;
			score = -Quiesce( -beta, -alpha, info);
		}
		move_unmake(&m[i]);
		if (info->stopped == TRUE) {
			return 0;
		}
		if (score >= beta) {
			if (legalMoves == 1)
				++info->fhf;
			else
				++info->fh;
			return beta;
		}
		if (score > alpha)
			alpha = score;
	}
	return alpha;
}

inline int razor_margin(int d) { return 512 + 16 * d; }

const int FullDepthMoves = 4;
const int ReductionLimit = 3;

int AlphaBeta(int depth, int alpha, int beta, LINE * pline, int doNull,S_SEARCHINFO *info) {
	int i;
	int val=alpha;
	LINE line;
	int PvMove = NOMOVE;
	HASHE *tte;
	int PvNode = (beta-alpha)>1;


	if (board.ply && isRepetition() ) {
		pline->cmove=0;
		++info->nodes;
		return 0;
	}

	if ( (tte=TT_probe(&PvMove, &val, depth, alpha, beta))
		&&	( PvNode ?  tte->flags == hashfEXACT
			            : val >= beta ? (tte->flags==hashfBETA)
			                              : (tte->flags==hashfALPHA))) {
		++info->hthit;
		++info->nodes;
//		if (PvMove != NOMOVE) {
//			pline->cmove = 1;
//			pline->argmove[0] = PvMove;
//		}
		return val;
	} else
		++info->htmiss;

	if (depth == 0) {
		pline->cmove = 0;
		if (PvNode && board.ply>info->maxSearchPly)
			info->maxSearchPly=board.ply;
		return Quiesce(alpha,beta,info);
	}

	++info->nodes;


	if ((info->nodes & 0xFFF) == 0) {
		CheckUp(info);
	}





	//is the square of _our_ king attacked by the other side?  i.e. are _we_ in check?
	int inCheck = isAttacked(board.sideToMove ^ BLACK,	kingLoc[board.sideToMove >> 3]);
	/* if our king is attacked by the other side, let's increment the depth */
	if (inCheck) {
		++depth;
		goto skipprunning;
	}

	int eval=Evaluate();

	//razor pruning
	if (   !PvNode
			//&& !inCheck  //no need cause of the goto above
	        &&  depth < 4
	        &&  eval + razor_margin(depth) <= alpha
	        &&  PvMove == NOMOVE
	        //&& !pos.pawn_on_7th(pos.side_to_move())
	        )
	    {
	        if (   depth <= 1
	            && eval + razor_margin(3) <= alpha)
	        	return Quiesce(alpha,beta,info);
	        int ralpha = alpha - razor_margin(depth);
	        int v = Quiesce(ralpha,ralpha+1,info);
	        if (v <= ralpha)
	            return v;
	    }


	//  Futility pruning: child node (skipped when in check) (origin stockfish)
	if (   !PvNode
		&& doNull
		//&& !inCheck	//no need cause of the goto above
		&&  depth < 7
		&&  eval - (depth*100) >= beta
		&&  abs(beta) < ISMATE
		&&  abs(eval) < 10000
		&&  (board.bigCount[board.sideToMove>>3] > 0)
		)
		return eval - (depth*100);


	if (doNull &&
			!PvNode &&
			//!inCheck &&	//no need cause of the goto above
			board.ply &&
			(board.bigCount[board.sideToMove>>3] > 0) &&
			depth >= 4) {
		smove nm;
		move_makeNull(&nm);
		int Score = -AlphaBeta(depth - 4, -beta, -beta + 1, &line, FALSE,info);
		move_unmakeNull(&nm);
		if (Score >= beta && abs(Score) <=(ISMATE)) {
			++info->nullCut;
			return beta;
		}
	}



	smove m[256];
	int mcount;

skipprunning:
	mcount = generateMoves(m);  //    GenerateLegalMoves();
	if (board.ply < pv.cmove) {
		for (i = 0; i < mcount; ++i) {
			if (m[i].move == pv.argmove[board.ply]) {
				m[i].score = PVMOVE_SCORE;
				//printf("Found %d\n",++found);
				//if (ply==0) printf("Pv move found (ply %d)\n",ply);
				break;
			}
		}
	}
//	else {
//		if (PvMove != NOMOVE) {
//			for (i = 0; i < mcount; ++i) {
//				if (m[i].move == PvMove) {
//					m[i].score = PVMOVE_SCORE;
//					break;
//				}
//			}
//
//		}
//	}

	int BestMove = NOMOVE;
	int legalMoves = 0;
	int OldAlpha = alpha;
	int BestScore = -INFINITE;

	for (i = 0; i < mcount; ++i) {
		pickMove(m,i,mcount);
		move_make(&m[i]);
		if (isAttacked(board.sideToMove, kingLoc[1 - (board.sideToMove >> 3)])) {
			move_unmake(&m[i]);
			continue;
		}

		/* LMR here */
		if (!legalMoves) {
			val = -AlphaBeta(depth - 1, -beta, -alpha, &line,TRUE,info);
		}
		else {
			if(legalMoves >= FullDepthMoves &&
					depth >= ReductionLimit &&
					!inCheck &&
					!PvNode &&
					!ISCAPTUREORPROMOTION(m[i].move)&&
					m[i].move!=board.searchKillers[0][board.ply] &&
					m[i].move!=board.searchKillers[1][board.ply])
			{
				// Search this move with reduced depth:
				val = -AlphaBeta(depth-2,-(alpha+1), -alpha, &line,TRUE,info);
				++info->lmr;
			} else
				val = alpha+1;
			if (val > alpha) {
				val = -AlphaBeta(depth - 1, -(alpha + 1), -alpha,	&line, TRUE, info);
				++info->lmr2;
				if (val > alpha && val < beta) {
					val = -AlphaBeta(depth - 1, -beta, -alpha,&line, TRUE, info);
					++info->lmr3;
				}
			}
		}
		++legalMoves;



		move_unmake(&m[i]);
		if (!board.ply && info->displayCurrmove) {
			printf("info depth %d currmove %s currmovenumber %d\n",depth,moveToUCI(m[i].move),i+1);
			fflush(stdout);
		}

		if (info->stopped == TRUE) {
			return 0;
		}

		if (val > BestScore) {
			BestScore = val;
			BestMove = m[i].move;
			if (val > alpha) {
				if (val >= beta) {
					if (legalMoves == 1)
						++info->fhf;
					else
						++info->fh;
					if (!(ISCAPTUREORPROMOTION(m[i].move))) {
						if (board.searchKillers[0][board.ply] != m[i].move) {
							board.searchKillers[1][board.ply] = board.searchKillers[0][board.ply];
							board.searchKillers[0][board.ply] = m[i].move;
						}
					}
					TT_RecordHash(depth, beta, hashfBETA, BestMove);
					++info->htBeta;
					return beta;
				}
				alpha = val;
				pline->argmove[0] = m[i].move;
				memcpy(pline->argmove + 1, line.argmove,line.cmove * sizeof(int));
				pline->cmove = line.cmove + 1;
				if (!(ISCAPTUREORPROMOTION(m[i].move))) {
					board.searchHistory[PIECE(m[i].move)][TO(m[i].move)]+=depth;
				}
			}
		}

	}
	if (!legalMoves) {
		if (inCheck) {
			return -CHECKMATE_SCORE+board.ply;
		} else {
			return STALEMATE_SCORE;
		}
	}
	if (alpha != OldAlpha) {
			TT_RecordHash(depth, BestScore, hashfEXACT, BestMove);
			++info->htExact;
		} else {
			++info->htAlpha;
			TT_RecordHash(depth, alpha, hashfALPHA, BestMove);
		}
	return alpha;
}

/*
info depth 16 (81.04%, NULLMOVES: 296264 LMR: 3961159 (69815264/11) nodes 283107225 time 170685 nps 3072410 score cp 25 pv e2e4 e7e5 b1c3 b8c6 g1f3 g8f6 d2d4 e5d4 f3d4 f8b4 d4c6 d7c6 d1d8+ e8d8 c1d2 c8e6 f1d3 (e2e4 e7e5 g1f3 b8c6 d2d4 )
Hash - Exact:1408 Alpha: 4531018 Beta: 33186460  -- Hits: 6570177 Misses: 38244828
bestmove e2e4
Time taken: 170685

After History heuristics in
info depth 16 (86.17%, NULLMOVES: 249517 LMR: 3020734 (52207576/40) nodes 185983015 time 124370 nps 2537077 score cp 20 pv e2e4 e7e5 g1f3 g8f6 b1c3 b8c6 d2d4 e5d4 f3d4 d7d5 d4c6 b7c6 e4e5 f6e4 c1e3 c8f5 (e2e4 e7e5 g1f3 g8f6 b1c3 )
Hash - Exact:546 Alpha: 3258458 Beta: 23372963  -- Hits: 7720423 Misses: 114779355
bestmove e2e4
Time taken: 124370

v0.3.3
info depth 16 seldepth 18 (85.55%, NULLMOVES: 194669 LMR: 2497018 (27622693/32) nodes 101965805 time 63926 nps 3097193 score cp 25 pv e4 e5 Nf3 Nf6 Nc3 Nc6 d4 exd4 Nxd4 Bb4 Nxc6 dxc6 Qxd8+ Kxd8 Bd2 Be6 Bd3 (e4 e5 Nf3 Nc6 Nc3 Nf6 d4 exd4 )
Hash - Exact:840 Alpha: 1982040 Beta: 7163984  -- Hits: 2747707 Misses: 67012774
bestmove e2e4
Time taken: 63927
 */
