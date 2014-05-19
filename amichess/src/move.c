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
		printf("%d) %c%c%s%c%c", i + 1, filestr[from & 7], rankstr[from >> 4],cap ? "x":"",
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
		printf("\n");
	}
}

int generateMoves(smove *moves) {

	int i;
	int ksq = -1;

	moveList=moves;
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
				fprintf(stderr, "Error in move.c. Piece: %d\n", piece);
				exit(1);
			}
		}
	}
	ASSERT(ksq != -1);

#define CASTLE_WK	(1<<3)
#define CASTLE_WQ	(1<<2)
#define CASTLE_BK	(1<<1)
#define CASTLE_BQ	(1<<0)

	if (board.sideToMove == WHITE) {
		if (board.castleRights & CASTLE_WK) {
			if ((board.bs[F1] == EMPTY) && (board.bs[G1] == EMPTY)
					&& (!isAttacked(!board.sideToMove, E1))
					&& (!isAttacked(!board.sideToMove, F1))
					&& (!isAttacked(!board.sideToMove, G1)))

				pushSpecialMove(E1, G1, WHITE_KING, EMPTY, EMPTY, SP_CASTLE);
		}
		if (board.castleRights & CASTLE_WQ) {
			if ((board.bs[B1] == EMPTY) && (board.bs[C1] == EMPTY)
					&& (board.bs[D1] == EMPTY)
					&& (!isAttacked(!board.sideToMove, E1))
					&& (!isAttacked(!board.sideToMove, D1))
					&& (!isAttacked(!board.sideToMove, C1)))

				pushSpecialMove(E1, C1, WHITE_KING, EMPTY, EMPTY, SP_CASTLE);
		}
	} else {
		if (board.castleRights & CASTLE_BK) {
			if ((board.bs[F8] == EMPTY) && (board.bs[G8] == EMPTY)
					&& (!isAttacked(!board.sideToMove, E8))
					&& (!isAttacked(!board.sideToMove, F8))
					&& (!isAttacked(!board.sideToMove, G8)))

				pushSpecialMove(E8, G8, BLACK_KING, EMPTY, EMPTY, SP_CASTLE);
		}
		if (board.castleRights & CASTLE_BQ) {
			if ((board.bs[B8] == EMPTY) && (board.bs[C8] == EMPTY)
					&& (board.bs[D8] == EMPTY)
					&& (!isAttacked(!board.sideToMove, E8))
					&& (!isAttacked(!board.sideToMove, D8))
					&& (!isAttacked(!board.sideToMove, C8)))

				pushSpecialMove(E8, C8, BLACK_KING, EMPTY, EMPTY, SP_CASTLE);
		}
	}

	return moveIndex;
}

void pushMove(int from, int to, int piece, int capturedPiece) {
	moveList[moveIndex++].move = (capturedPiece << 20) | (piece << 16) | (to << 8)
			| from;
}

void pushSpecialMove(int from, int to, int piece, int capturedPiece,
		int promotedPiece, int special) {

	moveList[moveIndex++].move = (special << 28)
			| ((board.sideToMove | promotedPiece) << 24) | (capturedPiece << 20)
			| (piece << 16) | (to << 8) | from;
}

void pushPromotion(int from, int to, int color, int capturedPiece) {
	moveList[moveIndex++].move = ((color | BISHOP) << 24) | (capturedPiece << 20)
			| ((color | PAWN) << 16) | (to << 8) | from;
	moveList[moveIndex++].move = ((color | KNIGHT) << 24) | (capturedPiece << 20)
			| ((color | PAWN) << 16) | (to << 8) | from;
	moveList[moveIndex++].move = ((color | ROOK) << 24) | (capturedPiece << 20)
			| ((color | PAWN) << 16) | (to << 8) | from;
	moveList[moveIndex++].move = ((color | QUEEN) << 24) | (capturedPiece << 20)
			| ((color | PAWN) << 16) | (to << 8) | from;
}

void pushenPassantMove(int from, int to, int piece, int capturedPiece) {
	int color = COLOR(from);
	moveList[moveIndex++].move = ((color | PAWN) << 24) | (capturedPiece << 20)
			| (piece << 16) | (to << 8) | from;
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
	//printf("Castle rights: %d\n",board.castleRights);
//	if (color == WHITE) {
//		if (board.castleRights & (1 << 3)) //king side to G1
//			pushSpecialMove(pos, G1, color | KING, board.bs[to],EMPTY,SP_CASTLE);
//		if (board.castleRights & (1 << 2)) //queen side to C1
//			pushSpecialMove(pos, C1, color | KING, board.bs[to],EMPTY,SP_CASTLE);
//	} else {
//		if (board.castleRights & (1 << 1)) //king side to G8
//			pushSpecialMove(pos, G8, color | KING, board.bs[to],EMPTY,SP_CASTLE);
//		if (board.castleRights & (1 << 0)) //queen side to C8
//			pushSpecialMove(pos, C8, color | KING, board.bs[to],EMPTY,SP_CASTLE);
//	}
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
			pushPromotion(pos, to, color, board.bs[to]);
		else
			pushMove(pos, to - 1, color | PAWN, board.bs[to - 1]);
	}
	if (board.bs[to + 1] != EMPTY && COLOR(to+1) != color) {
		if ((color == BLACK && rankt == 0) || (color == WHITE && rankt == 7))
			pushPromotion(pos, to, color, board.bs[to]);
		else
			pushMove(pos, to + 1, color | PAWN, board.bs[to + 1]);
	}

	if (board.bs[to - 1] == EMPTY && board.enPassant == (to - 1)) {
		pushSpecialMove(pos, to - 1, color | PAWN, (color ^ BLACK) | PAWN,
				EMPTY, SP_ENPASSANT);
	}
	if (board.bs[to + 1] == EMPTY && board.enPassant == (to + 1)) {
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


u64 perft(u8 depth) {
	int i;
    u64 nodes = 0;

    if (depth == 0) return 1;

    smove m[256];
    int mcount = generateMoves(m);

    for (i = 0; i < mcount; i++) {
       // move_make(m[i]);

       // if (!isAttacked(board.sideToMove, p.KingLoc[!b.stm]))
            nodes += perft(depth - 1);

        //move_unmake(m[i]);
    }

    return nodes;
}

