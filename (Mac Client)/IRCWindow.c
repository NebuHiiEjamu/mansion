/************************************************************************************
 * IRCWindow.c
 *
 * pasadena.ca.us.undernet.org 6667
 * w6yx.stanford.edu 6666
 ************************************************************************************/
#include "User.h"
#include "IRCWindow.h"
#include "CvtAddr.h"
#include "UserMenus.h"

// Create a new main window using a 'WIND' template from the resource fork
//
WindowPtr	gIRCWindow;
Boolean		gIRCClosing;

// Add to gIRCWindow...
UserRec		uRec;
RoomRec		rRec;

void LogMessage(char *str,...);
short HostnameDialog(short connectType);

void ProcessIRCMacro(char *str)
{
}

void TestBug()
{
	OSErr	oe;
	short	refNum;
	if ((oe = FSOpen("\p#blueroom",0,&refNum)) == noErr) {
		LogMessage("FSOpen ok\r");
		FSClose(refNum);
	}
	else
		LogMessage("FSOpen nok\r");
}

void tokenize(char *buff)
{
	IRCWindowPtr	tWin = (IRCWindowPtr) gIRCWindow;
	char *dp;
	short	i;
	i = 0;
	dp = tWin->tokens[i];
	while (isspace(*buff))
		++buff;
	while (*buff && *buff != '\r' && *buff != '\n') {
		if (!isspace(*buff)) {
			while (*buff && !isspace(*buff))
				*(dp++) = *(buff++);
		}
		else {
			*dp = 0;
			buff++;
			while (isspace(*buff))
				++buff;
			++i;
			if (i >= MaxTokens-1)
				break;
			dp = tWin->tokens[i];
		}
	}
	*dp = 0;
	++i;
	tWin->tokens[i][0] = 0;
}

void ConvertIRCString (LocalUserRecPtr cUser, char *content, Boolean privateFlag)
{
	short	n;

	if (cUser == NULL)
		return;

	switch (*((long *) content)) {
	case 'loc ':
		{
			Point	p;
			sscanf(content,"%*s %d,%d",&p.h,&p.v);
			ProcessMansionEvent('uLoc',cUser->user.userID,(Ptr) &p,sizeof(Point));
		}
		break;
	case 'col ':
		{
			short	col;
			sscanf(content,"%*s %d",&col);
			ProcessMansionEvent('usrC',cUser->user.userID,(Ptr) &col,sizeof(short));
		}
		break;
	case 'fac ':
		{
			short	face;
			sscanf(content,"%*s %d",&face);
			ProcessMansionEvent('usrF',cUser->user.userID,(Ptr) &face,sizeof(short));
			if (face != 0) {
				if (!cUser->ircSparkyAware) {
					cUser->ircSparkyAware = true;
					if (!privateFlag)
						CommPrintf("PRIVMSG %.*s :fac %d\r",
								cUser->user.name[0],&cUser->user.name[1],
								cUser->user.faceNbr);
				}
			}
		}
		break;
	case 'prp ':
		{
			short	prp[3];
			sscanf(content,"%*s %d,%d,%d",&prp[0],&prp[1],&prp[2]);
			ProcessMansionEvent('usrP',cUser->user.userID,(Ptr) &prp[0],sizeof(short)*3);
		}
		break;
	default:
		if (strcmp(content,":)") == 0 || strcmp(content,":-)") == 0) {
			n = 1;
			ProcessMansionEvent('usrF',cUser->user.userID,(Ptr) &n,sizeof(short));
		}
		else if (strcmp(content,":O") == 0 || strcmp(content,":-O") == 0) {
			n = 3;
			ProcessMansionEvent('usrF',cUser->user.userID,(Ptr) &n,sizeof(short));
		}
		else if (strcmp(content,";)") == 0 || strcmp(content,";-)") == 0) {
			n = 4;
			ProcessMansionEvent('usrF',cUser->user.userID,(Ptr) &n,sizeof(short));
		}
		else if (strcmp(content,":|") == 0) {
			n = 5;
			ProcessMansionEvent('usrF',cUser->user.userID,(Ptr) &n,sizeof(short));
		}
		else if (strcmp(content,":(") == 0 || strcmp(content,":-(") == 0) {
			n = 10;
			ProcessMansionEvent('usrF',cUser->user.userID,(Ptr) &n,sizeof(short));
		}
		else if (strcmp(content,">:(") == 0) {
			n = 12;
			ProcessMansionEvent('usrF',cUser->user.userID,(Ptr) &n,sizeof(short));
		}
		else if (privateFlag)
			ProcessMansionEvent('whis',cUser->user.userID,content,strlen(content)+1);
		else
			ProcessMansionEvent('talk',cUser->user.userID,content,strlen(content)+1);
		break;
	}
}



void OpenIRCWindow(short connectType)
{
	WindowPtr			theWindow;
	IRCWindowPtr		dWin;

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 

	if (HostnameDialog(connectType) == Cancel)
		return;

	dWin = (IRCWindowPtr) NewPtrClear(sizeof(IRCWindowRecord));
	theWindow = OpenTermWindow((Ptr) dWin, IRCWIND);
	SetWTitle(theWindow, "\pIRC");

	((ObjectWindowPtr) theWindow)->Idle = IRCIdle;
	((ObjectWindowPtr) theWindow)->ProcessKey = IRCKey;
	((ObjectWindowPtr) theWindow)->Dispose = DisposeIRCWindow;
	strcpy(dWin->remoteHostname,gPrefs.remoteHost);
	dWin->connectType = connectType;
	dWin->remotePort = gPrefs.remotePort;
	dWin->lp = dWin->lineBuffer;
	gIRCWindow = theWindow;
	gIRCClosing = false;
	InitIRC(theWindow);
}

void DisposeIRCWindow(WindowPtr theWindow)
{
	IRCWindowPtr	tWin = (IRCWindowPtr) theWindow;
	CloseIRC();
	DefaultDispose(theWindow);
	gIRCWindow = NULL;
	gIRCClosing = false;
}

// Respond to a mouse-click - highlight cells until the user releases the button
//
void IRCHandleClick(WindowPtr theWin, Point where, EventRecord *theEvent)
{
}

void InitIRC(WindowPtr theWindow)
{
	OSErr	oe;
	IRCWindowPtr	tWin = (IRCWindowPtr) theWindow;
	TermInit(theWindow);

	tWin->rcveBuffer = NewPtrClear(RcveBufferLength);

	if (tWin->connectType == C_IRCTCP) {
		TestBug();

		InitNetwork();

		TestBug();

	
		oe = ConvertStringToAddr(tWin->remoteHostname,&tWin->commVars.tcpVars.remoteHost);	

		TestBug();

		if (oe != noErr) {
			if (oe == cacheFault)
				LogMessage("Can't find host\r");
			else
				LogMessage("Error Converting Host Addr: %d\r",oe);
			DisposeIRCWindow(theWindow);
			return;
		}

		TestBug();

		oe = CreateStream(&tWin->commVars.tcpVars.tcpStream,IRCBufSize);
		if (oe != noErr) {
			LogMessage("Error Creating Stream: %d\r",oe);
			DisposeIRCWindow(theWindow);
			return;
		}

		TestBug();

		oe = OpenConnection(tWin->commVars.tcpVars.tcpStream,tWin->commVars.tcpVars.remoteHost,tWin->remotePort,IRCTimeOut);

		TestBug();

		if (oe != noErr) {
			if (oe == openFailed)
				LogMessage("Connection Refused\r");
			else
				LogMessage("Error Opening Connection: %d\r",oe);
			ReleaseStream(tWin->commVars.tcpVars.tcpStream);
			DisposeIRCWindow(theWindow);
			return;
		}
	}
	else if (tWin->connectType == C_IRCSerial) {
		long	dummy;
		OSErr	oe;
		static StringPtr	portNames[2][2] = {"\p.AOut","\p.AIn","\p.BOut","\p.BIn"};
		tWin->commVars.serVars.serBuf = NewPtrClear(1024);
		oe = OpenDriver(portNames[gPrefs.serPort][0],&tWin->commVars.serVars.outComm);
		if (oe != noErr) {
			LogMessage("Error Opening Connection: %d\r",oe);
			DisposeIRCWindow(theWindow);
			return;
		}
		oe = OpenDriver(portNames[gPrefs.serPort][1],&tWin->commVars.serVars.inComm);
		if (oe != noErr) {
			LogMessage("Error Opening Connection: %d\r",oe);
			DisposeIRCWindow(theWindow);
			return;
		}
		SerReset(tWin->commVars.serVars.outComm,gPrefs.serBaud+stop10+noParity+data8);
		SerReset(tWin->commVars.serVars.inComm,gPrefs.serBaud+stop10+noParity+data8);
		SerSetBuf(tWin->commVars.serVars.inComm, tWin->commVars.serVars.serBuf, 1024);
		tWin->commVars.serVars.commFlags.fDTR = TRUE;
		tWin->commVars.serVars.commFlags.fCTS = TRUE;
		// flags.fXOn = TRUE;
		// flags.fInX = TRUE;
		// flags.xOn = XONCHAR;
		// flags.xOff = XOFFCHAR;
		SerHShake(tWin->commVars.serVars.inComm,&tWin->commVars.serVars.commFlags);
		tWin->connectionOpen = true;
		CommPrintf("telnet %s %d\r",tWin->remoteHostname,tWin->remotePort);
		Delay(6*60L,&dummy);
	}
 	gConnectionType=tWin->connectType;
	tWin->connectionOpen = true;
	CommPrintf("NICK %.*s\r",gPrefs.name[0],&gPrefs.name[1]);
	CommPrintf("USER %.*s netcom.com netcom.com :Sparky in Use\r",gPrefs.name[0],&gPrefs.name[1]);
	BlockMove(gPrefs.name,tWin->myName,gPrefs.name[0]+1);
	PtoCstr((StringPtr) tWin->myName);
}

void CloseIRC()
{
	IRCWindowPtr	tWin = (IRCWindowPtr) gIRCWindow;
	if (tWin == NULL)
		return;

	if (tWin->connectType == C_IRCTCP) {
	
		if (tWin->connectionOpen) {
			if (!gIRCClosing)
				CommString("QUIT\r");
			CloseConnection(tWin->commVars.tcpVars.tcpStream);
			ReleaseStream(tWin->commVars.tcpVars.tcpStream);
			tWin->connectionOpen = false;
		}
	}
	else  if (tWin->connectType == C_IRCSerial) {
		if (tWin->connectionOpen) {
			if (!gIRCClosing)
				CommString("QUIT\r");
			if (tWin->commVars.serVars.inComm)
				CloseDriver(tWin->commVars.serVars.inComm);
			if (tWin->commVars.serVars.outComm)
				CloseDriver(tWin->commVars.serVars.outComm);
			tWin->connectionOpen = false;
		}
	}
	if (tWin->rcveBuffer) {
		DisposePtr(tWin->rcveBuffer);
		tWin->rcveBuffer = NULL;
	}

}

Boolean IRCCharAvail(WindowPtr theWin, char *c)
{
	unsigned short dataLength = 1;
	IRCWindowPtr	tWin = (IRCWindowPtr) gIRCWindow;
	if (tWin == NULL || !tWin->connectionOpen)
		return false;
	if (tWin->connectType == C_IRCTCP) {
		if (tWin->cCnt == 0L) {
			if (!tWin->receiveFlag) {
				RecvDataAsync(tWin->commVars.tcpVars.tcpStream,tWin->rcveBuffer,RcveBufferLength,&tWin->commVars.tcpVars.rcveBlock);
				tWin->receiveFlag = true;
				return false;
			}
			else {
				if (tWin->commVars.tcpVars.rcveBlock->ioResult < 0) {
					if (tWin->commVars.tcpVars.rcveBlock->ioResult == connectionClosing) {
						LogMessage("Connection Closing\r");
						gIRCClosing = true;
						return false;
					}
					else if (tWin->commVars.tcpVars.rcveBlock->ioResult != commandTimeout)
						LogMessage("Receive Error %d\r",tWin->commVars.tcpVars.rcveBlock->ioResult);
					tWin->receiveFlag = false;
				}
				else if (tWin->commVars.tcpVars.rcveBlock->ioResult == 0) {
					unsigned short len;
					GetDataLength(tWin->commVars.tcpVars.rcveBlock,&len);
					tWin->receiveFlag = false;
					tWin->cCnt = len;
					tWin->cp = tWin->rcveBuffer;
					return false;
				}
			}
		}
		else {
			*c = *tWin->cp;
			++tWin->cp;
			--tWin->cCnt;
			return true;
		}
	}
	else  if (tWin->connectType == C_IRCSerial) {
		if (tWin->cCnt == 0L) {
			SerGetBuf(tWin->commVars.serVars.inComm,&tWin->cCnt);
			if (tWin->cCnt == 0L)
				return false;
			if (tWin->cCnt > RcveBufferLength)
				tWin->cCnt = RcveBufferLength;
			FSRead(tWin->commVars.serVars.inComm, &tWin->cCnt, tWin->rcveBuffer);
			tWin->cp = tWin->rcveBuffer;
		}
		*c = *tWin->cp;
		++tWin->cp;
		--tWin->cCnt;
		return true;
	}
	return false;
}

char	source[128];

void AddStringToRoom(char *str, short *ofst)
{
	char	*p = &rRec.varBuf[rRec.lenVars];
	strcpy(p,str);
	CtoPstr(p);
	*ofst = rRec.lenVars;
	rRec.lenVars += p[0]+1;
}

void IRCProcessLine()
{
	char	*sp,*dp,*op;
	IRCWindowPtr	tWin = (IRCWindowPtr) gIRCWindow;
	LocalUserRecPtr	cUser;
	short			n,nbrArgs;

	sp = tWin->lineBuffer;
	dp = tWin->msgSource;

	if (*sp != ':') {
		TermString(gIRCWindow, sp);
		TermReturn(gIRCWindow);
		TermLinefeed(gIRCWindow);
		return;
	}


	while (*sp && *sp != ' ')
		*(dp++) = *(sp++);
	*dp = 0;
	if (*sp)
		++sp;
	dp = tWin->msgType;
	while (*sp && *sp != ' ')
		*(dp++) = *(sp++);
	*dp = 0;
	if (*sp)
		++sp;

	for (nbrArgs = 0; *sp && *sp != '\r' && *sp != '\n' &&
		 *sp != ':' && nbrArgs < 10; ++nbrArgs) {
		dp = tWin->arg[nbrArgs];
		while (*sp && !isspace(*sp))
			*(dp++) = *(sp++);
		*dp = 0;
		while (*sp && isspace(*sp))
			++sp;
	}
	tWin->arg[nbrArgs][0] = 0;

	if (strchr(tWin->msgSource,'!') != NULL) {
		op = tWin->msgSource;
		dp = tWin->msgNick;
		if (*op == ':')
			++op;
		if (*op == '@')
			++op;
		while (*op && *op != '!')
			*(dp++) = *(op++);
		*dp = 0;
	}
	else
		tWin->msgNick[0] = 0;

	if (isdigit(tWin->msgType[0])) {
		switch (atoi(tWin->msgType)) {
		case RPL_TOPIC:
			TermString(gIRCWindow, "*** Topic on ");
			TermString(gIRCWindow,tWin->arg[1]);
			strcpy(tWin->object,tWin->arg[1]);
			TermString(gIRCWindow, " is ");
			if (*sp == ':')
				++sp;
			TermString(gIRCWindow, sp);
			TermReturn(gIRCWindow);
			TermLinefeed(gIRCWindow);
			break;
		case RPL_NAMREPLY:
			TermString(gIRCWindow, "*** Users on ");
			TermString(gIRCWindow,tWin->arg[2]);
			TermString(gIRCWindow, ": ");
			if (*sp == ':')
				++sp;
			TermString(gIRCWindow, sp);
			TermReturn(gIRCWindow);
			TermLinefeed(gIRCWindow);
			tokenize(sp);
			// !!! Initialize Room using userlist in Content
			rRec.lenVars = 0;
			rRec.roomID = 100 + MyRandom(100);
			AddStringToRoom(tWin->arg[2],&rRec.roomNameOfst);
			{
				char	tbuf[81];
				sprintf(tbuf,":Pictures:%s",tWin->arg[2]);
				AddStringToRoom(tbuf,&rRec.pictNameOfst);
			}
			rRec.nbrHotspots = 0;
			rRec.nbrPeople = 0;
			ProcessMansionEvent('room',0L,(Ptr) &rRec, sizeof(RoomRec)+rRec.lenVars);
			ProcessMansionEvent('endr',0L,NULL,0L);
			n = 0;
			gRoomWin->noFades = true;
			while (tWin->tokens[n][0]) {
				uRec.userID = ((long) Random() << 16) | (long) Random();
				uRec.roomPos.h = 22 + MyRandom(RoomWidth-44);
				uRec.roomPos.v = 22 + MyRandom(RoomHeight-44);
				uRec.roomID = rRec.roomID;
				uRec.faceNbr = 0;
				uRec.colorNbr = MyRandom(16);
				uRec.propID[0] = 0;
				uRec.propID[1] = 0;
				uRec.propID[2] = 0;
				uRec.awayFlag = 0;
				uRec.openToMsgs = 0;
				if (strcmp(tWin->tokens[n],tWin->myName) == 0) {
					gRoomWin->meID = uRec.userID;
				}
				strcpy((char *) uRec.name,tWin->tokens[n]);
				if (uRec.name[0] == '@')
					strcpy((char *) uRec.name,(char *) &uRec.name[1]);
				CtoPstr((char *) uRec.name);
				ProcessMansionEvent('nprs',uRec.userID,(Ptr) &uRec,sizeof(UserRec));
				++n;
			}
			gRoomWin->noFades = false;
			gRoomWin->mePtr = GetUser(gRoomWin->meID);
			if (gRoomWin->mePtr)
				gRoomWin->mePtr->ircSparkyAware = true;
			CommPrintf("PRIVMSG %s :fac 5\r",tWin->object);	// Notify others of sparky
			break;
		case RPL_WHOISUSER:
			TermString(gIRCWindow, "*** ");
			TermString(gIRCWindow,tWin->arg[1]);
			TermString(gIRCWindow, " is ");
			if (*sp == ':')
				++sp;
			TermString(gIRCWindow, sp);
			TermReturn(gIRCWindow);
			TermLinefeed(gIRCWindow);
			break;
		case RPL_WHOISSERVER:
			TermString(gIRCWindow, "*** on server ");
			if (*sp == ':')
				++sp;
			TermString(gIRCWindow, sp);
			TermReturn(gIRCWindow);
			TermLinefeed(gIRCWindow);
			break;
		case RPL_WHOISIDLE:
			TermString(gIRCWindow, "*** ");
			TermString(gIRCWindow,tWin->arg[1]);
			TermString(gIRCWindow, " has been idle ");
			TermString(gIRCWindow, tWin->arg[2]);
			TermString(gIRCWindow, " seconds.");
			TermReturn(gIRCWindow);
			TermLinefeed(gIRCWindow);
			break;
		case 4:
		case RPL_ENDOFNAMES:
		case RPL_ENDOFWHOIS:
		case RPL_ENDOFMOTD:
		case 333:
			// Ignore These
			break;
		default: 
			TermString(gIRCWindow, "*** ");
			if (*sp == ':')
				++sp;
			TermString(gIRCWindow, sp);
			TermReturn(gIRCWindow);
			TermLinefeed(gIRCWindow);
			break;
		}
	}
	else {
		if (strcmp("PRIVMSG",tWin->msgType) == 0) {
			if (strncmp(sp,":\001ACTION",8) == 0) {
				TermString(gIRCWindow,"* ");
				TermString(gIRCWindow,tWin->msgNick);
				sp += 9;
				TermString(gIRCWindow, sp);
				TermReturn(gIRCWindow);
				TermLinefeed(gIRCWindow);
				cUser = GetUserByName(tWin->msgNick);
				if (cUser) {
					ConvertIRCString(cUser, sp, false);
				}
			}
			else {
				TermString(gIRCWindow, "<");
				TermString(gIRCWindow,tWin->msgNick);
				TermString(gIRCWindow, "> ");
				sp = strchr(sp,':');
				if (*sp == ':')
					++sp;
				TermString(gIRCWindow, sp);
				TermReturn(gIRCWindow);
				TermLinefeed(gIRCWindow);
				cUser = GetUserByName(tWin->msgNick);
				if (cUser) {
					if (stricmp(tWin->arg[0],tWin->object) == 0)
						ConvertIRCString(cUser, sp, false);
					else
						ConvertIRCString(cUser, sp, true);
				}
			}
		}
		else if (strcmp("QUIT",tWin->msgType) == 0 ||
				 strcmp("PART",tWin->msgType) == 0) {
			TermString(gIRCWindow, "*** ");
			TermString(gIRCWindow,tWin->msgNick);
			TermString(gIRCWindow," is leaving ");
			if (*sp == ':')
				++sp;
			TermString(gIRCWindow, sp);
			TermReturn(gIRCWindow);
			TermLinefeed(gIRCWindow);

			cUser = GetUserByName(tWin->msgNick);
			if (cUser) {
				if (cUser->user.userID == gRoomWin->meID) {
					ClearRoom();
					RefreshRoom(&gOffscreenRect);
					tWin->object[0] = 0;
				}
				ProcessMansionEvent('eprs',cUser->user.userID,NULL,0L);
			}
		}
		else if (strcmp("JOIN",tWin->msgType) == 0) {
			strcpy(tWin->object,sp+1);
			while (isspace(tWin->object[strlen(tWin->object)-1]))
				tWin->object[strlen(tWin->object)-1] = 0;

			if (tWin->msgNick[0]) {
				uRec.userID = ((long) Random() << 16) | (long) Random();
				uRec.roomPos.h = 22 + MyRandom(RoomWidth-44);
				uRec.roomPos.v = 22 + MyRandom(RoomHeight-44);
				uRec.roomID = gRoomWin->curRoom.roomID;
				uRec.faceNbr = 0;
				uRec.colorNbr = MyRandom(16);
				uRec.propID[0] = 0;
				uRec.propID[1] = 0;
				uRec.propID[2] = 0;
				uRec.awayFlag = 0;
				uRec.openToMsgs = 0;
				strcpy((char *) uRec.name,tWin->msgNick);
				CtoPstr((char *) uRec.name);
				ProcessMansionEvent('nprs',uRec.userID,(Ptr) &uRec,sizeof(UserRec));
			}
		}
		else if (strcmp("NOTICE",tWin->msgType) == 0) {
			TermString(gIRCWindow, "*** ");
			if (*sp == ':')
				++sp;
			TermString(gIRCWindow, sp);
			TermReturn(gIRCWindow);
			TermLinefeed(gIRCWindow);
		}
		else if (strcmp("NICK",tWin->msgType) == 0) {
			Str31	newNick;
			if (tWin->arg[0][0])
				sp = tWin->arg[0];
			else {
				if (*sp == ':')
					++sp;
				while (*sp && isspace(sp[strlen(sp)-1]))
					sp[strlen(sp)-1] = 0;
			}
			strcpy((char *) newNick, sp);
			CtoPstr((char *) newNick);
			cUser = GetUserByName(tWin->msgNick);
			if (cUser) {
				if (cUser->user.userID == gRoomWin->meID)
					strcpy(tWin->myName,sp);
				ProcessMansionEvent('usrN',cUser->user.userID,(Ptr) newNick,newNick[0]+1);
			}
		}
		else if (strcmp("PING",tWin->msgType) == 0) {
			CommPrintf("PONG %s %s\r",tWin->myName,tWin->arg[0]);
		}
		else {
			TermString(gIRCWindow, "*** ");
			TermString(gIRCWindow, tWin->msgType);
			TermString(gIRCWindow, " ");
			TermString(gIRCWindow, sp);
			TermReturn(gIRCWindow);
			TermLinefeed(gIRCWindow);
		}
	}
}

void IRCRecordChar(WindowPtr theWin, char c)
{
	IRCWindowPtr	tWin = (IRCWindowPtr) theWin;
	*(tWin->lp++) = c;
}

void IRCRecordLine(WindowPtr theWin)
{
	IRCWindowPtr	tWin = (IRCWindowPtr) theWin;
	if (tWin->lp > tWin->lineBuffer) {
		*tWin->lp = 0;
		IRCProcessLine();
		tWin->lp = tWin->lineBuffer;
	}
}

void ParseIRCString(char *str)
{
	IRCWindowPtr	tWin = (IRCWindowPtr) gIRCWindow;
	LocalUserRecPtr	cUser;

	tWin->sendBuffer[0] = 0;
	if (strlen(str) && str[strlen(str)-1] != '\r')
		strcat(str,"\r");
	if (str[0] == '/') {
		char	cmdToken[32];
		char	*sp,*dp;
		strcpy(str,&str[1]);
		sp = str;
		dp = cmdToken;
		while (*sp && !isspace(*sp)) {
			if (isalpha(*sp))
				*sp = toupper(*sp);
			*dp = *sp;
			++dp;
			++sp;
		}
		*dp = 0;
		if (strcmp(cmdToken,"QUIT") == 0) {
			gIRCClosing = true;
			strcpy(tWin->sendBuffer,str);
		}
		else if (strcmp(cmdToken,"MSG") == 0) {
			char	who[32];
			dp = who;
			if (*sp == ' ')
				++sp;
			while (*sp && *sp != ' ') {
				*(dp++) = *(sp++);
			}
			if (*sp == ' ')
				++sp;
			sprintf(tWin->sendBuffer,"PRIVMSG %s :%s",who,sp);
		}
		else if (strcmp(cmdToken,"LEAVE") == 0) {
			sprintf(tWin->sendBuffer,"PART %s\r",tWin->object);
		}
		else if (strcmp(cmdToken,"CHAN") == 0 ||
				 strcmp(cmdToken,"JOIN") == 0) {
			if (tWin->object[0]) {
				CommPrintf("PART %s\r",tWin->object);
				tWin->object[0] = 0;
			}
			*((long *) str) = 'JOIN';
			strcpy(tWin->sendBuffer,str);
		}
		else 
			strcpy(tWin->sendBuffer,str);
	}
	else {
		if (tWin->object[0] == 0) {
			TermString(gIRCWindow,"There is no channel to send to");
			TermReturn(gIRCWindow);
			TermLinefeed(gIRCWindow);
		}
		else {
			sprintf(tWin->sendBuffer,"PRIVMSG %s :%s",tWin->object,str);
			cUser = GetUser(gRoomWin->meID);
			if (cUser)
				ConvertIRCString(cUser,str, false);
		}
	}
	if (strlen(tWin->sendBuffer))
		CommString(tWin->sendBuffer);
}

#define CommLine	27

void IRCChar(char c)
{
	IRCWindowPtr	tWin = (IRCWindowPtr) gIRCWindow;
	static char	outBuf[256];
	static short outCnt = 0;
	Point	oldPos;

	if (tWin == NULL || !tWin->connectionOpen)
		return;

	SetPort(gIRCWindow);
	// Queue up chars so they don't conflict with moves...
	oldPos = gIRCWindow->pnLoc;
	switch (c) {
	case '\r':
		outBuf[outCnt++] = '\r';
		outBuf[outCnt] = 0;
		if (outBuf[0] == '~')
			ProcessIRCMacro(&outBuf[1]);
		else {
			ParseIRCString(outBuf);
		}
		MoveTo(LeftM+CharW,4+CommLine*11);
		while (--outCnt)
			DrawChar(' ');
		DrawChar(' ');
		break;
	case 127:
	case '\b':
		if (outCnt) {
			--outCnt;
			MoveTo(LeftM+CharW+CharW*outCnt,4+CommLine*11);
			DrawChar('_');
			DrawChar(' ');
		}
		break;
	default:
		MoveTo(LeftM+CharW+CharW*outCnt,4+CommLine*11);
		DrawChar(c);
		DrawChar('_');
		outBuf[outCnt++] = c;
		break;
	}
	MoveTo(oldPos.h,oldPos.v);
}

void CommString(char *str)
{
	IRCWindowPtr	tWin = (IRCWindowPtr) gIRCWindow;
	OSErr	oe;
	if (tWin == NULL || !tWin->connectionOpen)
		return;
	if (tWin->connectType == C_IRCTCP)
		oe = SendData(tWin->commVars.tcpVars.tcpStream,str,(unsigned short)strlen(str),false);
	else  if (tWin->connectType == C_IRCSerial) {
		long	count;
		count = strlen(str);
		oe = FSWrite(tWin->commVars.serVars.outComm,&count,str);
	}
}

void CommPrintf(char *str, ...)
{
	IRCWindowPtr	tWin = (IRCWindowPtr) gIRCWindow;
	va_list args;

	va_start(args,str);
	vsprintf(tWin->sendBuffer,str,args);
	va_end(args);

	// Send tbuf out the comm port....
	CommString(tWin->sendBuffer);
}


void CommPString(StringPtr str)
{
	IRCWindowPtr	tWin = (IRCWindowPtr) gIRCWindow;
	OSErr	oe;
	if (tWin == NULL || !tWin->connectionOpen)
		return;
	if (tWin->connectType == C_IRCTCP)
		oe = SendData(tWin->commVars.tcpVars.tcpStream,(char *) &str[1],(unsigned short)str[0],false);
	else  if (tWin->connectType == C_IRCSerial) {
		long	count;
		count = str[0];
		oe = FSWrite(tWin->commVars.serVars.outComm,&count,&str[1]);
	}
}

Boolean IRCPollChar(WindowPtr theWin)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	char	c;

	if (!IRCCharAvail(theWin, &c)) {
		if (TickCount() - tWin->lastBlink > 30) {
			TermCursorBlink(theWin);
			tWin->lastBlink = TickCount();
		}
		return false;
	}
	if (c == '\r' || c == '\n')
		IRCRecordLine(theWin);
	else
		IRCRecordChar(theWin,c);
	return true;
}


void IRCIdle(WindowPtr theWin, EventRecord *theEvent)
{
	IRCWindowPtr	tWin = (IRCWindowPtr) theWin;
	long	t;
	t = TickCount();
	
	while (TickCount() - t < 60 && IRCPollChar(theWin))
		;
}

void IRCKey(WindowPtr theWin, EventRecord *theEvent)
{
	char	c;
	c = theEvent->message & charCodeMask;
	switch (c) {
	case 0x1E:	CommPString("\p\033[A");	break;	// Up Arrow
	case 0x1F:	CommPString("\p\033[B");	break;	// Down Arrow
	case 0x1D:	CommPString("\p\033[C");	break;	// Right Arrow
	case 0x1C:	CommPString("\p\033[D");	break;	// Left Arrow
	case 0x08:	IRCChar(0x7F);			break;	// Backspace
	default:
		IRCChar(c);
	}
}

#define HostnameDLOG		501
#define P_NickItem			3
#define P_HostItem			4
#define P_PortItem			5
#define P_SerPortItem		9
#define P_SerBaudItem		10

short HostnameDialog(short connectType)
{
	GrafPtr		savePort;
	DialogPtr 	dp;
	short		itemHit;
	Str63		portName;
	Str255		hostName;

	GetPort(&savePort);

	if (connectType == C_IRCSerial) {
		gPortMenu = GetMenu(PortMENU);
		gBaudMenu = GetMenu(BaudMENU);
	}

	if ((dp = GetNewDialog (HostnameDLOG, NULL, (WindowPtr) -1)) == NULL)
		return Cancel;
	
	if (connectType == C_IRCSerial) {
		switch (gPrefs.serPort) {
		case 0:		SetControl(dp, P_SerPortItem, 1);	break;
		case 1:		SetControl(dp, P_SerPortItem, 2);	break;
		}
		switch (gPrefs.serBaud) {
		case baud1200:		SetControl(dp, P_SerBaudItem, 1);	break;
		case baud2400:		SetControl(dp, P_SerBaudItem, 2);	break;
		case baud9600:		SetControl(dp, P_SerBaudItem, 3);	break;
		case baud19200:		SetControl(dp, P_SerBaudItem, 4);	break;
		case baud57600:		SetControl(dp, P_SerBaudItem, 5);	break;
		}
	}
	else {
		HideDItem(dp, P_SerPortItem);
		HideDItem(dp, P_SerBaudItem);
	}

	sprintf((char *) portName, "%d",gPrefs.remotePort);
	CtoPstr((char *) portName);
	SetText(dp,P_PortItem, portName);
	BlockMove(gPrefs.remoteHost,hostName,gPrefs.remoteHost[0]+1);
	CtoPstr((char *) hostName);
	SetText(dp,P_HostItem, hostName);
	SetText(dp,P_NickItem, gPrefs.name);
	SelIText(dp,P_NickItem,0,32767);

	ShowWindow(dp);
	SetPort(dp);
	SetCursor(&qd.arrow);
	ShowCursor();
	do {
		ModalDialog(NULL, &itemHit);
	} while (itemHit != OK && itemHit != Cancel);
	if (itemHit == OK) {
		GetText(dp,P_NickItem, gPrefs.name);
		GetText(dp,P_HostItem, hostName);
		GetText(dp,P_PortItem, portName);
		PtoCstr(hostName);
		PtoCstr(portName);
		strcpy(gPrefs.remoteHost,(char *) hostName);
		gPrefs.remotePort = atoi((char *) portName);

		if (connectType == C_IRCSerial) {
			short	n;
			n = GetControl(dp, P_SerPortItem);
			gPrefs.serPort = n-1;
			n = GetControl(dp, P_SerBaudItem);
			switch (n) {
			case 1:	gPrefs.serBaud = baud1200;	break;
			case 2:	gPrefs.serBaud = baud2400;	break;
			case 3:	gPrefs.serBaud = baud9600;	break;
			case 4:	gPrefs.serBaud = baud19200;	break;
			case 5:	gPrefs.serBaud = baud57600;	break;
			}
		}
		StorePreferences();
	}
	DisposDialog(dp);
	SetPort(savePort);
	return itemHit;
}

void PostUserIRCEvent(unsigned long eventType, unsigned long refNum,
					Ptr buffer, long bufferLength)
{
	LocalUserRecPtr	cUser,lUser;
	IRCWindowPtr	tWin = (IRCWindowPtr) gIRCWindow;
	Point			tP;
	short			i;

	if (tWin == NULL)
		return;

	switch (eventType) {
	case 'talk':
		ParseIRCString(buffer);
		break;
	case 'whis':
		cUser = GetUser(*((long *) buffer));
		if (cUser) {
			CommPrintf("PRIVMSG %.*s :%s\r",cUser->user.name[0],&cUser->user.name[1],&buffer[4]);
			ConvertIRCString(gRoomWin->mePtr,&buffer[4], true);
		}
		break;
	case 'loc ':
		tP = *((Point *) buffer);
		lUser = gRoomWin->userList;
		for (i = 0; i < gRoomWin->curRoom.nbrPeople; ++i,++lUser)
			if (lUser->ircSparkyAware)
				CommPrintf("PRIVMSG %.*s :loc %d,%d\r",
						lUser->user.name[0],&lUser->user.name[1],
						tP.h, tP.v);
		break;
	case 'usrF':
		lUser = gRoomWin->userList;
		for (i = 0; i < gRoomWin->curRoom.nbrPeople; ++i,++lUser)
			if (lUser->ircSparkyAware)
				CommPrintf("PRIVMSG %.*s :fac %d\r",
					lUser->user.name[0],&lUser->user.name[1],
					*((short *) buffer));
		break;
	case 'usrC':
		lUser = gRoomWin->userList;
		for (i = 0; i < gRoomWin->curRoom.nbrPeople; ++i,++lUser)
			if (lUser->ircSparkyAware)
				CommPrintf("PRIVMSG %.*s :col %d\r",
						lUser->user.name[0],&lUser->user.name[1],
						*((short *) buffer));
		break;
	case 'usrP':
		lUser = gRoomWin->userList;
		for (i = 0; i < gRoomWin->curRoom.nbrPeople; ++i,++lUser)
			if (lUser->ircSparkyAware)
				CommPrintf("PRIVMSG %.*s :prp %d,%d,%d\r",
						lUser->user.name[0],&lUser->user.name[1],
						*((short *) buffer),*((short *) &buffer[2]),*((short *) &buffer[4]));
		break;
	}
}
