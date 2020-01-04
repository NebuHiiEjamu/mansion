// LocalServUtils.c
#include "S-SERVER.H"

Boolean	gCancel;		// 5/8/95	- used by TCP library...

// !!
// This function returns true if there is an additional
// packet in the user's send buffer waiting to go out
//
Boolean IsPendingSend(ServerUserPtr curUser)
{
	return curUser->groupLen > 0;
}

long PendingSendLength(ServerUserPtr curUser)
{
	return curUser->groupLen;
}

short DisconnectUser(ServerUserPtr curUser)
{
	// Possibly needed on some networks
	if (curUser->connectionType == C_MacTCP) {
		// Stay connected while message gets sent
		if (curUser->tcpSendFlag && curUser->tcpSendBlock->ioResult > 0
			&& curUser->whyKilled != K_ServerDown)
			return C_MacTCP;
		DisconnectTCPUser(curUser);
	}
	return C_None;
}

// We've run out of room to cache data, clear the byte count at all costs
//
void FlushGroupBuffer(ServerUserPtr cUser)	// Wait till Group Buffer is Sent
{
	// 7/22 If user has been killed - don't lock up on user
	//		- just flush the buffer
	if (!(cUser->flags & U_Kill)) {
		switch (cUser->connectionType) {
		case C_AppleTalk:
			// Post is OK for appletalk...
			PostServerLTBuffer(cUser, cUser->groupBuffer,cUser->groupLen);
			break;
		case C_MacTCP:
			// Group Buffer is full...
			// Wait for previous send to finish...
			if (cUser->tcpSendFlag) {
				while (cUser->tcpSendBlock->ioResult > 0)
					;
			}
			// Now Post is guaranteed to return TRUE
			PostServerTCPBuffer(cUser, cUser->groupBuffer,cUser->groupLen);
			break;
		}
	}
	cUser->groupLen = 0L;
	cUser->groupFlag = 0L;
}


// Attempt to send the buffer, if the data is transferred, clear the byte count
//
// If this happens frequently, it will cause severe performance degredation
//
void SendGroupBuffer(ServerUserPtr cUser)
{
	switch (cUser->connectionType) {
	case C_AppleTalk:
		if (PostServerLTBuffer(cUser, cUser->groupBuffer,cUser->groupLen))
			cUser->groupLen = 0L;
		break;
	case C_MacTCP:
		if (PostServerTCPBuffer(cUser, cUser->groupBuffer,cUser->groupLen))
			cUser->groupLen = 0L;
		break;
	}
}

void PostUserEvent(ServerUserPtr cUser, unsigned long eventType, unsigned long refNum,
					Ptr bufferContents, long bufferLength)
{
	Ptr	p;

	if (gDebugFlag)
		LogMessage("<-- %.4s\r",&eventType);

	if (cUser->groupLen + bufferLength+12 > cUser->groupAlloc) {
		if (gDebugFlag)
			LogMessage("Flush Required for %s\r",CvtToCString(cUser->user.name));
		FlushGroupBuffer(cUser);
	}
	p = cUser->groupBuffer + cUser->groupLen;
	*((long *) p) = eventType;		p += sizeof(long);
	*((long *) p) = bufferLength;	p += sizeof(long);
	*((long *) p) = refNum;			p += sizeof(long);
	BlockMove(bufferContents,p,bufferLength);
	cUser->groupLen += 12 + bufferLength;
	
	if (!cUser->groupFlag)
		SendGroupBuffer(cUser);
}

Boolean GiveTime(short sleepTime)	// 5/8/95 Needed by TCP Library.
{
	// SpinCursor(1);
	return true;
}

/* 12/3/95 */
void ConvertIPToString(unsigned LONG ip, char *dbuf)
{
	sprintf(dbuf,"%d.%d.%d.%d",
		(int) ((ip >> 24) & 0x00FF),
		(int) ((ip >> 16) & 0x00FF),
		(int) ((ip >> 8) & 0x00FF),
		(int) (ip & 0x00FF));
}

/* 12/3/95 */
void ConvertNetAddressToNumericString(ServerUserPtr cUser, char *dbuf)
{
	long	ip;
	switch (cUser->connectionType) {
	case C_MacTCP:
		ip = cUser->netAddress.ipAddress;
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
	switch (cUser->connectionType) {
	case C_MacTCP:
		ConvertNetAddressToNumericString(cUser, dbuf);
		break;		
	case C_AppleTalk:
		strcpy(dbuf,"(local)");
		break;
	default:
		dbuf[0] = 0;
		break;
	}
}

long GetIPAddress(ServerUserPtr cUser)
{
	if (cUser->connectionType == C_MacTCP)
		return cUser->netAddress.ipAddress;
	else
		return 0L;
}

// Modified to use new search path facility...
//

#define ServerPathList		150

StringPtr BuildPictureName(StringPtr pictureName)
{
	StringPtr	folderName;
	int			n;
	OSErr		oe;
	FInfo		fInfo;

	n = 0;
	do {
		++n;
		folderName = BuildMediaFolderName(pictureName, ServerPathList, n);
		if (folderName) {
			oe = GetFInfo(folderName, 0, &fInfo);
		}
	} while (folderName && oe != noErr);
	if (folderName == NULL || oe != noErr)
		return NULL;
	return folderName;
}

/* 8/5/96 JAB - need to switch to new system that
   uses a STR# resource and index to build the
   name, and returns NULL if index is past the
   end...
   typical STR#  \A\S\P\F
	// \S = Server Name:
	// \F = Filename

	Todo: Make directories properly (may need to make more than one - check this)
	Test speed.  Test Failure.  Fix for Sound.

 */
StringPtr	BuildMediaFolderName(StringPtr pictureName, int catalogNumber, int indexNumber)
{
	static char	assetName[256];
	char		pathTemplate[64]="";
	char		*sp,*dp,*sp2,*dp2;

	// Remove path specifiations from filename
	GetIndString((StringPtr) pathTemplate, catalogNumber, indexNumber);
	if (pathTemplate[0] == 0)
		return NULL;
	PtoCstr((StringPtr) pathTemplate);

	dp = assetName;
	sp = pathTemplate;

	while (*sp) {
		if (*sp == '\\') {
			++sp;
			switch (*sp) {
			case 'M':
				// Preferences "Pictures / Media" folder
				BlockMove(gPrefs.picFolder, dp, gPrefs.picFolder[0]+1);
				PtoCstr((StringPtr) dp);
				dp += strlen(dp);
				break;
			case 'S':
				BlockMove(gPrefs.serverName, dp, gPrefs.serverName[0]+1);
				PtoCstr((StringPtr) dp);
				for (sp2 = dp2 = dp; *sp2; ++sp2) {
					*dp2 = *sp2;
					if (isalnum(*sp2) || *sp2 == ' ' || *sp2 == '_')
						++dp2;
				}
				*dp2 = 0;
				if (strlen(dp) > 28) {
					dp[28] = ':';
					dp[29] = 0;
				}
				else if (dp[0]) {
					*(dp2++) = ':';
					*dp2 = 0;
				}
				dp += strlen(dp);
				break;
			case 'F':
				BlockMove(pictureName,dp,pictureName[0]+1);
				PtoCstr((StringPtr) dp);
				if (strrchr(dp,':') != NULL) {
					strcpy(dp,strrchr(dp,':')+1);
				}
				dp += strlen(dp);
				break;
			}
			++sp;
		}
		else {
			*(dp++) = *(sp++);
		}
	}
	*dp = 0;
	CtoPstr(assetName);

	// Append file name
	return (StringPtr) assetName;
}

Ptr SuckTextFile(StringPtr fileName)	/* p-string */
{
	OSErr		oe;
	FileHandle	iFile;
	Ptr			fileBuffer;
	long		length;

    if ((oe = OpenFileReadOnly(fileName, 0, &iFile)) != noErr) {
          LogMessage("Can't open %.*s for input (%d)\r",fileName[0],&fileName[1],oe);
          return NULL;
    }

    GetEOF(iFile,&length);
    fileBuffer = (HugePtr)NewPtrClear(length+1);
    FSRead(iFile,&length,(char *)fileBuffer);
    FSClose(iFile);
    fileBuffer[length] = 0;
    return fileBuffer;
}

void SpinCursor();
void SpinCursor()
{
}
