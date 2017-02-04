/*
 ============================================================================
 Name        : sillychess.c
 Author      : Alex Argiropoulos
 Version     :
 Copyright   : Free GPL V2.0
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "sillychess.h"
#include <string.h>

/*
 * Amiga exact 68000 speed on emulation
 * Depth			Nps		Time
 * DummyPerft(1)	2400	20ms
 * DummyPerft(2)	2560	900ms
 * DummyPerft(3)	2502	44200ms
 * DummyPerft(4)	2500	2122700ms
 */


// position fen 2k5/8/1pP1K3/1P6/8/8/8/8 w - - 0 1 this one is nice, stockfish finds mate in 596 ms (!!!)
//info depth 36 seldepth 40 score mate 24 nodes 2897303 nps 4861246 time 596 multipv 1 pv c6c7 c8c7 e6e7 c7c8 e7d6 c8b7 d6d7 b7b8 d7c6 b8a7 c6c7 a7a8 c7b6 a8b8 b6a6 b8c7 b5b6 c7c8 a6a7 c8d7 b6b7 d7e6 b7b8q e6f5 a7b6 f5g5 b6c6 g5h5 c6d5 h5g6 b8h2 g6f6 h2g3 f6f7 g3e5 f7f8 d5e6 f8g8 e6f6 g8h7 e5f4 h7g8


void testPerft(int n);

struct aboard board;


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
	board.gameply=0;
    board.ht=NULL;
    board.htSize=0;
#ifdef __AMIGA__
    TT_set_size(4);
#else
    TT_set_size(128);
#endif
}
int main(int argc, char **argv)
{
	S_SEARCHINFO info[1];
	info->GAME_MODE = GAMEMODE_CONSOLE;

	init();
	if (argc>=2 && !strcmp(argv[1],"-bench"))
		testEPD("../src/wac.epd",100);
	else if (argc>=2 && !strcmp(argv[1],"-bench2")) {
		info->starttime=get_ms();
		info->timeset = FALSE;
#ifdef __AMIGA__
		info->depth = 8;
#else
		info->depth = 10;
#endif
		think(info);
	} else if (argc>=2 && !strcmp(argv[1],"-bench3")) {
		fen2board("8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - -");
		info->starttime = get_ms();
		info->stoptime = info->starttime + 30000;
		info->timeset = TRUE;
		think(info);
		//
	} else if (argc>=2 && !strcmp(argv[1],"-bench4")) {
			fen2board("2Q5/8/3K4/8/2k5/8/P7/8 b - - 2 62");
			info->starttime = get_ms();
			info->stoptime = info->starttime + 30000;
			info->timeset = TRUE;
			think(info);
	} else
		input_loop(info);
	TT_free();
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
//v0.3.1 271/879 at 1000ms on ECM Total time: 880651ms Total nodes: 1535082412



// testepd 400 /home/alex/git/amichess/sillychess/src/wac.epd
//v0.7.2c Time: 110466ms Nodes: 258516379 226/300 Avg.Depth: 9.02667
//v0.3.6b 220/300 at 400ms Total nodes: 262266409
//v0.3.1 221/300 at 400ms razoring + futility pruning
//V0.3.1 214/300 at 400ms after a minor modification in search (seldepth logging + small bug in signedness of time variables fixed)
//v0.3.1 215/300 at 400ms  Total time: 120581ms Total nodes: 206948188 [198/300 on laptop, Total nodes: 92094101]
//v0.3   209/300 at 400ms on WAC [195/300 on laptop]

void testEPD(char *filename, int miliseconds) {
	S_SEARCHINFO info[1],totalInfo[1];
	char line[256];
	char fen[120];
	int linecount=0;
	int positions=0;
	int solved=0;
	int totalDepth = 0;

	info->GAME_MODE=GAMEMODE_SILLENT;
	FILE *f = fopen(filename, "r");
	if (!f) {
		printf("Can't open \"%s\" for reading.\n",filename);
		return;
	}

	totalInfo->failHigh=totalInfo->failHighFirst=totalInfo->htAlpha=totalInfo->htBeta=totalInfo->htExact=totalInfo->hthit=totalInfo->htmiss
			=totalInfo->lmr=totalInfo->lmr2=totalInfo->lmr3=totalInfo->nodes=totalInfo->nullCut=0;
	totalInfo->starttime=get_ms();
	while (fgets(line, 256, f)) {
		char *bm = strstr(line,"bm ");
		char *id = strchr(line, ';');
		line[strlen(line)-1]='\0';		// eat the \n at the end
		if (!bm) {
			printf("Can't find bm (best move start marker) at line %d\n",linecount);
			break;
		} else {
			bm+=3;
		}
		*(bm-1)='\0';
		strcpy(fen, line);
		fen2board(fen);
		//printBoard();
		if (!id) {
			id = "No id!!!\n";
		} else {
			*id++='\0';
			while(*id==' ')
				++id;
		}
		//printf("Best move(s): %s\n",bm);
		//printf("%s\n",id);
		/* 
		   Since the amiga is kind of slow we clear the TT and then
		   we'll start counting time.
		*/
		#ifdef __AMIGA__
		TT_clear();
		#endif
		info->starttime=get_ms();
		info->stoptime=get_ms()+miliseconds;
		info->stopped=FALSE;
		info->timeset=TRUE;
		#ifndef __AMIGA__
		TT_clear();
		#endif
		int bestMove=think(info);

		smove m;
		m.move=bestMove;
		smove lm[256];
		int mcount=generateLegalMoves(lm);
		move_make(&m);
		char *engine=move_to_san(m,mcount,lm);
		char *res=strstr(bm,engine);
		move_unmake(&m);
		if (res) {
			++solved;
		}
		++positions;
		totalDepth += info->depth;
		printf("%d/%d (%s) Engine: %s (D:%d) EPD: %s -- %s\n",solved,positions,id,engine, info->depth,bm,res?"SOLVED":"NOT SOLVED");
		totalInfo->nodes+=info->nodes;


	}
	totalInfo->stoptime=get_ms();
	char buf[80];
	sprintf(buf, "cmd: testepd %d %s",miliseconds, filename);
	printf("Time: %dms Nodes: %"INT64_FORMAT"d %d/%d Avg.Depth: %g (%s)\n%s\n",totalInfo->stoptime-totalInfo->starttime,totalInfo->nodes, solved, positions, ((double)totalDepth)/positions, getCPUModel(), buf);
	fclose(f);
}
