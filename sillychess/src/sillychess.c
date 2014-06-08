/*
 ============================================================================
 Name        : sillychess.c
 Author      : Alex Argiropoulos
 Version     :
 Copyright   : Free GPL V2.0
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sillychess.h"
#include "move.h"

struct aboard board;

/*
 * Amiga exact 68000 speed on emulation
 * Depth			Nps		Time
 * DummyPerft(1)	2400	20ms
 * DummyPerft(2)	2560	900ms
 * DummyPerft(3)	2502	44200ms
 * DummyPerft(4)	2500	2122700ms
 */

LINE pv;
void testPerft(int n);

void printLine(LINE *line)
{
	int i;
	smove m[MAXDEPTH];
	for (i = 0; i < line->cmove; ++i) {
		m[i].move = line->argmove[i];
		move_make(&m[i]);
		printMove(m[i]);
		if (isAttacked(board.sideToMove ^ BLACK,
				kingLoc[1 - ((board.sideToMove ^ BLACK) >> 3)])) {
			if (numOfLegalMoves()) {
				printf("+");
			} else {
				printf("#");
				++i;
				break;
			}
		}
		printf(" ");
	}
	for (--i; i >= 0; --i) {
		move_unmake(&m[i]);
	}
	printf("\n");
	fflush(stdout);
}

void think(S_SEARCHINFO *info)
{
	int depth, finalDepth;
	LINE line;

	line.cmove = 0;

	info->nodes=info->fh=info->fhf=info->nullCut=0;
	info->stopped = FALSE;
	if (info->timeset == FALSE)
		finalDepth = info->depth;
	else
		finalDepth = MAXDEPTH;

	for (depth = 0; depth < MAXDEPTH; ++depth) {
		board.searchKillers[0][depth] = board.searchKillers[1][depth] = NOMOVE;
		pv.argmove[depth] = NOMOVE;
	}
	for (depth = 1; depth <= finalDepth; ++depth) {
		info->lmr=info->fh=info->fhf=info->nullCut=0;

		int starttime = get_ms();
		int score = AlphaBeta(0, depth, -CHECKMATE_SCORE - 1000,
				+CHECKMATE_SCORE + 1000, &line, TRUE, info);
		int endtime = get_ms();
		CheckUp(info);
		if (info->stopped == TRUE) {
			break;
		}

		if (endtime == starttime)
			++endtime;

		int mate = 0;
		if ((score >= (CHECKMATE_SCORE - depth))
				|| (score <= -(CHECKMATE_SCORE - depth))) {
			if (score >= (CHECKMATE_SCORE - depth))
				mate = (CHECKMATE_SCORE - score+1) / 2;
			else
				mate = (-CHECKMATE_SCORE - score) / 2;
		}

		if (info->GAME_MODE == GAMEMODE_CONSOLE) {
			printf(
					"info depth %d (%.2f%%, NULLMOVES: %d LMR: %d) nodes %lld time %d nps %lld score cp %d pv ",
					depth, (info->fhf * 100.0f / (info->fh + info->fhf)), info->nullCut,info->lmr, info->nodes,
					(endtime - info->starttime),
					(1000 * info->nodes) / (endtime - starttime), score);
		} else {
			if (!mate)
				printf(
						"info depth %d score cp %d nodes %lld nps %lld time %d pv ",
						depth, score, info->nodes,
						(1000 * info->nodes) / (endtime - starttime),
						(endtime - info->starttime));
			else
				printf(
						"info depth %d score mate %d nodes %lld nps %lld time %d pv ",
						depth, mate, info->nodes,
						(1000 * info->nodes) / (endtime - starttime),
						(endtime - info->starttime));
		}
		printLine(&line);
		if (mate)
			depth = finalDepth;
		pv = line;
		/* if the time needed for search of depth D will exceed the remaining time, then D+1 will exceed also...probably */
		if (info->timeset
				&& (info->starttime + (endtime - starttime)) > info->stoptime)
			break;
	}
	printf("bestmove %s\n",moveToUCI(pv.argmove[0]));
//	smove t;
//	t.move = pv.argmove[0];
//	printMove(t);
//	printf("\n");
}

//depth 1 (-nan%, nullcuts: 0) nodes 21 time 1 nps 21000 score cp 50 b1c3
//depth 2 (89.47%, nullcuts: 0) nodes 106 time 1 nps 106000 score cp 0 b1c3 b8c6
//depth 3 (94.74%, nullcuts: 0) nodes 633 time 1 nps 633000 score cp 50 b1c3 b8c6 g1f3
//depth 4 (87.97%, nullcuts: 0) nodes 2450 time 2 nps 1225000 score cp 0 b1c3 b8c6 g1f3 g8f6
//depth 5 (87.28%, nullcuts: 16) nodes 6531 time 4 nps 3265500 score cp 40 b1c3 b8c6 g1f3 g8f6 d2d4
//depth 6 (74.69%, nullcuts: 18) nodes 54221 time 43 nps 1390282 score cp 0 b1c3 b8c6 g1f3 g8f6 d2d4 d7d5
//depth 7 (82.76%, nullcuts: 173) nodes 214454 time 160 nps 1832940 score cp 35 e2e4 b8c6 g1f3 g8f6 e4e5 f6e4 b1c3
//depth 8 (58.13%, nullcuts: 419) nodes 2938255 time 1941 nps 1649778 score cp 5 e2e4 b8c6 g1f3 g8f6 e4e5 f6g4 d2d4 d7d5

//depth 1 (-nan%, nullcuts: 0) nodes 21 time 1 nps 21000 score cp 50 b1c3
//depth 2 (89.47%, nullcuts: 0) nodes 106 time 1 nps 106000 score cp 0 b1c3 b8c6
//depth 3 (94.74%, nullcuts: 0) nodes 633 time 1 nps 633000 score cp 50 b1c3 b8c6 g1f3
//depth 4 (87.97%, nullcuts: 0) nodes 2450 time 7 nps 408333 score cp 0 b1c3 b8c6 g1f3 g8f6
//depth 5 (87.28%, nullcuts: 16) nodes 6531 time 11 nps 1632750 score cp 40 b1c3 b8c6 g1f3 g8f6 d2d4
//depth 6 (74.69%, nullcuts: 18) nodes 54221 time 47 nps 1506138 score cp 0 b1c3 b8c6 g1f3 g8f6 d2d4 d7d5
//depth 7 (82.76%, nullcuts: 173) nodes 214454 time 156 nps 1967467 score cp 35 e2e4 b8c6 g1f3 g8f6 e4e5 f6e4 b1c3
//depth 8 (58.13%, nullcuts: 419) nodes 2938255 time 1967 nps 1622448 score cp 5 e2e4 b8c6 g1f3 g8f6 e4e5 f6g4 d2d4 d7d5
//depth 9 (80.52%, nullcuts: 1008) nodes 5279342 time 3459 nps 3538432 score cp 30 e2e4 b8c6 b1c3 g8f6 d2d4 d7d5 e4e5 f6e4 g1e2
//depth 10 (76.92%, nullcuts: 7741) nodes 25149657 time 16281 nps 1961445 score cp 20 e2e4 b8c6 b1c3 g8f6 d2d4 d7d5 e4e5 f6e4 g1f3 e7e6

//cutechess-cli -fcp cmd=~/workspacecpp/sillychess/Release/sillychess tc=40/600 proto=uci -scp cmd=fairymax proto=xboard tc=120/1 -games 12

//cutechess-cli -fcp cmd=~/workspacecpp/sillychess/Release/sillychess proto=uci -scp cmd="java -jar /home/alex/t/chess4j-1.2/chess4j-1.2-uber.jar" proto=xboard -both tc=30/10 book=./varied.bin -games 10 -pgnout st.pgn

void init(void)
{
	initHash();
	InitMvvLva();
	initBoard();

}
int main(int argc, char **argv)
{
	S_SEARCHINFO info[1];
	info->GAME_MODE = GAMEMODE_CONSOLE;

	int i;

	init();

	input_loop(info);
	return 0;

	//fen2board("3Q4/p3b1k1/2p2rPp/2q5/4B3/P2P4/7P/6RK w - -");  // mate in 4
	//fen2board("8/6r1/p7/1p6/3kBn1P/P2p1P2/1P6/3R1K2 b - -");

	//fen2board("2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - -");
	//fen2board("8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - -");  //wac002 fails
	//fen2board("5rk1/1ppb3p/p1pb4/6q1/3P1p1r/2P1R2P/PP1BQ1P1/5RKN w - -");
	//fen2board("r1bq2rk/pp3pbp/2p1p1pQ/7P/3P4/2PB1N2/PP3PPR/2KR4 w - -");
	//fen2board("5k2/6pp/p1qN4/1p1p4/3P4/2PKP2Q/PP3r2/3R4 b - -");
	//fen2board("7k/p7/1R5K/6r1/6p1/6P1/8/8 w - -");

	//fen2board("2n4r/p1pr1kpp/3bp1q1/PP2p3/1PRP3R/1Q3N2/5P2/B3K3 w - -");
	info->timeset = TRUE;
	info->depth = MAXDEPTH;

	info->starttime = get_ms();
	info->stoptime = get_ms() + 6000;

	think(info);
	printf("%d ms\n", get_ms() - info->starttime);
	return 0;

	LINE line;
	int j;

	for (j = 0; j < 200; ++j) {

		line.cmove = 0;

		int res = AlphaBeta(0, DDEPTH, -CHECKMATE_SCORE - 1000,
				+CHECKMATE_SCORE + 1000, &line, TRUE, info);

		smove m;
		printf("res=%d number of moves in pv: %d\n", res, line.cmove);

		for (i = 0; i < line.cmove; ++i) {
			m.move = line.argmove[i];
			printMove(m);
			printf(" ");

		}
		m.move = line.argmove[0];
		move_make(&m);
		printBoard();
	}

	return 0;

	if (argc != 2)
		i = 5;
	else
		i = atol(argv[1]);
	fen2board(
			"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
	printBoard();
	int starttime = get_ms();
	u64 nodes = Divide(i);
	int endtime = get_ms();
	if (endtime == starttime)
		++endtime;
	printf("Divide - Nodes:%lld Nps: %lld (%d ms)\n", nodes,
			(1000 * nodes) / (endtime - starttime), (endtime - starttime));

//	smove moves[200];
//	int numOfMoves = generateMoves(moves);
//	printMoveList();
//	printf("Moves: %d\n", numOfMoves);
//	for (i = 1; i < 6; ++i) {
//		int starttime = get_ms();
//		u64 nodes = Perft(i);
//		int endtime = get_ms();
//		if (endtime==starttime) ++endtime;
//		printf("Perft(%d)=%lld Nps: %lld (%d ms)\n", i, nodes,
//				(1000*nodes) / (endtime - starttime), (endtime - starttime));
//	}
	//testPerft(3);
	return EXIT_SUCCESS;
}

void testPerft(int n)
{
	char line[256];
	char fen[120];

	FILE *f = fopen("/home/alex/git/local/amichess/src/perfsuite.epd", "r");

	int correct = 0;
	int error = 0;
	int lineno = 0;
	while (fgets(line, 256, f)) {
		//printf("%s",line);
		char *start = NULL;
		char *semi = strchr(line, ';');
		*semi = '\0';
		strcpy(fen, line);
		int c = n;
		while (c--) {
			++semi;
			//printf("%s\n",fen);
			start = semi;
			semi = strchr(semi, ';');
			if (!semi) {
				semi = start;
				semi = strchr(semi, '\n');
			}
			*semi = '\0';
		}
		//printf("--->%s\n",start);
		if (start[0] != 'D') {
			++lineno;
			continue;
		}
		int depth = start[1] - '0';
		start += 3;
		int value = atol(start);

		fen2board(fen);
		int result = Perft(depth);
		printf("#%3d Depth: %d, value=%d - Perft: %d %s\n", ++lineno, depth,
				value, result, (result == value) ? "" : fen);
		if (result == value)
			++correct;
		else
			++error;

	}
	printf("Correct: %d -- Error: %d\n", correct, error);
	fclose(f);
}
