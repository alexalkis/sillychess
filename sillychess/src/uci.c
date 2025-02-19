/*
 * uci.c
 *
 *  Created on: Jun 1, 2014
 *      Author: alex
 */
#ifdef WIN32
#include "windows.h"
#endif

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include "move.h"
#include "sillychess.h"

#ifdef __amiga__
#include <exec/execbase.h>
extern struct ExecBase *SysBase;
#endif
#define INPUTBUFFER        (4096)
#define NOMOVE            0
#define FR2SQ(f, r) ( (f)  + (r) * 16 )

#ifdef WIN32
/* http://stackoverflow.com/questions/735126/are-there-alternate-implementations-of-gnu-getline-interface/735472#735472 */
/* This code is public domain -- Will Hartung 4/9/09 */
size_t getline(char **lineptr, size_t *n, FILE *stream) {
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL) {
        return -1;
    }
    if (stream == NULL) {
        return -1;
    }
    if (n == NULL) {
        return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF) {
        return -1;
    }
    if (bufptr == NULL) {
        bufptr = malloc(128);
        if (bufptr == NULL) {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while(c != EOF) {
        if ((p - bufptr) > (size - 1)) {
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL) {
                return -1;
            }
        }
        *p++ = c;
        if (c == '\n') {
            break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;

    return p - bufptr - 1;
}
#endif

char *getCPUModel(void) {
    static char cpustr[160];

    cpustr[0] = '\0';
#ifdef __linux__
    FILE *cpuinfo = fopen("/proc/cpuinfo", "rb");
    char *arg = 0;
    size_t size = 0;

    if (cpuinfo) {
        while (getline(&arg, &size, cpuinfo) != -1) {
            if (strstr(arg, "model name")) {
                char *t = strchr(arg, ':');
                if (t) {
                    t += 2;
                    if (t[strlen(t) - 1] == '\n')
                        t[strlen(t) - 1] = '\0';
                    strcat(cpustr, t);
                }
                break;
            }
        }
        free(arg);
        fclose(cpuinfo);
    }
    if (!strlen(cpustr)) {
        char buf[256];

        FILE *pf = popen("lscpu", "r");
        if (pf) {
            while (fgets(buf, sizeof(buf), pf)) {
                buf[strlen(buf) - 1] = '\0'; // eat \n
                if (strstr(buf, "Model name:")) {
                    char *s = buf + sizeof("Model name:");
                    while (!isalpha(*s))
                        ++s;
                    strcat(cpustr, ", ");
                    strcat(cpustr, s);
                }

                if (strstr(buf, "Architecture:")) {
                    char *s = buf + sizeof("Architecture:");
                    while (!isalpha(*s))
                        ++s;
                    strcat(cpustr, s);
                }
            }
            pclose(pf);
        }
    }
#endif
#ifdef WIN32
    HKEY hKey;
    DWORD cbData=sizeof(cpustr);
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0\\", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
        if(RegQueryValueEx(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)&cpustr, &cbData) == ERROR_SUCCESS) {
            ;
        }
        RegCloseKey(hKey);
    }
#endif
#ifdef __amiga__
    UWORD attnflags = SysBase->AttnFlags;
    strcpy(cpustr,"68000");
    if (attnflags & 0x80) cpustr[3]='6';
    if (attnflags & AFF_68040) cpustr[3]='4';
    if (attnflags & AFF_68030) cpustr[3]='3';
    if (attnflags & AFF_68020) cpustr[3]='2';
    if (attnflags & AFF_68010) cpustr[3]='1';
#endif
    return cpustr;
}

void showMoveList(void) {
    int i;
    smove m[256];
    int mcount = generateMoves(m);

    for (i = 0; i < mcount; i++) {
        move_make(&m[i]);
        if (!isAttacked(board.sideToMove,
                        kingLoc[1 - (board.sideToMove >> 3)])) {
            printMove(m[i]);
            printf(" ");
        }
        move_unmake(&m[i]);
    }
    printf("\n");
}

void ParseScore(char *line, S_SEARCHINFO *info) {
    LINE scline;
    int pdepth = atoi(&line[6]);
    int i;
    smove m[256];
    int mcount = generateMoves(m);

    for (i = 0; i < mcount; i++) {
        move_make(&m[i]);
        if (!isAttacked(board.sideToMove, kingLoc[1 - (board.sideToMove >> 3)])) {
            printMove(m[i]);
            printf(" %d\n", -AlphaBeta(pdepth, -INFINITE, INFINITE, &scline, TRUE, info));
//			printf("Board posKey: %"INT64_FORMAT"X\n",board.posKey);
//			for(j=board.gameply-board.fiftyCounter-1; j<board.gameply; ++j)
//				printf("%d %"INT64_FORMAT"X\n",j,board.historyPosKey[j]);
        }
        move_unmake(&m[i]);
    }
}

void ParsePerft(char *line) {
    int pdepth = atoi(&line[6]);

    unsigned int starttime = get_ms();
    u64 nodes = Perft(pdepth);
    unsigned int endtime = get_ms();
    if (endtime == starttime)
        ++endtime;
    printf("Perft(%d)=%"INT64_FORMAT"d Nps: %"INT64_FORMAT"d (%d ms)\n", pdepth, nodes,
           (1000 * nodes) / (endtime - starttime), (endtime - starttime));
}

void ParseDivide(char *line) {
    int pdepth = atoi(&line[7]);

    unsigned int starttime = get_ms();
    u64 nodes = Divide(pdepth);
    unsigned int endtime = get_ms();
    if (endtime == starttime)
        ++endtime;
    printf("Perft(%d)=%"INT64_FORMAT"d Nps: %"INT64_FORMAT"d (%d ms)\n", pdepth, nodes,
           (1000 * nodes) / (endtime - starttime), (endtime - starttime));
}

void ParseTestEPD(char *line) {
    // testepd xxxx filename
    char *ptr = strstr(line + 8, " ");
    if (!ptr) {
        testEPD("../src/wac.epd", 100);
        printf("testepd xxxx filename (where xxxx is time per position in milliseconds.\n");
        return;
    }
    *ptr++ = '\0';
    unsigned int e = strlen(ptr) - 1;
    while (!isalpha(ptr[e])) {
        ptr[e--] = '\0';  // eat any space or \n
    }
    int time = atoi(line + 7);
    testEPD(ptr, time);
}

void ParseGo(char *line, S_SEARCHINFO *info) {
    int depth = -1, movestogo = 30, movetime = -1;
    int time = -1, inc = 0;
    char *ptr = NULL;
    info->timeset = FALSE;

    if ((ptr = strstr(line, "infinite"))) { ;
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
        time += (movestogo-1)*inc;
        time /= movestogo;
        if (time>10)
            time -= 10;
        info->stoptime = info->starttime + time;
    }

    if (depth == -1) {
        info->depth = MAXDEPTH;
    } else
        info->timeset = FALSE;
#ifndef NDEBUG
    printf("time:%d start:%d stop:%d depth:%d timeset:%d\n", time,
           info->starttime, info->stoptime, info->depth, info->timeset);
#endif
    unsigned int move = think(info);
    unsigned int timeTaken = get_ms();
    if (info->GAME_MODE != GAMEMODE_UCI)
        printf("Time taken: %d\n", timeTaken - info->starttime);
    fflush(stdout);

    if (!move) {
        FILE *problem = fopen("error.txt", "a");
        fprintf(problem, "%s\n", board2fen());
        fprintf(problem, "%s\n", line);
        fprintf(problem, "time:%d start:%d stop:%d depth:%d timeset:%d\n", time,
                info->starttime, info->stoptime, info->depth, info->timeset);
        fprintf(problem, "Time taken: %d\n\n", timeTaken - info->starttime);
        fclose(problem);
    }
}

unsigned int ParseMove(const char *ptrChar) {
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

    int MoveNum;
    unsigned int Move;
    int PromPce;

    for (MoveNum = 0; MoveNum < mcount; ++MoveNum) {
        Move = list[MoveNum].move;
        if (FROM(Move) == from && TO(Move) == to) {
            PromPce = COLORLESSPROMOTED(Move);
            if (PromPce != EMPTY) {
                if ((PromPce == ROOK && ptrChar[4] == 'r')
                    || (PromPce == BISHOP && ptrChar[4] == 'b')
                    || (PromPce == QUEEN && ptrChar[4] == 'q')
                    || (PromPce == KNIGHT && ptrChar[4] == 'n')) {
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

    for (i = 0; i < mcount; ++i) {
        pickMove(m, i, mcount);
        move_make(&m[i]);
        if (isAttacked(board.sideToMove, kingLoc[1 - (board.sideToMove >> 3)])) {
            //printf("ILLEGAL %s %d\n",moveToUCI(m[i].move),m[i].score);
            move_unmake(&m[i]);
            continue;
        }
        printf("%s %d (move in decimal if you want a breakpoint %d ;-)\n", moveToUCI(m[i].move), m[i].score, m[i].move);
        move_unmake(&m[i]);
    }
}

void ParsePosition(char *lineIn) {

    lineIn += 9;
    char *ptrChar;

    if (strncmp(lineIn, "startpos", 8) == 0) {
        board.gameply = 0;
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
    unsigned int move;

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

#ifdef __linux__

#include <readline/readline.h>
#include <readline/history.h>

// TODO: Autocomplete internal commands
// See here https://eli.thegreenplace.net/2016/basics-of-using-the-readline-library/
char *getInput(void) {
    char *inpt = readline("");
    if (inpt && *inpt)
        add_history(inpt);
    return inpt;
}

#endif

void input_loop(S_SEARCHINFO *info) {
    int exit = FALSE;
    char line[INPUTBUFFER];

    printf("%s, written by Alex Argiropoulos "
           "(compiler's version used: "__VERSION__")\n", NAME);
    // printf("%s\n", getCPUModel());
#ifndef NDEBUG
    printf("Terminal is %s\n", isatty(fileno(stdout)) ? "true" : "false");
    printf("Terminal is %s (stdin)\n", isatty(fileno(stdin)) ? "true" : "false");
    printf("NOTE: NDEBUG not defined at compilation stage."
           "  Performance will not be optimum!\n");
#endif
    while (!exit) {
        memset(line, 0, sizeof(line));
        fflush(stdout);
#ifdef __linux__
        if (isatty(fileno(stdin))) {
            char *inp = getInput();
            if (!inp) {
                exit = TRUE;
                continue;
            }
            strncpy(line, inp, sizeof(line) - 1);
        } else {
            if (!fgets(line, INPUTBUFFER, stdin))
                continue;
        }
#else
        if (!fgets(line, INPUTBUFFER, stdin))
            continue;
#endif
        if (line[0] == '\n')
            continue;
        if (!strncmp(line, "isready", 7)) {
            printf("readyok\n");
            continue;
        } else if (!strncmp(line, "uci", 3)) {
            if (!strncmp(line, "ucinewgame", 10)) {
                TT_clear();
                ParsePosition("position startpos\n");
            } else {
                printf("id name %s\n", NAME);
                printf("id author Alex Argiropoulos GR\n");
                printf("uciok\n");
                info->GAME_MODE = GAMEMODE_UCI;
            }
        } else if (!strncmp(line, "position", 8)) {
            ParsePosition(line);
        } else if (!strncmp(line, "go", 2)) {
            ParseGo(line, info);
        } else if (!strncmp(line, "perft", 5)) {
            ParsePerft(line);
        } else if (!strncmp(line, "score", 5)) {
            ParseScore(line, info);
        } else if (!strncmp(line, "divide", 6)) {
            ParseDivide(line);
        } else if (!strncmp(line, "movelist", 8)) {
            showMoveList();
        } else if (!strncmp(line, "d", 1)) {
            printBoard();
        } else if (!strncmp(line, "ls", 2)) {
            listMoves();
        } else if (!strncmp(line, "testepd", 7)) {
            ParseTestEPD(line);
        } else if (!strncmp(line, "eval", 4)) {
            printf("Eval=%d\n", Evaluate());
        } else if (!strncmp(line, "version", 7)) {
            printf("%s\n%s\n- 0x88 board\n"
                   "- Iterative deepening\n"
                   "- Null move reduction\n"
                   "- Futility pruning\n"
                   "- Razoring pruning\n"
                   "- MVV/LVA\n"
                   "- Killer heuristics\n"
                   "- History heuristics\n"
                   "- Transposition tables\n"
                   "- Late Move Reductions\n",
                   FULLNAME,
                   getCPUModel());
        } else if (!strncmp(line, "quit", 4)) {
            exit = TRUE;
            break;
        }
    }
}
