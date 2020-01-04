/************************************************************************************
 * UserTCP.c
 *
 * TCP Routines for Palace Client
 ************************************************************************************/
#include "U-USER.H"
#include "UserTCP.h"
#include "CvtAddr.h"
#include "u-timout.h"

#define SIGNOFF_WAIT	(10*60L)

TCPRecordPtr	gTCPRecord;

void InitTCPStuff();

void InitTCPStuff()
{
	TCPRecordPtr	tWin = (TCPRecordPtr) gTCPRecord;
	if ((tWin = (TCPRecordPtr) NewPtrClear(sizeof(TCPRecord))) == NULL)
		return;
	if ((tWin->rcveBuffer = NewPtrClear(RcveBufferLength)) == NULL)
		return;
	if ((tWin->sendBuffer = NewPtrClear(SendBufferLength)) == NULL)
		return;
	/* 7/1/96 JAB Changed to NewPtrClear */
	if ((tWin->groupBuffer = NewPtrClear(LenGroupBuffer)) == NULL)		// 8/15/95
		return;

	gTCPRecord = tWin;
	strcpy(tWin->remoteHostname,gPrefs.remoteHost);
	tWin->remotePort = gPrefs.remotePort;
	tWin->connectType = C_PalaceTCP;

	// 8/2/95 Additional Error Checking
	if (InitNetwork() != noErr) {
		ClosePalTCP();
	}
}

// DEBUG test
/***
void TestTCPCorruption(short flag);
void TestTCPCorruption(short flag)
{
	TCPRecordPtr	tWin = (TCPRecordPtr) gTCPRecord;
	static			StringPtr msg = "\p[X]DEBUG: TCP Corruption;hc";
	Boolean			badFlag = false;
	short			i;
	if (tWin == NULL)
		return;
	if (tWin->corruptTest)
		badFlag = true;	
	for (i = 0; i < 10; ++i) {
		if (tWin->charTest[i] != i+1) {
			badFlag = true;
			break;
		}
	}
	for (i = 32; i < 128; ++i) {
		if (tWin->remoteHostname[i]) {
			badFlag = true;
			break;
		}
	}
	if (badFlag) {
		msg[2] = flag;
		SysBreakStr(msg);
	}
}
***/

void OpenTCPSession(short connectType)
{
	TCPRecordPtr	tWin = (TCPRecordPtr) gTCPRecord;
	OSErr				oe;
	// unsigned long		userRef;
	// Str32				userName;

	SetDesiredRoom(NULL);	// 1/14/97 JAB

	if (HostnameDialog(connectType) == Cancel)
		return;

	// 8/2/96 JAB - Clear dialog...
	RefreshRoom(&gOffscreenRect);

	oe = BeginTCPConnection(gPrefs.remoteHost,gPrefs.remotePort);

	// 8/6/96 add 	if (oe != noErr && gConnectionType != C_PalaceTCP)
	// Don't disconnect if we're reconnecting from a previous session
	//
	if (oe != noErr && gConnectionType != C_PalaceTCP)
		ClosePalTCP();

}

#if KIDS_CLIENT
Boolean	SiteApproved(char *name);

Boolean	SiteApproved(char *name)
{
	Handle	h;
	Ptr		p;
	short	n,i;
	Str255	nameP;

	Boolean	retValue= false;

	strcpy((char *) nameP, name);
	CtoPstr((char *) nameP);

	h = Get1Resource('STR#',170);

	if (h != NULL) {
		HLock(h);
		p = *h;
		n = *((short *) p);	p += 2;
		for (i = 0; i < n; ++i) {
			if (EqualString(nameP,(StringPtr) p, false, false))
			{
				retValue = true;
				break;
			}
			p += p[0]+1;
		}		

		HUnlock(h);
		ReleaseResource(h);	
	}
	return retValue;
}

#endif

OSErr BeginTCPConnection(char *name, short remotePort)		// 6/15/95
{
	OSErr	oe;
	unsigned long	remoteHost;
	TCPRecordPtr	tWin = (TCPRecordPtr) gTCPRecord;
	if (tWin == NULL) {
		InitTCPStuff();
		tWin = gTCPRecord;
		if (tWin == NULL) {
			ReportVerboseMessage(VE_NoTCP);
			return -1;
		}
	}
	oe = ConvertStringToAddr(name,&remoteHost);	
	if (oe != noErr) {
		// Known Errors: cacheFault, nameSynaxErr
		ErrStatusMessage(SM_CantLocateSite,oe);
		ReportVerboseMessage(VE_CantLocateSite);
		return oe;
	}
	
#if KIDS_CLIENT
	if (!SiteApproved(name)) {
		ErrStatusMessage(SM_SiteNotApproved,0);
		return -1;
	}
#endif

	StoreNetAddress(name,remoteHost,remotePort);

	oe = CreateStream(&tWin->newStream,TCPBufSize);
	if (oe != noErr) {
		ErrStatusMessage(SM_ErrorCreatingStream,oe);
		return oe;
	}
	AsyncOpenConnection(tWin->newStream,remoteHost,remotePort,TCPTimeOut,&tWin->connectPB);
	StdStatusMessage(SM_Connecting);
	tWin->connecting = true;
	tWin->timeNotify = false;
	tWin->startConnectTime = GetTicks();
	return noErr;
}

void TCPConnectIdle();

void TCPConnectIdle()
{
	TCPRecordPtr	tWin = (TCPRecordPtr) gTCPRecord;
	OSErr	result = tWin->connectPB->ioResult;
	if (tWin == NULL)
		return;
	if (result < 0) {
		SetCursor(&qd.arrow);
		tWin->connecting = false;
		if (result == openFailed) {
			StdStatusMessage(SM_ConnectionRefused);
			ReportVerboseMessage(VE_ConnectionRefused);
		}
		else {
			ErrStatusMessage(SM_ErrorOpeningConnection, result);
			ReportVerboseMessage(VE_ConnectionRefused);
		}
		ReleaseStream(tWin->newStream);
		tWin->newStream = 0L;
		if (!tWin->connectionOpen)
			ClosePalTCP();
	}
	else if (result == 0) {
		FinishOpenConnection(tWin->connectPB);
		SetCursor(&qd.arrow);
		// Disconnect Previous Session
		if (tWin->connectionOpen) {
			long dmy;
			PostServerEvent(MSG_LOGOFF,gRoomWin->meID,NULL,0L);

			// 8/4/95 Complete Send
			if (tWin->sendFlag) {
				unsigned long	t;
				t = TickCount();
				while (tWin->sendBlock->ioResult > 0 && TickCount() - t < SIGNOFF_WAIT)
					IdleTCPSend(false);
			}
			// Delay(10*60L, &dmy);
			MyCloseConnection(tWin->tcpStream);
			ReleaseStream(tWin->tcpStream);
			tWin->tcpStream = 0L;
			tWin->connectionOpen = false;
			tWin->receiveFlag = false;		// Clear pending receive (connection was aborted)
			tWin->sendFlag = false;		// Clear pending receive (connection was aborted)
			Delay(120,&dmy);

		}
		else if (gConnectionType != C_None)
			PostServerEvent(MSG_LOGOFF,gRoomWin->meID,NULL,0L);
		tWin->tcpStream = tWin->newStream;
		tWin->newStream = 0L;

		SetConnectionType(C_None, false);	// 10/26/95 reset

		// 1/11/95
		gRoomWin->navInProgress = false;  // 9/12/95
		KillAuthoring();
		AbortDownloads();
		memset(&gRoomWin->curRoom,0,sizeof(RoomRec));
		gRoomWin->curHotspot = 0;
		gRoomWin->totalPeople = 0;
		gRoomWin->meID = 0;
		gRoomWin->targetID = 0;
		gRoomWin->nbrFadeIns = 0;
		gRoomWin->nbrFadeOuts = 0;
		gRoomWin->mePtr = NULL;
		tWin->receiveIdx = 0;		// JAB 7/20/96 - reset receiveIdx
										// previous connection may have had a packet fragment
		tWin->receiveFlag = false;	// 7/20/96 lear pending receive (connection was aborted)
		tWin->sendFlag = false;		// 7/20/96

		ClearBalloons();
		ClearBackStack();

		if (gRLWin)
			((ObjectWindowPtr) gRLWin)->Dispose(gRLWin);	// JAB 7/20/96
		if (gULWin)
			((ObjectWindowPtr) gULWin)->Dispose(gULWin);	// JAB 7/20/96
			
		SetConnectionType(C_PalaceTCP, true);
		// gConnectionType = C_PalaceTCP;
		tWin->connectionOpen = true;
		tWin->connecting = false;
		// StatusMessage(NULL,0);			8/2/95
		gRoomWin->swapPackets = false;
	}
	else if (UserIsTryingToInterrupt()) {	// 8/19/95
		tWin->connecting = false;
		// Show a status message
		AbortConnection(tWin->newStream);
		ReleaseStream(tWin->newStream);
		tWin->newStream = 0L;
		SetCursor(&qd.arrow);
		if (!tWin->connectionOpen) {
			StdStatusMessage(SM_NoConnection);
			ClosePalTCP();
		}
	}
	else {
		if (!tWin->timeNotify && GetTicks() - tWin->startConnectTime > 10*TICK_SECONDS) {
			tWin->timeNotify = true;
			StdStatusMessage(SM_StillTrying);
		}
		SpinCursor();
	}
}

void ClosePalTCP()
{
	TCPRecordPtr	tWin = (TCPRecordPtr) gTCPRecord;

	if (tWin == NULL)
		return;

	// 7/29/95 disconnect new stream
	// even if not currently connected.
	if (tWin->connecting && tWin->newStream) {
		AbortConnection(tWin->newStream);
		ReleaseStream(tWin->newStream);
		tWin->newStream = 0L;
		tWin->connecting = false;
	}

	if (gConnectionType == C_PalaceTCP) {
		if (tWin->connectionOpen && !tWin->aborting) {
			SignOff();
		}
		// 8/6/96 Signoff causes this routine to be called recursively
		if (gTCPRecord == NULL)
			return;
		if (tWin->connectType == C_PalaceTCP) {
			if (tWin->connectionOpen) {
				// 8/4/95 Complete Last Send
				if (tWin->sendFlag) {
					unsigned long	t;
					t = TickCount();
					while (tWin->sendBlock->ioResult > 0 && TickCount() - t < SIGNOFF_WAIT)
						IdleTCPSend(false);
				}

				MyCloseConnection(tWin->tcpStream);
				ReleaseStream(tWin->tcpStream);
				tWin->connectionOpen = false;
			}
		}
	}
	if (gTCPRecord == NULL)
		return;
	if (tWin->rcveBuffer) {
		DisposePtr(tWin->rcveBuffer);
		tWin->rcveBuffer = NULL;
	}
	if (tWin->sendBuffer) {
		DisposePtr(tWin->sendBuffer);
		tWin->sendBuffer = NULL;
	}
	if (tWin->groupBuffer) {
		DisposePtr(tWin->groupBuffer);
		tWin->groupBuffer = NULL;
	}
	if (gConnectionType == C_PalaceTCP)	// Could be AppleTalk
		SetConnectionType(C_None, true);
	DisposePtr((Ptr) gTCPRecord);
	gTCPRecord = NULL;
}


void PalTCPAbort()
{
	TCPRecordPtr	tWin = (TCPRecordPtr) gTCPRecord;
	if (gConnectionType == C_PalaceTCP) {
		// Finish Send  8/10/95
		if (tWin->sendFlag) {
			unsigned long	t;
			t = TickCount();
			while (tWin->sendBlock->ioResult > 0 && TickCount() - t < SIGNOFF_WAIT)
				IdleTCPSend(false);
		}
		tWin->aborting = true;
		if (tWin->receiveFlag)
			AbortConnection(tWin->tcpStream);
		tWin->receiveFlag = false;
	}
}

Boolean PalTCPPollPacket()
{
	TCPRecordPtr	tWin = (TCPRecordPtr) gTCPRecord;
	if (tWin == NULL || !tWin->connectionOpen)
		return false;
	if (!tWin->receiveFlag) {
		RecvDataAsync(tWin->tcpStream,tWin->rcveBuffer+tWin->receiveIdx,RcveBufferLength - tWin->receiveIdx,&tWin->rcveBlock);
		tWin->receiveFlag = true;
	}
	else {
		if (tWin->rcveBlock->ioResult < 0) {
			switch (tWin->rcveBlock->ioResult) {
			case connectionClosing:
			case connectionTerminated:
			case connectionDoesntExist:
				tWin->aborting = true;
				DisconnectUser();
				ErrStatusMessage(SM_ConnectionSevered,tWin->rcveBlock->ioResult);
				ReportVerboseMessage(VE_ConnectionSevered);
				return false;		// return immediately
			case commandTimeout:
				tWin->receiveFlag = false;	// normal
				break;
			default:
				tWin->receiveFlag = false;
				tWin->aborting = true;
				DisconnectUser();
				ErrStatusMessage(SM_ReceiveError,tWin->rcveBlock->ioResult);
				ReportVerboseMessage(VE_ConnectionSevered);
				return false;		// return immediately
			}
		}
		else if (tWin->rcveBlock->ioResult == 0) {
			unsigned short	sBufLen=0;	// 7/20/96
			Ptr				p,rbuf;
			long			eventType,refNum,cmdLength;
			long			totalLength,packetLen;

			GetDataLength(tWin->rcveBlock,&sBufLen);
			tWin->receiveFlag = true;
			totalLength = tWin->receiveIdx + sBufLen;
			//
			// bufLen = initBufLength = sBufLen;
			// OK if as long as totalLength < RcveBufferLength (32766L)
			if (gDebugFlag && totalLength > RcveBufferLength) {
				LogMessage("Long Buffer!!: %ld\r",totalLength);
				SysBeep(1);
			}
			//
			rbuf = tWin->rcveBuffer;
		    packetLen = *((long *) &rbuf[4]);
		    if (gRoomWin->swapPackets) 
		        packetLen = LongSwap(packetLen);

			// 7/25/96 Do packet swap check here, in case we received
			// extraneous 'bye' from server before ditl
			//
			// This shouldn't happen anymore, so we treat it as an error
			//
			if (totalLength >= 8 && (packetLen & 0xFFF00000) != 0) {
				// gRoomWin->swapPackets = true;
				LogMessage("Extraneous Packet %.4s 0x%lx\r",rbuf,packetLen);
				tWin->aborting = true;
				DisconnectUser();
				ErrStatusMessage(SM_ReceiveError,tWin->rcveBlock->ioResult);
				ReportVerboseMessage(VE_BadData);
				return false;
			}

		    packetLen += 12;
			// TestTCPCorruption('J');
    		while (totalLength >= 12 && packetLen <= totalLength) {
				p = rbuf;
				if (gRoomWin->swapPackets)
					SwapIncomingClientTCPPacket(p);
				eventType = *((long *) p);	p += sizeof(long);
				cmdLength = *((long *) p);	p += sizeof(long);
				refNum = *((long *) p);	p += sizeof(long);
				if (gDebugFlag) {
					LogMessage("--> %.4s [%ld]\r",&eventType,(long) cmdLength);
				}
				ProcessMansionEvent(eventType,refNum,p,cmdLength);
				// TestTCPCorruption('K');
				if (gConnectionType == C_None)	// 12/19/95
					return false;
		        BlockMove(&rbuf[packetLen],rbuf,totalLength - packetLen);
		        totalLength -= packetLen;
		        packetLen = *((long *) &rbuf[4]);
		        if (gRoomWin->swapPackets) 
		            packetLen = LongSwap(packetLen);
		        packetLen += 12;
			}
			if (totalLength) {
				if (totalLength >= 8 && (packetLen < 0 || packetLen > RcveBufferLength)) {
					// TestTCPCorruption('L');
					LogMessage("Bad Packet Length: %ld\r",(long) packetLen);
					tWin->aborting = true;
					DisconnectUser();
					ErrStatusMessage(SM_ReceiveError,tWin->rcveBlock->ioResult);
					ReportVerboseMessage(VE_BadData);
					return false;
				}
				// SysBeep(1);
				if (gDebugFlag)
					LogMessage("TCP Fragment %ld\r",(long) totalLength);
				tWin->receiveIdx = totalLength;
			}
			else
				tWin->receiveIdx = 0;
			RecvDataAsync(tWin->tcpStream,tWin->rcveBuffer+totalLength,RcveBufferLength-totalLength,&tWin->rcveBlock);
			tWin->receiveFlag = true;
			return true;
		}
	}
	return false;
}


Boolean PalTCPIdle(EventRecord *theEvent)
{
	TCPRecordPtr	tWin = (TCPRecordPtr) gTCPRecord;
	long	t;
	Boolean	retVal = false;		// 8/30/95 - don't return early when connecting

	if (tWin->connecting) {
		TCPConnectIdle();
		retVal = true;
	}

	if (gConnectionType != C_PalaceTCP)
		return retVal;

	IdleTCPSend(true);

	t = TickCount();
	
	while (TickCount() - t < 60 && PalTCPPollPacket())
		;
	return retVal;
}

void IdleTCPSend(Boolean abortOK)
{
	TCPRecordPtr	tWin = (TCPRecordPtr) gTCPRecord;
	OSErr	oe;
	if (tWin->sendFlag) {
		if (tWin->sendBlock->ioResult <= 0) {
			tWin->sendFlag = 0;
			oe = tWin->sendBlock->ioResult;
			if (oe != noErr && oe != commandTimeout) {
				if (abortOK) {
					tWin->aborting = true;
					DisconnectUser();
					if (oe == connectionTerminated)
						ErrStatusMessage(SM_ConnectionSevered,oe);
					else
						ErrStatusMessage(SM_SendError,oe);
					ReportVerboseMessage(VE_ConnectionSevered);
				}
			}
			else {
				if (tWin->groupLen)
					SendGroupTCPBuffer();
			}
		}
	}
}

Boolean PostTCPBuffer(Ptr buffer, long bufferLength)
{
	TCPRecordPtr	tWin = (TCPRecordPtr) gTCPRecord;
	Ptr				p;

	if (tWin == NULL)
		return false;
	if (tWin->sendFlag)
		return false;
	p = tWin->sendBuffer;
	BlockMove(buffer, p, bufferLength);
	// if (gDebugFlag)
	//	LogMessage("<< %.4s\r",&eventType);
	SendDataAsync(tWin->tcpStream,tWin->sendBuffer,(unsigned short)bufferLength,&tWin->sendBlock);
	tWin->sendFlag = true;
	return true;
}

void FlushGroupTCPBuffer()	// Wait till Group Buffer is Sent
{
	TCPRecordPtr	tWin = (TCPRecordPtr) gTCPRecord;
	// 7/22 If user has been killed - don't lock up on user
	//		- just flush the buffer
	if (gConnectionType != C_None) {
		// Group Buffer is full...
		// Wait for previous send to finish...
		if (tWin->sendFlag) {
			while (tWin->sendBlock->ioResult > 0)
				;
		}
		// Now Post is guaranteed to return TRUE
		PostTCPBuffer(tWin->groupBuffer,tWin->groupLen);
	}
	tWin->groupLen = 0L;
}


// Attempt to send the buffer, if the data is transferred, clear the byte count
//
// If this happens frequently, it will cause severe performance degredation
//
void SendGroupTCPBuffer()
{
	TCPRecordPtr	tWin = (TCPRecordPtr) gTCPRecord;
	if (PostTCPBuffer(tWin->groupBuffer,tWin->groupLen))
		tWin->groupLen = 0L;
}

void PostUserPalTCPEvent(unsigned long eventType, unsigned long refNum,
					Ptr buffer, long bufferLength)
{
	TCPRecordPtr	tWin = (TCPRecordPtr) gTCPRecord;
	Ptr	p,startPacket;
	if (gDebugFlag)
		LogMessage("<-- TCP %.4s\r",&eventType);

	// TestTCPCorruption('M');

	if (tWin->groupLen + bufferLength+12 > LenGroupBuffer) {
		if (gDebugFlag) {
			// SysBreakStr("\pDEBUG: Flush Required;hc;g");
			LogMessage("Flush Required\r");
		}
		FlushGroupTCPBuffer();
		// if (gDebugFlag) {
		// 	SysBreakStr("\pDEBUG: Done Flushing;hc;g");
		// }
	}
	if (tWin->groupLen < 0 || tWin->groupLen + bufferLength+12 > LenGroupBuffer) {
			// SysBreakStr("\pDEBUG: Bad Group len;hc");
		LogMessage("Bad Group Len\r");
		tWin->aborting = true;
		DisconnectUser();
		ErrStatusMessage(SM_ReceiveError,-1);
		ReportVerboseMessage(VE_ConnectionSevered);
	}
	p = tWin->groupBuffer + tWin->groupLen;
	startPacket = p;
	*((long *) p) = eventType;		p += sizeof(long);
	*((long *) p) = bufferLength;	p += sizeof(long);
	*((long *) p) = refNum;			p += sizeof(long);
	BlockMove(buffer,p,bufferLength);
	if (gRoomWin->swapPackets)
		SwapOutgoingClientTCPPacket(startPacket);
	tWin->groupLen += 12 + bufferLength;
	SendGroupTCPBuffer();
}
