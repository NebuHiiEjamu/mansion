/* FrontEnd.h */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <arpa/inet.h>

/* !! define basic data types LONG Ptr etc. */
#include "mansion.h"
#include "s-local.h"
#include "frontendcmds.h"


#define DEFAULT_PALACE_TCP_PORT			9998
#define DEFAULT_PALACE_FRONTEND_PORT		10001

#define ClientRcveBufferLength    		20000
#define ServerRcveBufferLength			32000
#define ServerSendBufferLength        	32000
#define ClientSendBufferLength          16384

/* 8/20/96 JAB
   To reduce flushes, which cause lag,
   send buffers will dynamically increase up to MaxSendBufferLength
   in increments of BufferSizeIncrement as needed 

   TimeOut reduced from 15 to 5 because 99% of flushes are now
   timing out at 15.

 */

#define MaxSendBufferLength	131072
#define BufferSizeIncrement	16384
#define FlushTimeOut		5


#define UA_HideFrom		0x01
#define UA_Mute			0x02
#define UA_Follow		0x04
#define UA_Kick			0x08

typedef struct SockBuff {
	int		socket,ipAddr;
	unsigned short ipPort;
	unsigned char	*tcpReceiveBuffer;
	unsigned char	*groupBuffer;
	int				maxInLength,maxOutLength;
	int				tcpReceiveIdx;
	int				groupLen,groupFlag;
	int				commErrFlag;
	int				dataInCounter,dataOutCounter;
} SockBuffers, *SockBuffersPtr;

typedef struct FEUser {
	struct FEUser	*nextUser;
	unsigned short	userRoomID;
	LONG			userID;
	LONG			userFlags;
	int				killFlag,killReason,notifyServer;
	SockBuffers	sockBuf;
	int		active;
} FEUser, *FEUserPtr;

typedef struct FEActionRec {
	struct FEActionRec	*nextRec;
	LONG	srcUser;
	LONG	dstUser;
	LONG	action;
} FEActionRec, *FEActionPtr;

extern fd_set			gAfds,gRfds;

/* error and logging */
void OpenLog(char *name);
void CloseLog(void);
void err_sys(char *str);
void err_dump(char *str);
void LogStringToFile(char *str,...);
void ErrorMessage(char *str,...);
void ReportError(short code, char *name);
void ErrorExit(char *str,...);
void LogMessage(char *str,...);
void LogString(char *str);
void TimeLogMessage(char *str,...);

/* user stuff */
FEUserPtr NewFEUser(void);
FEUserPtr GetFEUser(int ipAddr, unsigned short ipPort);
void FEAcceptNewUser(void);
void ScheduleUserKill(FEUserPtr cUser, int why, int notifyServer);
void TerminateUser(FEUserPtr dUser, int reason, int notifyServer);
void FEUserRead(FEUserPtr cUser);

/* server stuff */
void FEServerRead(void);

void Process_bi_room(BiHeader *hdrPtr, Ptr p);
void Process_bi_packet(BiHeader *hdrPtr, Ptr p);
void Process_bi_assoc(BiHeader *hdrPtr, Ptr p);
void Process_bi_userflags(BiHeader *hdrPtr, Ptr p);
void Process_bi_kill(BiHeader *hdrPtr, Ptr p);
void Process_bi_global(BiHeader *hdrPtr, Ptr p);
void Process_bi_serverdown(BiHeader *hdrPtr, Ptr p);
void Process_bi_serverfull(BiHeader *hdrPtr, Ptr p);
void Process_bi_serveravail(BiHeader *hdrPtr, Ptr p);
void Process_bi_begingroup(BiHeader *hdrPtr, Ptr p);
void Process_bi_endgroup(BiHeader *hdrPtr, Ptr p);
void Process_bi_addaction(BiHeader *hdrPtr, Ptr p);
void Process_bi_delaction(BiHeader *hdrPtr, Ptr p);


/* shell stuff */
void FrontEndIdle(void);
void InitDaemon();
void InitFrontEnd(int listenPort, int serverPort, char *servHostAddr);
void CloseFrontEnd(void);
int main(int argc, char **argv);

/* SOCK STUFF (frontendsock.c - self contained) */
void InitSockBuffers(int socket, int ipAddr, int ipPort, 
					 int maxIn, int maxOut, SockBuffersPtr sp);
void CloseSockBuffers(SockBuffersPtr sp);
int PostSockTCPBuffer(SockBuffersPtr	sp, int *bytesWritten);
void SendSockGroupBuffer(SockBuffersPtr sp);
void SockIdle(SockBuffersPtr sp);
void BeginSockGroup(SockBuffersPtr sp);
void EndSockGroup(SockBuffersPtr sp);
void FlushSockGroupBuffer(SockBuffersPtr sp, LONG spaceNeeded);
void PostSockBuffer(SockBuffersPtr sp, Ptr bufferContents, LONG bufferLength);
void PostSockPalaceEvent(SockBuffersPtr sp, unsigned LONG eventType, unsigned LONG refNum,
					Ptr bufferContents, LONG bufferLength);
void PostSockFrontEndEvent(SockBuffersPtr sp, int cmd, int dip, int dport, 
							int length, unsigned char *buffer);
void PassUserEvent(FEUserPtr cUser, unsigned LONG eventType, unsigned LONG refNum,
					Ptr bufferContents, LONG bufferLength);

void SetNonBlocking(int fd);
char *SockIPToString(SockBuffersPtr sp);	/* not implmented yet */

/* M-UTILS */
void EncryptString(StringPtr inStr, StringPtr outStr);
LONG GetSRand(void);
void MySRand(LONG s);
void Randomize(void);
LONG LongRandom(void);
double DoubleRandom(void);
short MyRandom(short max);
void EncryptString(StringPtr inStr, StringPtr outStr);
void DecryptString(StringPtr inStr, StringPtr outStr);
void EncryptCString(unsigned char *inStr, unsigned char *outStr, int len);
void DecryptCString(unsigned char *inStr, unsigned char *outStr, int len);
Boolean EqualPString(StringPtr inStr, StringPtr outStr, Boolean caseSens);
int stricmp(const char *str1,const char *str2);
int strincmp(const char *str1,const char *str2, int len);	/* 6/15/95 */
char *CvtToCString(StringPtr str);
unsigned LONG SwapLong(unsigned LONG *n);
unsigned short SwapShort(unsigned short *n);
unsigned LONG LongSwap(unsigned LONG n);
Boolean LittleEndian();

/* action records */
void AddActionRecord (LONG srcUser, LONG dstUsr, LONG action);
void DelActionRecord (LONG srcUser, LONG dstUsr, LONG action);
Boolean IsActionRecord(LONG srcUser, LONG dstUsr, LONG action);

