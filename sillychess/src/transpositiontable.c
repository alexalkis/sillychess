/*
 * transpositiontable.c
 *
 *  Created on: Jun 8, 2014
 *      Author: alex
 */

#include "sillychess.h"


void TT_free(void)
{
	if (board.ht)
		free(board.ht);
}

void TT_set_size(unsigned int mbSize) {

  u64 newSize = 1024;


  while (2ULL * newSize * sizeof(Hash_Entry) <= (mbSize << 20))
      newSize *= 2;

  /*
   * newSize is a power of two. Normally we get the index to the hash table like board.posKey % board.htSize
   * But, if we have a size of power of two minus 1, we can replace it with board.posKey & board.htSize
   * which is faster.
   */
  if (board.ht) free(board.ht);

  board.ht = (Hash_Entry *)malloc(newSize*sizeof(Hash_Entry));
  --newSize;  /* alloc size is power of two, we substract one so hash & newsize will always be within array limits */
  board.htSize=newSize;

  if (!board.ht)
  {
      printf("Hash table memory allocation problem.  Exiting...\n");
      exit(EXIT_FAILURE);
  } else
      printf("Transposition Table: %"INT64_FORMAT"d entries allocated...%"INT64_FORMAT"d bytes.\n",newSize,newSize*sizeof(Hash_Entry));
  memset(board.ht, 0, newSize * sizeof(Hash_Entry));
}


void TT_clear(void)
{
	if (board.ht)
		memset(board.ht, 0, board.htSize * sizeof(Hash_Entry));
}

void TT_RecordHash(int depth, int score, int hashf, unsigned int best)
{
        Hash_Entry *phashe = &board.ht[board.posKey & board.htSize];

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
//        	fprintf(f,"%s %s %"INT64_FORMAT"X (D:%d Score:%d) Move in decimal: %d\n",qprintMove(best),board2fen(),board.posKey,depth,value,best);
//        	fclose(f);
//        }
}

Hash_Entry * TT_probe(unsigned int *move, int *score, int depth, int alpha, int beta) {
	Hash_Entry *phashe = &board.ht[board.posKey & board.htSize];
	if (phashe->key == board.posKey) {
		*move = phashe->bestMove;
		if (phashe->depth >= depth) {
			if (phashe->flags == hashfEXACT) {
				*score = phashe->value;
				if (*score > ISMATE)
					*score -= board.ply;
				else if (*score < -ISMATE)
					*score += board.ply;
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
	}
	return NULL;
}

void TT_fillPVLineFromTT(int deep, LINE *tLine)
{
	int i;
	int moveCounter=0;
	smove m[MAXDEPTH];
	int score;
	unsigned int BestMove;

	for (i=0; i<deep; ++i) {
		BestMove=NOMOVE;
		TT_probe(&BestMove,&score,0,0,0);
		if (BestMove==NOMOVE) break;
		//printf("Got move for position %d\n",i);
		if (moveExists(BestMove)) {
			m[moveCounter].move=BestMove;
			//printf("Before making move %"INT64_FORMAT"X\n",board.posKey);
			move_make(&m[moveCounter]);
			//printf("After  making move %"INT64_FORMAT"X\n",board.posKey);
			++moveCounter;
			//smove n;n.move=BestMove;printMove(n);printf(" <--- move legal\n");
		} else {
			smove n;
			n.move=BestMove;
			printBoard();
			printMove(n);printf(" <--- move not legal\n");
			break;
		}
	}

	if (moveCounter) {
		for(i=moveCounter-1; i>=0; --i)
			move_unmake(&m[i]);

		for(i=0; i<moveCounter; ++i)
			tLine->argmove[i]=m[i].move;
		tLine->cmove=moveCounter;
		//printf(" Moves: %d\n",moveCounter);
		//printLine(&tLine);
	} else {
		printf("No moves from hash table probing!!!\n");
	}
}
