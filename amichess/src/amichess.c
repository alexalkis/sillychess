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

//http://www.albert.nu/programs/sharper/perft/
//The above link was used for good fen positions

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
	//fen2board("rnbqkbnr/pppppppp/8/8/8/1P6/P1PPPPPP/RNBQKBNR b KQkq - 0 1");
	//fen2board("rnbqkbnr/1ppppppp/p7/8/8/N7/PPPPPPPP/1RBQKBNR b Kkq - 1 2");

	//fen2board("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1");
	//fen2board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"); //48 moves
	//fen2board("8/3K4/2p5/p2b2r1/5k2/8/8/1q6 b - 1 67");  //50 moves
	//board.sideToMove=BLACK;
	//fen2board("8/7p/p5pb/4k3/P1pPn3/8/P5PP/1rB2RK1 b - d3 0 28");
	if (argc!=2)
		i=6;
	else
		i=atol(argv[1]);
	printBoard();
	int starttime = get_ms();
	u64 nodes=Divide(i);
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
	return EXIT_SUCCESS;
}
