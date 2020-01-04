/***************************************************************************
 *
 * How to deal with multiple sockets:
   a. use Non-blocking sockets, via fcntl (current implementation)

   b. use Child Processes that communicate with parent process via:
   		pipe,fifo,message queue, shared memory/semaphore,socket
   c. Aynchronous I/O (signal catching)
   4. Select system call
   
  Since fork makes copies of variables, we need a method for updating the
  database.
  
method 1
  parent pid forks a process for Accepts.
  	Accept Process sends messages to parent process via pipe.
    Accept Process spawns client socket processes which send messagees to parent process via same pipe

method 2
  parent pid forks a process for maintaining the database (maintain pid)
    parent pid spawns client socket processes which send messages to 
    maintain pid via same pipe

*
*
*
*/


/* UnixTCP.c */
#include "s-server.h"
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

int gSockfd;
struct sockaddr_in serv_addr;

Ptr		gBigBuffer2;

#define DEFAULT_PALACE_TCP_PORT			9998
#define DEFAULT_PALACE_FRONTEND_PORT	10001
#define RcveBufferLength    			32000
#define SendBufferLength        		32000

#define UseSelect			1	/* use Select to queue read requests */

#if UseSelect
fd_set	gAfds,gRfds;
int		gDtSize;
#endif

void SetNonBlocking(int fd);

void ignore_signal(int sig)
{
	LogMessage("Signal %d received\n",sig);
}

void InitDaemon()
{
	int	fd;
	int	childpid;
/**
	fd = open("/dev/tty", O_RDWR);
	(void) ioctl(fd, TIOCNOTTY, 0);
	(void) close(fd);
 **/

	umask(027);

#ifdef SIGTTOU
	signal(SIGTTOU, SIG_IGN);
#endif
#ifdef SIGTTIN
	signal(SIGTTIN, SIG_IGN);
#endif
#ifdef SIGTSTP
	signal(SIGTSTP, SIG_IGN);
#endif

	if (!gNoForkFlag) {
		if ((childpid = fork()) < 0)
			exit(0);
		else if (childpid > 0)
			exit(0);
	}

#ifdef PALBSD /* BSD */
	setpgrp(0, getpid());
	if ((fd = open("/dev/tty", O_RDWR)) >= 0) 
	{
		ioctl(fd, TIOCNOTTY, (char *) 0); /* lose control tty */
		close(fd);
	}
#else
	setpgrp();
	/* system V */
	/* second fork needed for sys 5 */
	if (!gNoForkFlag) {
		if ( ( childpid = fork()) < 0)
			exit(0);
		else if (childpid > 0)
			exit(0);
	}
#endif

#ifdef SIGHUP
	signal(SIGHUP,SIG_IGN);
#endif
#ifdef SIGPIPE
	signal(SIGPIPE, SIG_IGN);
#endif
	/* close open file descriptors */
#ifndef NOFILE
#define NOFILE	20
#endif
	for (fd = 0; fd < NOFILE; fd++)
		close(fd);
	errno = 0; /* probably got screwed from a close */

}

void InitServerTCP(int thePort)	/* todo */
{
	struct servent *pse;
	int		err;
	if (thePort == 0) {
#if BISERVER
		pse = getservbyname("pserv","tcp");
#else
		pse = getservbyname("palace","tcp");
#endif
		if (pse)
			thePort = pse->s_port;
		else
#if BISERVER
			thePort = htons(DEFAULT_PALACE_FRONTEND_PORT);
#else
			thePort = htons(DEFAULT_PALACE_TCP_PORT);
#endif
	}
	else
		thePort= htons(thePort);
	LogMessage("Using Port %d\r",ntohs(thePort));

    if ((gSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
       ErrorExit("server: can't open stream socket");


	{
		struct linger linger;
		int			reuseFlag = 1;
		linger.l_onoff = 1;	/* also try 1,0 */
		linger.l_linger = 0;
		setsockopt(gSockfd, SOL_SOCKET, SO_LINGER, (char *) &linger, sizeof(linger));
		setsockopt(gSockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuseFlag, sizeof(reuseFlag));
	}

    bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port        = thePort;

    if ((err = bind(gSockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0){
		perror("bind failed");
    	ErrorExit("server: can't bind local address");
	}

    /*
	if ((gSemID = sem_create(SEM_KEY, 1)) < 0)
		ErrorExit("server: can't open semaphore");
     */
	/* turn linger off */
#if !UseSelect
    SetNonBlocking(gSockfd);
#endif

   {
                int reuseFlag = 0;
		setsockopt(gSockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuseFlag, sizeof(reuseFlag));
   }

    listen(gSockfd, 5);

#if UseSelect
	{


		gDtSize = getdtablesize();
		FD_ZERO(&gAfds);
		FD_SET(gSockfd,&gAfds);
	}
#endif

	gBigBuffer2 = NewPtrClear(InitLenGroupBuffer);
}


#if BISERVER

void BeginSendGroup(ServerUserPtr cUser)
{
	/* notify front end of group */
	cUser->groupFlag = true; /* not needed */
	SendFrontEndEvent((BiFrontEndPtr) cUser->frontEndPtr,
						bi_begingroup,
						cUser->feIPPort,
						cUser->feIPAddr,
						0,
						NULL);
}

void EndSendGroup(ServerUserPtr cUser)
{
	/* notify front end of end group */
	cUser->groupFlag = false; /* not needed */
	SendFrontEndEvent((BiFrontEndPtr) cUser->frontEndPtr,
						bi_endgroup,
						cUser->feIPPort,
						cUser->feIPAddr,
						0,
						NULL);
}

void KillFrontEnd(BiFrontEndPtr fePtr)
{
	fePtr->killFlag = true;
	fePtr->commErrFlag = true;
}

ServerUserPtr	GetUserFromFrontEnd(LONG addr, short port)
{
	ServerUserPtr	cUser;
	for (cUser = gUserList; cUser; cUser = cUser->nextUser)
		if (cUser->feIPAddr == addr &&
			cUser->feIPPort == port)
			return cUser;
	return NULL;
}

void IdleFrontEnd(BiFrontEndPtr fePtr, Boolean needsRead)
{
	/* get packet stream from front end, process commands */
	/* equiv of Idle User */
	int				nRead;
	ServerUserPtr	curUser;
	extern int errno;

	if (fePtr->groupLen)	/* 9/28/95 */
		SendFEGroupBuffer(fePtr);

	if (!needsRead)
		return;

	nRead = read(fePtr->sockFD, fePtr->tcpReceiveBuffer+fePtr->tcpReceiveIdx, RcveBufferLength - fePtr->tcpReceiveIdx);  
	if (nRead <= 0 && (errno == 11 || errno == 35)) {
        /* resource temporarily unavailable */
		errno = 0;
		return;
	}
	if (nRead <= 0) {	/* 5/23/96 changed from < to <= */
        LogMessage("Read Error from Front End [%d,%d]\n",fePtr->ipNbr,(int) fePtr->portNbr);
		KillFrontEnd(fePtr);
		return;
	}
	else if (nRead > 0) {
		Ptr			rbuf, p;
		LONG		packetLen,eventType,cmdLength,refNum;
		BiHeader	*hdrPtr;
		int			cmd;

		rbuf = fePtr->tcpReceiveBuffer;
		fePtr->tcpReceiveIdx += nRead;
		hdrPtr = (BiHeader *) rbuf;
		packetLen = hdrPtr->length;
		packetLen += 12;
		while (fePtr->tcpReceiveIdx >= 12 && packetLen <= fePtr->tcpReceiveIdx) {
			if (packetLen < 0 || packetLen > 16000L) {
				LogMessage("Bad Packet Length!  Cmd: %d\n",&hdrPtr->cmd);
				KillFrontEnd(fePtr);
				return;
			}
			p = rbuf + 12;
			cmd = hdrPtr->cmd & 0x00FF;
			if (gDebugFlag)
				LogMessage("FE--> %d [%d,%d]\r",cmd,hdrPtr->dip,(int) hdrPtr->dport);

			switch (cmd) {
			case bi_packet:
				eventType = *((LONG *) p);	p += sizeof(LONG);
				cmdLength = *((LONG *) p);	p += sizeof(LONG);
				refNum = *((LONG *) p);		p += sizeof(LONG);
				if (gDebugFlag)
					LogMessage("TCP> %.4s\n",&eventType);
				/* determine user */
				curUser = GetUserFromFrontEnd(hdrPtr->dip,hdrPtr->dport);
				if (curUser)
					ProcessMansionEvent(curUser,eventType,refNum,p,cmdLength);
				break;
			case bi_newuser:	/* establish new user */
				curUser = NewUser();	/* incremented after accept call */
				curUser->connectionType = C_BerkSockets;
				curUser->frontEndPtr = (void *) fePtr;
				curUser->netAddress.ipAddress = hdrPtr->dip;
				curUser->feIPAddr = hdrPtr->dip;
				curUser->feIPPort = hdrPtr->dport;
				PostUserEvent(curUser,MSG_TIYID, curUser->user.userID, NULL, 0L);
				LogMessage("Sending TCP user ID [%8lxH]\n",curUser->user.userID);
				break;
			case bi_kill:	/* kill user (quietly - don't notify front end) !!! */
				curUser = GetUserFromFrontEnd(hdrPtr->dip,hdrPtr->dport);
				if (curUser)
					ScheduleUserKill(curUser, (hdrPtr->cmd >> 8) & 0x00FF, 0);
				break;
			case bi_frontendup:
				LogMessage("Front End is up [%d,%d]\n",fePtr->ipNbr,(int) fePtr->portNbr);
				fePtr->upFlag = true;
				break;
			default:
				LogMessage("Invalid Front End Event [%d,%d]\n",fePtr->ipNbr,(int) fePtr->portNbr);
				fePtr->killFlag = true;
				break;
			}
			/* skip multiple moves from the same user */
			if (fePtr->killFlag)
				break;
			BlockMove(&rbuf[packetLen],rbuf,fePtr->tcpReceiveIdx - packetLen);
			fePtr->tcpReceiveIdx -= packetLen;		
			packetLen = hdrPtr->length;
			packetLen += 12;
		}
	}
}

void FlushFEGroupBuffer(BiFrontEndPtr fePtr, int spaceNeeded)  /* same as FlushGroupBuffer */
{
	time_t	startFlush,elapsed;
	/* 10/4 lock up till buffer is sent */
	startFlush = time(NULL);
	do {
		SendFEGroupBuffer(fePtr);
	} while (LenBiGroupBuffer - fePtr->groupLen < spaceNeeded && 
				time(NULL) - startFlush < 300);
	elapsed = time(NULL) - startFlush;
	if (elapsed > 5) {
		TimeLogMessage("Long Flush Required for Front End [%d,%d] %d seconds\n",fePtr->ipNbr,(int) fePtr->portNbr,elapsed);
	}
	if (LenBiGroupBuffer - fePtr->groupLen < spaceNeeded) {
		LogMessage("FE Time Out\n");
		KillFrontEnd(fePtr);
		fePtr->groupLen = 0;
	}
	fePtr->groupFlag = 0L;
}

Boolean PostFEBuffer(BiFrontEndPtr fePtr, Ptr buffer, LONG len, LONG *bytesWritten)
{
	Ptr	p;
	int	nLeft,nWritten;

	*bytesWritten = 0;
	if (fePtr->commErrFlag) {
		*bytesWritten = len;
		return true;
	}
	/* p = fePtr->tcpSendPtr;
	BlockMove(buffer,p,len);
	 */
	p = buffer;
	nLeft = len;
	while (nLeft > 0) {
		nWritten = write(fePtr->sockFD, p, nLeft);
		if (nWritten <= 0)	{
			if (errno != 11 && errno != 35) {
				LogMessage("Write Error: %d\n",errno);
				KillFrontEnd(fePtr);
				return true;
			}
			if (nLeft == len) {	/* no bytes written, return false */
				return false;
			}
			else {	/* partial buffer sent */
				errno = 0;
				return true;
			}
		}
		*bytesWritten += nWritten;
		nLeft -= nWritten;
		p += nWritten;
	}
	return true;
}

void SendFEGroupBuffer(BiFrontEndPtr fePtr)	/* same as SendGroupBuffer */
{
	LONG	bytesWritten = 0;
	if (PostFEBuffer(fePtr, fePtr->groupBuffer,fePtr->groupLen, &bytesWritten)) {
		if (bytesWritten < fePtr->groupLen && bytesWritten > 0)
			BlockMove(fePtr->groupBuffer+bytesWritten,fePtr->groupBuffer,fePtr->groupLen - bytesWritten);
		fePtr->groupLen -= bytesWritten;
	}
}

void UserToFrontEnd(ServerUserPtr cUser, short cmd, unsigned LONG eventType, unsigned LONG refNum,
					Ptr bufferContents, LONG bufferLength)
{
	Ptr	p;

	if (gDebugFlag) {
		if (bufferLength >= 4)
			LogMessage("<-- %.4s [%08x]\r",&eventType,*((LONG *) bufferContents));
		else
			LogMessage("<-- %.4s\r",&eventType);
	}
	p = gBigBuffer2;
    /* use bcopy to avoid alignment problems */
	bcopy((char*) &eventType, p, sizeof(LONG)); 	p += sizeof(LONG);
	bcopy((char*) &bufferLength, p, sizeof(LONG));	p += sizeof(LONG);
	bcopy((char*) &refNum, p, sizeof(LONG));		p += sizeof(LONG);
	if (bufferLength)
		BlockMove(bufferContents,p,bufferLength);
	SendFrontEndEvent((BiFrontEndPtr) cUser->frontEndPtr,
						cmd,
						cUser->feIPPort,
						cUser->feIPAddr,
						bufferLength+12,
						gBigBuffer2);
}

void GlobalFrontEndEvent(LONG addr, short port, short cmd, Ptr bufferContents, LONG bufferLength)
{
	BiFrontEndPtr	curFE;
	
	for (curFE = gFrontEndList; curFE; curFE = curFE->nextFrontEnd) {
		SendFrontEndEvent(curFE,
							cmd,
							port,
							addr,
							bufferLength,
							bufferContents);
	}
}


void PostUserEvent(ServerUserPtr cUser, unsigned LONG eventType, unsigned LONG refNum,
					Ptr bufferContents, LONG bufferLength)
{
	UserToFrontEnd(cUser,bi_packet,eventType,refNum,bufferContents,bufferLength);
}


/******************************************************************************************/

void PostGlobalEvent(unsigned LONG eventType,unsigned LONG refNum,Ptr bufferContents,LONG bufferLength)
{
	Ptr	p;
	p = gBigBuffer2;
    /* use bcopy to avoid alignment problems */

	if (gDebugFlag)
		LogMessage("<-- %.4s  (global)\r",&eventType);

	bcopy((char*) &eventType, p, sizeof(LONG)); 	p += sizeof(LONG);
	bcopy((char*) &bufferLength, p, sizeof(LONG));	p += sizeof(LONG);
	bcopy((char*) &refNum, p, sizeof(LONG));		p += sizeof(LONG);
	if (bufferLength)
		BlockMove(bufferContents,p,bufferLength);

	GlobalFrontEndEvent(0,0,bi_global,gBigBuffer2,bufferLength+12);
}

/* fix to re-enable mute on client side */
void PostRoomEvent(short roomID,unsigned LONG eventType,unsigned LONG refNum,Ptr bufferContents,LONG bufferLength)
{
	Ptr		p;
	LONG	roomNumber;
	p = gBigBuffer2;
	roomNumber = roomID;

	if (gDebugFlag)
		LogMessage("<-- %.4s  (room %d)\r",&eventType,roomNumber);

    /* use bcopy to avoid alignment problems */
	bcopy((char*) &roomNumber, p, sizeof(LONG)); 	p += sizeof(LONG);
	bcopy((char*) &eventType, p, sizeof(LONG)); 	p += sizeof(LONG);
	bcopy((char*) &bufferLength, p, sizeof(LONG));	p += sizeof(LONG);
	bcopy((char*) &refNum, p, sizeof(LONG));		p += sizeof(LONG);
	if (bufferLength)
		BlockMove(bufferContents,p,bufferLength);

	GlobalFrontEndEvent(0,0,bi_room,gBigBuffer2,bufferLength+12+4);
}

void PostNeighborEvent(ServerUserPtr cUser,unsigned LONG eventType,unsigned LONG refNum,Ptr bufferContents,LONG bufferLength)
{
	Ptr		p;
	LONG	roomNumber;

	p = gBigBuffer2;
	roomNumber = cUser->user.roomID;
	/* use bcopy to avoid alignment problems */
	if (gDebugFlag)
		LogMessage("<-- %.4s  (neighbor %d)\r",&eventType,roomNumber);

	bcopy((char*) &roomNumber, p, sizeof(LONG)); 	p += sizeof(LONG);
	bcopy((char*) &eventType, p, sizeof(LONG)); 	p += sizeof(LONG);
	bcopy((char*) &bufferLength, p, sizeof(LONG));	p += sizeof(LONG);
	bcopy((char*) &refNum, p, sizeof(LONG));		p += sizeof(LONG);
	if (bufferLength)
		BlockMove(bufferContents,p,bufferLength);

	GlobalFrontEndEvent(cUser->feIPAddr,cUser->feIPPort,
					bi_room,gBigBuffer2,bufferLength+12+4);
}


void SendFrontEndEvent(BiFrontEndPtr fePtr,
						short		cmd,
						short		dport,
						int 		dip,
						int			length,
						unsigned char	*buffer)
{
	/* equiv of PostUserEvent - higher level, has buffering mechanism */
	Ptr		p;
	LONG	lengthNeeded = length+12;

	if (gDebugFlag) {
		if (cmd == bi_packet) {
			if (length >= 16)
				LogMessage("FE<-- %d [%d,%d] %.4s [%08x]\r",cmd,dip,(int) dport,buffer,*((LONG *) &buffer[12]));
			else if (length >= 12)
				LogMessage("FE<-- %d [%d,%d] %.4s\r",cmd,dip,(int) dport,buffer);
			else
				LogMessage("FE<-- %d [%d,%d]\r",cmd,dip,(int) dport);
		}
		else
			LogMessage("FE<-- %d [%d,%d]\r",cmd,dip,(int) dport);
	}
	if (fePtr->groupLen + lengthNeeded > LenBiGroupBuffer) {
		/* 1/28/95 important bug fix (args to flushgroupbuffer) */
		TimeLogMessage("Flush Required for Front End [%d,%d]\n",fePtr->ipNbr,(int) fePtr->portNbr);
		FlushFEGroupBuffer(fePtr, lengthNeeded);
	}
	p = fePtr->groupBuffer + fePtr->groupLen;
    /* use bcopy to avoid alignment problems */
	bcopy((char*) &cmd, p, sizeof(short)); 		p += sizeof(short);
	bcopy((char*) &dport, p, sizeof(short)); 	p += sizeof(short);
	bcopy((char*) &dip, p, sizeof(LONG)); 		p += sizeof(LONG);
	bcopy((char*) &length, p, sizeof(LONG)); 		p += sizeof(LONG);
	BlockMove(buffer,p,length);

	fePtr->groupLen += lengthNeeded;
	
	if (!fePtr->groupFlag)
		SendFEGroupBuffer(fePtr);
}

BiFrontEndPtr NewFrontEnd(void)
{
	BiFrontEndPtr	newFE;
	newFE = (BiFrontEndPtr)NewPtrClear(sizeof(BiFrontEnd));
	newFE->nextFrontEnd = gFrontEndList;
	gFrontEndList = newFE;
	return newFE;
}

						
void ProcessNewFrontEnd(int newsockfd, int ipNbr, short portNbr)
{
	BiFrontEndPtr	fePtr;

	FD_SET(newsockfd, &gAfds);
    SetNonBlocking(newsockfd); /* 11/8/95 */
	fePtr = NewFrontEnd();	/* incremented after accept call */
	fePtr->sockFD = newsockfd;
	fePtr->ipNbr = ipNbr;
	fePtr->portNbr = portNbr;
	fePtr->tcpReceiveBuffer = NewPtrClear(RcveBufferLength);
	fePtr->groupBuffer = NewPtrClear(LenBiGroupBuffer);		/* 6/24/95 */
	fePtr->groupLen = 0L;
	SendFrontEndEvent(fePtr, bi_serveravail,0,0,0,NULL);
}

void TerminateFrontEnd(BiFrontEndPtr dFEPtr)
{
	BiFrontEndPtr	cFEPtr,lastFEPtr=NULL;
	/* search for front end, if it exists */
	for (cFEPtr = gFrontEndList; cFEPtr; cFEPtr = cFEPtr->nextFrontEnd) {
		if (cFEPtr == dFEPtr) {
			LogMessage("Terminating Front End [%d,%d]\r",cFEPtr->ipNbr,(int) cFEPtr->portNbr);

			/* close socket & dispose of buffers */
			FD_CLR(cFEPtr->sockFD, &gAfds);
			close(cFEPtr->sockFD);
			cFEPtr->sockFD = 0;
			if (cFEPtr->tcpReceiveBuffer) {
				DisposePtr(cFEPtr->tcpReceiveBuffer);
				cFEPtr->tcpReceiveBuffer = NULL;
			}
			if (cFEPtr->groupBuffer) {
				DisposePtr(cFEPtr->groupBuffer);
				cFEPtr->groupBuffer = NULL;
			}

			/* unlink front end record and dispose of it */
			if (lastFEPtr == NULL)
				gFrontEndList = cFEPtr->nextFrontEnd;
			else
				lastFEPtr->nextFrontEnd = cFEPtr->nextFrontEnd;
			DisposePtr((Ptr) cFEPtr);

			return;
		}
		lastFEPtr = cFEPtr;
	}
}


void TCPAcceptIdle(void);
void TCPAcceptIdle(void)
{
   struct sockaddr_in cli_addr;
   int cliLen;
   int newsockfd;

    cliLen = sizeof(cli_addr);
    newsockfd = accept(gSockfd, (struct sockaddr *) &cli_addr, &cliLen);
	if (newsockfd < 0) {
		errno = 0;
		return;
	}
    LogMessage("Accepted Connection\n");

    if (newsockfd < 0)
    	ErrorExit("server: accept error");

	ProcessNewFrontEnd(newsockfd, cli_addr.sin_addr.s_addr, cli_addr.sin_port);
}

void ServerTCPIdle()    /* Front Ended Version */
{
	BiFrontEndPtr	curFE;

	struct timeval	tval,*tv;
	short	n;
	int		needsRead,needsKill=0;
	
	bcopy((char *) &gAfds,(char *) &gRfds,sizeof(gRfds));
	if (gNbrFileSends) {
		tval.tv_sec = 1;
		tval.tv_usec = 0;
		tv = &tval;
	}
	else {
		tv = NULL;
	}
	if ((n = select(gDtSize,&gRfds,(fd_set *) 0, (fd_set *) 0, tv)) < 0) {
		perror("Select Failed");
	       ErrorExit("select error");
	}
	else if (n) {
		if (FD_ISSET(gSockfd,&gRfds)) {
			TCPAcceptIdle();
		}
		for (curFE = gFrontEndList; curFE; curFE = curFE->nextFrontEnd) {
			needsRead = FD_ISSET(curFE->sockFD, &gRfds);
			if (needsRead && !curFE->killFlag) {
				IdleFrontEnd(curFE, needsRead);
			}
			if (curFE->killFlag)
				needsKill = true;
		}
	}
	while (needsKill) {
		needsKill = false;
		for (curFE = gFrontEndList; curFE; curFE = curFE->nextFrontEnd) {
			if (curFE->killFlag) {
				needsKill = true;
				TerminateFrontEnd(curFE);
				break;
			}
		}
	}
}

void DisconnectTCPUser(ServerUserPtr cUser)
{
/*
	this is done from ScheduleUserKill now...

	SendFrontEndEvent((BiFrontEndPtr) cUser->frontEndPtr,
						bi_kill,
						cUser->feIPPort,
						cUser->feIPAddr,
						0,
						NULL);
*/
}

#else  /* not a BISERVER */

void ProcessNewClient(int sockfd, LONG ipAddress)
{
    /* create new client record with sockfd */
	ServerUserPtr	cUser;

    SetNonBlocking(sockfd); /* 11/8/95 */

	/* sem_wait(gSemID); */
	cUser = NewUser();	/* incremented after accept call */
	cUser->connectionType = C_BerkSockets;
	cUser->sockfd = sockfd;
	cUser->netAddress.ipAddress = ipAddress;
	cUser->tcpReceiveBuffer = NewPtrClear(RcveBufferLength);

	PostUserEvent(cUser,MSG_TIYID, cUser->user.userID, NULL, 0L);
	LogMessage("Sending TCP user ID [%8lxH]\n",cUser->user.userID);
}


void IdleTCPUser(ServerUserPtr cUser, int needsRead);
void IdleTCPUser(ServerUserPtr cUser, int needsRead)
{
	int				nRead;
	extern int errno;

	if (cUser->groupLen)	/* 9/28/95 */
		SendGroupBuffer(cUser);

	if (!needsRead)
		return;

	nRead = read(cUser->sockfd, cUser->tcpReceiveBuffer+cUser->tcpReceiveIdx, RcveBufferLength - cUser->tcpReceiveIdx);  
	if (nRead <= 0 && (errno == 11 || errno == 35)) {
        /* resource temporarily unavailable */
		errno = 0;
		return;
	}
	if (nRead <= 0) { /*  5/23/96 changed from < to <= */
        LogMessage("Read Error from user %s %d\n",CvtToCString(cUser->user.name),errno);
		ScheduleUserKill(cUser, K_CommError, 0);
		return;
	}
	else if (nRead > 0) {
		Ptr		rbuf, p;
		LONG	packetLen,eventType,cmdLength,refNum;
		rbuf = cUser->tcpReceiveBuffer;
		cUser->tcpReceiveIdx += nRead;
		packetLen = *((LONG *) &rbuf[4]);
		packetLen += 12;
		while (cUser->tcpReceiveIdx >= 12 && packetLen <= cUser->tcpReceiveIdx) {
			if (packetLen < 0 || packetLen > 16000L) {
				LogMessage("Bad Packet Length!  Event: %.4s (%08x)\n",&eventType,eventType);
				ScheduleUserKill(cUser, K_CommError, 0);
				return;
			}
			p = rbuf;
			eventType = *((LONG *) p);	p += sizeof(LONG);
			cmdLength = *((LONG *) p);	p += sizeof(LONG);
			refNum = *((LONG *) p);		p += sizeof(LONG);
			if (gDebugFlag)
				LogMessage("TCP> %.4s\n",&eventType);
			/* skip multiple moves from the same user */
			if (!(eventType == MSG_USERMOVE && 
				  cUser->tcpReceiveIdx - packetLen > 4 &&
				  *((LONG *) &p[4]) == MSG_USERMOVE))
				ProcessMansionEvent(cUser,eventType,refNum,p,cmdLength);
			if (cUser->flags & U_Kill)
				break;
			BlockMove(&rbuf[packetLen],rbuf,cUser->tcpReceiveIdx - packetLen);
			cUser->tcpReceiveIdx -= packetLen;		
			packetLen = *((LONG *) &rbuf[4]);
			packetLen += 12;
		}
	}
}

void TCPAcceptIdle(void);
void TCPAcceptIdle(void)
{
   struct sockaddr_in cli_addr;
    int cliLen;
    int newsockfd;

    cliLen = sizeof(cli_addr);
    newsockfd = accept(gSockfd, (struct sockaddr *) &cli_addr, &cliLen);
	if (newsockfd < 0) {
		errno = 0;
		return;
	}
    LogMessage("Accepted Connection\n");

    if (newsockfd < 0)
    	ErrorExit("server: accept error");

#if UseSelect
	FD_SET(newsockfd, &gAfds);
#endif

#if !UseSelect
	SetNonBlocking(newsockfd);
#endif
	ProcessNewClient(newsockfd, (LONG) cli_addr.sin_addr.s_addr);
}

void ServerTCPIdle()    /*todo */
{
	ServerUserPtr	curUser;
	static			int needsPush;

#if UseSelect
	struct timeval	tval,*tv;
	short	n;
	int		needsRead;

	bcopy((char *) &gAfds,(char *) &gRfds,sizeof(gRfds));
	if (needsPush || gToBeKilled != NULL || gNbrFileSends) {
		tval.tv_sec = 1;
		tval.tv_usec = 0;
		tv = &tval;
	}
	else {
		tv = NULL;
	}
	if ((n = select(gDtSize,&gRfds,(fd_set *) 0, (fd_set *) 0, tv)) < 0) {
		perror("Select Failed");
	       ErrorExit("select error");
	}
	else if (n || needsPush) {
		needsPush = 0;
		if (FD_ISSET(gSockfd,&gRfds)) {
			TCPAcceptIdle();
		}
		/* Note: will lockup on downloads for single users... */
		for (curUser = gUserList; curUser; curUser = curUser->nextUser) 
		{
			needsRead = FD_ISSET(curUser->sockfd, &gRfds);
			if ((curUser->groupLen || needsRead) && !(curUser->flags & U_Kill))
			{
				IdleTCPUser(curUser, needsRead);
				if (curUser->groupLen > 0)
					needsPush = 1;
			}
		}
	}
#else
	TCPAcceptIdle();
	for (curUser = gUserList; curUser; curUser = curUser->nextUser) 
	{
		if (!(curUser->flags & U_Kill))
			IdleTCPUser(curUser, 1);
	}
#endif
}

Boolean PostServerTCPBuffer(ServerUserPtr cUser, Ptr buffer, LONG len, LONG *bytesWritten)
{
	Ptr	p;
	int	nLeft,nWritten;

	*bytesWritten = 0;
	if (cUser->flags & U_CommError) {
		*bytesWritten = len;
		return true;
	}
	/* p = cUser->tcpSendPtr;
	 * BlockMove(buffer,p,len);
	 */
	p = buffer;
	nLeft = len;
	while (nLeft > 0) {
		nWritten = write(cUser->sockfd, p, nLeft);
		if (nWritten <= 0)	{
			if (errno != 11 && errno != 35) {
				LogMessage("Write Error: %d\n",errno);
				ScheduleUserKill(cUser,K_CommError, 0);
				return true;
			}
			if (nLeft == len) {	/* no bytes written, return false */
				return false;
			}
			else {	/* partial buffer sent, lockup */
				/* LogMessage("Write Lock\n"); */
				errno = 0;
				return true;
			}
		}
		*bytesWritten += nWritten;
		nLeft -= nWritten;
		p += nWritten;
	}
	return true;
}

void DisconnectTCPUser(ServerUserPtr cUser)
{
	if (cUser == NULL)
		return;
	if (cUser->tcpReceiveBuffer) {
		DisposePtr(cUser->tcpReceiveBuffer);
		cUser->tcpReceiveBuffer = NULL;
	}
	/*
		if (cUser->tcpSendBuffer) {
			DisposePtr(cUser->tcpSendBuffer);
			cUser->tcpSendBuffer = NULL;
		}
	*/
	if (cUser->sockfd) {
#if UseSelect
		FD_CLR(cUser->sockfd, &gAfds);
#endif
		close(cUser->sockfd);
		cUser->sockfd = 0;
	}
}

#endif

/* 1/9/95
   returns TRUE if anything written
   returns number of bytes written
 */


void CleanupServerTCP() /* todo */
{
	while (gToBeKilled)
		ServerIdle();
	/* Kill all Users */
#if UseSelect
	FD_CLR(gSockfd, &gAfds);
#endif
	close(gSockfd);
	/* sem_close(gSemID); */
}

void SetNonBlocking(int fd)
{
	int flags;
	if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
		err_dump("fcntl err");
	fcntl(fd, F_SETFL, flags | FNDELAY);
}
