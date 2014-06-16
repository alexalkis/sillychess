/*
 * transpositiontable.c
 *
 *  Created on: Jun 8, 2014
 *      Author: alex
 */
#include "sillychess.h"
#include <stdlib.h>
#include <string.h>



void TT_set_size(int mbSize) {

  int newSize = 1024;


  while (2ULL * newSize * sizeof(HASHE) <= (mbSize << 20))
      newSize *= 2;


  if (board.ht) free(board.ht);

  board.ht = (HASHE *)malloc(newSize*sizeof(HASHE));
  board.htSize=newSize;

  if (!board.ht)
  {
      printf("Hash table memory allocation problem.  Exiting...\n");
      exit(EXIT_FAILURE);
  } else
      printf("Transposition Table: %d entries allocated...%ld bytes.\n",newSize,newSize*sizeof(HASHE));
  memset(board.ht, 0, newSize * sizeof(HASHE));
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

void TT_clear(void)
{
	if (board.ht)
		memset(board.ht, 0, board.htSize * sizeof(HASHE));
}

void TT_RecordHash(int depth, int score, int hashf, int best)
{
        HASHE *phashe = &board.ht[board.posKey % board.htSize];

        /* if an entry arrives at a shalower depth for this position we just return (and keep the deeper entry) */
        if (phashe->key==board.posKey && phashe->depth>depth) {
        	return;
        }

        //if (hashf!=hashfEXACT) best=NOMOVE;

        if(score > ISMATE)
        	score += board.ply;
        else if(score < -ISMATE)
        	score -= board.ply;

        phashe->key = board.posKey;
        phashe->bestMove = best;
        phashe->value = score;
        phashe->flags = hashf;
        phashe->depth = depth;
//        FILE *f = fopen("/home/alex/foo.txt","a");
//        if (f) {
//        	fprintf(f,"%s %s %llX (D:%d Score:%d) Move in decimal: %d\n",qprintMove(best),board2fen(),board.posKey,depth,value,best);
//        	fclose(f);
//        }
}

HASHE * TT_probe(int *move, int *score, int depth, int alpha, int beta)
{
	HASHE *phashe = &board.ht[board.posKey % board.htSize];
	if (phashe->key == board.posKey && phashe->depth >= depth) {
		*move = phashe->bestMove;
		if (phashe->flags == hashfEXACT) {
			*score = phashe->value;
			if(*score > ISMATE)
				*score -= board.ply;
			else if(*score < -ISMATE)
				*score += board.ply;
//			if (*score>alpha && *score<beta)
//				return TRUE;
//			else
//				return FALSE;
			return phashe;
		}
		if ((phashe->flags == hashfALPHA) && (phashe->value <= alpha)) {
			*score = alpha;
			return phashe;
		}
		if ((phashe->flags == hashfBETA) && (phashe->value >= beta)) {
			*score = beta;
			return phashe;
		}
	}
	return NULL;
}
