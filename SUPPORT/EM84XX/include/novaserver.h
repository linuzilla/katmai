
#define DEATHMESSAGE 0xdeaddead
#define UNUSED_ARG     0xDeadBeef

#define SUICIDE(x) Suicide(x,x ## RC)

#define ERRORSIGINTRC 7
#define ERRORMAXthread "reached max thread count"
#define ERRORMAXthreadRC 8
#define ERRORMAXsemaphore "reached max semaphore count"
#define ERRORMAXsemaphoreRC 9
#define ERRORMAXmessagequeue "reached max message queue count"
#define ERRORMAXmessagequeueRC 10
#define ERRORMAXdmabuf "reached max dma buffer count"
#define ERRORMAXdmabufRC 11
#define ERRORPTHREADCREATEFAILURE "pthread_create failed"
#define ERRORPTHREADCREATEFAILURERC 12
#define ERRORMAXMSGLENGTHEXCEEDED "exceeded max message length"
#define ERRORMAXMSGLENGTHEXCEEDEDRC 13
#define ERRORUNKNOWNFMPCOMMAND "unknown FMP command"
#define ERRORUNKNOWNFMPCOMMANDRC 14
#define ERRORTOOMUCHEINTR "catching too much EINTR"
#define ERRORTOOMUCHEINTRRC 15
#define ERRORLIBC "error in libc function call"
#define ERRORLIBCRC 16
#define ERRORBADTRANSLATION "bad virtual/physical translation attempt"
#define ERRORBADTRANSLATIONRC 17
#define ERRORCANNOTOPENDVD "cannot open DVD"
#define ERRORCANNOTOPENDVDRC 18
#define ERRORCANNOTOPENQUASAR "cannot open quasar"
#define ERRORCANNOTOPENQUASARRC 19
#define ERRORBADREGION "player region code is not compatible with allowed region codes on DVD"
#define ERRORBADREGIONRC 20
#define ERRORREGIONTOOMANYCHANGES "out of allowed region code changes"
#define ERRORREGIONTOOMANYCHANGESRC 21
#define ERRORBADDMA "bad DMA"
#define ERRORBADDMARC 22
#define ERRORUNKNOWNRPCONMSGQ "received bad rpc from message queue"
#define ERRORUNKNOWNRPCONMSGQRC 23

#define PERRORIZE(x) { perror(x); SUICIDE(ERRORLIBC); }

#define INFOFORSTDOUT(sometextandargs...) printf(sometextandargs)

#define INFOFORSTDOUT(sometextandargs...) printf(sometextandargs)
#define SUICIDE(x) Suicide(x,x ## RC)

/// ok for the following
#ifndef _LOCAL_TYPES_
#define _LOCAL_TYPES_
typedef int STATUS,FILEHANDLE,BOOLEAN;

typedef unsigned char UCHAR, *PUCHAR;
typedef char CHAR, *PCHAR;

typedef short *PSHORT;
typedef unsigned short USHORT;

typedef long *PLONG;
typedef unsigned long ULONG,LCID, *PULONG;
typedef unsigned long *PDWORD;

typedef long long LONGLONG;
typedef unsigned long long LARGE_INTEGER,MPEG_SCR,INT64,DWORDLONG;
typedef unsigned long long *PDWORDLONG,*PLARGE_INTEGER;

typedef void VOID, *PVOID, *HGLOBAL, *HANDLE;
#endif /*_LOCAL_TYPES_*/

// Enhanced type definitions
typedef void *(*start_routine_type)(void *);
typedef start_routine_type FUNCPTR;
typedef unsigned long PLATFORM_MEMORY_ADDRESS;
typedef void (*INTERRUPT_HANDLER)(int HwContext);
typedef void (*VOIDFUNCPTR)(void);

typedef struct {
	int inuse;
	pthread_t t;
	int sched_priority;
	pid_t p;
	sem_t cleanup_completed;
	start_routine_type sr;
	void *arg;
} thread_helper;

typedef struct {
	int inuse;
	int q;
} messagequeue_helper;

typedef struct {
	int inuse;
	sem_t s;
} semaphore_helper;

typedef thread_helper *THREADHANDLE;
typedef semaphore_helper *SEMAPHORE_OBJECT,*CEvent;
typedef messagequeue_helper *MESSAGE_QUEUE_OBJECT;

THREADHANDLE OSCreateThread(
  TCHAR *name,
  unsigned long dwStackSize,	// initial thread stack size
  FUNCPTR lpStartAddress,	// pointer to thread function
  int lpParameter		// argument for new thread
 );
void OSCancelThread(THREADHANDLE hThread);
MESSAGE_QUEUE_OBJECT OSCreatemessagequeue(key_t key, int msgflg);
void OSDestroymessagequeue(MESSAGE_QUEUE_OBJECT pm);
void OSPostInmessagequeue(MESSAGE_QUEUE_OBJECT pm,char *msg,int length,long type);
void OSBlockingReceiveFrommessagequeue(MESSAGE_QUEUE_OBJECT pm,char *msg,int length,long type);
void Suicide(char *x,int rc);
