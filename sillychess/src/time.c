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
    info->stopped = SC_TRUE;
    info->displayCurrmove=SC_FALSE;
    //printf("Crap input found triggered!!\n");
    do {
      bytes=read(fileno(stdin),input,256);
    } while (bytes<0);
    //printf("****************************** \"%s\"",input);
    endc = strchr(input,'\n');
    if (endc) *endc=0;

    if (strlen(input) > 0) {
      if (!strncmp(input, "quit", 4))    {
	info->quit = SC_TRUE;
      }
    }
    return;
  }
}


#ifdef ALKIS
/*  AUTHOR
 *        Anders 'ALi' Lindgren
 *        Sweden
 *        Tel: (Sweden) 08 876 678
*/

#include <exec/types.h>

#include <exec/memory.h>

#include <libraries/dos.h>
#include <libraries/dosextens.h>

#include <proto/exec.h>
#include <proto/dos.h>

extern int __builtin_strlen(char *);

/*
 *  DEFINITION
 *        DOS_PRINT
 *
 *  DESCRIPTION
 *        A simple string print routine, which uses the dos.library
 *        functions. (Make sure you'we got a window to print to.)
 */

#define DOS_PRINT(x)        Write(Output(),x,__builtin_strlen(x))

extern BOOL set_screen_mode (BOOL);
extern LONG send_packet     (LONG, LONG *, struct MsgPort *);

struct DOSBase * DOSBase;


/*
 *  FUNCTION
 *        not_main
 *
 *  DESCRIPTION
 *        This function simply demonstrates the abilities of RAW:.
 *        It sets the console into RAW: mode and it waits for a
 *        charachter to be typed by the user. It then print out
 *        the character and resets the console into CON: mode.
 *
 *  NOTE
 *        This function is named "not_main", since I doesn't use
 *        the normal C startup code. This way I can see if something
 *        went wrong during the linkage stage.
 */

int
not_main(void)
{
    char ch;

    if (DOSBase = (struct DOSBase *)OpenLibrary("dos.library", 0L)) {

        if (set_screen_mode(TRUE)) {        /* Set the console into RAW: mode */

            DOS_PRINT("ConHotKey, by Anders Lindgren\n");
            DOS_PRINT("Press a key within 15 sek!\n");


                /*
                 * Get a "hot" key from the console.
                 */

            if (WaitForChar(Input(), 15*1000000) == -1) {
                Read(Input(), & ch, 1);
                DOS_PRINT("You pressed:");
                Write(Output(), & ch, 1);
                DOS_PRINT("\n");
            }
            else {
                DOS_PRINT("No key pressed!\n");
            }

            set_screen_mode(FALSE);        /* Set it back to CON: */
        }
        CloseLibrary((struct Library *) DOSBase);
    }
    return(0);        /* Exit the program. */
}


/*
 *  FUNCTION
 *        set_screen_mode
 *
 *  DESCRIPTION
 *        This function sets the screenmode of the con:/raw: device
 *        associated with the current process.
 *        If the "type" argument is TRUE, the console if switched
 *        into raw mode. If it's FALSE it's switched into CON: mode.
 *
 *  RESULT
 *        TRUE if a console exists, and FALSE if it fails.
 *        (I don't check the returnvalue since this packet
 *        is so badly documented. Nobode seems to know
 *        what it returns, Lets hope the documentation for 2.0
 *        are better.)
 */

BOOL
set_screen_mode(BOOL type)
{
    register LONG               args[7];
    register struct Process * proc;

    proc = (struct Process *)FindTask(NULL);

    if (proc->pr_ConsoleTask) {
        args[0] = type;
        send_packet(ACTION_SCREEN_MODE, args,
                        (struct MsgPort *) proc->pr_ConsoleTask);

        return(TRUE);
    }
    return(FALSE);
}


/*
 *  FUNCTION
 *        send_packet
 *
 *  DESCRIPTION
 *        A general purpose packet sending routine.
 *
 *  SYNOPSIS
 *        LONG send_packet(LONG action, LONG * args, struct MsgPort * handler)
 *
 *        action              - Which action to performed, normally
 *                        a ACTION_* definition.
 *        args              - A pointer to the seven arguments. These
 *                        are copied into the actual packet structure.
 *        handler              - The adress of the recieving handlers message port.
 *
 *  RETURNS
 *        dp_Res1 of the returned packet.
 *        A call to IoErr() returns the errorcode.
 *        If this routine failed to allocate memory, a zer0 is returned,
 *        and IoErr() returns ERROR_NO_FREE_STORE (i.e. out of memory).
 *
 *  NOTE
 *        This function is written to be compatible with the Arp
 *        SendPacket() function.
 */

LONG
send_packet( LONG action, LONG * args, struct MsgPort * handler)
{
    register struct Process           * proc;
    register struct StandardPacket * packet;
    register LONG                     res1;

    proc = (struct Process *)FindTask(NULL);

    if (packet = (struct StandardPacket *)AllocMem( sizeof(struct StandardPacket),
                                MEMF_CLEAR | MEMF_PUBLIC) ) {

        packet->sp_Msg.mn_Node.ln_Name        = (char *)&(packet->sp_Pkt);
        packet->sp_Pkt.dp_Link                = & packet->sp_Msg;
        packet->sp_Pkt.dp_Port                = & proc->pr_MsgPort;
        packet->sp_Pkt.dp_Type                = action;

        if (args) {
            CopyMem ((char *)args, (char *)& packet->sp_Pkt.dp_Arg1,
                                                         7*sizeof(LONG));
        }

        /*
         *        If the user has got a special Packet recieving
         *        routine, call it.
         */

        PutMsg (handler, & packet->sp_Msg);
        if (proc->pr_PktWait) {
            ( * ((struct Message (*) (void)) proc->pr_PktWait) ) ();
        }
        else {
            WaitPort (& proc->pr_MsgPort);
            GetMsg (& proc->pr_MsgPort);
        }

        /*
         *        Store the secondary result so that the program
         *        can get it with IoErr()
         */

        ((struct CommandLineInterface *)BADDR(proc->pr_CLI))->
                                cli_Result2 = packet->sp_Pkt.dp_Res2;

        res1 = packet->sp_Pkt.dp_Res1;

        FreeMem( (char *)packet, sizeof(struct StandardPacket) );
    }
    else {        /* No memory */
        res1 = 0;
        ((struct CommandLineInterface *)BADDR(proc->pr_CLI))->
                                cli_Result2 = ERROR_NO_FREE_STORE;
    }

    return(res1);
}
#endif