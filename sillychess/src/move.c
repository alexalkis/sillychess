/*
 * move.c
 *
 *  Created on: May 27, 2014
 *      Author: alex
 */


#include "sillychess.h"
#include "move.h"

#define COLOR(x) (board.bs[x] & BLACK)

void generateKingMoves(int color, int pos);
void generateKnightMoves(int color, int pos);
void generateBishopMoves(int color, int pos);
void generateRookMoves(int color, int pos);
void generateQueenMoves(int color, int pos);
void generatePawnMoves(int color, int pos);

void generateKingCaptureMoves(int color, int pos);
void generateKnightCaptureMoves(int color, int pos);
void generateBishopCaptureMoves(int color, int pos);
void generateRookCaptureMoves(int color, int pos);
void generateQueenCaptureMoves(int color, int pos);
void generatePawnCaptureMoves(int color, int pos);

void pushSpecialMove(int from, int to, int piece, int capturedPiece,
		int promotedPiece, int special);

smove *moveList;
int moveIndex;
//int ksq; /* square the king is on */

int kingLoc[2];

const int VictimScore[16] = { 	0, 100,600, 200,0,300, 400, 500,
								0, 100,600, 200,0,300, 400, 500};
static int MvvLvaScores[16][16];

void InitMvvLva(void) {
	int Attacker;
	int Victim;
	for(Attacker = WHITE_PAWN; Attacker <= BLACK_QUEEN; ++Attacker) {
		for(Victim = WHITE_PAWN; Victim <= BLACK_QUEEN; ++Victim) {
			MvvLvaScores[Victim][Attacker] = VictimScore[Victim] + 6 - ( VictimScore[Attacker] / 100);
			//printf("%d captures %d, score: %d\n",Attacker,Victim,MvvLvaScores[Victim][Attacker]);
		}
	}
}

void printMove(smove m) {
	static char *filestr = "abcdefgh";
	static char *rankstr = "12345678";
	int from = m.move & 0xff;
	int to = (m.move >> 8) & 0xff;
	int prom = (m.move >> 24) & 0x7;


	//printf("%c%c%s%c%c", filestr[from & 7], rankstr[from >> 4], cap ? "x" : "",	filestr[to & 7], rankstr[to >> 4]);
	printf("%c%c%c%c", filestr[from & 7], rankstr[from >> 4],filestr[to & 7], rankstr[to >> 4]);
	switch (prom) {
	case KNIGHT:
		printf("n");
		break;
	case BISHOP:
		printf("b");
		break;
	case ROOK:
		printf("r");
		break;
	case QUEEN:
		printf("q");
		break;
	}
}

char *moveToUCI(int move) {
	static char *filestr = "abcdefgh";
	static char *rankstr = "12345678";
	static char buffer[10];

	int from = move & 0xff;
	int to = (move >> 8) & 0xff;
	int prom = (move >> 24) & 0x7;


	//printf("%c%c%s%c%c", filestr[from & 7], rankstr[from >> 4], cap ? "x" : "",	filestr[to & 7], rankstr[to >> 4]);
	sprintf(buffer,"%c%c%c%c", filestr[from & 7], rankstr[from >> 4],filestr[to & 7], rankstr[to >> 4]);
	switch (prom) {
	case KNIGHT:
		buffer[4]='n';
		break;
	case BISHOP:
		buffer[4]='b';
		break;
	case ROOK:
		buffer[4]='r';
		break;
	case QUEEN:
		buffer[4]='q';
		break;
	}
	buffer[5]='\0';
	return buffer;
}
void printMoveList(/* smove *moves, int len*/) {
	int i;
	static char *filestr = "abcdefgh";
	static char *rankstr = "12345678";
	for (i = 0; i < moveIndex; ++i) {
		//printf("-> %d\n",moveList[i].move);
		int from = moveList[i].move & 0xff;
		int to = (moveList[i].move >> 8) & 0xff;
		int prom = (moveList[i].move >> 24) & 0x7;
		int cap = (moveList[i].move >> 20) & 0xf;
		printf("%d) %c%c%s%c%c", i + 1, filestr[from & 7], rankstr[from >> 4],
				cap ? "x" : "", filestr[to & 7], rankstr[to >> 4]);
		switch (prom) {
		case KNIGHT:
			printf("=n");
			break;
		case BISHOP:
			printf("=b");
			break;
		case ROOK:
			printf("=r");
			break;
		case QUEEN:
			printf("=q");
			break;
		}
		printf("\n");
	}
}

//void printLine(LINE *line) {
//	int i;
//	smove m;
//	for(i=0; i<line->cmove; ++i) {
//		m.move=line->argmove[i];
//		printMove(m);
//		printf(" ");
//	}
//	printf("\n");
//}



int generateMoves(smove *moves) {

	int i;

	//ksq = -1;
	moveList = moves;
	moveIndex = 0;
	for (i = 0; i < 128; ++i) {
		if (!(i & 0x88)
				&& (board.bs[i] != EMPTY
						&& (board.bs[i] & BLACK) == board.sideToMove)) {
			int color = board.bs[i] & BLACK;
			int piece = board.bs[i] & 0x07;
			switch (piece) {
			case PAWN:
				generatePawnMoves(color, i);
				break;
			case KING:
				generateKingMoves(color, i);
				//ksq = i; /* grab the king square to use for possible castling later */
				break;
			case KNIGHT:
				generateKnightMoves(color, i);
				break;
			case BISHOP:
				generateBishopMoves(color, i);
				break;
			case ROOK:
				generateRookMoves(color, i);
				break;
			case QUEEN:
				generateQueenMoves(color, i);
				break;
			default:
				fprintf(stderr,
						"Error in move.c. Piece: %d (%d at %s).  Move #%d\n",
						piece, board.bs[i], sq2algebraic(i), moveIndex);
				printBoard();
				exit(1);
			}
		}
	}
	//ASSERT(ksq != -1);

	if (board.sideToMove == WHITE) {
		if (board.castleRights & CASTLE_WK) {
			if ((board.bs[F1] == EMPTY) && (board.bs[G1] == EMPTY)
					&& (!isAttacked(board.sideToMove^BLACK, E1))
					&& (!isAttacked(board.sideToMove^BLACK, F1))
					&& (!isAttacked(board.sideToMove^BLACK, G1)))

				pushSpecialMove(E1, G1, WHITE_KING, EMPTY, EMPTY, SP_CASTLE);
		}
		if (board.castleRights & CASTLE_WQ) {
			if ((board.bs[B1] == EMPTY) && (board.bs[C1] == EMPTY)
					&& (board.bs[D1] == EMPTY)
					&& (!isAttacked(board.sideToMove^BLACK, E1))
					&& (!isAttacked(board.sideToMove^BLACK, D1))
					&& (!isAttacked(board.sideToMove^BLACK, C1)))

				pushSpecialMove(E1, C1, WHITE_KING, EMPTY, EMPTY, SP_CASTLE);
		}
	} else {
		if (board.castleRights & CASTLE_BK) {
			if ((board.bs[F8] == EMPTY) && (board.bs[G8] == EMPTY)
					&& (!isAttacked(board.sideToMove^BLACK, E8))
					&& (!isAttacked(board.sideToMove^BLACK, F8))
					&& (!isAttacked(board.sideToMove^BLACK, G8)))

				pushSpecialMove(E8, G8, BLACK_KING, EMPTY, EMPTY, SP_CASTLE);
		}
		if (board.castleRights & CASTLE_BQ) {
			if ((board.bs[B8] == EMPTY) && (board.bs[C8] == EMPTY)
					&& (board.bs[D8] == EMPTY)
					&& (!isAttacked(board.sideToMove^BLACK, E8))
					&& (!isAttacked(board.sideToMove^BLACK, D8))
					&& (!isAttacked(board.sideToMove^BLACK, C8)))

				pushSpecialMove(E8, C8, BLACK_KING, EMPTY, EMPTY, SP_CASTLE);
		}
	}

	return moveIndex;
}




void pushMove(int from, int to, int piece, int capturedPiece) {
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);

	moveList[moveIndex].move = (capturedPiece << 20) | (piece << 16)
			| (to << 8) | from;
	if (capturedPiece) {
		moveList[moveIndex].score = MvvLvaScores[capturedPiece][piece] + CAPTURE_SCORE;
		//printf("Capture score: %d\n",moveList[moveIndex].score);
	} else {
		if (board.searchKillers[0][board.ply] == moveList[moveIndex].move) {
			moveList[moveIndex].score = FIRST_KILLER_SCORE;
			//printf("First killer\n");
		} else if (board.searchKillers[1][board.ply]== moveList[moveIndex].move) {
			moveList[moveIndex].score = SECOND_KILLER_SCORE;
			//printf("Second killer\n");
		} else {
			moveList[moveIndex].score = board.searchHistory[piece][to];
		}
	}
	++moveIndex;
	to = TO(moveList[moveIndex - 1].move);
	from = FROM(moveList[moveIndex - 1].move);
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
}

void pushSpecialMove(int from, int to, int piece, int capturedPiece,
		int promotedPiece, int special) {
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
	ASSERT(promotedPiece==EMPTY);

//	if (promotedPiece != EMPTY)
//		moveList[moveIndex++].move = (special << 28)
//				| ((board.sideToMove | promotedPiece) << 24)
//				| (capturedPiece << 20) | (piece << 16) | (to << 8) | from;
//	else
		moveList[moveIndex].move = (special << 28) | (capturedPiece << 20)
				| (piece << 16) | (to << 8) | from;
		if (special==SP_ENPASSANT)
			moveList[moveIndex].score=105+CAPTURE_SCORE;
		++moveIndex;
	to = TO(moveList[moveIndex - 1].move);
	from = FROM(moveList[moveIndex - 1].move);
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
}

void pushPromotion(int from, int to, int color, int capturedPiece) {
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);

	moveList[moveIndex].move = ((color | BISHOP) << 24)
			| (capturedPiece << 20) | ((color | PAWN) << 16) | (to << 8) | from;
	moveList[moveIndex++].score = MvvLvaScores[capturedPiece][PAWN]+MvvLvaScores[BISHOP][PAWN]+CAPTURE_SCORE;
	to = TO(moveList[moveIndex - 1].move);
	from = FROM(moveList[moveIndex - 1].move);
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
	moveList[moveIndex].move = ((color | KNIGHT) << 24)
			| (capturedPiece << 20) | ((color | PAWN) << 16) | (to << 8) | from;
	moveList[moveIndex++].score = MvvLvaScores[capturedPiece][PAWN]+MvvLvaScores[KNIGHT][PAWN]+CAPTURE_SCORE;
	to = TO(moveList[moveIndex - 1].move);
	from = FROM(moveList[moveIndex - 1].move);
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
	moveList[moveIndex].move = ((color | ROOK) << 24) | (capturedPiece << 20)
			| ((color | PAWN) << 16) | (to << 8) | from;
	moveList[moveIndex++].score = MvvLvaScores[capturedPiece][PAWN]+MvvLvaScores[ROOK][PAWN]+CAPTURE_SCORE;
	to = TO(moveList[moveIndex - 1].move);
	from = FROM(moveList[moveIndex - 1].move);
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
	moveList[moveIndex].move = ((color | QUEEN) << 24) | (capturedPiece << 20)
			| ((color | PAWN) << 16) | (to << 8) | from;
	moveList[moveIndex++].score = MvvLvaScores[capturedPiece][PAWN]+MvvLvaScores[QUEEN][PAWN]+CAPTURE_SCORE;
	to = TO(moveList[moveIndex - 1].move);
	from = FROM(moveList[moveIndex - 1].move);
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
}

//void pushenPassantMove(int from, int to, int piece, int capturedPiece) {
//	ASSERT(to < 128 && to >= 0);
//	ASSERT(from < 128 && from >= 0);
//	int color = COLOR(from);
//	moveList[moveIndex].move = ((color | PAWN) << 24) | (capturedPiece << 20)
//			| (piece << 16) | (to << 8) | from;
//	moveList[moveIndex].score=105 + 1000000;
//	++moveIndex;
//	to = TO(moveList[moveIndex - 1].move);
//	from = FROM(moveList[moveIndex - 1].move);
//	ASSERT(to < 128 && to >= 0);
//	ASSERT(from < 128 && from >= 0);
//}
void generateKingMoves(int color, int pos) {
	static int rmoves[] = { -17, -16, -15, -1, 1, 15, 16, 17 };
	int i;
	int to;

	for (i = 0; i < 8; ++i) {
		to = pos + rmoves[i];
		if (!(to & 0x88)) {
			if (board.bs[to] == EMPTY || COLOR(to) != color) {
				pushMove(pos, to, color | KING, board.bs[to]);
			}
		}
	}
}

void generateKnightMoves(int color, int pos) {
	static int rmoves[] = { -33, -31, -18, -14, 14, 18, 31, 33 };
	int i;
	int to;

	for (i = 0; i < 8; ++i) {
		to = pos + rmoves[i];
		if (!(to & 0x88)) {
			if (board.bs[to] == EMPTY || COLOR(to) != color) {
				pushMove(pos, to, color | KNIGHT, board.bs[to]);
			}
		}
	}
}

void generateBishopMoves(int color, int pos) {
	static int rmoves[] = { -17, -15, 15, 17 };
	int i;
	int to;

	for (i = 0; i < 4; ++i) {
		to = pos + rmoves[i];
		while (!(to & 0x88)) {
			if (board.bs[to] == EMPTY || COLOR(to) != color) {
				pushMove(pos, to, color | BISHOP, board.bs[to]);
			}
			if (board.bs[to] != EMPTY)
				break;
			to += rmoves[i];
		}
	}
}
void generateRookMoves(int color, int pos) {
	static int rmoves[] = { -16, -1, 1, 16 };
	int i;
	int to;

	for (i = 0; i < 4; ++i) {
		to = pos + rmoves[i];
		while (!(to & 0x88)) {
			if (board.bs[to] == EMPTY || COLOR(to) != color) {
				pushMove(pos, to, color | ROOK, board.bs[to]);
			}
			if (board.bs[to] != EMPTY)
				break;
			to += rmoves[i];
		}
	}
}

void generateQueenMoves(int color, int pos) {
	static int rmoves[] = { -17, -16, -15, -1, 1, 15, 16, 17 };
	int i;
	int to;

	for (i = 0; i < 8; ++i) {
		to = pos + rmoves[i];
		while (!(to & 0x88)) {
			if (board.bs[to] == EMPTY || COLOR(to) != color) {
				pushMove(pos, to, color | QUEEN, board.bs[to]);
			}
			if (board.bs[to] != EMPTY)
				break;
			to += rmoves[i];
		}
	}
}

void generatePawnMoves(int color, int pos) {
	int rank = pos >> 4;
	int rankt;
	int dy;
	int to;

	if (color == BLACK)
		dy = -16;
	else
		dy = 16;
	to = pos + dy;
	rankt = to >> 4;
	if (board.bs[to] == EMPTY) {
		if ((color == BLACK && rankt == 0) || (color == WHITE && rankt == 7))
			pushPromotion(pos, to, color, board.bs[to]);
		else
			pushMove(pos, to, color | PAWN, board.bs[to]);
	}
	if (board.bs[to - 1] != EMPTY && COLOR(to-1) != color) {
		if ((color == BLACK && rankt == 0) || (color == WHITE && rankt == 7))
			pushPromotion(pos, to - 1, color, board.bs[to - 1]);
		else
			pushMove(pos, to - 1, color | PAWN, board.bs[to - 1]);
	}
	if (board.bs[to + 1] != EMPTY && COLOR(to+1) != color) {
		if ((color == BLACK && rankt == 0) || (color == WHITE && rankt == 7))
			pushPromotion(pos, to + 1, color, board.bs[to + 1]);
		else
			pushMove(pos, to + 1, color | PAWN, board.bs[to + 1]);
	}

	/* TODO: Move en passant check out of loop and have it checked _once_ after loop (as we do with castling)*/
	if (board.enPassant == (to - 1)) {
		ASSERT(board.bs[to - 1] == EMPTY);
		pushSpecialMove(pos, to - 1, color | PAWN, (color ^ BLACK) | PAWN,
				EMPTY, SP_ENPASSANT);
	}
	if (board.enPassant == (to + 1)) {
		ASSERT(board.bs[to + 1] == EMPTY);
		pushSpecialMove(pos, to + 1, color | PAWN, (color ^ BLACK) | PAWN,
				EMPTY, SP_ENPASSANT);
	}

	if ((color == BLACK && rank == 6) || (color == WHITE && rank == 1)) {
		to = pos + dy + dy;
		if (board.bs[to] == EMPTY && board.bs[to - dy] == EMPTY) {
			pushMove(pos, to, color | PAWN, board.bs[to]);
		}
	}
}

/*
 * dummyPerft, no make/unmake.  Just for generateMoves optimization
 */
u64 dummyPerft(u8 depth) {
	int i;
	u64 nodes = 0;

	if (depth == 0)
		return 1;

	smove m[256];
	int mcount = generateMoves(m);

	for (i = 0; i < mcount; i++) {
		// move_make(m[i]);

		// if (!isAttacked(board.sideToMove, p.KingLoc[!b.stm]))
		nodes += dummyPerft(depth - 1);

		//move_unmake(m[i]);
	}

	return nodes;
}

//Perft(6)=119060324 Nps: 12288195 (9689 ms)
u64 Perft(u8 depth) {
	int i;
	u64 nodes = 0;

	if (depth == 0)
		return 1;
	smove m[256];
	int mcount = generateMoves(m);

	for (i = 0; i < mcount; i++) {
		move_make(&m[i]);
		if (!isAttacked(board.sideToMove, kingLoc[1 - (board.sideToMove >> 3)]))
			nodes += Perft(depth - 1);
		move_unmake(&m[i]);
	}
	return nodes;
}

u64 Divide(u8 depth) {
	int i;
	u64 nodes = 0;
	u64 partial = 0;

	if (depth == 0)
		return 1;

	smove m[256];
	int mcount = generateMoves(m);

	for (i = 0; i < mcount; i++) {
		move_make(&m[i]);
		if (!isAttacked(board.sideToMove,
				kingLoc[1 - (board.sideToMove >> 3)])) {
			printMove(m[i]);
			partial += Perft(depth - 1);
			printf("  %lld\n", partial);
			nodes += partial;
			partial = 0;
		}
		move_unmake(&m[i]);
	}
	printf("Total: %lld\n", nodes);

	return nodes;
}


void move_makeNull(smove *move) {
	move->fiftycounter = board.fiftyCounter;
	move->enPassantsq = board.enPassant;
	move->castleRights = board.castleRights;
	board.historyPosKey[board.gameply++]=board.posKey;
	board.sideToMove ^= BLACK;
	board.posKey ^= side;

	/* if there was an enpassant square on, hash it out */
	if (move->enPassantsq!=ENPASSANTNULL)
		board.posKey ^= pieceKeys[EMPTY][move->enPassantsq];
	board.enPassant=ENPASSANTNULL;

	++board.ply;
}

void move_unmakeNull(smove *move) {
	board.sideToMove ^= BLACK;
	board.enPassant = move->enPassantsq;
	board.fiftyCounter = move->fiftycounter;
	board.castleRights = move->castleRights;
	board.posKey=board.historyPosKey[--board.gameply];
	--board.ply;
}

int move_make(smove *move)
{
	ASSERT(board.bs[kingLoc[0]]==WHITE_KING);
	ASSERT(board.bs[kingLoc[1]]==BLACK_KING);

	move->fiftycounter = board.fiftyCounter;
	move->enPassantsq = board.enPassant;
	move->castleRights = board.castleRights;
	board.historyPosKey[board.gameply++] = board.posKey;
	++board.ply;
	board.sideToMove ^= BLACK;
	++board.fiftyCounter;

	int from = FROM(move->move);
	int to = TO(move->move);
	int promoted = PROMOTED(move->move);
	int piece = PIECE(move->move);
	int capture = ISCAPTURE(move->move);

	ASSERT(from < 128 && from >= 0);
	ASSERT(to < 128 && to >= 0);
	ASSERT((capture&7) != KING)

	board.posKey ^= pieceKeys[piece][from] ^ pieceKeys[piece][to];
	//if (board.sideToMove)
		board.posKey ^= side;

	if ( COLORLESSPIECE(move->move) == PAWN || capture)
		board.fiftyCounter = 0;

	if ((piece & 7) == KING) {
		kingLoc[piece >> 3] = to;
	}

	board.bs[from] = EMPTY;
	if (promoted) {
		board.posKey ^= pieceKeys[piece][to] ^ pieceKeys[promoted][to];
		board.bs[to] = promoted;
	} else
		board.bs[to] = piece;

	board.posKey ^= castleKeys[board.castleRights];	//xor the old value
			switch (from) {
			case H1:
				board.castleRights &= (~CASTLE_WK) & 0xf;
				break;
			case E1:
				board.castleRights &= (~(CASTLE_WK | CASTLE_WQ)) & 0xf;
				break;
			case A1:
				board.castleRights &= (~CASTLE_WQ) & 0xf;
				break;
			case H8:
				board.castleRights &= (~CASTLE_BK) & 0xf;
				break;
			case E8:
				board.castleRights &= (~(CASTLE_BK | CASTLE_BQ)) & 0xf;
				break;
			case A8:
				board.castleRights &= (~CASTLE_BQ) & 0xf;
				break;
			}
			switch (to) {
			case H1:
				board.castleRights &= (~CASTLE_WK) & 0xf;
				break;
			case E1:
				board.castleRights &= (~(CASTLE_WK | CASTLE_WQ));
				break;
			case A1:
				board.castleRights &= (~CASTLE_WQ) & 0xf;
				break;
			case H8:
				board.castleRights &= (~CASTLE_BK) & 0xf;
				break;
			case E8:
				board.castleRights &= (~(CASTLE_BK | CASTLE_BQ)) & 0xf;
				break;
			case A8:
				board.castleRights &= (~CASTLE_BQ) & 0xf;
				break;
			}
			board.posKey ^= castleKeys[board.castleRights]; //xor the new value
	/* if the move is castling move the rook */
	if (SPECIAL(move->move) == SP_CASTLE) {


		if (to == G1) {
			board.bs[H1] = EMPTY;
			board.bs[F1] = WHITE_ROOK;
			board.posKey ^= pieceKeys[WHITE_ROOK][H1]
					^ pieceKeys[WHITE_ROOK][F1];
		} else if (to == C1) {
			board.bs[A1] = EMPTY;
			board.bs[D1] = WHITE_ROOK;
			board.posKey ^= pieceKeys[WHITE_ROOK][A1]
					^ pieceKeys[WHITE_ROOK][D1];
		} else if (to == G8) {
			board.bs[H8] = EMPTY;
			board.bs[F8] = BLACK_ROOK;
			board.posKey ^= pieceKeys[BLACK_ROOK][H8]
					^ pieceKeys[BLACK_ROOK][F8];
		} else if (to == C8) {
			board.bs[A8] = EMPTY;
			board.bs[D8] = BLACK_ROOK;
			board.posKey ^= pieceKeys[BLACK_ROOK][A8]
					^ pieceKeys[BLACK_ROOK][D8];
		}
	}

	/* if previously the board had an enpassant square, hash it out */
	if (move->enPassantsq!=ENPASSANTNULL)
		board.posKey ^= pieceKeys[EMPTY][move->enPassantsq];

	if ( COLORLESSPIECE(move->move) == PAWN && abs(to - from) == 32) {
		board.enPassant = (from + to) / 2;
		board.posKey ^= pieceKeys[EMPTY][board.enPassant];
	} else
		board.enPassant = ENPASSANTNULL;

	if (SPECIAL(move->move) == SP_ENPASSANT) {
		if (board.sideToMove == BLACK) {
			board.bs[to - 16] = 0;
			board.posKey ^= pieceKeys[BLACK_PAWN][to - 16];
		} else {
			board.bs[to + 16] = 0;
			board.posKey ^= pieceKeys[WHITE_PAWN][to + 16];
		}
		/* artificially mark the move as non-capture so we don't hash-out anything at the destination square of the move */
		capture=0;
	}
	if (capture) {
		board.posKey ^= pieceKeys[capture][to];
	}
	ASSERT(board.bs[kingLoc[0]]==WHITE_KING);
	if (board.bs[kingLoc[1]]!=BLACK_KING) {
		printBoard();
		printf("\nmove was (%d) ",move->move);printMove(*move);
		printf("\n");
	}
	ASSERT(board.bs[kingLoc[1]]==BLACK_KING);
	return 0;
}

int move_unmake(smove *move) {
	ASSERT(board.bs[kingLoc[0]]==WHITE_KING);
	ASSERT(board.bs[kingLoc[1]]==BLACK_KING);
	board.sideToMove ^= BLACK;
	board.enPassant = move->enPassantsq;
	board.fiftyCounter = move->fiftycounter;
	board.castleRights = move->castleRights;
	board.posKey=board.historyPosKey[--board.gameply];
	--board.ply;

	int from = FROM(move->move);
	int to = TO(move->move);
	int piece = PIECE(move->move);
	ASSERT(from < 128 && from >= 0);
	ASSERT(to < 128 && to >= 0);

	if ((piece & 7) == KING) {
		kingLoc[piece >> 3] = from;
	}
	board.bs[to] = EMPTY;
	board.bs[from] = piece;

	int capturedpiece = ISCAPTURE(move->move);

	if (SPECIAL(move->move) == SP_ENPASSANT) {
		if (board.sideToMove == BLACK)
			board.bs[to + 16] = WHITE_PAWN; //3279374
		else
			board.bs[to - 16] = BLACK_PAWN;
	} else if (capturedpiece)
		board.bs[to] = capturedpiece;

	if (SPECIAL(move->move) == SP_CASTLE) {
		if (to == G1) {
			board.bs[H1] = WHITE_ROOK;
			board.bs[F1] = EMPTY;
		} else if (to == C1) {
			board.bs[A1] = WHITE_ROOK;
			board.bs[D1] = EMPTY;
		} else if (to == G8) {
			board.bs[H8] = BLACK_ROOK;
			board.bs[F8] = EMPTY;
		} else if (to == C8) {
			board.bs[A8] = BLACK_ROOK;
			board.bs[D8] = EMPTY;
		}
	}
	ASSERT(board.bs[kingLoc[0]]==WHITE_KING);
	ASSERT(board.bs[kingLoc[1]]==BLACK_KING);
	return 0;
}







int generateCaptureMoves(smove *moves) {
	int i;

	moveList = moves;
	moveIndex = 0;
	for (i = 0; i < 128; ++i) {
		if (!(i & 0x88)
				&& (board.bs[i] != EMPTY
						&& (board.bs[i] & BLACK) == board.sideToMove)) {
			int color = board.bs[i] & BLACK;
			int piece = board.bs[i] & 0x07;
			switch (piece) {
			case PAWN:
				generatePawnCaptureMoves(color, i);
				break;
			case KING:
				generateKingCaptureMoves(color, i);
				//ksq = i; /* grab the king square to use for possible castling later */
				break;
			case KNIGHT:
				generateKnightCaptureMoves(color, i);
				break;
			case BISHOP:
				generateBishopCaptureMoves(color, i);
				break;
			case ROOK:
				generateRookCaptureMoves(color, i);
				break;
			case QUEEN:
				generateQueenCaptureMoves(color, i);
				break;
			default:
				fprintf(stderr,
						"Error in move.c. Piece: %d (%d at %s).  Move #%d\n",
						piece, board.bs[i], sq2algebraic(i), moveIndex);
				printBoard();
				exit(1);
			}
		}
	}
	return moveIndex;
}


void generatePawnCaptureMoves(int color, int pos) {
	int rankt;
	int dy;
	int to;

	if (color == BLACK)
		dy = -16;
	else
		dy = 16;
	to = pos + dy;
	rankt = to >> 4;
	if (board.bs[to] == EMPTY) {
		if ((color == BLACK && rankt == 0) || (color == WHITE && rankt == 7))
			pushPromotion(pos, to, color, board.bs[to]);
	}
	if (board.bs[to - 1] != EMPTY && COLOR(to-1) != color) {
		if ((color == BLACK && rankt == 0) || (color == WHITE && rankt == 7))
			pushPromotion(pos, to - 1, color, board.bs[to - 1]);
		else
			pushMove(pos, to - 1, color | PAWN, board.bs[to - 1]);
	}
	if (board.bs[to + 1] != EMPTY && COLOR(to+1) != color) {
		if ((color == BLACK && rankt == 0) || (color == WHITE && rankt == 7))
			pushPromotion(pos, to + 1, color, board.bs[to + 1]);
		else
			pushMove(pos, to + 1, color | PAWN, board.bs[to + 1]);
	}

	/* TODO: Move en passant check out of loop and have it checked _once_ after loop (as we do with castling)*/
	if (board.enPassant == (to - 1)) {
		ASSERT(board.bs[to - 1] == EMPTY);
		pushSpecialMove(pos, to - 1, color | PAWN, (color ^ BLACK) | PAWN,
				EMPTY, SP_ENPASSANT);
	}
	if (board.enPassant == (to + 1)) {
		ASSERT(board.bs[to + 1] == EMPTY);
		pushSpecialMove(pos, to + 1, color | PAWN, (color ^ BLACK) | PAWN,
				EMPTY, SP_ENPASSANT);
	}
}


void generateKingCaptureMoves(int color, int pos) {
	static int rmoves[] = { -17, -16, -15, -1, 1, 15, 16, 17 };
	int i;
	int to;

	for (i = 0; i < 8; ++i) {
		to = pos + rmoves[i];
		if (!(to & 0x88)) {
			if (board.bs[to] != EMPTY && COLOR(to) != color) {
				pushMove(pos, to, color | KING, board.bs[to]);
			}
		}
	}
}

void generateKnightCaptureMoves(int color, int pos) {
	static int rmoves[] = { -33, -31, -18, -14, 14, 18, 31, 33 };
	int i;
	int to;

	for (i = 0; i < 8; ++i) {
		to = pos + rmoves[i];
		if (!(to & 0x88)) {
			if (board.bs[to] != EMPTY && COLOR(to) != color) {
				pushMove(pos, to, color | KNIGHT, board.bs[to]);
			}
		}
	}
}



void generateBishopCaptureMoves(int color, int pos) {
	static int rmoves[] = { -17, -15, 15, 17 };
	int i;
	int to;

	for (i = 0; i < 4; ++i) {
		to = pos + rmoves[i];
		while (!(to & 0x88)) {
			if (board.bs[to] == EMPTY || COLOR(to) != color) {
				if (board.bs[to] != EMPTY) {
					pushMove(pos, to, color | BISHOP, board.bs[to]);
					break;
				}
			}
			if (board.bs[to] != EMPTY)
				break;
			to += rmoves[i];
		}
	}
}
void generateRookCaptureMoves(int color, int pos) {
	static int rmoves[] = { -16, -1, 1, 16 };
	int i;
	int to;

	for (i = 0; i < 4; ++i) {
		to = pos + rmoves[i];
		while (!(to & 0x88)) {
			if (board.bs[to] == EMPTY || COLOR(to) != color) {
				if (board.bs[to] != EMPTY) {
					pushMove(pos, to, color | ROOK, board.bs[to]);
					break;
				}
			}
			if (board.bs[to] != EMPTY)
							break;
			to += rmoves[i];
		}
	}
}

void generateQueenCaptureMoves(int color, int pos) {
	static int rmoves[] = { -17, -16, -15, -1, 1, 15, 16, 17 };
	int i;
	int to;

	for (i = 0; i < 8; ++i) {
		to = pos + rmoves[i];
		while (!(to & 0x88)) {
			if (board.bs[to] == EMPTY || COLOR(to) != color) {
				if (board.bs[to] != EMPTY) {
					pushMove(pos, to, color | QUEEN, board.bs[to]);
					break;
				}
			}
			if (board.bs[to] != EMPTY)
							break;
			to += rmoves[i];
		}
	}
}
