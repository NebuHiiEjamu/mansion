/* biserver.h */
#include "frontendcmds.h"

#define LenBiGroupBuffer	512000L /* jbum - increased from 128k */

typedef struct BiFrontEnd {
	struct BiFrontEnd	*nextFrontEnd;
	int		sockFD;
	int		ipNbr;
	unsigned short	portNbr;
	int		tcpReceiveIdx,groupLen;
	unsigned char	
			*tcpReceiveBuffer, *groupBuffer;
	short	killFlag, groupFlag, commErrFlag, upFlag;
} BiFrontEnd, *BiFrontEndPtr;

BiFrontEndPtr	gFrontEndList;

void IdleFrontEnd(BiFrontEndPtr fePtr, Boolean needsRead);

BiFrontEndPtr NewFrontEnd(void);

void ProcessNewFrontEnd(int newsockfd, int ip, short port);

void SendFrontEndEvent(BiFrontEndPtr fePtr,
						short		cmd,
						short		dport,
						int 		dip,
						int			length,
						unsigned char	*buffer);
void SendFEGroupBuffer(BiFrontEndPtr fePtr);
void FlushFEGroupBuffer(BiFrontEndPtr fePtr, int lengthNeeded);
Boolean PostFEBuffer(BiFrontEndPtr fePtr, Ptr buffer, LONG len, LONG *bytesWritten);
void KillFrontEnd(BiFrontEndPtr fePtr);

void GlobalFrontEndEvent(LONG  addr, short port, short cmd, Ptr bufferContents, LONG bufferLength);
void UserToFrontEnd(ServerUserPtr cUser, short cmd, unsigned LONG eventType, unsigned LONG refNum,
					Ptr bufferContents, LONG bufferLength);
ServerUserPtr	GetUserFromFrontEnd(LONG addr, short port);
void TerminateFrontEnd(BiFrontEndPtr dFEPtr);


