/*
 * uci.c
 *
 *  Created on: Jun 1, 2014
 *      Author: alex
 */

#include "move.h"
#include "sillychess.h"

#define INPUTBUFFER		(4096)
#define NOMOVE			0
#define FR2SQ(f,r) ( (f)  + (r) * 16 )

void showMoveList(void)
{
	int i;
	smove m[256];
	int mcount = generateMoves(m);

	for (i = 0; i < mcount; i++) {
		move_make(&m[i]);
		if (!isAttacked(board.sideToMove,
				kingLoc[1 - (board.sideToMove >> 3)])) {
			printMove(m[i]);printf(" ");
		}
		move_unmake(&m[i]);
	}
	printf("\n");
}


void ParseScore(char* line, S_SEARCHINFO *info)
{
	LINE scline;
	int pdepth = atoi(&line[6]);
	int i;
	smove m[256];
	int mcount = generateMoves(m);



	for (i = 0; i < mcount; i++) {
		move_make(&m[i]);
		if (!isAttacked(board.sideToMove,kingLoc[1 - (board.sideToMove >> 3)])) {
			printMove(m[i]);printf(" %d\n",-AlphaBeta(pdepth,-INFINITE,INFINITE,&scline,TRUE,info));
//			printf("Board posKey: %llX\n",board.posKey);
//			for(j=board.gameply-board.fiftyCounter-1; j<board.gameply; ++j)
//				printf("%d %llX\n",j,board.historyPosKey[j]);
		}
		move_unmake(&m[i]);
	}
}


void ParsePerft(char* line, S_SEARCHINFO *info)
{
	int pdepth = atoi(&line[6]);

	unsigned int starttime = get_ms();
	u64 nodes = Perft(pdepth);
	unsigned int endtime = get_ms();
	if (endtime == starttime)
		++endtime;
	printf("Perft(%d)=%lld Nps: %lld (%d ms)\n", pdepth, nodes,
			(1000 * nodes) / (endtime - starttime), (endtime - starttime));
}

void ParseDivide(char* line, S_SEARCHINFO *info)
{
	int pdepth = atoi(&line[7]);

	int starttime = get_ms();
	u64 nodes = Divide(pdepth);
	int endtime = get_ms();
	if (endtime == starttime)
		++endtime;
	printf("Perft(%d)=%lld Nps: %lld (%d ms)\n", pdepth, nodes,
			(1000 * nodes) / (endtime - starttime), (endtime - starttime));
}

void ParseTestEPD(char *line, S_SEARCHINFO *info) {
	// testepd xxxx filename
	char *ptr=strstr(line+8," ");
	if (!ptr) {
		printf("testepd xxxx filename (where xxxx is time per position in milliseconds.\n");
		return;
	}
	*ptr++='\0';
	ptr[strlen(ptr)-1]='\0'; //eat the \n
	int time=atoi(line+7);
	testEPD(ptr,time);
	return;
}

void ParseGo(char* line, S_SEARCHINFO *info)
{

	int depth = -1, movestogo = 30, movetime = -1;
	int time = -1, inc = 0;
	char *ptr = NULL;
	info->timeset = FALSE;

	if ((ptr = strstr(line, "infinite"))) {
		;
	}

	if ((ptr = strstr(line, "binc")) && board.sideToMove == BLACK) {
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line, "winc")) && board.sideToMove == WHITE) {
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line, "wtime")) && board.sideToMove == WHITE) {
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line, "btime")) && board.sideToMove == BLACK) {
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line, "movestogo"))) {
		movestogo = atoi(ptr + 10);
	}

	if ((ptr = strstr(line, "movetime"))) {
		movetime = atoi(ptr + 9);
	}

	if ((ptr = strstr(line, "depth"))) {
		depth = atoi(ptr + 6);
	}

	if (movetime != -1) {
		time = movetime;
		movestogo = 1;
	}

	info->starttime = get_ms();
	info->depth = depth;

	if (time != -1) {
		info->timeset = TRUE;
		time /= movestogo;
		//time -= 50;
		info->stoptime = info->starttime + time + inc;
	}

	if (depth == -1) {
		info->depth = MAXDEPTH;
	} else
		info->timeset = FALSE;

	printf("time:%d start:%d stop:%d depth:%d timeset:%d\n", time,
			info->starttime, info->stoptime, info->depth, info->timeset);
	think(info);
	printf("Time taken: %d\n", get_ms() - info->starttime);
	fflush(stdout);
}

int ParseMove(char *ptrChar)
{
	if (ptrChar[1] > '8' || ptrChar[1] < '1')
		return NOMOVE;
	if (ptrChar[3] > '8' || ptrChar[3] < '1')
		return NOMOVE;
	if (ptrChar[0] > 'h' || ptrChar[0] < 'a')
		return NOMOVE;
	if (ptrChar[2] > 'h' || ptrChar[2] < 'a')
		return NOMOVE;

	int from = FR2SQ(ptrChar[0] - 'a', ptrChar[1] - '1');
	int to = FR2SQ(ptrChar[2] - 'a', ptrChar[3] - '1');

	smove list[256];
	int mcount = generateMoves(list);

	int MoveNum = 0;
	int Move = 0;
	int PromPce = EMPTY;

	for (MoveNum = 0; MoveNum < mcount; ++MoveNum) {
		Move = list[MoveNum].move;
		if (FROM(Move) == from && TO(Move) == to) {
			PromPce = COLORLESSPROMOTED(Move);
			if (PromPce != EMPTY) {
				if (PromPce == ROOK && ptrChar[4] == 'r') {
					return Move;
				} else if (PromPce == BISHOP && ptrChar[4] == 'b') {
					return Move;
				} else if (PromPce == QUEEN && ptrChar[4] == 'q') {
					return Move;
				} else if (PromPce == KNIGHT && ptrChar[4] == 'n') {
					return Move;
				}
				continue;
			}
			return Move;
		}
	}
	return NOMOVE;
}

void listMoves(void) {
	smove m[256];
	int mcount = generateMoves(m);
	int i;

	for(i=0; i<mcount; ++i) {
		pickMove(m,i,mcount);
		move_make(&m[i]);
		if (isAttacked(board.sideToMove, kingLoc[1 - (board.sideToMove >> 3)])) {
			//printf("ILLEGAL %s %d\n",moveToUCI(m[i].move),m[i].score);
			move_unmake(&m[i]);
			continue;
		}
		printf("%s %d (move in decimal if you want a breakpoint %d ;-)\n",moveToUCI(m[i].move),m[i].score,m[i].move);
		move_unmake(&m[i]);
	}
}
void ParsePosition(char* lineIn)
{

	lineIn += 9;
	char *ptrChar = lineIn;

	if (strncmp(lineIn, "startpos", 8) == 0) {
		board.gameply=0;
		fen2board(START_FEN);
	} else {
		ptrChar = strstr(lineIn, "fen");
		if (ptrChar == NULL) {
			fen2board(START_FEN);
		} else {
			ptrChar += 4;
			fen2board(ptrChar);
		}
	}

	ptrChar = strstr(lineIn, "moves");
	int move;


	if (ptrChar != NULL) {
		ptrChar += 6;
		while (*ptrChar) {
			move = ParseMove(ptrChar);
			if (move == NOMOVE) {
				break;
			}
 			smove m;
			m.move = move;
			move_make(&m);
			//pos->ply=0;
			while (*ptrChar && *ptrChar != ' ')
				ptrChar++;
			ptrChar++;
		}
	}
	printBoard();
	printf("Ply: %d FiftyCounter: %d\n", board.ply, board.fiftyCounter);
}

void input_loop(S_SEARCHINFO *info)
{
	int exit = FALSE;
	char line[INPUTBUFFER];

	printf("%s, written by Alex Argiropoulos\n", NAME);

#ifndef NDEBUG
	printf("NOTE: NDEBUG not defined at compilation stage.  Performance will not be optimum!\n");
#endif
	while (!exit) {
		memset(line, 0, sizeof(line));
		fflush(stdout);
		if (!fgets(line, INPUTBUFFER, stdin))
			continue;

		if (line[0] == '\n')
			continue;
		if (!strncmp(line, "isready", 7)) {
			printf("readyok\n");
			continue;
		} else if (!strncmp(line, "uci", 3)) {
			printf("id name %s\n", NAME);
			printf("id author Alex Argiropoulos, Greece\n");
			printf("uciok\n");
			info->GAME_MODE = GAMEMODE_UCI;
		} else if (!strncmp(line, "position", 8)) {
			ParsePosition(line);
		} else if (!strncmp(line, "go", 2)) {
			ParseGo(line, info);
			//think(2);
		} else if (!strncmp(line, "perft", 5)) {
			ParsePerft(line, info);
		} else if (!strncmp(line, "score", 5)) {
			ParseScore(line, info);
		} else if (!strncmp(line, "divide", 6)) {
			ParseDivide(line, info);
		} else if (!strncmp(line, "ucinewgame", 10)) {
			TT_clear();
			ParsePosition("position startpos\n");
		} else if (!strncmp(line, "movelist", 8)) {
			showMoveList();
		} else if (!strncmp(line, "d", 1)) {
			printBoard();
		} else if (!strncmp(line, "ls", 2)) {
			listMoves();
		} else if (!strncmp(line, "testepd", 7)) {
			ParseTestEPD(line,info);
		} else if (!strncmp(line, "eval", 4)) {
			printf("Eval=%d\n",Evaluate());
		} else if (!strncmp(line, "version", 4)) {
			puts(FULLNAME);
			printf(	"- 0x88 board\n"
					"- Iterative deepening\n"
					"- Null move reduction (TODO: move the material counting from eval to make_move and have it incremental)\n"
					"- Futility pruning\n"
					"- Razoring pruning\n"
					"- MVV/LVA\n"
					"- Killer heuristics\n"
					"- History heuristics\n"
					"- Transposition tables\n"
					"- Late Move Reductions\n"
					);
		} else if (!strncmp(line, "quit", 4)) {
			exit = TRUE;
			break;
		}
	}
}
