/*
 * time.c
 *
 *  Created on: May 27, 2014
 *      Author: alex
 */

#include "sillychess.h"
#ifdef WIN32
#include "windows.h"
#else
#include "sys/time.h"
#include "sys/select.h"
#include "unistd.h"
#include "string.h"
#endif

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

// http://home.arcor.de/dreamlike/chess/
int InputWaiting(void)
{
#ifndef WIN32
  fd_set readfds;
  struct timeval tv;
  FD_ZERO (&readfds);
  FD_SET (fileno(stdin), &readfds);
  tv.tv_sec=0; tv.tv_usec=0;
//  int a=get_ms();
  select(16, &readfds, 0, 0, &tv);
//  int b=get_ms();
//  b-=a;
//  if (b)
//	  printf("select needed %d milliseconds.\n",b);

  return (FD_ISSET(fileno(stdin), &readfds));
#else
   static int init = 0, pipe;
   static HANDLE inh;
   DWORD dw;

   if (!init) {
     init = 1;
     inh = GetStdHandle(STD_INPUT_HANDLE);
     pipe = !GetConsoleMode(inh, &dw);
     if (!pipe) {
        SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
        FlushConsoleInputBuffer(inh);
      }
    }
    if (pipe) {
      if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
      return dw;
    } else {
      GetNumberOfConsoleInputEvents(inh, &dw);
      return dw <= 1 ? 0 : dw;
	}
#endif
}


void ReadInput(S_SEARCHINFO *info) {
  int             bytes;
  char            input[256] = "", *endc;

    if (InputWaiting()) {
		info->stopped = TRUE;
		do {
		  bytes=read(fileno(stdin),input,256);
		} while (bytes<0);
		//printf("****************************** \"%s\"",input);
		endc = strchr(input,'\n');
		if (endc) *endc=0;

		if (strlen(input) > 0) {
			if (!strncmp(input, "quit", 4))    {
			  info->quit = TRUE;
			}
		}
		return;
    }
}
