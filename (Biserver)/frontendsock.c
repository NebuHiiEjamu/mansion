/* frontendsock.c */
/* library for buffered communications via sockets */

#include "frontend.h"


void InitSockBuffers(int socket, int ipAddr, int ipPort, 
					 int maxInLength, int maxOutLength,	/* use RcveBufferLength and LenGroupBuffer */
					 SockBuffersPtr sp)
{
	memset(sp,0,sizeof(SockBuffers));
	sp->ipAddr = ipAddr;
	sp->ipPort = ipPort;
	sp->socket = socket;
	sp->maxInLength = maxInLength;
	sp->maxOutLength = maxOutLength;
	sp->tcpReceiveBuffer = NewPtrClear(maxInLength);
	sp->groupBuffer = NewPtrClear(maxOutLength);
	sp->groupLen = 0L;
	sp->dataInCounter = 0; /* 8/9/96 */
	sp->dataOutCounter = 0; /* 8/9/96 */
}

void CloseSockBuffers(SockBuffersPtr sp)
{
	if (sp->socket) {
		FD_CLR(sp->socket, &gAfds);
		close(sp->socket);
		sp->socket = 0;
		if (sp->tcpReceiveBuffer) {
			DisposePtr(sp->tcpReceiveBuffer);
			sp->tcpReceiveBuffer = NULL;
		}
		if (sp->groupBuffer) {
			DisposePtr(sp->groupBuffer);
			sp->groupBuffer = NULL;
		}
	}
}

/* !!! note, it's quite possible that group buffer / send buffer are redundant */
/* examine how returns from this function are handled... */
int PostSockTCPBuffer(SockBuffersPtr	sp, int *bytesWritten)
{
	Ptr	p;
	int	nLeft,nWritten;

	*bytesWritten = 0;
	if (sp->commErrFlag) {
		*bytesWritten = sp->groupLen;
		return true;
	}
	/* p = sp->tcpSendPtr;
	  BlockMove(sp->groupBuffer,p,sp->groupLen);
	 */
	p = sp->groupBuffer;
	nLeft = sp->groupLen;
	while (nLeft > 0) {
		nWritten = write(sp->socket, p, nLeft);
		if (nWritten <= 0)	{
			if (errno != 11 && errno != 35) {
				TimeLogMessage("Write Error: %d (%s)\n",errno,SockIPToString(sp));
				sp->commErrFlag = true;
				return true;
			}
			if (nLeft == sp->groupLen) {	/* no bytes written, return false */
				return false;
			}
			else {	/* partial buffer sent, lockup */
				/* LogMessage("Write Lock\n"); */
				errno = 0;
				return true;
			}
		}
		sp->dataOutCounter += nWritten;
		*bytesWritten += nWritten;
		nLeft -= nWritten;
		p += nWritten;
	}
	return true;
}

void SendSockGroupBuffer(SockBuffersPtr sp)
{
	LONG	bytesWritten = 0;
	if (PostSockTCPBuffer(sp, &bytesWritten)) {
		if (bytesWritten < sp->groupLen && bytesWritten > 0)
			BlockMove(sp->groupBuffer+bytesWritten,sp->groupBuffer,sp->groupLen - bytesWritten);
		sp->groupLen -= bytesWritten;
	}
}


void SockIdle(SockBuffersPtr sp)
{
	/* send buffered data if need be */
	if (sp->groupLen)	/* 9/28/95 */
		SendSockGroupBuffer(sp);
}

void BeginSockGroup(SockBuffersPtr sp)
{
	sp->groupFlag = true;
}

void EndSockGroup(SockBuffersPtr sp)
{
	sp->groupFlag = false;
	if (sp->groupLen)
		SendSockGroupBuffer(sp);
}

void FlushSockGroupBuffer(SockBuffersPtr sp, LONG spaceNeeded)
{
	time_t	startFlush;

	/* 8/24/96 - Dynamically allocate more space as neccesary */
	if (sp->maxOutLength < MaxSendBufferLength && 
		sp->groupLen+spaceNeeded < sp->maxOutLength+BufferSizeIncrement) 
	{
		Ptr	p;
		TimeLogMessage("Increasing buffer size to %d: (i/o = %d/%d, req=%d) %s\n",
				sp->maxOutLength+BufferSizeIncrement,
				sp->dataInCounter,sp->dataOutCounter,
				(int) spaceNeeded,
				SockIPToString(sp));
		p = NewPtrClear(sp->maxOutLength+BufferSizeIncrement);
		if (p) {
			BlockMove(sp->groupBuffer,p,sp->groupLen);
			DisposePtr(sp->groupBuffer);
			sp->groupBuffer = p;
			sp->maxOutLength += BufferSizeIncrement;
			return;
		}
		else {
			TimeLogMessage("Allocation Failed\n");
		}
	}

	startFlush = time(NULL);
	do {
		SendSockGroupBuffer(sp);
	} while (sp->maxOutLength - sp->groupLen < spaceNeeded && 
		time(NULL) - startFlush < FlushTimeOut &&
			!sp->commErrFlag);

	TimeLogMessage("Flush Required for %s (i/o=%d,%d, req=%d, time=%d secs)\r",
			SockIPToString(sp),
			sp->dataInCounter,sp->dataOutCounter,
			(int) spaceNeeded,(int) (time(NULL) - startFlush));

	if (sp->maxOutLength - sp->groupLen < spaceNeeded) {
		TimeLogMessage("User Time Out: %s\n",SockIPToString(sp));
		sp->commErrFlag = 1;
		sp->groupLen = 0;
	}
	sp->groupFlag = 0L;
}

void PostSockBuffer(SockBuffersPtr sp, Ptr bufferContents, LONG bufferLength)
{
	Ptr	p;
	LONG	lengthNeeded = bufferLength;

	if (sp->commErrFlag)
		return;

	if (sp->groupLen + lengthNeeded > sp->maxOutLength) {
		/* 8/9/96 - kill user if no input received yet */
		if (sp->dataInCounter == 0 || sp->dataOutCounter == 0) {
			TimeLogMessage("User Time Out on Entry: %s\n",SockIPToString(sp));
			sp->commErrFlag = 1;
			sp->groupLen = 0;
			return;
		}
		else {
			FlushSockGroupBuffer(sp, lengthNeeded);
			if (sp->commErrFlag)
				return;
		}
	}
	p = sp->groupBuffer + sp->groupLen;
    /* use bcopy to avoid alignment problems */
	BlockMove(bufferContents,p,bufferLength);
	sp->groupLen += lengthNeeded;
	
	if (!sp->groupFlag)
		SendSockGroupBuffer(sp);
}

void PostSockPalaceEvent(SockBuffersPtr sp, unsigned LONG eventType, unsigned LONG refNum,
					Ptr bufferContents, LONG bufferLength)
{
	Ptr	p;
	LONG	lengthNeeded = bufferLength+12;

	if (sp->commErrFlag)
		return;

	if (gDebugFlag)
		LogMessage("<-- %.4s\r",&eventType);

	if (sp->groupLen + lengthNeeded > sp->maxOutLength) {
		/* 8/9/96 kill user if no data received yet */
		if (sp->dataInCounter == 0 || sp->dataOutCounter == 0) {
			TimeLogMessage("User Time Out on Entry: %s\n",SockIPToString(sp));
			sp->commErrFlag = 1;
			sp->groupLen = 0;
			return;
		}
		else {
			FlushSockGroupBuffer(sp, lengthNeeded);
			if (sp->commErrFlag)
				return;
		}
	}
	p = sp->groupBuffer + sp->groupLen;
    /* use bcopy to avoid alignment problems */
	bcopy((char*) &eventType, p, sizeof(LONG)); 	p += sizeof(LONG);
	bcopy((char*) &bufferLength, p, sizeof(LONG));	p += sizeof(LONG);
	bcopy((char*) &refNum, p, sizeof(LONG));		p += sizeof(LONG);
	BlockMove(bufferContents,p,bufferLength);
	sp->groupLen += lengthNeeded;
	
	if (!sp->groupFlag)
		SendSockGroupBuffer(sp);
}

void PostSockFrontEndEvent(SockBuffersPtr sp,
							int			cmd,
							int 		dip,
							int			dport,
							int			length,
							unsigned char	*buffer)
{
	Ptr		p;
	short	sport;
	LONG	lengthNeeded = length+12;

	if (gDebugFlag)
		LogMessage("<--FE %d [%d,%d]\r",cmd,dip,dport);

	if (sp->groupLen + lengthNeeded > sp->maxOutLength) {
		FlushSockGroupBuffer(sp, lengthNeeded);
	}
	p = sp->groupBuffer + sp->groupLen;
    /* use bcopy to avoid alignment problems */
	
	bcopy((char*) &cmd, p, sizeof(short)); 		p += sizeof(short);

	sport = (short) dport;
	bcopy((char*) &sport, p, sizeof(short)); 	p += sizeof(short);

	bcopy((char*) &dip, p, sizeof(LONG)); 		p += sizeof(LONG);
	bcopy((char*) &length, p, sizeof(LONG)); 	p += sizeof(LONG);
	BlockMove(buffer,p,length);

	sp->groupLen += lengthNeeded;
	
	if (!sp->groupFlag)
		SendSockGroupBuffer(sp);
}

/* used to spoof server events */
void PassUserEvent(FEUserPtr cUser, unsigned LONG eventType, unsigned LONG refNum,
					Ptr bufferContents, LONG bufferLength)
{
	PostSockPalaceEvent(&cUser->sockBuf, eventType,refNum,bufferContents,bufferLength);
}

char *SockIPToString(SockBuffersPtr sp)
{
	static	char ipBuf[64];
	unsigned LONG ip;

	ip = sp->ipAddr;
	if (LittleEndian())
		ip = SwapLong(&ip);

	sprintf(ipBuf,"%d.%d.%d.%d",
		(int) ((ip >> 24) & 0x00FF),
		(int) ((ip >> 16) & 0x00FF),
		(int) ((ip >> 8) & 0x00FF),
		(int) (ip & 0x00FF));

	return ipBuf;
}

void SetNonBlocking(int fd)
{
	int flags;
	if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
		err_dump("fcntl err");
	fcntl(fd, F_SETFL, flags | FNDELAY);
}
