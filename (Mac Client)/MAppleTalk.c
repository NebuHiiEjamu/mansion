// MansionAppleTalk.c

#include "U-USER.H"
#include <PPCToolBox.h>

TargetID			destTargetID;		// Include in UserRecord
AEAddressDesc		destAddressDesc;	// Ditto
char				*rcveBuffer,*sendBuffer;
long				rcveBufferLength=4096L,sendBufferLength=4096L;

void InitAppleTalkBuffers()
{
	rcveBuffer = NewPtrClear(rcveBufferLength);
	sendBuffer = NewPtrClear(sendBufferLength);
}


void ProcessAppleTalkEvent(EventRecord *er)
{
	unsigned long	eventType,refNum,cmdLength;
	TargetID		tID;
	unsigned long	msgRefCon;
	unsigned long	bufLen;
	OSErr			oe;
	Ptr				p;

	eventType = (unsigned long) er->where.v;
	eventType <<= 16;
	eventType |= (unsigned long) er->where.h;

	bufLen = rcveBufferLength;
	oe = AcceptHighLevelEvent(&tID, &msgRefCon, rcveBuffer, &bufLen);
	if (oe == bufferIsSmall) {
		Ptr		newBuffer;
		long	prevLength,remLength;
		// SysBeep(1);
		prevLength = rcveBufferLength;
		remLength = bufLen;
		newBuffer = NewPtrClear(rcveBufferLength + bufLen + 1024L);
		if (newBuffer == NULL)
			return;
		BlockMove(rcveBuffer,newBuffer,prevLength);
		rcveBufferLength += bufLen + 1024L;
		DisposePtr(rcveBuffer);
		rcveBuffer = newBuffer;
		oe = AcceptHighLevelEvent(&tID, &msgRefCon, rcveBuffer+prevLength, &bufLen);
		bufLen = prevLength + remLength;
	}
	// LogMessage("Msg In: <%.4s>\r",&eventType);
	if (oe == noErr && gConnectionType == C_AppleTalk) {
		p = rcveBuffer;
		do {
			// SwapIncomingServTCPPacket(p);
			eventType = *((long *) p);	p += sizeof(long);
			cmdLength = *((long *) p);	p += sizeof(long);
			refNum = *((long *) p);	p += sizeof(long);
			if (bufLen >= 12 && cmdLength >=0 && cmdLength <= bufLen-12) {
				if (gDebugFlag)
					LogMessage("--> %.4s",&eventType);
				ProcessMansionEvent(eventType,refNum,p,cmdLength);
				if (gConnectionType == C_None)
					return;
				if (gDebugFlag)
					LogMessage(".\r");
			}
			bufLen -= cmdLength+12;
			p += cmdLength;
		} while (bufLen >= 12);
	}
}

void PostUserAppleTalkEvent(unsigned long eventType, unsigned long refNum,
					Ptr bufferContents, long bufferLength)
{
	EventRecord	er;
	long		myOpts,myRID;
	OSErr		oe;
	Ptr			p;

	if (gConnectionType != C_AppleTalk)
		return;

	er.what = kHighLevelEvent;
	er.message = UserEventID;
	*((unsigned long *) &er.where) = eventType;
	myOpts = receiverIDisTargetID;
	myRID = (long) &destTargetID;
	if (bufferLength+12 > sendBufferLength) {
		DisposePtr(sendBuffer);
		sendBuffer = NewPtrClear(bufferLength+1024L);
		if (sendBuffer == NULL) {
			sendBuffer = NewPtrClear(sendBufferLength);
			if (sendBuffer == NULL)
				ExitToShell();
			return;
		}
		sendBufferLength = bufferLength+1024;
	}
	p = sendBuffer;
	*((long *) p) = eventType;		p += sizeof(long);
	*((long *) p) = bufferLength;	p += sizeof(long);
	*((long *) p) = refNum;			p += sizeof(long);
	if (bufferLength)
		BlockMove(bufferContents, p, bufferLength);
	oe = PostHighLevelEvent(&er, myRID, refNum, sendBuffer, bufferLength+12, myOpts);
	if (oe != noErr) {
		switch (oe) {
		case sessClosedErr:
		case destPortErr:
		case noSessionErr:
		case userCanceledErr:
		case noResponseErr:
			LogMessage("Server unexpectedly disconnected\r");
	  		DisconnectUser();
			break;
		default:
			LogMessage("AppleTalk Send Error: %d\r",oe);
		}
	}
}

pascal Boolean MyPortFilter(LocationNamePtr	theLoc, PortInfoPtr thePort);
pascal Boolean MyPortFilter(LocationNamePtr	theLoc, PortInfoPtr thePort)
{
	long	type;

	if (thePort->name.portKindSelector == ppcByString) {
		BlockMove(thePort->name.u.portTypeStr + 1, (Ptr)&type, 4);
			/* The BlockMove is so that we don't get an address error
			** on a 68000-based machine due to referencing a long at
			** an odd-address. */
		if (type == 'mSrv') return(true);
	}
	return false;

	// if (thePort->name.u.port.portCreator == 'mSrv')
	//	return true;	// For Debugging...
	// else
	//	return false;
/*	if (EqualString(thePort->name.name,"\pPalace",false,false) ||*/
/*		EqualString(thePort->name.name,"\pMServ.¹",false,false))*/
/*		return true;*/
/*	else*/
/*		return false;*/
}

void SignOnAppleTalk()
{
	PortInfoRec		pRec;
	OSErr			oe;
	PPCFilterUPP	filterProc = NewPPCFilterProc(MyPortFilter);
	oe = PPCBrowser("\pChoose a Palace Server:", "\pPalace Servers", false, &destTargetID.location, &pRec, filterProc, "\pPalace");
	if (oe == noErr) {

		if (gConnectionType != C_None)
			SignOff();

		destTargetID.name = pRec.name;
		AECreateDesc(typeTargetID, &destTargetID, sizeof(destTargetID), &destAddressDesc);
		SetConnectionType(C_AppleTalk,true);
		PostServerEvent(MSG_INITCONNECTION, 0L, NULL, 0L);
	}
}



