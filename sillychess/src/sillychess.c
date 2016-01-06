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

#define NOSIGHANDLER
#if !defined(WIN32) && !defined(__AMIGA__)
#include <execinfo.h>
#include <signal.h>
#include <ucontext.h>
#include <unistd.h>


/* This structure mirrors the one found in /usr/include/asm/ucontext.h */
typedef struct _sig_ucontext {
 unsigned long     uc_flags;
 struct ucontext   *uc_link;
 stack_t           uc_stack;
 struct sigcontext uc_mcontext;
 sigset_t          uc_sigmask;
} sig_ucontext_t;

void crit_err_hdlr(int sig_num, siginfo_t * info, void * ucontext)
{
 void *             array[50];
 void *             caller_address;
 char **            messages;
 int                size, i;
 sig_ucontext_t *   uc;

 uc = (sig_ucontext_t *)ucontext;

 /* Get the address at the time the signal was raised */
#if defined(__i386__) // gcc specific
 caller_address = (void *) uc->uc_mcontext.eip; // EIP: x86 specific
#elif defined(__x86_64__) // gcc specific
 caller_address = (void *) uc->uc_mcontext.rip; // RIP: x86_64 specific
#else
#error Unsupported architecture. // TODO: Add support for other arch.
#endif

 fprintf(stderr, "signal %d (%s), address is %p from %p\n",
  sig_num, strsignal(sig_num), info->si_addr,
  (void *)caller_address);

 size = backtrace(array, 50);

 /* overwrite sigaction with caller's address */
 array[1] = caller_address;

 messages = backtrace_symbols(array, size);

 /* skip first stack frame (points here) */
 for (i = 1; i < size && messages != NULL; ++i)
 {
  fprintf(stderr, "[bt]: (%d) %s\n", i, messages[i]);
 }

 free(messages);

 exit(EXIT_FAILURE);
}
#endif

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
	int i;
#if !defined(WIN32) && !defined(NOSIGHANDLER)
	struct sigaction sigact;

	 sigact.sa_sigaction = crit_err_hdlr;
	 sigact.sa_flags = SA_RESTART | SA_SIGINFO;

	 if (sigaction(SIGSEGV, &sigact, (struct sigaction *)NULL) != 0)
	 {
	  fprintf(stderr, "error setting signal handler for %d (%s)\n",
	    SIGSEGV, strsignal(SIGSEGV));

	  exit(EXIT_FAILURE);
	 }
#endif

	init();

	//fen2board("8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - -");
	//testEPD("alkis",1000);
	//fen2board("2r3k1/1q1nbppp/r3p3/3pP3/pPpP4/P1Q2N2/2RN1PPP/2R4K b - b3 0 23");
	input_loop(info);
	TT_free();
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

		int res = AlphaBeta(DDEPTH, -INFINITE,
				+INFINITE, &line, TRUE, info);

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
	printf("Divide - Nodes:%"INT64_FORMAT"d Nps: %"INT64_FORMAT"d (%d ms)\n", nodes,
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
//		printf("Perft(%d)=%"INT64_FORMAT"d Nps: %"INT64_FORMAT"d (%d ms)\n", i, nodes,
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
//v0.3.1 271/879 at 1000ms on ECM Total time: 880651ms Total nodes: 1535082412



// testepd 400 /home/alex/git/amichess/sillychess/src/wac.epd
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
		info->starttime=get_ms();
		info->stoptime=get_ms()+miliseconds;
		info->stopped=FALSE;
		info->timeset=TRUE;
		TT_clear();
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
		printf("%d/%d (%s) Engine: %s EPD: %s -- %s\n",solved,positions,id,engine,bm,res?"SOLVED":"NOT SOLVED");
		totalInfo->nodes+=info->nodes;


	}
	totalInfo->stoptime=get_ms();
	printf("Total time: %dms Total nodes: %"INT64_FORMAT"d\n",totalInfo->stoptime-totalInfo->starttime,totalInfo->nodes);
	fclose(f);
}
