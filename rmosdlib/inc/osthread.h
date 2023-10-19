/*****************************************************************************/

/*
 *      osthread.h -- REALmagic Celeste Application
 *
 *      Copyright (C) 1999-2000 Sigma Designs
 *                   written by Pascal Cannenterre <pascal_cannenterre@sdesigns.com>
 *     
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*****************************************************************************/
#define _LARGEFILE64_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <termio.h>

#include <sys/errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <pthread.h>
#include <semaphore.h>
#include <getopt.h>

#define _DEFINE_FMP_TYPES_
#include "fmp.h"

// macros
#define interface struct
#define CONST_VTBL
#define __RPC_FAR
#define BEGIN_INTERFACE
#define END_INTERFACE

#ifdef __cplusplus
#define EXTERN_C    extern "C"
#else
#define EXTERN_C    extern
#endif

#define _tprintf printf
#define _stprintf sprintf

#define UNUSED_ARG 0xdeadbeef

#define THREAD_SELF_ID UNUSED_ARG

#define Malloc(type,n) (type *)malloc(n*sizeof(type))

//#define OSD_MANAGER_THREAD_PRIORITY UNUSED_ARG
#define TIMER_THREAD_PRIORITY UNUSED_ARG
#define MAIN_THREAD_PRIORITY UNUSED_ARG
//#define PORT_READING_THREAD_PRIORITY UNUSED_ARG

#define S_OK 0
#define ERROR 1

#ifndef max
#define max(a,b)	(((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif

// notice: remote control serial port is chosen by environment REMOTECONTROLPORT
// #define REM_PORT NULL 
// #define KBD_PORT "/dev/console"

#define TEXT(quote) quote

#define VOID void

// int types
typedef int PORTHANDLE,HRESULT;

// 8bit types
typedef char *PCHAR,*LPCTSTR;
typedef unsigned char *PBYTE;
typedef char CHAR;

// 16bit types
typedef unsigned short USHORT;

// 32bit types
// typedef long LONG;
typedef unsigned long ULONG;
typedef unsigned long UINT;
typedef long INT;

// pointer types
typedef void *LPVOID;

// function pointer type
typedef void *(*start_routine_type)(void *);
typedef start_routine_type FUNCPTR;

// MULTITHREADING FUNCTIONS

/* IMPORTANT NOTE ABOUT HOW LINUX HANDLES MULTITHREADING 

   Usually multithreading under Linux is handled by the pthread library.

   pthread library authors do not recommend to mix multithreading
   and signals; but people appreciate correct handling of most used 
   signals (Control-C, KILL).

   To solve this problem, libEM8400 maintains a table of spawned threads 
   and manages them properly. To handle threads use the following functions:
 */

// fmp os exposed types and functions:

/* those defines comes from linuxbase.h */
#define MYSIGZONECHOOSE0 SIGXCPU
#define MYSIGZONECHOOSE1 SIGXFSZ
#define MYSIGZONECONFIRM SIGVTALRM

typedef void * THREADHANDLE,*MESSAGE_QUEUE_OBJECT;

THREADHANDLE OSCreateThread(char *name,unsigned long dwStackSize,
			    FUNCPTR lpStartAddress,int lpParameter);
void OSCancelThread(THREADHANDLE hThread);

/* #define MSGOTHERKIND 4012 */
/* #define REASONABLE_UMASK 0644 */
/* #define MAXMSGLENGTH 320 */
/* MESSAGE_QUEUE_OBJECT OSCreatemessagequeue(key_t key, int msgflg); */
/* unsigned long OSDestroyMessageQueue(MESSAGE_QUEUE_OBJECT MsgQ); */
/* void OSPostInmessagequeue(MESSAGE_QUEUE_OBJECT pm,char *msg,int length,long type); */
/* void OSBlockingReceiveFrommessagequeue(MESSAGE_QUEUE_OBJECT pm,char *msg,int length,long type); */
// fmp os exposed types and functions (over)

