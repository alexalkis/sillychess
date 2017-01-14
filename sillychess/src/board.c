/*
 * board.c
 *
 *  Created on: May 27, 2014
 *      Author: alex
 */


#include "sillychess.h"
#include <ctype.h>

#define NORTH	16
#define SOUTH	-16
#define EAST	1
#define WEST	-1

#define NE	17
#define NW  15
#define SE  -15
#define SW  -17

#define IS_SQ(x)	(!((x)&0x88))

/* lifted from CPW engine http://chessprogramming.wikispaces.com */
int diagAttack(int byColor, int sq, int vect) {
	int nextSq = sq + vect;
	int color;
	int piece;

	while (IS_SQ(nextSq)) {
		if (board.bs[nextSq] != EMPTY) {
			piece = board.bs[nextSq];
			color = piece & BLACK;
			piece &= 7;
			if ((color == byColor) && (piece == BISHOP || piece == QUEEN))
				return 1;
			return 0;
		}
		nextSq = nextSq + vect;
	}

	return 0;
}

/* lifted from CPW engine http://chessprogramming.wikispaces.com */
int straightAttack(int byColor, int sq, int vect) {
	int nextSq = sq + vect;
	int color;
	int piece;
	while (IS_SQ(nextSq)) {

		if (board.bs[nextSq] != EMPTY) {
			piece = board.bs[nextSq];
			color = piece & BLACK;
			piece &= 7;
			if ((color == byColor) && (piece == ROOK || piece == QUEEN))
				return 1;
			return 0;
		}
		nextSq = nextSq + vect;
	}
	return 0;
}

/* mostly lifted from CPW engine http://chessprogramming.wikispaces.com */
int isAttacked(int byColor, int sq) {
	static int knight[] = { -33, -31, -18, -14, 14, 18, 31, 33 };
	static int king[] = { -17, -16, -15, 1, -1, 15, 16, 17 };
	int i;

	if (byColor == WHITE) {
		/* pawns first */
		if (IS_SQ(sq+SE) && board.bs[sq + SE] == WHITE_PAWN)
			return 1;
		if (IS_SQ(sq+SW) && board.bs[sq + SW] == WHITE_PAWN)
			return 1;
		/* knights and king */
		for (i = 0; i < 8; ++i) {
			if (IS_SQ(sq+knight[i]) && board.bs[sq + knight[i]] == WHITE_KNIGHT)
				return 1;
			if (IS_SQ(sq+king[i]) && board.bs[sq + king[i]] == WHITE_KING)
				return 1;
		}

	} else {
		if (IS_SQ(sq+NE) && board.bs[sq + NE] == BLACK_PAWN)
			return 1;
		if (IS_SQ(sq+NW) && board.bs[sq + NW] == BLACK_PAWN)
			return 1;
		for (i = 0; i < 8; ++i) {
			if (IS_SQ(sq+knight[i]) && board.bs[sq + knight[i]] == BLACK_KNIGHT)
				return 1;
			if (IS_SQ(sq+king[i]) && board.bs[sq + king[i]] == BLACK_KING)
				return 1;
		}
	}
	/* straight line sliders */
	if (straightAttack(byColor, sq, NORTH) || straightAttack(byColor, sq, SOUTH)
			|| straightAttack(byColor, sq, EAST)
			|| straightAttack(byColor, sq, WEST))
		return 1;

	/* diagonal sliders */
	if (diagAttack(byColor, sq, NE) || diagAttack(byColor, sq, SE)
			|| diagAttack(byColor, sq, NW) || diagAttack(byColor, sq, SW))
		return 1;

	return 0;
}

char *sq2algebraic(u8 sq) {
	static char buf[3];
	buf[0] = COL(sq) + 'a';
	buf[1] = ROW(sq) + '1';
	buf[2] = '\0';
	return buf;
}

void printpsq(int psq[2][64], int n) {
	int i;
	for (i = 0; i < 64; ++i) {
		if ((i % 8) == 0)
			printf("\n");
		printf("%4d", psq[n][i]);

	}
	printf("\n");
}
void initBoard(void) {
//	u8 temp[128] = { WHITE_ROOK, WHITE_KNIGHT, WHITE_BISHOP, WHITE_QUEEN,
//			WHITE_KING, WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK, 0, 0, 0, 0, 0,
//			0, 0, 0, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN,
//			WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, 0, 0, 0, 0, 0, 0, 0, 0, EMPTY,
//			EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//			EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//			EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//			EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//			EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//			EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//			EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//			BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN,
//			BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, 0, 0, 0, 0, 0, 0, 0, 0,
//			BLACK_ROOK, BLACK_KNIGHT, BLACK_BISHOP, BLACK_QUEEN, BLACK_KING,
//			BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK, 0, 0, 0, 0, 0, 0, 0, 0 };
//	memcpy(board.bs, temp, 128 * sizeof(u8));
//	board.sideToMove = WHITE;
//	board.enPassant = ENPASSANTNULL;
//	board.castleRights = 0xf;
//	board.fiftyCounter = 0;
//	board.ply = 0;
//	board.posKey=generatePosKey();
	board.gameply=0;
	fen2board(START_FEN);
	kingLoc[0] = E1;
	kingLoc[1] = E8;

	int i;

	for (i = 0; i < 64; ++i) {
		psq_pawns[0][i] = raw_psq_pawns[(7 - i / 8) * 8 + (i & 7)];
		//printf("%d=%d,",(7-i/8)*8+(i&7),t);
		psq_pawns[1][i] = -raw_psq_pawns[i];

		psq_knights[0][i] = raw_psq_knights[(7 - i / 8) * 8 + (i & 7)];
		psq_knights[1][i] = -raw_psq_knights[(7 - i / 8) * 8 + (i & 7)];

		psq_bishops[0][i] = raw_psq_bishops[(7 - i / 8) * 8 + (i & 7)];
		psq_bishops[1][i] = -raw_psq_bishops[i];

		psq_rooks[0][i] = raw_psq_rooks[(7 - i / 8) * 8 + (i & 7)];
		psq_rooks[1][i] = -raw_psq_rooks[i];

		psq_queens[0][i] = raw_psq_queens[(7 - i / 8) * 8 + (i & 7)];
		psq_queens[1][i] = -raw_psq_queens[i];

	}
	//printpsq(psq_rooks,1);
	//printpsq(psq_pawns,0);
	//printf("\n\n");
	//printpsq(psq_pawns,1);
	//exit(1);

}

char *board2fen(void) {
	static char buf[128];
	static char lookup[] = " PKN BRQ pkn brq";
	int rank;
	int file;
	int pos = 0;
	int emptyCounter = 0;

	for (rank = 7; rank >= 0; --rank) {
		for (file = 0; file < 8; ++file) {
			if (board.bs[(rank << 4) + file] != EMPTY) {
				if (emptyCounter) {
					buf[pos++] = (emptyCounter + '0');
					emptyCounter = 0;
				}
				buf[pos++] = lookup[board.bs[(rank << 4) + file]];
			} else
				++emptyCounter;
		}
		if (emptyCounter) {
			buf[pos++] = (emptyCounter + '0');
			emptyCounter = 0;
		}
		if (rank)
			buf[pos++] = '/';
	}
	buf[pos++] = ' ';
	buf[pos++] = board.sideToMove == WHITE ? 'w' : 'b';
	buf[pos++] = ' ';
	if (board.castleRights == 0)
		buf[pos++] = '-';
	else {
		if (board.castleRights & (1 << 3))
			buf[pos++] = 'K';
		if (board.castleRights & (1 << 2))
			buf[pos++] = 'Q';
		if (board.castleRights & (1 << 1))
			buf[pos++] = 'k';
		if (board.castleRights & (1 << 0))
			buf[pos++] = 'q';
	}
	buf[pos++] = ' ';
	if (board.enPassant == ENPASSANTNULL)
		buf[pos++] = '-';
	else {
		strcpy(&buf[pos++], sq2algebraic(board.enPassant));
		pos++;
	}
	buf[pos]='\0';
	return buf;
}

void printBoard(void) {
	int i;
	short int row, col;
	short int color;
	short int piece;
	u8 c=' ';

	printf("\n   a b c d e f g h\n\n");
	for (row = 7; row >= 0; --row) {
		for (col = 0; col < 8; ++col) {
			i = 16 * row + col;
			if (!(i & 0x88)) {
				//printf("(%d,%d) ",col,row);

				color = board.bs[i] & 0x08;
				piece = board.bs[i] & 0x07;
				switch (piece) {
				case EMPTY:
					c = '.';
					break;
				case PAWN:
					c = 'P';
					break;
				case KNIGHT:
					c = 'N';
					break;
				case BISHOP:
					c = 'B';
					break;
				case ROOK:
					c = 'R';
					break;
				case KING:
					c = 'K';
					break;
				case QUEEN:
					c = 'Q';
					break;

				}
				if (col == 0)
					printf("%d ", row + 1);
				if (color == BLACK)
					printf("*");
				else
					printf(" ");
				printf("%c", (color == BLACK) ? tolower(c) : c);
			}
		}
		printf("\n");
	}



	printf("\n   a b c d e f g h\t\tPosition key: %"INT64_FORMAT"X",board.posKey);
	printf("\nSide to move: %s e.p.: %s\n",
			board.sideToMove == WHITE ? "white" : "black",
			board.enPassant == ENPASSANTNULL ?
					" -" : sq2algebraic(board.enPassant));
	printf("Fen: %s\n", board2fen());
#ifndef NDEBUG
	printf("Last move: %s (move number %d in the game)\n", moveToUCI(board.gameply-1), board.gameply-1);
#endif
}

void fen2board(char *str) {
	int i;
	int rank = 7;
	int file = 0;
	char c;

	for (i = 0; i < 128; ++i)
		board.bs[i] = EMPTY;

	board.bigCount[0]=board.bigCount[1]=0;
	board.matValues[0]=board.matValues[1]=0;
	board.pawnCount[0]=board.pawnCount[1]=0;
	while ((c = *str++)) {
		switch (c) {
		case 'r':
			board.bs[(rank << 4) + file] = BLACK_ROOK;
			++board.bigCount[1];
			board.matValues[1]+=matValues[BLACK_ROOK]+psq_rooks[1][(rank<<3)+file];
			++file;
			break;
		case 'n':
			board.bs[(rank << 4) + file] = BLACK_KNIGHT;
			++board.bigCount[1];
			board.matValues[1]+=matValues[BLACK_KNIGHT]+psq_knights[1][(rank<<3)+file];
			++file;
			break;
		case 'b':
			board.bs[(rank << 4) + file] = BLACK_BISHOP;
			++board.bigCount[1];
			board.matValues[1]+=matValues[BLACK_BISHOP]+psq_bishops[1][(rank<<3)+file];
			++file;
			break;
		case 'q':
			board.bs[(rank << 4) + file] = BLACK_QUEEN;
			++board.bigCount[1];
			board.matValues[1]+=matValues[BLACK_QUEEN]+psq_queens[1][(rank<<3)+file];
			++file;
			break;
		case 'k':
			board.bs[(rank << 4) + file] = BLACK_KING;
			kingLoc[1] = (rank << 4) + file;
			++file;
			break;
		case 'p':
			board.bs[(rank << 4) + file] = BLACK_PAWN;
			++board.pawnCount[1];
			board.matValues[1]+=matValues[BLACK_PAWN]+psq_pawns[1][(rank<<3)+file];
			++file;
			break;

		case 'R':
			board.bs[(rank << 4) + file] = WHITE_ROOK;
			++board.bigCount[0];
			board.matValues[0]+=matValues[WHITE_ROOK]+psq_rooks[0][(rank<<3)+file];
			++file;
			break;
		case 'N':
			board.bs[(rank << 4) + file] = WHITE_KNIGHT;
			++board.bigCount[0];
			board.matValues[0]+=matValues[WHITE_KNIGHT]+psq_knights[0][(rank<<3)+file];
			++file;
			break;
		case 'B':
			board.bs[(rank << 4) + file] = WHITE_BISHOP;
			++board.bigCount[0];
			board.matValues[0]+=matValues[WHITE_BISHOP]+psq_bishops[0][(rank<<3)+file];
			++file;
			break;
		case 'Q':
			board.bs[(rank << 4) + file] = WHITE_QUEEN;
			++board.bigCount[0];
			board.matValues[0]+=matValues[WHITE_QUEEN]+psq_queens[0][(rank<<3)+file];
			++file;
			break;
		case 'K':
			board.bs[(rank << 4) + file] = WHITE_KING;
			kingLoc[0] = (rank << 4) + file;
			++file;
			break;
		case 'P':
			board.bs[(rank << 4) + file] = WHITE_PAWN;
			++board.pawnCount[0];
			board.matValues[0]+=matValues[WHITE_PAWN]+psq_pawns[0][(rank<<3)+file];
			++file;
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			file += c - '0';

		}
		if (file == 8) {
			file = 0;
			--rank;
			if (rank < 0) {
				//printf("Got it...\n");
				break;
			}
		}
	}
	i = 0;
	while (str[i] == ' ' || str[i] == ';')
		++i;
	if (str[i] == 'b')
		board.sideToMove = BLACK;

	if (str[i] == 'w')
		board.sideToMove = WHITE;

	if (str[i])
		++i;
	if (!str[i]) {
		board.castleRights=0;
		board.enPassant=ENPASSANTNULL;
		return;
	}
	while (str[i] == ' ' || str[i] == ';')
		++i;

	int whiteCastle = 0;
	int blackCastle = 0;
	if (str[i] != '-') {
		do {
			if (str[i] == 'K')
				whiteCastle += 2;
			if (str[i] == 'Q')
				whiteCastle += 1;
			if (str[i] == 'k')
				blackCastle += 2;
			if (str[i] == 'q')
				blackCastle += 1;
		} while (str[++i] != ' ');
	} else
		++i;
	board.castleRights = whiteCastle << 2 | blackCastle;

	//printf("wc: %d bc: %d\n",whiteCastle,blackCastle);

	u8 epsq;

	//printf("------------> %s\n",&str[i]);
	while (str[i] == ' ' || str[i] == ';')
		++i;
	board.enPassant = ENPASSANTNULL;
	if (str[i] == '-') {
		epsq = EMPTY;
	} else {
		// translate a square coordinate (as string) to int (eg 'e3' to 20):
		//printf("Here is is: %s\n",str[i]);
		ASSERT(str[i]>='a' && str[i]<='h');
		epsq = (str[i] - 'a') + 16 * (str[i + 1] - '1');
		board.enPassant = epsq;
		++i;
	}
	board.posKey=generatePosKey();
	/// TODO: fiftycounter and plycounter
	board.fiftyCounter=board.ply=0;
//	printf("*******************\n");
//	Evaluate();
//	printf("*******************\n");
}
