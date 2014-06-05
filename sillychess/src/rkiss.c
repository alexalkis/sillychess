/*
 * rkiss.c
 *
 *  Created on: Jun 3, 2014
 *      Author: lifted from stockfish source  code
 */
#include "sillychess.h"

static u64 a, b, c, d;

static u64 rotate_L(u64 x, unsigned k)
{
	return (x << k) | (x >> (64 - k));
}

u64 rand64(void)
{

	const u64 e = a - rotate_L(b, 7);
	a = b ^ rotate_L(c, 13);
	b = c + rotate_L(d, 37);
	c = d + e;
	return d = e + a;
}

void rkissSeed(int seed)
{
	int i;
	if (seed == 0)
		seed = 73;
	a = 0xF1EA5EED, b = c = d = 0xD4E12C77;
	for (i = 0; i < seed; ++i) // Scramble a few rounds
		rand64();
}
