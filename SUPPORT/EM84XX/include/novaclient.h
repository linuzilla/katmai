#define UNDETERMINED_STREAM_TYPE 0xfeeddeaf
#define UNSUPPORTED_STREAM_TYPE  0xdeaeeeed

extern char *REALmagicdevice;

void FMPConnectToMessageQueue();

void FMPConnectToMSGQUEUE();
void FMPDestroyMSGQUEUE();
void waitForAnalogOverlayAccess();

unsigned long getStreamType(char *);
