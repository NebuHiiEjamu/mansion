// ServerLocalTalk.c - LOCAL for MACS

#include "S-SERVER.H"

TargetID			destTargetID;		// Include in UserRecord
AEAddressDesc		destAddressDesc;	// Ditto
char				*gRcveBuffer;
TargetID			lastTargetID;
long				gRcveBufferLength=4096L;

OSErr			aliasErr;
NamesTableEntry	nEntry;
EntityName		eName;

void InitPPCStuff()
{
	gRcveBuffer = NewPtrClear(gRcveBufferLength);
	aliasErr = AddPPCNBPAlias(&nEntry, "\pPalace", &eName);
}

void ClosePPCStuff()
{
	if (!aliasErr)
		RemoveNBPAlias(&eName);
}

void ProcessLocalTalkEvent(EventRecord *er)
{
	unsigned long	eventType,refNum,cmdLength;
	TargetID		tID;
	unsigned long	msgRefCon;
	unsigned long	bufLen;
	OSErr			oe;
	Ptr				p;

	if (!gPrefs.allowAppletalk)
		return;

	// eventType = (unsigned long) er->where.v;
	// eventType <<= 16;
	// eventType |= (unsigned long) er->where.h;

	bufLen = gRcveBufferLength;	// 6/13/95
	oe = AcceptHighLevelEvent(&tID, &msgRefCon, gRcveBuffer, &bufLen);
	if (oe == bufferIsSmall) {
		Ptr		newBuffer;
		long	prevLength,remLength;
		prevLength = gRcveBufferLength;
		remLength = bufLen;
		newBuffer = NewPtrClear(gRcveBufferLength + bufLen + 1024L);
		if (newBuffer == NULL) {
			LogMessage("Can't receive message: not enough memory\r");
			return;
		}
		BlockMove(gRcveBuffer,newBuffer,prevLength);
		gRcveBufferLength += bufLen + 1024L;
		DisposePtr(gRcveBuffer);
		gRcveBuffer = newBuffer;
		// JAB 12/21/94
		oe = AcceptHighLevelEvent(&tID, &msgRefCon, gRcveBuffer+prevLength, &bufLen);
		bufLen = prevLength + remLength;
	}

	if (oe == noErr) {
		p = gRcveBuffer;
		do {
			eventType = *((long *) p);	p += sizeof(long);
			cmdLength = *((long *) p);	p += sizeof(long);
			refNum = *((long *) p);	p += sizeof(long);
			if (bufLen >= 12 && cmdLength >=0 && cmdLength <= bufLen-12) {
				if (eventType == MSG_INITCONNECTION) {
					ServerUserPtr	newUser;
					Boolean		isOldUser = false;
					ServerUserPtr	curUser;
					for (curUser = gUserList; curUser; curUser = curUser->nextUser)  {
						if (curUser->connectionType == C_AppleTalk &&
							curUser->netAddress.appleTalkAddress.sessionID == tID.sessionID) {
							isOldUser = true;
							ScheduleUserKill(curUser, K_CommError, 0);
						}
					}
					newUser = NewUser();
					if (newUser) {

						newUser->connectionType = C_AppleTalk;
						newUser->netAddress.appleTalkAddress = tID;	// Local Connection
					
						PostUserEvent(newUser,MSG_TIYID, newUser->user.userID, NULL, 0L);
					}
				}
				else {
					ServerUserPtr	cUser;
					for (cUser = gUserList; cUser; cUser = cUser->nextUser) {
						if (cUser->connectionType == C_AppleTalk &&
							cUser->netAddress.appleTalkAddress.sessionID == tID.sessionID)
							break;
					}
					if (gDebugFlag)
						LogMessage("LT> %.4s\r",&eventType);
					if (cUser)
						ProcessMansionEvent(cUser,eventType,refNum,p,cmdLength);
				}
			}
			else {
				LogMessage("Bad LT Packet!\r");
				break;
			}
			bufLen -= cmdLength+12;
			p += cmdLength;
		} while (bufLen >= 12);
	}
}

Boolean PostServerLTBuffer(ServerUserPtr user, Ptr buffer, long bufferLength)
{
	EventRecord		er;
	long			myOpts,myRID;
	OSErr			oe;
	long			msgRefNum;
	er.what = kHighLevelEvent;
	er.message = ServerEventID;
	myOpts = receiverIDisTargetID;
	myRID = (long) &user->netAddress.appleTalkAddress;
	msgRefNum = user->user.userID;		// Currently Unused
	oe = PostHighLevelEvent(&er, myRID, msgRefNum, buffer, bufferLength, myOpts);
	if (oe != noErr) {
		switch (oe) {
		case sessClosedErr:
		case destPortErr:
		case noSessionErr:
		case userCanceledErr:
		case noResponseErr:
		default:
			ScheduleUserKill(user,K_CommError, 0);
			LogMessage("Post Error to %s: %d\r",CvtToCString(user->user.name),oe);
			break;
		}
		return false;
	}
	return true;
}





