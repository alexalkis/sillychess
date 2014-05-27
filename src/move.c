/*
 * move.c
 *
 *  Created on: May 14, 2014
 *      Author: alex
 */

#include "amichess.h"
#include "move.h"

#define COLOR(x) (board.bs[x] & BLACK)

void generateKingMoves(int color, int pos);
void generateKnightMoves(int color, int pos);
void generateBishopMoves(int color, int pos);
void generateRookMoves(int color, int pos);
void generateQueenMoves(int color, int pos);
void generatePawnMoves(int color, int pos);
void pushSpecialMove(int from, int to, int piece, int capturedPiece,
		int promotedPiece, int special);

smove *moveList;
int moveIndex;
int ksq; /* square the king is on */

int kingLoc[2];

void printMove(smove m) {
	static char *filestr = "abcdefgh";
	static char *rankstr = "12345678";
	int from = m.move & 0xff;
	int to = (m.move >> 8) & 0xff;
	int prom = (m.move >> 24) & 0x7;
	int cap = (m.move >> 20) & 0xf;
	printf("%c%c%s%c%c", filestr[from & 7], rankstr[from >> 4], cap ? "x" : "",
			filestr[to & 7], rankstr[to >> 4]);
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

int alkis=0;

int generateMoves(smove *moves) {

	int i;

	ksq = -1;
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
				ksq = i; /* grab the king square to use for possible castling later */
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
	ASSERT(ksq != -1);

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
	if (from == 116 && to == 118)
		from = 116;
	moveList[moveIndex++].move = (capturedPiece << 20) | (piece << 16)
			| (to << 8) | from;
//	if (TO(moveList[moveIndex-1].move) >= 128) {
//		printf("from: %d to: %d piece: %d capturedpiece: %d\n", from, to, piece,
//				capturedPiece);
//	}
	to = TO(moveList[moveIndex - 1].move);
	from = FROM(moveList[moveIndex - 1].move);
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
}

void pushSpecialMove(int from, int to, int piece, int capturedPiece,
		int promotedPiece, int special) {
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);

	if (promotedPiece != EMPTY)
		moveList[moveIndex++].move = (special << 28)
				| ((board.sideToMove | promotedPiece) << 24)
				| (capturedPiece << 20) | (piece << 16) | (to << 8) | from;
	else
		moveList[moveIndex++].move = (special << 28) | (capturedPiece << 20)
				| (piece << 16) | (to << 8) | from;
	to = TO(moveList[moveIndex - 1].move);
	from = FROM(moveList[moveIndex - 1].move);
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
}

void pushPromotion(int from, int to, int color, int capturedPiece) {
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
	if (from == 116 && to == 118)
		from = 116;

	moveList[moveIndex++].move = ((color | BISHOP) << 24)
			| (capturedPiece << 20) | ((color | PAWN) << 16) | (to << 8) | from;
	to = TO(moveList[moveIndex - 1].move);
	from = FROM(moveList[moveIndex - 1].move);
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
	moveList[moveIndex++].move = ((color | KNIGHT) << 24)
			| (capturedPiece << 20) | ((color | PAWN) << 16) | (to << 8) | from;
	to = TO(moveList[moveIndex - 1].move);
	from = FROM(moveList[moveIndex - 1].move);
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
	moveList[moveIndex++].move = ((color | ROOK) << 24) | (capturedPiece << 20)
			| ((color | PAWN) << 16) | (to << 8) | from;
	to = TO(moveList[moveIndex - 1].move);
	from = FROM(moveList[moveIndex - 1].move);
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
	moveList[moveIndex++].move = ((color | QUEEN) << 24) | (capturedPiece << 20)
			| ((color | PAWN) << 16) | (to << 8) | from;
	to = TO(moveList[moveIndex - 1].move);
	from = FROM(moveList[moveIndex - 1].move);
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
}

void pushenPassantMove(int from, int to, int piece, int capturedPiece) {
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
	int color = COLOR(from);
	moveList[moveIndex++].move = ((color | PAWN) << 24) | (capturedPiece << 20)
			| (piece << 16) | (to << 8) | from;
	to = TO(moveList[moveIndex - 1].move);
	from = FROM(moveList[moveIndex - 1].move);
	ASSERT(to < 128 && to >= 0);
	ASSERT(from < 128 && from >= 0);
}
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

u64 Perft(u8 depth) {
	int i;
	u64 nodes = 0;

	if (depth == 0)
		return 1;
	smove m[256];
	int mcount = generateMoves(m);
	if (alkis) printMoveList();
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
		if (FROM(m[i].move)==E8 && TO(m[i].move)==F8)
			alkis=1;
		else
			alkis=0;
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

int move_make(smove *move) {
	move->fiftycounter = board.fiftyCounter;
	move->enPassantsq = board.enPassant;
	move->castleRights = board.castleRights;
	board.sideToMove ^= BLACK;
	++board.fiftyCounter;
	if ( COLORLESSPIECE(move->move) == PAWN || ISCAPTURE(move->move))
		board.fiftyCounter = 0;
	int from = FROM(move->move);
	int to = TO(move->move);
	int promoted = PROMOTED(move->move);
	int piece = PIECE(move->move);
	ASSERT(from < 128 && from >= 0);
	ASSERT(to < 128 && to >= 0);

	if ((piece & 7) == KING) {
		kingLoc[piece >> 3] = to;
	}

	board.bs[from] = EMPTY;
	if (promoted)
		board.bs[to] = promoted;
	else
		board.bs[to] = piece;

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

	/* if the move is castling, move the rook */
	if (SPECIAL(move->move) == SP_CASTLE) {
		if (to == G1) {
			board.bs[H1] = EMPTY;
			board.bs[F1] = WHITE_ROOK;
		} else if (to == C1) {
			board.bs[A1] = EMPTY;
			board.bs[D1] = WHITE_ROOK;
		} else if (to == G8) {
			board.bs[H8] = EMPTY;
			board.bs[F8] = BLACK_ROOK;
		} else if (to == C8) {
			board.bs[A8] = EMPTY;
			board.bs[D8] = BLACK_ROOK;
		}
	}

	if ( COLORLESSPIECE(move->move) == PAWN && abs(to - from) == 32)
		board.enPassant = (from + to) / 2;
	else
		board.enPassant = ENPASSANTNULL;
	if (SPECIAL(move->move) == SP_ENPASSANT) {
		if (board.sideToMove == BLACK)
			board.bs[to - 16] = 0;
		else
			board.bs[to + 16] = 0;
	}
	if (board.bs[G8] == 8)
		board.bs[G8] = 8;
	return 0;
}

int move_unmake(smove *move) {

	board.sideToMove ^= BLACK;
	board.enPassant = move->enPassantsq;
	board.fiftyCounter = move->fiftycounter;
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

	board.castleRights = move->castleRights;
	return 0;
}
