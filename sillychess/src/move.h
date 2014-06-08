/*
 * move.h
 *
 *  Created on: May 27, 2014
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

#define COLORLESSPIECE(x)	((x>>16)&0x07)
#define PIECE(x)			((x>>16)&0x0f)
#define ISCAPTURE(x)		((x>>20)&0x0f)
#define ISCAPTUREORPROMOTION(x)		((x>>20)&0xff)
#define FROM(x)				((x&0xff))
#define TO(x)				(((x>>8)&0xff))
#define SPECIAL(x)			((x>>28) & 0x0f)
#define PROMOTED(x)			((x>>24) & 0x0f)
#define ISPROMOTION(x)		((x>>24) & 0x0f)
#define COLORLESSPROMOTED(x)	((x>>24) & 0x07)



#endif /* MOVE_H_ */
