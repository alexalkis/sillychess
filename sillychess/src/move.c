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

void pushSpecialMove(int from, int to, int piece, int capturedPiece, int special);

smove *moveList;
int moveIndex;
//int ksq; /* square the king is on */

int kingLoc[2];

const int VictimScore[16] = { 	0, 100,600, 200,0,300, 400, 500,
                                0, 100,600, 200,0,300, 400, 500};
static int MvvLvaScores[16][16];

int moveExists(unsigned int move)
{
  int i;
  int legalMoves = 0;
  smove m[256];
  int mcount = generateMoves(m);

  //printBoard();
  for ( i = 0; i < mcount; ++i) {

    move_make(&m[i]);
    if (!isAttacked(board.sideToMove, kingLoc[1 - (board.sideToMove >> 3)])) {
      //printf("Legal: ");printMove(m[i]);printf(" ");
      ++legalMoves;
      if (m[i].move==move) {
        move_unmake(&m[i]);
        return TRUE;
      }

    }
    move_unmake(&m[i]);
  }
  return FALSE;
}

int generateLegalMoves(smove *m)
{
  int i;
  int legalmoves=0;

  int mcount=generateMoves(m);
  for ( i = 0; i < mcount; ++i) {
    move_make(&m[i]);
    if (isAttacked(board.sideToMove, kingLoc[1 - (board.sideToMove >> 3)])) {
      move_unmake(&m[i]);
      m[i]=m[mcount-1];
      --mcount;
      --i;
      continue;
    }
    ++legalmoves;
    move_unmake(&m[i]);
  }
  return legalmoves;
}

void printLine(LINE *line,S_SEARCHINFO *info)
{
  int i;
  smove m[MAXDEPTH];
  for (i = 0; i < line->cmove; ++i) {
    m[i].move = line->argmove[i];
    if (!moveExists(m[i].move)) {
      printf("Move (");printMove(m[i]);printf(") not legal, but in PV-line. It's the %d out of %d moves.\n",i+1,line->cmove);
      int j;
      for(j=i+1; j<line->cmove; ++j) {
        printMove(m[j]);
        if (!moveExists(m[i].move))
          printf(" - Illegal\n");
        else
          printf(" - Legal\n");
      }

      break;
    }
    smove lm[256];
    int mlegalcount=generateLegalMoves(lm);
    move_make(&m[i]);
    if (isAttacked(board.sideToMove,kingLoc[((board.sideToMove ^ BLACK) >> 3)])) {
#ifndef NDEBUG
      printf(" PV-Line is messed up\n");
#endif
      ++i;
      break;
    }
    if (info->GAME_MODE!=GAMEMODE_UCI)
      printf("%s",move_to_san(m[i],mlegalcount,lm));
    else {
      printMove(m[i]);
      if (isAttacked(board.sideToMove ^ BLACK,kingLoc[1 - ((board.sideToMove ^ BLACK) >> 3)])) {
        if (numOfLegalMoves()) {
          printf("+");
        } else {
          printf("#");
          ++i;
          break;
        }
      }
    }
    printf(" ");
  }
  for (--i; i >= 0; --i) {
    move_unmake(&m[i]);
  }
}
char * qprintMove(int move) {
  static char *filestr = "abcdefgh";
  static char *rankstr = "12345678";
  static char buffer[8];
  int from = move & 0xff;
  int to = (move >> 8) & 0xff;
  int prom = (move >> 24) & 0x7;

  if (move==NOMOVE) {
    strcpy(buffer,"NOMOVE");
    return buffer;
  }
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
  if (prom)
    buffer[5]='\0';
  else
    buffer[4]='\0';
  return buffer;
}

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

char *move_to_san(smove m,int mcount,smove *moves)
{
  static char *filestr = "abcdefgh";
  static char *rankstr = "12345678";
  static char *piecestr= " PKN BRQ";
  static char buffer[12];
  int i;
  unsigned int from = m.move & 0xff;
  unsigned int to = (m.move >> 8) & 0xff;
  int prom = (m.move >> 24) & 0x7;
  int piece=COLORLESSPIECE(m.move);

  if (SPECIAL(m.move)==SP_CASTLE) {
    if (to>from)
      strcpy(buffer,"O-O");
    else
      strcpy(buffer,"O-O-O");
    return buffer;
  }
  memset(buffer,0,sizeof(buffer));
  int rowsmatch,colsmatch;
  rowsmatch=colsmatch=0;
  int cursor=0;
  //printf("Piece=%d\n",piece);

  if (piece!=PAWN) {
    buffer[cursor++]=piecestr[piece];
    int disambiguity=0;
    for(i=0; i<mcount; ++i) {
      if (moves[i].move==m.move) continue;
      if (TO(moves[i].move)==to && PIECE(moves[i].move)==PIECE(m.move)) {
        disambiguity=1;
        unsigned int from2=FROM(moves[i].move);
        if (ROW(from)==ROW(from2))
          rowsmatch=1;
        if (COL(from)==COL(from2))
          colsmatch=1;
      }
    }
    if (rowsmatch || disambiguity)
      buffer[cursor++]=filestr[COL(from)];
    if (colsmatch)
      buffer[cursor++]=rankstr[ROW(from)];
  } else if (ISCAPTURE(m.move)) {
    cursor=0;
    buffer[cursor++]=filestr[COL(from)];
  }

  if (ISCAPTURE(m.move)) {
    buffer[cursor++]='x';
  }
  buffer[cursor++]=filestr[COL(to)];
  buffer[cursor++]=rankstr[ROW(to)];
  if (prom) {
    buffer[cursor++]='=';
    buffer[cursor++]=piecestr[prom];
  }
  /* the following ASSUMES we have move_make the move before calling here */
  if (isAttacked(board.sideToMove ^ BLACK,kingLoc[1 - ((board.sideToMove ^ BLACK) >> 3)])) {
    if (numOfLegalMoves()) {
      buffer[cursor++]='+';
    } else {
      buffer[cursor++]='#';
    }
  }
  buffer[cursor]='\0';
  return buffer;
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

        pushSpecialMove(E1, G1, WHITE_KING, EMPTY, SP_CASTLE);
    }
    if (board.castleRights & CASTLE_WQ) {
      if ((board.bs[B1] == EMPTY) && (board.bs[C1] == EMPTY)
          && (board.bs[D1] == EMPTY)
          && (!isAttacked(board.sideToMove^BLACK, E1))
          && (!isAttacked(board.sideToMove^BLACK, D1))
          && (!isAttacked(board.sideToMove^BLACK, C1)))

        pushSpecialMove(E1, C1, WHITE_KING, EMPTY, SP_CASTLE);
    }

    if (board.enPassant!=ENPASSANTNULL && ROW(board.enPassant)==5) {
      if (board.bs[board.enPassant-16-1]==WHITE_PAWN) {
        pushSpecialMove(board.enPassant-16-1,board.enPassant,WHITE_PAWN,BLACK_PAWN,SP_ENPASSANT);
      }
      if (board.bs[board.enPassant-16+1]==WHITE_PAWN) {
        pushSpecialMove(board.enPassant-16+1,board.enPassant,WHITE_PAWN,BLACK_PAWN,SP_ENPASSANT);
      }
    }
  } else {
    if (board.castleRights & CASTLE_BK) {
      if ((board.bs[F8] == EMPTY) && (board.bs[G8] == EMPTY)
          && (!isAttacked(board.sideToMove^BLACK, E8))
          && (!isAttacked(board.sideToMove^BLACK, F8))
          && (!isAttacked(board.sideToMove^BLACK, G8)))

        pushSpecialMove(E8, G8, BLACK_KING, EMPTY,  SP_CASTLE);
    }
    if (board.castleRights & CASTLE_BQ) {
      if ((board.bs[B8] == EMPTY) && (board.bs[C8] == EMPTY)
          && (board.bs[D8] == EMPTY)
          && (!isAttacked(board.sideToMove^BLACK, E8))
          && (!isAttacked(board.sideToMove^BLACK, D8))
          && (!isAttacked(board.sideToMove^BLACK, C8)))

        pushSpecialMove(E8, C8, BLACK_KING, EMPTY,  SP_CASTLE);
    }


    if (board.enPassant!=ENPASSANTNULL && ROW(board.enPassant)==2) {
      if (board.bs[board.enPassant+16-1]==BLACK_PAWN) {
        pushSpecialMove(board.enPassant+16-1,board.enPassant,BLACK_PAWN,WHITE_PAWN,SP_ENPASSANT);
      }
      if (board.bs[board.enPassant+16+1]==BLACK_PAWN) {
        pushSpecialMove(board.enPassant+16+1,board.enPassant,BLACK_PAWN,WHITE_PAWN,SP_ENPASSANT);
      }
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
    if (board.searchKillers[0][board.gameply] == moveList[moveIndex].move) {
      moveList[moveIndex].score = FIRST_KILLER_SCORE;
      //printf("First killer\n");
    } else if (board.searchKillers[1][board.gameply]== moveList[moveIndex].move) {
      moveList[moveIndex].score = SECOND_KILLER_SCORE;
      //printf("Second killer\n");
    } else {
      moveList[moveIndex].score = board.searchHistory[piece][to];
    }
  }
  ++moveIndex;
#ifndef NDEBUG
  to = TO(moveList[moveIndex - 1].move);
  from = FROM(moveList[moveIndex - 1].move);
  ASSERT(to < 128 && to >= 0);
  ASSERT(from < 128 && from >= 0);
#endif
}

void pushSpecialMove(int from, int to, int piece, int capturedPiece, int special) {
  ASSERT(to < 128 && to >= 0);
  ASSERT(from < 128 && from >= 0);

  moveList[moveIndex].move = (special << 28) | (capturedPiece << 20)	| (piece << 16) | (to << 8) | from;
  if (special==SP_ENPASSANT)
    moveList[moveIndex].score=105+CAPTURE_SCORE;
  //printf("score=%d enpassant sq: %s\n",moveList[moveIndex].score,sq2algebraic(board.enPassant));
  ++moveIndex;
#ifndef NDEBUG
  to = TO(moveList[moveIndex - 1].move);
  from = FROM(moveList[moveIndex - 1].move);
  ASSERT(to < 128 && to >= 0);
  ASSERT(from < 128 && from >= 0);
#endif
}

void pushPromotion(int from, int to, int color, int capturedPiece) {
  ASSERT(to < 128 && to >= 0);
  ASSERT(from < 128 && from >= 0);

  moveList[moveIndex].move = ((color | BISHOP) << 24)
      | (capturedPiece << 20) | ((color | PAWN) << 16) | (to << 8) | from;
  moveList[moveIndex++].score = MvvLvaScores[capturedPiece][PAWN]+MvvLvaScores[BISHOP][PAWN]+CAPTURE_SCORE;

  moveList[moveIndex].move = ((color | KNIGHT) << 24)
      | (capturedPiece << 20) | ((color | PAWN) << 16) | (to << 8) | from;
  moveList[moveIndex++].score = MvvLvaScores[capturedPiece][PAWN]+MvvLvaScores[KNIGHT][PAWN]+CAPTURE_SCORE;

  moveList[moveIndex].move = ((color | ROOK) << 24) | (capturedPiece << 20)
      | ((color | PAWN) << 16) | (to << 8) | from;
  moveList[moveIndex++].score = MvvLvaScores[capturedPiece][PAWN]+MvvLvaScores[ROOK][PAWN]+CAPTURE_SCORE;

  moveList[moveIndex].move = ((color | QUEEN) << 24) | (capturedPiece << 20)
      | ((color | PAWN) << 16) | (to << 8) | from;
  moveList[moveIndex++].score = MvvLvaScores[capturedPiece][PAWN]+MvvLvaScores[QUEEN][PAWN]+CAPTURE_SCORE;
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

/*
 * Here is a simple idea using posix threads in Linux:

main program:
int global_sum = 0;
pthread_create(process1, sumarray, 0);
pthread_create(process2, sumarray, 1);
pthread_join(process1);
pthread_join(process2);

done...

int sumarray(my_id) {
int i, start, end, sum = 0;
start = size_of_array * my_id / 2;
end = start + size_of_array / 2;
for (i = start; i < end; i++)
sum += array[i];
lock(sum_lock)
global_sum += sum;
unlock(sum_lock);
return 0;
}
 */

u64 Divide(u8 depth) {
  int i;
  u64 nodes = 0;
  u64 partial = 0;

  if (depth == 0)
    return 1;

  smove m[256];
  int mcount = generateMoves(m);

  //#pragma omp parallel for private(i)
  for (i = 0; i < mcount; i++) {
    move_make(&m[i]);
    if (!isAttacked(board.sideToMove,
                    kingLoc[1 - (board.sideToMove >> 3)])) {
      printMove(m[i]);
      partial += Perft(depth - 1);
      printf("  %"INT64_FORMAT"d\n", partial);
      nodes += partial;
      partial = 0;
    }
    move_unmake(&m[i]);
  }
  printf("Total: %"INT64_FORMAT"d\n", nodes);

  return nodes;
}


void move_makeNull(smove *move) {
  move->fiftycounter = board.fiftyCounter;
  move->enPassantsq = board.enPassant;
  move->castleRights = board.castleRights;
  board.historyPosKey[board.gameply++] = board.posKey;
  board.sideToMove ^= BLACK;
  board.posKey ^= side;

  /* if there was an enpassant square on, hash it out */
  if (move->enPassantsq != ENPASSANTNULL)
    board.posKey ^= pieceKeys[EMPTY][move->enPassantsq];
  board.enPassant = ENPASSANTNULL;

  ++board.ply;
}

void move_unmakeNull(smove *move) {
  board.sideToMove ^= BLACK;
  board.enPassant = move->enPassantsq;
  board.fiftyCounter = move->fiftycounter;
  board.castleRights = move->castleRights;
  board.posKey = board.historyPosKey[--board.gameply];
  --board.ply;
}

int valuePiecePSQ(int piece,int from){
	int from8x8 = (from + (from & 7)) >> 1;

	switch(piece) {
	case WHITE_PAWN:
		return psq_pawns[WHITE][from8x8];
	case WHITE_BISHOP:
		return psq_bishops[WHITE][from8x8];
	case WHITE_KNIGHT:
		return psq_knights[WHITE][from8x8];
	case WHITE_ROOK:
		return psq_rooks[WHITE][from8x8];
	case WHITE_QUEEN:
		return psq_queens[WHITE][from8x8];
	case BLACK_PAWN:
		return psq_pawns[1][from8x8];
	case BLACK_BISHOP:
		return psq_bishops[1][from8x8];
	case BLACK_KNIGHT:
		return psq_knights[1][from8x8];
	case BLACK_ROOK:
		return psq_rooks[1][from8x8];
	case BLACK_QUEEN:
		return psq_queens[1][from8x8];
	case WHITE_KING:
	case BLACK_KING:
		return 0;
	default:
		ASSERT(0==printf("You should never see this...\nPiece: %d\n", piece));
		return 20;
	}
}


int move_make(smove *move)
{
  ASSERT(board.bs[kingLoc[0]]==WHITE_KING);
  ASSERT(board.bs[kingLoc[1]]==BLACK_KING);

  move->fiftycounter = board.fiftyCounter;
  move->enPassantsq = board.enPassant;
  move->castleRights = board.castleRights;
  board.historymatValue[0][board.gameply]=board.matValues[0];
  board.historymatValue[1][board.gameply]=board.matValues[1];
#ifndef NDEBUG
  board.historymoves[board.gameply] = move->move;
#endif
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

  board.posKey ^= side;

  if ( COLORLESSPIECE(move->move) == PAWN || capture)
    board.fiftyCounter = 0;

  if ((piece & 7) == KING) {
    kingLoc[piece >> 3] = to;
  }

  board.bs[from] = EMPTY;

  if (promoted) {
	  //printf("Yeah, ***************************** %s\n", moveToUCI(move->move));
    board.posKey ^= pieceKeys[piece][to] ^ pieceKeys[promoted][to];
    board.bs[to] = promoted;
    if (board.sideToMove == BLACK) {
      board.matValues[0] += matValues[promoted]-matValues[piece]
														  -valuePiecePSQ(piece,from)
														  +valuePiecePSQ(promoted,to);// add the new piece's psq value
      ++board.bigCount[0];
    } else {
      board.matValues[1] += matValues[promoted]-matValues[piece]
														  -valuePiecePSQ(piece,from)
														  +valuePiecePSQ(promoted,to);
      ++board.bigCount[1];
    }
  } else {
    board.bs[to] = piece;
	if (board.sideToMove==BLACK) {
		board.matValues[0] += valuePiecePSQ(piece,to)-valuePiecePSQ(piece,from);
	} else {
		board.matValues[1] += valuePiecePSQ(piece,to)-valuePiecePSQ(piece,from);
	}
  }

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
      board.matValues[0] += 5;
    } else if (to == G8) {
      board.bs[H8] = EMPTY;
      board.bs[F8] = BLACK_ROOK;
      board.posKey ^= pieceKeys[BLACK_ROOK][H8]
          ^ pieceKeys[BLACK_ROOK][F8];
    } else if (to == C8) {
      board.bs[A8] = EMPTY;
      board.bs[D8] = BLACK_ROOK;
      board.matValues[1] += -5;
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
      board.matValues[1] -= (valuePiecePSQ(BLACK_PAWN,to-16) + matValues[BLACK_PAWN]);

    } else {
      board.bs[to + 16] = 0;
      board.posKey ^= pieceKeys[WHITE_PAWN][to + 16];
      board.matValues[0] -= (valuePiecePSQ(WHITE_PAWN,to+16)+matValues[WHITE_PAWN]);
    }
    /* artificially mark the move as non-capture so we don't hash-out anything at the destination square of the move */
    capture=0;
  }
	if (capture) {
		board.posKey ^= pieceKeys[capture][to];
		if (board.sideToMove == BLACK) {
			ASSERT(capture&(1<<3));
			board.matValues[1] -= (matValues[capture]+valuePiecePSQ(capture,to));
			if (capture != BLACK_KING && capture != BLACK_PAWN)
				--board.bigCount[1];
		} else {
			board.matValues[0] -= (matValues[capture]+valuePiecePSQ(capture,to));
			if (capture != WHITE_KING && capture != WHITE_PAWN)
				--board.bigCount[0];
		}
	}

  ASSERT(board.bs[kingLoc[0]]==WHITE_KING);
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
  board.matValues[0]=board.historymatValue[0][board.gameply];
  board.matValues[1]=board.historymatValue[1][board.gameply];

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

  int promoted = ISPROMOTION(move->move);
  if (promoted) {
    if (board.sideToMove == BLACK) {

      --board.bigCount[1];
    } else {

      --board.bigCount[0];
    }
  }

  int capturedpiece = ISCAPTURE(move->move);

  if (SPECIAL(move->move) == SP_ENPASSANT) {
    if (board.sideToMove == BLACK) {
      board.bs[to + 16] = WHITE_PAWN;

    } else {
      board.bs[to - 16] = BLACK_PAWN;

    }
  } else if (capturedpiece) {
    board.bs[to] = capturedpiece;
    if (board.sideToMove==BLACK) {

      if (capturedpiece!=WHITE_KING && capturedpiece!=WHITE_PAWN)
        ++board.bigCount[0];
    }else {

      if (capturedpiece!=BLACK_KING && capturedpiece!=BLACK_PAWN)
        ++board.bigCount[1];
    }
  }

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

  if (board.sideToMove == WHITE) {
    if (board.enPassant != ENPASSANTNULL && ROW(board.enPassant) == 5) {
      if (board.bs[board.enPassant - 16 - 1] == WHITE_PAWN) {
        pushSpecialMove(board.enPassant - 16 - 1, board.enPassant,WHITE_PAWN, BLACK_PAWN, SP_ENPASSANT);
      }
      if (board.bs[board.enPassant - 16 + 1] == WHITE_PAWN) {
        pushSpecialMove(board.enPassant - 16 + 1, board.enPassant,WHITE_PAWN, BLACK_PAWN, SP_ENPASSANT);
      }
    }
  } else {
    if (board.enPassant != ENPASSANTNULL && ROW(board.enPassant) == 2) {
      if (board.bs[board.enPassant + 16 - 1] == BLACK_PAWN) {
        pushSpecialMove(board.enPassant + 16 - 1, board.enPassant,BLACK_PAWN, WHITE_PAWN, SP_ENPASSANT);
      }
      if (board.bs[board.enPassant + 16 + 1] == BLACK_PAWN) {
        pushSpecialMove(board.enPassant + 16 + 1, board.enPassant,BLACK_PAWN, WHITE_PAWN, SP_ENPASSANT);
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
