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
int main(int argc, char **argv) {
	int i;

	initBoard();
	fen2board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
	//fen2board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/Pp2P3/2N2Q1p/1PPBBPPP/R3K2R b KQkq a3 0 1"); //e8f8 gives 47
	//fen2board("r4k1r/p1ppqpb1/bn2pnp1/3PN3/Pp2P3/2N2Q1p/1PPBBPPP/R3K2R w KQ - 1 2");// when we play e8f8, we get the correct 49



	/*
	if (argc != 2)
		i = 3;
	else
		i = atol(argv[1]);
	printBoard();
	int starttime = get_ms();
	u64 nodes = Divide(i);
	int endtime = get_ms();
	if (endtime == starttime)
		++endtime;
	printf("Divide - Nodes:%lld Nps: %lld (%d ms)\n", nodes,
			(1000 * nodes) / (endtime - starttime), (endtime - starttime));
	*/
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
	testPerft(6);
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
				semi=start;
				semi = strchr(semi, '\n');
			}
			*semi = '\0';
		}
		//printf("--->%s\n",start);
		if (start[0]!='D') {
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
