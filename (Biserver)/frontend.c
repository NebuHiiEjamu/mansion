/* frontend.c */
#include "frontend.h"

/* todo:
	[ ] make sure server is bullet proof when clients attempt to connect on frontend port
       note: still a problem with clients staying connected, but otherwise should be ok...
       might want to make a list of "known" ports for front ends...
       	
	lower prio:
		add support for file sends implemented by front end
		
		add muting support to room/neighbor events
			(server will have to communicate additional info to front end)

		add server full / avail support

		consider adding ip-based banlist support on front ends

 */

int gListenSocket,gServerSocket;

fd_set			gAfds,gRfds;
int				gDtSize;
FEUserPtr		gFEUserList;
FEActionPtr		gFEActionList;

SockBuffersPtr	gServerSP;

char gLogFileName[512] = "pserver.log";

int				gNbrFileSends; /* unimplmented as yet */
Boolean				gDebugFlag,gQuitFlag,gNoFork;

void SetNonBlocking(int fd);


void OpenLog(char *name)
{
	strcpy(gLogFileName, name);
}

void CloseLog(void)
{
}

void err_sys(char *str)
{
	ErrorExit(str);
}

void err_dump(char *str)
{
	ErrorExit(str);
}

void LogStringToFile(char *str,...)
{
	FILE	*lFile;
	char	tbuf[512];

	va_list args;
	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);


	if ((lFile = fopen(gLogFileName,"a")) != NULL) {
		fputs(tbuf,lFile);
		fclose(lFile);
	}
}

void ErrorMessage(char *str,...)
{
	FILE	*lFile;
	char	tbuf[512];
	va_list args;
	
	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);


	if ((lFile = fopen(gLogFileName,"a")) != NULL) {
		fputs(tbuf,lFile);
		fclose(lFile);
	}
}

void ReportError(short code, char *name)
{
	if (name && name[0]) {
		if (code)
			LogStringToFile("Error %d in %s\n",code,name);
		else
			LogStringToFile("%s\n",name);
	}
	else
		LogStringToFile("Error %d\n",code,name);
}

void ErrorExit(char *str,...)
{
	char	tbuf[512];
	va_list args;
	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	LogStringToFile("%s\n",tbuf);

	abort();
}

void LogMessage(char *str,...)
{
	int		len;
	char	tbuf[512];
	va_list args;
	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);
	len = strlen(tbuf);
	if (len && tbuf[len-1] == '\r')
		tbuf[len-1] = '\n';

	LogStringToFile("%s",tbuf);
}

void LogString(char *str)
{
	int		len;
	char	tbuf[512];
	strcpy(tbuf,str);
	len = strlen(tbuf);
	if (len && tbuf[len-1] == '\r')
		tbuf[len-1] = '\n';

	LogStringToFile("%s",tbuf);
}

void TimeLogMessage(char *str,...)
{
	int		len;
	char	*timeStr;
	time_t	tim;
	struct tm *tm;
	char	tbuf[512];

	va_list args;
	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);
	time(&tim);
	tm = localtime(&tim);
	timeStr = asctime(tm);
	len = strlen(tbuf);
	if (len && tbuf[len-1] == '\r')
		tbuf[len-1] = '\n';
	LogStringToFile("%s : %s",timeStr,tbuf);
}

/* USER STUFF */


FEUserPtr NewFEUser(void)
{
	/* Allocate and initialize new feuser record */
	FEUserPtr	nUser;
	nUser = (FEUserPtr) NewPtrClear(sizeof(FEUser));
	if (nUser) {
		nUser->nextUser = gFEUserList;
		gFEUserList = nUser;
	}
	return nUser;
}

FEUserPtr GetFEUser(int ipAddr, unsigned short ipPort)
{
	FEUserPtr	cUser;
	for (cUser = gFEUserList; cUser != NULL; cUser = cUser->nextUser) {
		if (cUser->sockBuf.ipAddr == ipAddr &&
			cUser->sockBuf.ipPort == ipPort)
				return cUser;
	}
	return NULL;
}

void FEAcceptNewUser(void)
{	
	/* establish new user connection (gListenSocket) */
    struct sockaddr_in cli_addr;
    int cliLen;
    int newsockfd;
	FEUserPtr	cUser;

    cliLen = sizeof(cli_addr);
    newsockfd = accept(gListenSocket, (struct sockaddr *) &cli_addr, &cliLen);
	if (newsockfd < 0) {
		errno = 0;
		return;
	}
    if (newsockfd < 0)
    	ErrorExit("server: accept error");

	FD_SET(newsockfd, &gAfds);

    SetNonBlocking(newsockfd);

	cUser = NewFEUser();
	InitSockBuffers(newsockfd, cli_addr.sin_addr.s_addr, cli_addr.sin_port,
					ClientRcveBufferLength, ClientSendBufferLength,
					 &cUser->sockBuf);

    LogMessage("Accepted Connection [%s,%d]\n",SockIPToString(&cUser->sockBuf),(int) cUser->sockBuf.ipPort);

	/* Post event to server announcing new user - provide user's ip/port */
	/* Server will respond by sending user a TIYID */
	PostSockFrontEndEvent(gServerSP, bi_newuser, 
							cUser->sockBuf.ipAddr,
							cUser->sockBuf.ipPort,
							0, NULL);
}

void ScheduleUserKill(FEUserPtr cUser, int why, int notifyServer)
{
	/* set flags to kill user on next idle */
	if (!cUser->killFlag) {
		cUser->killFlag = true;
		cUser->killReason = why;
		cUser->notifyServer = notifyServer;
	}
}

void TerminateUser(FEUserPtr dUser, int reason, int notifyServer)
{
	FEUserPtr	cUser,lastUser=NULL;
	/* search for user, if it exists */
	for (cUser = gFEUserList; cUser; cUser = cUser->nextUser) {
		if (cUser == dUser) {
			LogMessage("Terminating [%s,%d]\r",SockIPToString(&cUser->sockBuf),(int) cUser->sockBuf.ipPort);
			/* post message to user */
			if (reason != K_CommError && reason != K_LoggedOff && !cUser->sockBuf.commErrFlag)
				PassUserEvent(cUser, MSG_SERVERDOWN, reason, NULL, 0L);
			/* post message to server */
			if (notifyServer)
				PostSockFrontEndEvent(gServerSP, bi_kill | (reason << 8),
							cUser->sockBuf.ipAddr,
							cUser->sockBuf.ipPort,
							0, NULL);

			/* close socket & dispose of buffers */
			CloseSockBuffers(&cUser->sockBuf);

			/* dispose of related action records */
			if (cUser->userID) {
				DelActionRecord(cUser->userID, 0, 0);
				DelActionRecord(0, cUser->userID, 0);
			}

			/* unlink user record and dispose of it */
			if (lastUser == NULL)
				gFEUserList = cUser->nextUser;
			else
				lastUser->nextUser = cUser->nextUser;
			DisposePtr((Ptr) cUser);

			return;
		}
		lastUser = cUser;
	}
}

void FEUserRead(FEUserPtr cUser)
{
	/* read data from user cUser->sockBuf.socket into cUser->sockBuf.tcpReceiveBuffer */
	/* pass to server */
	int				nRead;
	extern int 		errno;


	if (cUser->sockBuf.commErrFlag)	/* 8/9/96 */
		return;

	nRead = read(cUser->sockBuf.socket, cUser->sockBuf.tcpReceiveBuffer+cUser->sockBuf.tcpReceiveIdx, 
				 cUser->sockBuf.maxInLength - cUser->sockBuf.tcpReceiveIdx);
	if (nRead <= 0 && (errno == 11 || errno == 35)) {
        /* resource temporarily unavailable */
		errno = 0;
		return;
	}
	/* new!! 5/23/96  TCP/IP FAQ indicatees that 0 indicates closed socket changed from < to <= */
	else if (nRead <= 0) {
        TimeLogMessage("Read Error from user %s %d\n",SockIPToString(&cUser->sockBuf),errno);
		ScheduleUserKill(cUser, K_CommError, 1);
		return;
	}
	else if (nRead > 0) {
		Ptr		rbuf, p;
		LONG	packetLen,eventType,cmdLength,refNum;

		cUser->sockBuf.dataInCounter += nRead;	/* 8/8/96 */
		rbuf = cUser->sockBuf.tcpReceiveBuffer;
		cUser->sockBuf.tcpReceiveIdx += nRead;
		packetLen = *((LONG *) &rbuf[4]);
		packetLen += 12;
		while (cUser->sockBuf.tcpReceiveIdx >= 12 && packetLen <= cUser->sockBuf.tcpReceiveIdx) {
			if (packetLen < 0 || packetLen > 16000L) {
				LogMessage("Bad Packet Length!  Event: %.4s (%x)\n",&eventType,eventType);
				ScheduleUserKill(cUser, K_CommError, 1);
				return;
			}
			p = rbuf;
			eventType = *((LONG *) p);	p += sizeof(LONG);
			cmdLength = *((LONG *) p);	p += sizeof(LONG);
			refNum = *((LONG *) p);		p += sizeof(LONG);
			if (gDebugFlag)
				LogMessage("TCP> %.4s\n",&eventType);
			/* lag optimization - skip multiple moves from the same user */
			if (!(eventType == MSG_USERMOVE && 
				  cUser->sockBuf.tcpReceiveIdx - packetLen > 4 &&
				  *((LONG *) &p[4]) == MSG_USERMOVE)) {
				PostSockFrontEndEvent(gServerSP, bi_packet, 
										cUser->sockBuf.ipAddr,
										cUser->sockBuf.ipPort,
										packetLen,
										p-12);
			}
			if (eventType == MSG_LOGOFF) {
				ScheduleUserKill(cUser, K_LoggedOff, 0);
				return;
			}
			BlockMove(&rbuf[packetLen],rbuf,cUser->sockBuf.tcpReceiveIdx - packetLen);
			cUser->sockBuf.tcpReceiveIdx -= packetLen;		
			packetLen = *((LONG *) &rbuf[4]);
			packetLen += 12;
		}
	}
}

/* SERVER STUFF */
void Process_bi_packet(BiHeader *hdrPtr, Ptr p)
{
	FEUserPtr	curUser;
	LONG		eventType;

	curUser = GetFEUser(hdrPtr->dip,hdrPtr->dport);

	if (curUser) {
		if (gDebugFlag) {
			if (hdrPtr->length >= 16)
				LogMessage("TCP< %.4s [%x]\r",p,*((long *) &p[12]));
			else
				LogMessage("TCP< %.4s\r",p);

		}

		eventType = *((LONG *) p);	/* 5/16/96 get local copy of userid */
		switch (eventType) {
		case MSG_TIYID:
			curUser->userID = *((LONG *) (p+8));
			curUser->active = true;  /* 7/25/96 JAB */
			break;
		}

		PostSockBuffer(&curUser->sockBuf, p, hdrPtr->length);
	}
	else
		LogMessage("bi_packet User not found\r");
}


/* deal with multiplexing of room/neighbor event */
/* if ip/port is 0,0 it goes to everyone in room,
   otherwise to everyone except the person specified by ip/port */
/* !!! will need to add support for muting */
void Process_bi_room(BiHeader *hdrPtr, Ptr p)
{
	LONG		roomNumber;
	LONG		bufferLength = hdrPtr->length-sizeof(LONG);
	FEUserPtr	curUser;
	LONG		eventType,refNum;			/* 5/16/96 */
	Boolean		muteEvent = false;	/* 5/16/96 */

	roomNumber = *((LONG *) p);	p += sizeof(LONG);

	eventType = *((LONG *) p);	/* 5/16/96 add support for muting */
	refNum = *((LONG *) (p+8));

	switch (eventType) {
	case MSG_TALK:
	case MSG_XTALK:
		if (refNum != 0)
			muteEvent = true;
		break;
	}

	if (gDebugFlag)
		LogMessage("TCP< %.4s (room %d)\r",p,roomNumber);

	for (curUser = gFEUserList; curUser != NULL; curUser = curUser->nextUser) {
		if (curUser->userRoomID == roomNumber &&
		    curUser->active && /* 7/25/96 */
			(curUser->sockBuf.ipAddr != hdrPtr->dip ||
			 curUser->sockBuf.ipPort != hdrPtr->dport) &&
			 (!muteEvent || !IsActionRecord(curUser->userID, refNum,UA_Mute)))
			PostSockBuffer(&curUser->sockBuf, p, bufferLength);
	}
}

void Process_bi_assoc(BiHeader *hdrPtr, Ptr p)
{
	FEUserPtr	curUser;

	/* associate user with room number */
	curUser = GetFEUser(hdrPtr->dip,hdrPtr->dport);
	if (curUser)
		curUser->userRoomID = *((LONG *) p);
	else
		LogMessage("bi_assoc User not found\r");
}

void Process_bi_addaction(BiHeader *hdrPtr, Ptr p)
{
	LONG		srcUsr,dstUsr,command;

	/* add mute record */
	srcUsr = *((LONG *) p);
	dstUsr = *((LONG *) (p+4));
	command = *((LONG *) (p+8));
	AddActionRecord(srcUsr,dstUsr,command);
}

void Process_bi_delaction(BiHeader *hdrPtr, Ptr p)
{
	LONG		srcUsr,dstUsr,command;

	/* add mute record */
	srcUsr = *((LONG *) p);
	dstUsr = *((LONG *) (p+4));
	command = *((LONG *) (p+8));
	DelActionRecord(srcUsr,dstUsr,command);
}


void Process_bi_userflags(BiHeader *hdrPtr, Ptr p)
{
	FEUserPtr	curUser;
	/* set user's flags */
	curUser = GetFEUser(hdrPtr->dip,hdrPtr->dport);
	if (curUser)
		curUser->userFlags = *((LONG *) p);
	else
		LogMessage("bi_userflags User not found\r");
}

void Process_bi_kill(BiHeader *hdrPtr, Ptr p)
{		
	FEUserPtr	curUser;
	/* kill user, don't notify server */
	curUser = GetFEUser(hdrPtr->dip,hdrPtr->dport);
	if (curUser)
		ScheduleUserKill(curUser, (hdrPtr->cmd >> 8) & 0x00FF, 0);
	/* else
		LogMessage("bi_kill User not found\r");
	 */
}

void Process_bi_global(BiHeader *hdrPtr, Ptr p)
{
	FEUserPtr	curUser;
	if (gDebugFlag)
		LogMessage("TCP< %.4s (global)\r",p);
	for (curUser = gFEUserList; curUser != NULL; curUser = curUser->nextUser) {
		if (curUser->active) /* 7/25/96 don't send globals to new users */
			PostSockBuffer(&curUser->sockBuf, p, hdrPtr->length);
	}
}

void Process_bi_serverdown(BiHeader *hdrPtr, Ptr p)
{
	/* server is going down */
	gQuitFlag = true;
}

void Process_bi_serverfull(BiHeader *hdrPtr, Ptr p)
{
	/* !!! server is full - start rejecting connections */
}

void Process_bi_serveravail(BiHeader *hdrPtr, Ptr p)
{
	/* server is not full - start accepting connections */
	PostSockFrontEndEvent(gServerSP, bi_frontendup, 0, 0, 0, NULL);
}

void Process_bi_begingroup(BiHeader *hdrPtr, Ptr p)
{
	FEUserPtr	curUser;

	/* begin grouping */
	curUser = GetFEUser(hdrPtr->dip,hdrPtr->dport);
	if (curUser)
		BeginSockGroup(&curUser->sockBuf);
	else
		LogMessage("bi_begingroup User not found\r");
}

void Process_bi_endgroup(BiHeader *hdrPtr, Ptr p)
{
	FEUserPtr	curUser;

	/* end grouping */
	curUser = GetFEUser(hdrPtr->dip,hdrPtr->dport);
	if (curUser)
		EndSockGroup(&curUser->sockBuf);
	else
		LogMessage("bi_endgroup User not found\r");
}

void FEServerRead(void)
{
	/* read data from server (gServerSocket) */
	/* process front end cmds */
	int				nRead;
	extern int 		errno;


	nRead = read(gServerSP->socket, gServerSP->tcpReceiveBuffer+gServerSP->tcpReceiveIdx, 
				 gServerSP->maxInLength - gServerSP->tcpReceiveIdx);  
	if (nRead <= 0 && (errno == 11 || errno == 35)) {
        /* resource temporarily unavailable */
		errno = 0;
		return;
	}
    else if (nRead <= 0) {	/* 5/23/95 */
    	TimeLogMessage("Read Error from Server [%d] - Terminating\n",nRead);
        gServerSP->commErrFlag = true;
        gQuitFlag = true;
        return;
    }
	else if (nRead > 0) {
		Ptr			rbuf, p;
		LONG		packetLen;
		BiHeader	*hdrPtr;
		int			cmd;

		gServerSP->dataInCounter += nRead;	/* 8/9/96 */
		rbuf = gServerSP->tcpReceiveBuffer;
		gServerSP->tcpReceiveIdx += nRead;
		hdrPtr = (BiHeader *) rbuf;
		packetLen = hdrPtr->length;
		packetLen += 12;
		while (gServerSP->tcpReceiveIdx >= 12 && packetLen <= gServerSP->tcpReceiveIdx) {
			if (packetLen < 0 || packetLen > 16000L) {
				LogMessage("Bad Packet Length!  Cmd: %d - terminating\n",&hdrPtr->cmd);
				gServerSP->commErrFlag = true;
				gQuitFlag = true;
				return;
			}
			p = rbuf + 12;
			cmd = hdrPtr->cmd & 0x00FF;

			if (gDebugFlag)
				LogMessage("-->FE %d [%d,%d]\r",cmd,hdrPtr->dip,(int) hdrPtr->dport);

			switch (cmd) {
			case bi_packet:		Process_bi_packet(hdrPtr, p);			break;
			case bi_global:		Process_bi_global(hdrPtr, p);			break;
			case bi_room:		Process_bi_room(hdrPtr, p);				break;
			case bi_serverdown:	Process_bi_serverdown(hdrPtr, p);		break;
			case bi_serverfull:	Process_bi_serverfull(hdrPtr, p);		break;
			case bi_serveravail: Process_bi_serveravail(hdrPtr, p);		break;
			case bi_begingroup:	Process_bi_begingroup(hdrPtr, p);		break;
			case bi_endgroup:	Process_bi_endgroup(hdrPtr, p);		break;
			case bi_assoc:		Process_bi_assoc(hdrPtr, p);			break;
			case bi_userflags:	Process_bi_userflags(hdrPtr, p);		break;
			case bi_addaction:	Process_bi_addaction(hdrPtr, p);		break;
			case bi_delaction:	Process_bi_delaction(hdrPtr, p);		break;
			case bi_kill:		Process_bi_kill(hdrPtr, p);				break;
			}
			/* skip multiple moves from the same user */
			if (gServerSP->commErrFlag)
				break;
			BlockMove(&rbuf[packetLen],rbuf,gServerSP->tcpReceiveIdx - packetLen);
			gServerSP->tcpReceiveIdx -= packetLen;		
			packetLen = hdrPtr->length;
			packetLen += 12;
		}
	}
}

/* SHELL STUFF */

void FrontEndIdle(void)
{
	struct timeval	tval,*tv;
	short			n;
	int				needsRead,needsKill=0;
	FEUserPtr		cUser;

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

	/* accept a new connection, if need be */
	if (FD_ISSET(gListenSocket,&gRfds)) {
		FEAcceptNewUser();
	}


	/* read from the server if need be */
	if (FD_ISSET(gServerSocket, &gRfds)) {
		FEServerRead();
	}
	/* send server data, if need be */
	SockIdle(gServerSP);

	/* for each user */
	for (cUser = gFEUserList; cUser; cUser = cUser->nextUser) {
		needsRead = FD_ISSET(cUser->sockBuf.socket, &gRfds);
		/* if select readbit is set */
		if (needsRead) {
			/* read a packet, pass it to server */
			FEUserRead(cUser);
		}

		/* if data remaining to be sent */
			/* attempt to send the data */
		if (!cUser->killFlag && !cUser->sockBuf.commErrFlag)
			SockIdle(&cUser->sockBuf);

		if (cUser->sockBuf.commErrFlag)
			ScheduleUserKill(cUser, K_CommError, 1);

		if (cUser->killFlag)
			needsKill = 1;
	}

	while (needsKill) {
		/* kill all users who need it */
		needsKill = false;
		for (cUser = gFEUserList; cUser; cUser = cUser->nextUser) {
			if (cUser->killFlag) {
				needsKill = true;
				TerminateUser(cUser, cUser->killReason, cUser->notifyServer);
				break;
			}
		}
	}
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

	if (!gNoFork) {
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
	if (!gNoFork) {
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

void InitFrontEnd(int listenPort, int serverPort, char *servHostAddr)
{
	struct servent *pse;
	struct hostent *phe;
	struct sockaddr_in fe_addr,serv_addr;
	int		err;

	/* establish a connection with the server */

	if (serverPort == 0) {
		pse = getservbyname("pserv","tcp");
		if (pse)
			serverPort = pse->s_port;
		else
			serverPort = htons(DEFAULT_PALACE_FRONTEND_PORT);
	}
	else
		serverPort= htons(serverPort);
	LogMessage("Using Server Port %d\r",ntohs(serverPort));
    if ((gServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
       ErrorExit("server: can't open stream socket");
	{
		struct linger linger;
		int			reuseFlag = 1;
		linger.l_onoff = 1;	/* also try 1,0 */
		linger.l_linger = 0;
		setsockopt(gServerSocket, SOL_SOCKET, SO_LINGER, (char *) &linger, sizeof(linger));
		setsockopt(gServerSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &reuseFlag, sizeof(reuseFlag));
	}
    bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port   = serverPort;
	if (servHostAddr == NULL || *servHostAddr == 0) {
		/* use local host id if unspecified */
		serv_addr.sin_addr.s_addr = gethostid();
	}
	else if ((phe = gethostbyname(servHostAddr)) != NULL) {
		BlockMove(phe->h_addr, (char *) &serv_addr.sin_addr, phe->h_length);
	}
	else if ((serv_addr.sin_addr.s_addr = inet_addr(servHostAddr)) == INADDR_NONE) {
		ErrorExit("Can't get host");
	}

   if ((err = connect(gServerSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
		/* if connection fails, exit, with a useful msg */
		perror("can't connect to server");
    	ErrorExit("front end: can't connect to server");
   }
   {
        int reuseFlag = 0;
		setsockopt(gServerSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &reuseFlag, sizeof(reuseFlag));
   }
	gServerSP = (SockBuffersPtr) NewPtrClear(sizeof(SockBuffers));
	InitSockBuffers(gServerSocket,serv_addr.sin_addr.s_addr,serv_addr.sin_port,
					ServerRcveBufferLength, ServerSendBufferLength,
					gServerSP);

	/**/
	/* start listening on usual port */
	/**/
	if (listenPort == 0) {
		pse = getservbyname("palace","tcp");
		if (pse)
			listenPort = pse->s_port;
		else
			listenPort = htons(DEFAULT_PALACE_TCP_PORT);
	}
	else
		listenPort= htons(listenPort);
	LogMessage("Listening on Port %d\r",ntohs(listenPort));
    if ((gListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
       ErrorExit("server: can't open stream socket");
	{
		struct linger linger;
		int			reuseFlag = 1;
		linger.l_onoff = 1;	/* also try 1,0 */
		linger.l_linger = 0;
		setsockopt(gListenSocket, SOL_SOCKET, SO_LINGER, (char *) &linger, sizeof(linger));
		setsockopt(gListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &reuseFlag, sizeof(reuseFlag));
	}
    bzero((char *) &fe_addr, sizeof(fe_addr));
	fe_addr.sin_family = AF_INET;
	fe_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	fe_addr.sin_port        = listenPort;

    if ((err = bind(gListenSocket, (struct sockaddr *) &fe_addr, sizeof(fe_addr))) < 0){
		perror("bind failed");
    	ErrorExit("server: can't bind local address");
	}
   {
        int reuseFlag = 0;
		setsockopt(gListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &reuseFlag, sizeof(reuseFlag));
   }
   listen(gListenSocket, 5);
   {


		gDtSize = getdtablesize();
		FD_ZERO(&gAfds);
		FD_SET(gListenSocket,&gAfds);
		FD_SET(gServerSocket,&gAfds);
   }

}

void CloseFrontEnd(void)
{
	/* shut down all users */
	while (gFEUserList)
		TerminateUser(gFEUserList, K_ServerDown, false);

	if (gListenSocket)
		close(gListenSocket);

	if (gServerSocket)
		close(gServerSocket);
}

int main(int argc, char **argv) 
{
	int		i, listenPort=0,serverPort=0;
	char	*serverAddr = NULL;
	char	*p;
	char	logFileName[512];
	char	*syntaxMsg = "Syntax: %s [-p listenport#] [-a serveraddr] [-s serverport# ] [-d] [-l logfile]\n";

	/* Read command line arguments, set debug flag */
	sprintf(logFileName,"%s.log",argv[0]);

	for (i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'a':
			case 'A':
				if (argv[i][2])
					serverAddr = &argv[i][2];
				else {
					++i;
					serverAddr = argv[i];
				}
				break;
			case 'l':
			case 'L':
				if (argv[i][2])
					p = &argv[i][2];
				else {
					++i;
					p = argv[i];
				}
				strcpy(logFileName,p);
				break;
			case 'd':
			case 'D':
				gDebugFlag = 1;
				LogMessage("Debug On\r");
				break;
			case 'n':
			case 'N':
				gNoFork = 1;
				LogMessage("No Fork\r");
				break;
			case 'p':
			case 'P':
				if (argv[i][2])
					p = &argv[i][2];
				else {
					++i;
					p = argv[i];
				}
				listenPort = atoi(p);
				break;
			case 's':
			case 'S':
				if (argv[i][2])
					p = &argv[i][2];
				else {
					++i;
					p = argv[i];
				}
				serverPort = atoi(p);
				break;
			default:
				fprintf(stderr,syntaxMsg,argv[0]);
				exit(0);
				break;
			}
		}
		else {
			if (isdigit(argv[i][0]))
				listenPort = atoi(argv[i]);
			else {
				fprintf(stderr,syntaxMsg,argv[0]);
				exit(0);
			}
		}
	}

	InitDaemon();
	OpenLog(logFileName);
	InitFrontEnd(listenPort,serverPort,serverAddr);
	LogMessage("Front End Active\n");
	/* Main Loop */
	while (!gQuitFlag) {
		FrontEndIdle();
	}
	LogMessage("Shutting down\n");
	/* Done with Main Loop */

	CloseFrontEnd();
	CloseLog();
	return 0;
}

void AddActionRecord (LONG srcUser, LONG dstUser, LONG action)
{
	FEActionPtr	newRec;
	newRec = (FEActionPtr) NewPtrClear(sizeof(FEActionRec));
	if (newRec) {
		newRec->nextRec = gFEActionList;
		newRec->srcUser = srcUser;
		newRec->dstUser = dstUser;
		newRec->action = action;
		gFEActionList = newRec;
	}
}

/* 0 in srce means all records that match dst */
/* 0 in dest means all records that match src */
/* 0 in action means all actions */
void DelActionRecord (LONG srcUser, LONG dstUser, LONG action)
{
	FEActionPtr	lastRec = NULL,curRec,nextRec;

	if (srcUser == 0 && dstUser == 0)
		return;

	for (curRec = gFEActionList; curRec; curRec = nextRec) 
	{
		nextRec = curRec->nextRec;
		if ((srcUser == 0 || curRec->srcUser == srcUser) &&
			(dstUser == 0 || curRec->dstUser == dstUser) &&
			(action == 0 || curRec->action == action))
		{
			if (lastRec)
				lastRec->nextRec = curRec->nextRec;
			else
				gFEActionList = curRec->nextRec;
			DisposePtr((Ptr) curRec);
		}
		else
			lastRec = curRec;
	}
}


Boolean IsActionRecord(LONG srcUser, LONG dstUser, LONG action)
{
	FEActionPtr	curRec;
	for (curRec = gFEActionList; curRec; curRec = curRec->nextRec) 
	{
		if ((srcUser == 0 || curRec->srcUser == srcUser) &&
			(dstUser == 0 || curRec->dstUser == dstUser) &&
			(action == 0 || curRec->action == action))
		{
			return true;
		}
	}
	return false;
}
