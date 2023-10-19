// we are sure that we can attach to the inode of the device we open!
#define DEFAULTREALMAGICDEVICE "/dev/realmagic0"
extern char defaultREALmagicdevice[];
#define REALMAGICDEVICEFROMENV "USE_REALMAGIC8400"

#define IPC_PROJECT_KEY 'B'
#define REASONABLE_UMASK 0644 /* rw-r--r-- */

#define MSGFROMCLIENTTOSERVER 1
#define MSGFROMSERVERTOCLIENT 2
#define MSGFROMCLIENTTOPLAYFILESERVER 3
#define MSGFROMPLAYFILESERVERTOCLIENT 4
/* I could have found more original names ... But that works anyway !*/
#define MSGFROMCLIENTTOPLAYFILESERVER_PROPERTY 5
#define MSGFROMPLAYFILESERVERTOCLIENT_PROPERTY 6
#define MSGOTHERKIND 4012

#define MAXMSGLENGTH 320
#define MAXFILENAME 288

typedef struct tagmsgbigbuf {
	long mtype;
	char mtext[MAXMSGLENGTH];
} msgbigbuf;

typedef struct{
  unsigned long dwPlayCommand;
  char filename[MAXFILENAME];
} ClientPlayRequest;

typedef struct {
	unsigned long dwCommand;
	unsigned long dwArg1;
	unsigned long dwArg2;
	unsigned long dwArg3;
	unsigned long dwArg4;
	unsigned long dwArg5;
	unsigned long dwArg6;
} ClientQuestion;

typedef struct {
	unsigned long dwReturn;
} ServerAnswer;
