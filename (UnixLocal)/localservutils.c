/* LocalServUtils.c */
#include "s-server.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

Boolean	gCancel;		/* 5/8/95	- used by TCP library... */

Boolean IsPendingSend(ServerUserPtr curUser)
{
	return curUser->groupLen > 0;
}

LONG PendingSendLength(ServerUserPtr curUser)
{
	return curUser->groupLen;
}

short DisconnectUser(ServerUserPtr curUser)
{
	/* Possibly needed on some networks */
	if (curUser->connectionType == C_BerkSockets)
		DisconnectTCPUser(curUser);
	return C_None;
}

/* We've run out of room to cache data, clear the byte count at all costs */
/* */

#if !BISERVER

void FlushGroupBuffer(ServerUserPtr cUser, LONG spaceNeeded)	/* Wait till Group Buffer is Sent */
{
	time_t	startFlush;
	/* 10/4 lock up till buffer is sent */

	if (cUser->groupAlloc < MaxLenGroupBuffer &&
		cUser->groupLen + spaceNeeded < cUser->groupAlloc + GroupBufferIncrement)
	{
		Ptr	p;
		TimeLogMessage("Increasing buffer size to %d for %s\n",
				cUser->groupAlloc+GroupBufferIncrement,CvtToCString(cUser->user.name));
		p = NewPtrClear(cUser->groupAlloc+GroupBufferIncrement);
		if (p) {
			BlockMove(cUser->groupBuffer,p,cUser->groupLen);
			DisposePtr(cUser->groupBuffer);
			cUser->groupBuffer = p;
			cUser->groupAlloc += GroupBufferIncrement;
			SendGroupBuffer(cUser);
			return;
		}
		else {
			TimeLogMessage("Allocation Failed\n");
		}
	}

	startFlush = time(NULL);
	do {
		SendGroupBuffer(cUser);
	} while (cUser->groupAlloc - cUser->groupLen < spaceNeeded && time(NULL) - startFlush < 15);
	TimeLogMessage("Flush Required for %s\r",CvtToCString(cUser->user.name));

	if (cUser->groupAlloc - cUser->groupLen < spaceNeeded) {
		LogMessage("User Time Out: %s\n",CvtToCString(cUser->user.name));
		ScheduleUserKill(cUser,K_CommError, 0);
		cUser->groupLen = 0;
	}
	cUser->groupFlag = 0L;
}


/* Attempt to send the buffer, if the data is transferred, clear the byte count */
/* */
void SendGroupBuffer(ServerUserPtr cUser)
{
	LONG	bytesWritten = 0;
	switch (cUser->connectionType) {
	case C_BerkSockets:
		if (PostServerTCPBuffer(cUser, cUser->groupBuffer,cUser->groupLen, &bytesWritten)) {
			if (bytesWritten < cUser->groupLen && bytesWritten > 0)
				BlockMove(cUser->groupBuffer+bytesWritten,cUser->groupBuffer,cUser->groupLen - bytesWritten);
			cUser->groupLen -= bytesWritten;
		}
		break;
	}
}


void PostUserEvent(ServerUserPtr cUser, unsigned LONG eventType, unsigned LONG refNum,
					Ptr bufferContents, LONG bufferLength)
{
	Ptr	p;
	LONG	lengthNeeded = bufferLength+12;

	if (gDebugFlag)
		LogMessage("<-- %.4s\r",&eventType);

	if (cUser->groupLen + lengthNeeded > cUser->groupAlloc) {
		/* 1/28/95 important bug fix (args to flushgroupbuffer) */
		FlushGroupBuffer(cUser, lengthNeeded);
	}
	p = cUser->groupBuffer + cUser->groupLen;
    /* use bcopy to avoid alignment problems */
	bcopy((char*) &eventType, p, sizeof(LONG)); 	p += sizeof(LONG);
	bcopy((char*) &bufferLength, p, sizeof(LONG));	p += sizeof(LONG);
	bcopy((char*) &refNum, p, sizeof(LONG));		p += sizeof(LONG);
	BlockMove(bufferContents,p,bufferLength);
	cUser->groupLen += lengthNeeded;
	
	if (!cUser->groupFlag)
		SendGroupBuffer(cUser);
}

#endif

void ConvertIPToString(unsigned LONG ipSrce, char *dbuf)
{
	unsigned LONG ip;
	if (LittleEndian())
		ip = SwapLong(&ipSrce);
	else
		ip = ipSrce;
	sprintf(dbuf,"%d.%d.%d.%d",
		(int) ((ip >> 24) & 0x00FF),
		(int) ((ip >> 16) & 0x00FF),
		(int) ((ip >> 8) & 0x00FF),
		(int) (ip & 0x00FF));
}

void ConvertNetAddressToNumericString(ServerUserPtr cUser, char *dbuf)
{
	unsigned LONG	ip;
	unsigned LONG	ipSrce = cUser->netAddress.ipAddress;

	switch (cUser->connectionType) {
	case C_BerkSockets:
		if (LittleEndian())
			ip = SwapLong(&ipSrce);
		else
			ip = ipSrce;
		sprintf(dbuf,"%d.%d.%d.%d",
			(int) ((ip >> 24) & 0x00FF),
			(int) ((ip >> 16) & 0x00FF),
			(int) ((ip >> 8) & 0x00FF),
			(int) (ip & 0x00FF));
		break;		
	default:
		dbuf[0] = 0;
		break;
	}
}

void ConvertNetAddressToString(ServerUserPtr cUser, char *dbuf)
{
	struct hostent	*he;
	switch (cUser->connectionType) {
	case C_BerkSockets:
		if (cUser->verbalIP[0]) {
			strcpy(dbuf, cUser->verbalIP);
		}
		else {
			he = gethostbyaddr((char *) &cUser->netAddress.ipAddress, 4, AF_INET);
			if (he) {
				strcpy(dbuf, he->h_name);
			}
			else {
				ConvertNetAddressToNumericString(cUser, dbuf);
			}
			strcpy(cUser->verbalIP, dbuf);
		}
		break;		
	default:
		dbuf[0] = 0;
		break;
	}
}

LONG GetIPAddress(ServerUserPtr cUser)
{
	if (cUser->connectionType == C_BerkSockets)
		return cUser->netAddress.ipAddress;
	else
		return 0L;
}

StringPtr BuildPictureName(StringPtr pictureName)
{
	static char	picName[256];
	char		tempName[64];
	StringPtr	folderName;

	if (pictureName[0] > 31) {
		pictureName[0] = 31;
		LogMessage("Bad Picture Name!\r");
	}
	BlockMove(pictureName,tempName,pictureName[0]+1);
	PtoCstr((StringPtr) tempName);
	if (strrchr(tempName,'/') != NULL) {
		strcpy(tempName,strrchr(tempName,'/')+1);
	}
	CtoPstr(tempName);

	folderName = (StringPtr) gPrefs.picFolder;
	BlockMove(folderName,picName,folderName[0]+1);
	BlockMove(&tempName[1],&picName[picName[0]+1],tempName[0]);
	picName[0] += tempName[0];
	return (StringPtr) picName;
}


void YellowPagesIdle(void)
{
}

