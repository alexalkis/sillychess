/*
 * time.c
 *
 *  Created on: May 27, 2014
 *      Author: alex
 */

#include "sillychess.h"
#ifdef WIN32
#include "windows.h"
#include <unistd.h>
#else
#ifndef __AMIGA__
	#include "sys/time.h"
	#include <sys/types.h>
	#include <unistd.h>
	#include <errno.h>
	#include <stdio.h>
#endif
#endif

#ifndef __AMIGA__
#include <stdlib.h>
#include <sys/timeb.h>

unsigned int get_ms()
{
	struct timeb timebuffer;
	ftime(&timebuffer);

	return (timebuffer.time * 1000) + timebuffer.millitm;
}
#else

#include <dos/dos.h>
#include <proto/dos.h>

int read(int fd, void *buf, size_t count); /* can't include <unistd.h> */
/* has a conflict with timer */

unsigned int get_ms()
{
  struct DateStamp now;
  DateStamp(&now);
  return now.ds_Minute*60000+now.ds_Tick*20; /*( 20 = 1000 / 50(tickspersec))*/
}

#endif

/* http://home.arcor.de/dreamlike/chess/ */
int InputWaiting(void)
{
#ifdef __AMIGA__
	return 0;
#else

#ifndef NDEBUG
	static int numIntrSignal=0;
#endif

#ifndef WIN32
  fd_set readfds;
  struct timeval tv;
  FD_ZERO (&readfds);
  FD_SET (fileno(stdin), &readfds);
  tv.tv_sec=0; tv.tv_usec=0;
//  int a=get_ms();
  int ret=select(16, &readfds, 0, 0, &tv);
  if (ret==-1) {
#ifndef NDEBUG
	  switch(errno) {
	  case EBADF:
		  printf("Bad file number???\n");
		  break;
	  case EINTR:
		  printf("******************************* Interrupt signal #%d\n",++numIntrSignal);
		  return 0;	//HACK
		  break;
	  case EINVAL:
		  printf("The timeout argument is invalid; one of the components is negative or too large.\n");
		  break;
	  }
#else
	  // in release build just return 0 when select errors out.
	  return 0;
#endif
  }

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
#endif
}


void ReadInput(S_SEARCHINFO *info) {
  int  bytes;
  char input[256] = "", *endc;

  if (InputWaiting()) {
    info->stopped = TRUE;
    info->displayCurrmove=FALSE;
    //printf("Crap input found triggered!!\n");
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
