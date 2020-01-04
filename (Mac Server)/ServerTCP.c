// ServerTCP.c
#include "s-server.h"
#include "m-assets.h"

// #include <Threads.h>

#define USE_TCP				1

#if THINK_C
#if __option(profile)
#include <Console.h>
#include <Profile.h>
#endif
#else
#if __profile__
#include <profiler.h>
#endif
#endif

#include <MacTCP.h>
#include "TCPHi.h"
#include "TCPRoutines.h"

#define RcveBufferLength	16384L	// 5/25/95 increased buffer length
#define SendBufferLength	16384L	// 5/25/95 increased buffer length
#define TCPBufSize			16384L

// Variables for Setting up a New Connection
Boolean	gWaitingForConnection;
long	newStream;
long	newHost;
short	newPort;
TCPiopb	*asyncPB;

void SwapOutgoingServTCPPacket(Ptr	buf);
void SwapIncomingServTCPPacket(Ptr	buf);

Boolean	tcpInited;

unsigned long GetNewStream()
{
	unsigned long	ns;
	OSErr			oe;
	oe = CreateStream(&ns,TCPBufSize);
	if (oe == noErr)
		return ns;
	else
		return 0L;
}

void DisposeStream(unsigned long stream)
{
	if (stream)
		ReleaseStream(stream);
}

void MyAddrToStr(long myAddr, char *name);
void MyAddrToStr(long myAddr, char *name)
{
	int	a,b,c,d;
	a = (myAddr >> 24) & 0x00FF;
	b = (myAddr >> 16) & 0x00FF;
	c = (myAddr >> 8) & 0x00FF;
	d = myAddr & 0x00FF;
	sprintf(name,"%d.%d.%d.%d",a,b,c,d);
}

#include "AddressXLation.h"

pascal void
__DNRDone(struct hostInfo* , char* );
pascal void
__DNRDone(struct hostInfo* , char* )
{
	// do nothing by default
}

void InitServerTCP()
{
	// Initialize Stuff
	if (InitNetwork() != noErr) {
		gPrefs.allowTCP = false;
	}
	else {
		ip_addr	myAddr;
		char	name[128] = "";
		short	curVol;
		GetVol(NULL,&curVol);
		if (OpenResolver(nil) == noErr)  {
			hostInfo hinfo;
			OSErr	oe;
			int		len;
			ResultUPP	done;
			if (GetMyIP(&myAddr) == noErr) {
				done = NewResultProc(__DNRDone);
				if ((oe = AddrToName(myAddr, &hinfo, done, NULL)) == noErr || oe == cacheFault) {
					while (hinfo.rtnCode == cacheFault)
						SystemTask();
					while (hinfo.rtnCode > 0)
						;
					CloseResolver();
					strcpy(name,hinfo.cname);
					len = strlen(name) - 1;
					if (len && name[len] == '.')
						name[len] = 0;
					LogMessage("IP Address: %s\r",name);
					OpenResolver(nil);
				}
				if (AddrToStr(myAddr,name) == noErr) {
					CloseResolver();
					LogMessage("Numeric IP Address: %s\r",name);
				}
				else {
					CloseResolver();
					LogMessage("Numeric IP Number: %s\r",MyAddrToStr(myAddr,name));
				}
			}
			else
				CloseResolver();
		}
		SetVol(NULL,curVol);

		tcpInited = true;
		SetIdleTime(TIMER_FASTEST);
	}
}

void CleanupServerTCP()
{
	// Close Stuff
	if (gWaitingForConnection) {
		if (asyncPB->ioResult > 0) {
			LogMessage("Aborting TCP Listen\r");
			AbortConnection(newStream);
		}
		DisposeStream(newStream);
	}
	RegisterOnYellowPages(false, false);
}

void SetupNewTCPUser()
{
	ServerUserPtr	newUser;
	// Setup New User Connection with newStream,newHost,newPort
	// sessionID = newHost ^ ((long) newPort << 16) ^ ((long) newPort);
	// !!! Insure sessionID is unique
	// !!!
	newUser = NewUser();
	if (newUser) {
		newUser->connectionType = C_MacTCP;
		newUser->tcpStream = newStream;
		newUser->tcpRemoteHost = newHost;
		newUser->netAddress.ipAddress = newHost;
		newUser->tcpRemotePort = newPort;
		newUser->tcpReceiveBuffer = NewPtrClear(RcveBufferLength);
		newUser->tcpSendBuffer = NewPtrClear(SendBufferLength);
		newUser->tcpSendPtr = newUser->tcpSendBuffer;

		PostUserEvent(newUser,MSG_TIYID, newUser->user.userID, NULL, 0L);
		LogMessage("Sending TCP user ID [%8lxH]\r",newUser->user.userID);
		// 7/29/95
		if (gNbrUsers > gPrefs.maxOccupancy ||
			gNbrUsers > gMaxPeoplePerServer) {
			ScheduleUserKill(newUser,K_ServerFull, 0);
		}
	}
}

void DisconnectTCPUser(ServerUserPtr	cUser)
{
	// Close Streams etc...
	if (cUser->tcpStream) {
		MyCloseConnection(cUser->tcpStream);
		ReleaseStream(cUser->tcpStream);
	}
	if (cUser->tcpReceiveBuffer)
		DisposePtr(cUser->tcpReceiveBuffer);
	if (cUser->tcpSendBuffer)
		DisposePtr(cUser->tcpSendBuffer);
}

// !! Modify to automatically go into group/cache mode when
// held up...
// !! First, get a lock of which messages cause locks - optimize to avoid...
//  common: room->nprs  uLoc->room  uLoc->uLoc


Boolean PostServerTCPBuffer(ServerUserPtr cUser, Ptr buffer, long len)
{
	Ptr	p;
	// Wait for previous send to finish
	// 
	if (cUser->tcpSendFlag) {
		return false;
	}
	p = cUser->tcpSendPtr;
	BlockMove(buffer,p,len);
	SendDataAsync(cUser->tcpStream,cUser->tcpSendPtr,len,&cUser->tcpSendBlock);
	cUser->tcpSendFlag = true;
	cUser->tcpSendLength= len;
	return true;
}

// 5/25/95 JAB
//
// Returns true if user should be disconnected.

Boolean IdleTCPUser(ServerUserPtr cu)
{
	if (cu->tcpSendFlag) {
		if (cu->tcpSendBlock->ioResult <= 0) {
			cu->tcpSendFlag = 0;
			if (cu->tcpSendBlock->ioResult < 0) {
				if (cu->tcpSendBlock->ioResult == commandTimeout) {
					LogMessage("Send Timeout to user %s - trying again\r",CvtToCString(cu->user.name));
					// Try Again
					SendDataAsync(cu->tcpStream,cu->tcpSendPtr,cu->tcpSendLength,&cu->tcpSendBlock);
					cu->tcpSendFlag = 1;
				}
				else
					LogMessage("Send Error to user %s [%d]\r",CvtToCString(cu->user.name),cu->tcpSendBlock->ioResult);
			}
		}
		if (cu->groupLen)
			SendGroupBuffer(cu);
	}
	if (!cu->tcpReceiveFlag) {
		RecvDataAsync(cu->tcpStream,cu->tcpReceiveBuffer+cu->tcpReceiveIdx,RcveBufferLength-cu->tcpReceiveIdx,&cu->tcpReceiveBlock);
		cu->tcpReceiveFlag = true;
	}
	else {
		if (cu->tcpReceiveBlock->ioResult < 0) {
			// Error - Close Connection
			switch (cu->tcpReceiveBlock->ioResult) {
			case commandTimeout:	// normal
				break;
			case connectionDoesntExist:
			case connectionTerminated:
			case connectionClosing:
				LogMessage("Connection broken from user %s [%d]\r",CvtToCString(cu->user.name),cu->tcpReceiveBlock->ioResult);
				return true;
			default:
				LogMessage("TCP Error from user %s [%d]\r",CvtToCString(cu->user.name),cu->tcpReceiveBlock->ioResult);
				return true;
			}
			cu->tcpReceiveFlag = false;
		}
		// 6/15/95 Added Defragmentation
		else if (cu->tcpReceiveBlock->ioResult == 0) {	// Got Something!
			unsigned short	bufLen;
			long	eventType,refNum,cmdLength,packetLen;
			Ptr		p,rbuf;
			GetDataLength(cu->tcpReceiveBlock,&bufLen);
			rbuf = cu->tcpReceiveBuffer;
			cu->tcpReceiveIdx += bufLen;
			packetLen = *((long *) &rbuf[4]);
			packetLen += 12;
			while (cu->tcpReceiveIdx >= 12 && packetLen <= cu->tcpReceiveIdx) {
				p = rbuf;
				eventType = *((long *) p);	p += sizeof(long);
				cmdLength = *((long *) p);	p += sizeof(long);
				refNum = *((long *) p);		p += sizeof(long);
				if (gDebugFlag)
					LogMessage("TCP> %.4s\r",&eventType);
				ProcessMansionEvent(cu,eventType,refNum,p,cmdLength);
				if (cu->flags & U_Kill)	// 7/22/95 - don't continue if killed
					break;
				BlockMove(&rbuf[packetLen],rbuf,cu->tcpReceiveIdx - packetLen);
				cu->tcpReceiveIdx -= packetLen;		
				packetLen = *((long *) &rbuf[4]);
				packetLen += 12;
			}
#if DEBUG
			if (cu->tcpReceiveIdx) {
				// SysBeep(1);
				LogMessage("Packet Fragment\r");
			}
#endif
			RecvDataAsync(cu->tcpStream,cu->tcpReceiveBuffer+cu->tcpReceiveIdx,RcveBufferLength-cu->tcpReceiveIdx,&cu->tcpReceiveBlock);
			cu->tcpReceiveFlag = true;
		}
	}
	return false;
}

void ServerTCPIdle()
{
#if USE_TCP
	ServerUserPtr	curUser,nextUser;

	if (!gPrefs.allowTCP)
		return;

#if THINK_C
#if __option(profile)
				_profile = 1;
#endif
#else
#if __profile__
				ProfilerSetStatus(true);
#endif
#endif

	if (!tcpInited) {
		InitServerTCP();
		if (!tcpInited)
			return;
	}

	// Listen for New Users
	if (!gWaitingForConnection)
	{
		if (newStream == 0L)
			newStream = GetNewStream();
		if (newStream) {
			AsyncWaitForConnection(newStream,0,gPrefs.localPort,0,0,&asyncPB);
			gWaitingForConnection = true;
			LogMessage("Waiting for TCP Connection...\r");
		}
	}
	else {
		if (asyncPB->ioResult <= 0) {
			if (asyncPB->ioResult == 0) {
				// Setup New Connection
				LogMessage("Got TCP Connection!\r");
				AsyncGetConnectionData(asyncPB,&newHost,&newPort);
				SetupNewTCPUser();
				newStream = 0L;
			}
			else {
				switch (asyncPB->ioResult) {
				case commandTimeout:
					LogMessage("TCP Connection Timeout\r");
					break;
				case openFailed:	// it's happened...
					LogMessage("TCP Connection Open Failed\r");
					break;
				default:
					LogMessage("TCP Connection Error: %d\r",asyncPB->ioResult);
					break;
				}
			}
			gWaitingForConnection = 0;
		}
	}

	// Listen to Users
	for (curUser = gUserList; curUser; curUser = nextUser) 
	{
		nextUser = curUser->nextUser;			// 6/14/95  - user may get killed!!
		if (curUser->connectionType == C_MacTCP) 
		{
			//
			// 5/25/95 JAB - disconnect user if necessary
			//
			if (!(curUser->flags & U_Kill) && IdleTCPUser(curUser)) {
				ScheduleUserKill(curUser,K_CommError, 0);
				// LogoffUser(curUser->user.userID, K_CommError);
				break;	// Can't use nextUser ptr...
			}
		}
	}
#endif
#if THINK_C
#if __option(profile)
				_profile = 0;
#endif
#else
#if __profile__
				ProfilerSetStatus(false);
#endif
#endif
}


