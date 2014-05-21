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
	board.castleRights=0;
	initBoard();
	//fen2board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
	//fen2board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/2KR3R b kq - 1 1");

	fen2board("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - 0 1");
	if (argc!=2)
		i=4;
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
