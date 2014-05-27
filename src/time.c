/*
 * time.c
 *
 *  Created on: May 19, 2014
 *      Author: alex
 */


#ifndef __AMIGA__
#include <stdlib.h>
#include <sys/timeb.h>
int get_ms()
{
	struct timeb timebuffer;
	ftime(&timebuffer);

	return (timebuffer.time * 1000) + timebuffer.millitm;
}
#else
//#include <exec/types.h>
//#include <exec/exec.h>
#include <dos/dos.h>
#include <proto/dos.h>
int get_ms()
{
  struct DateStamp now;
  DateStamp(&now);
  return now.ds_Minute*60000+now.ds_Tick*20; /*( 20 = 1000 / 50(tickspersec))*/
}
#endif
