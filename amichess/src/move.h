/*
 * move.h
 *
 *  Created on: May 14, 2014
 *      Author: alex
 */

#ifndef MOVE_H_
#define MOVE_H_
/*
Number of bits		Position of bits	Description
8					0-7					fromSquare
8					8-15				toSquare
4					16-19				piece
4					20-23				capturedPiece
4					24-27				promotedPiece
4					28-31				special

Special can be:
1 - castle
2 - en passant
*/

enum {SP_NONE=0,SP_CASTLE, SP_ENPASSANT};

#endif /* MOVE_H_ */
