/*
 ============================================================================
 Name        : amichess.c
 Author      : Alex Argiropoulos
 Version     :
 Copyright   : GPL V2
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "amichess.h"
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

void testPerft(int n);

int main(int argc, char **argv) {
	int i;

	initBoard();
	//fen2board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
	//fen2board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/Pp2P3/2N2Q1p/1PPBBPPP/R3K2R b KQkq a3 0 1"); //e8f8 gives 47
	//fen2board("r4k1r/p1ppqpb1/bn2pnp1/3PN3/Pp2P3/2N2Q1p/1PPBBPPP/R3K2R w KQ - 1 2");// when we play e8f8, we get the correct 49


	//thinkFen("1r6/4b2k/1q1pNrpp/p2Pp3/4P3/1P1R3Q/5PPP/5RK1 w",DDEPTH);
	//thinkFen("2q1nk1r/4Rp2/1ppp1P2/6Pp/3p1B2/3P3P/PPP1Q3/6K1 w",DDEPTH);
	//thinkFen("r5rk/5p1p/5R2/4B3/8/8/7P/7K w",DDEPTH);
	//thinkFen("5B2/6P1/1p6/8/1N6/kP6/2K5/8 w",DDEPTH);
	//thinkFen("8/R7/4kPP1/3ppp2/3B1P2/1K1P1P2/8/8 w",DDEPTH);
	//thinkFen("3r1r1k/1p3p1p/p2p4/4n1NN/6bQ/1BPq4/P3p1PP/1R5K w",DDEPTH);
	//thinkFen("r1bq2r1/b4pk1/p1pp1p2/1p2pP2/1P2P1PB/3P4/1PPQ2P1/R3K2R w",DDEPTH);
//	thinkFen("3n2nr/4Pqpp/2k5/8/8/8/2B3PP/6K1 w",DDEPTH);
//	return 0;


//	LINE line;
//	int j;
//
//	for (j = 0; j < 200; ++j) {
//
//		line.cmove = 0;
//
//		int res = AlphaBeta(0,DDEPTH, -CHECKMATE_SCORE-1000, +CHECKMATE_SCORE+1000, &line);
//
//		smove m;
//		printf("res=%d number of moves in pv: %d\n", res, line.cmove);
//
//		for (i = 0; i < line.cmove; ++i) {
//			m.move = line.argmove[i];
//			printMove(m);
//			printf(" ");
//
//		}
//		m.move = line.argmove[0];
//		move_make(&m);
//		printBoard();
//	}

	 if (argc != 2)
	 i = 4;
	 else
	 i = atol(argv[1]);
	 fen2board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
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

void testPerft(int n) {
	char line[256];
	char fen[120];

	FILE *f = fopen("/home/alex/git/local/amichess/src/perfsuite.epd", "r");

	int correct = 0;
	int error = 0;
	int lineno = 0;
	while (fgets(line, 256, f)) {
		//printf("%s",line);
		char *start;
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
