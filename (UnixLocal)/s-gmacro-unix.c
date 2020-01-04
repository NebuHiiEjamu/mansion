/* God Macros */
#include "s-server.h"
#include "s-secure.h"


void UserMessage(ServerUserPtr cUser, char *tmp,...)
{
	char 	tbuf[256];
	va_list args;

	va_start(args,tmp);
	vsprintf(tbuf,tmp,args);
	va_end(args);

	if (cUser) {
		PostUserEvent(cUser,MSG_TALK,0L,tbuf,strlen(tbuf)+1);
	}
}

void UserPrivateMessage(ServerUserPtr cUser, char *tmp,...)
{
	char 	tbuf[256];
	va_list args; 

	va_start(args,tmp);
	vsprintf(tbuf,tmp,args);
	va_end(args);

	if (cUser) {
		PostUserEvent(cUser,MSG_WHISPER,0L,tbuf,strlen(tbuf)+1);
	}
}

void RoomMessage(RoomID roomID, char *tmp,...)
{
	char 	tbuf[256];
	va_list args;

	va_start(args,tmp);
	vsprintf(tbuf,tmp,args);
	va_end(args);

	PostRoomEvent(roomID,MSG_TALK,0L,tbuf,strlen(tbuf)+1);
}

void WizRoomMessage(RoomID roomID, char *tmp,...)
{
	char 	tbuf[256];
	int		len;
	ServerUserPtr	curUser;
	va_list args;

	va_start(args,tmp);
	vsprintf(tbuf,tmp,args);
	va_end(args);

	len = strlen(tbuf)+1;

	for (curUser = gUserList; curUser; curUser = curUser->nextUser)
		/* 8/4/95 Don't post global events to killed users */
		if (curUser->user.roomID == roomID && 
			!(curUser->flags & U_Kill) && 
			(curUser->flags & U_SuperUser))
			PostUserEvent(curUser,MSG_TALK, 0L,tbuf,len);
}

void WizGlobalMessage(char *tmp,...)
{
	char 	tbuf[512];
	int		len;
	ServerUserPtr	curUser;

	va_list args;

	va_start(args,tmp);
	vsprintf(tbuf,tmp,args);
	va_end(args);

	len = strlen(tbuf)+1;

	for (curUser = gUserList; curUser; curUser = curUser->nextUser)
		/* 8/4/95 Don't post global events to killed users */
		if (!(curUser->flags & U_Kill) && 
			(curUser->flags & U_SuperUser))
			PostUserEvent(curUser,MSG_TALK, 0L,tbuf,len);
	TimeLogMessage("%s\n",tbuf);
}

void GodGlobalMessage(char *tmp,...)
{
	char 	tbuf[512];
	int		len;
	ServerUserPtr	curUser;

	va_list args;

	va_start(args,tmp);
	vsprintf(tbuf,tmp,args);
	va_end(args);

	len = strlen(tbuf)+1;

	for (curUser = gUserList; curUser; curUser = curUser->nextUser)
		/* 8/4/95 Don't post global events to killed users */
		if (!(curUser->flags & U_Kill) && 
			(curUser->flags & U_God))
			PostUserEvent(curUser,MSG_TALK, 0L,tbuf,len);

	TimeLogMessage("%s\n",tbuf);
}


void GodMessage(ServerUserPtr cUser, char *tmp,...);
void GodMessage(ServerUserPtr cUser, char *tmp,...)
{
	char 	tbuf[128];
	va_list args;

	va_start(args,tmp);
	vsprintf(tbuf,tmp,args);
	va_end(args);

	if (cUser) {
		/* !! probably should encrypt */
		PostUserEvent(cUser,MSG_TALK,0L,tbuf,strlen(tbuf)+1);
	}
	LogMessage("%s\r",tbuf);
}

char *GetGodToken(char *tok, char *p)
{
	while (isspace(*p))
		++p;
	while (*p && !isspace(*p))
		*(tok++) = *(p++);
	*tok = 0;
	while (isspace(*p))
		++p;
	return p;
}


Boolean GodMacro(ServerUserPtr cUser, char *buffer, Boolean encrypted)
{
	char	tbuf[256]="";
	short	len;
	if (buffer[0] == '`' && !encrypted)	/* 9/29/95 process unencrypted god commands too */
		strcpy(tbuf,buffer);
	else if (encrypted) {
		len = *((short *) buffer) - 3;
		if (len > 0 && len < 256)
			DecryptCString((unsigned char*) buffer+2, (unsigned char *) tbuf, len);
	}
	else
		return false;
	if (tbuf[0] == '`') {
		ProcessGodCommand(cUser, &tbuf[1]);
		return true;
	}
	else
		return false;
}

void ListUsers(ServerUserPtr cUser, char *arg, Boolean globalFlag)
{
	ServerUserPtr	curUser,tUser=NULL;
	int				numericFlag = false,seedFlag = false;
	LONG			usrCnt = 0;
	char			seedStr[32]="";
	unsigned long	seedCounter;
	
	while (*arg == '-') {
		++arg;
		if (*arg == 'n')
			numericFlag = true;
		else if (*arg == 's')
			seedFlag = true;
		++arg;
		while (*arg == ' ')
			++arg;
	}

	if (*arg) {
		CtoPstr(arg);
		tUser = GetServerUserByName((StringPtr) arg);
		PtoCstr((StringPtr)arg);
	}
	if (tUser || *arg)
		globalFlag = true;
	for (curUser = gUserList; curUser; curUser = curUser->nextUser) {
		char	addrBuf[128],numBuf[64],userStatusChar;
		char	tbuf[256];
		StringPtr	rName;
		ServerRoomPtr	rp;
		if (!globalFlag && curUser->user.roomID != cUser->user.roomID)
				continue;
		if (tUser != NULL && curUser != tUser)
				continue;
		if (*arg && tUser == NULL) {
			ConvertNetAddressToNumericString(curUser,numBuf);
			if (strstr(numBuf, arg) == NULL)
				continue;
		}
		++usrCnt;
		rp = GetRoom(curUser->user.roomID);
		if (rp)
			rName = (StringPtr) &rp->room.varBuf[rp->room.roomNameOfst];
		else
			rName = (StringPtr) "";
		if (numericFlag)
			ConvertNetAddressToNumericString(curUser,addrBuf);
		else
			ConvertNetAddressToString(curUser,addrBuf);
		ConvertNetAddressToNumericString(curUser,numBuf);
		if (curUser->flags & U_Guest)
			userStatusChar = 'n';
		else if (curUser->flags & U_God)
			userStatusChar = 'g';
		else if (curUser->flags & U_SuperUser)
			userStatusChar = 'w';
		else
			userStatusChar = 'm';
		if (seedFlag) {
			seedCounter = curUser->counter ^ MAGIC_LONG ^ curUser->crc;
			sprintf(seedStr,"[%ld]",(long) seedCounter);
		}
		if (strcmp(addrBuf,numBuf) == 0) {
			sprintf(tbuf,"; %c %s %s %s %s",
				userStatusChar,
				CvtToCString(curUser->user.name),
				addrBuf,CvtToCString(rName),seedStr);
		}
		else {
			sprintf(tbuf,"; %c %s %s (%s) %s %s",
				userStatusChar,
				CvtToCString(curUser->user.name),
				addrBuf,numBuf,CvtToCString(rName),seedStr);
		}
		UserMessage(cUser,"%s", tbuf);
	}
	if (usrCnt == 0)
		UserMessage(cUser,"; can't find %s",arg);
}

void ProcessGodCommand(ServerUserPtr cUser, char *p)
{
	char	token[256],*arg;
	Boolean	prefsChange = false;
	p = GetGodToken(token, p);
	arg = p;

/* Wizard/God Commands First */
	if (strcmp(token,"help") == 0) {
		if (cUser->flags & U_Guest)
			return;
		UserMessage(cUser, "; `mute name               - mute another user");
		UserMessage(cUser, "; `hidefrom name           - hide from another user");
		UserMessage(cUser, "; `hide on|off             - hide from all other users");
		UserMessage(cUser, "; `rejectesp on|off        - reject ESP messages");
		UserMessage(cUser, "; `rejectprivate on|off    - reject private messages");
		if (!(cUser->flags & U_SuperUser))
			return;
		UserMessage(cUser, "; `list [-n] username      - list ip address of specific user");
		UserMessage(cUser, "; `list [-n] ipaddr        - list users matching ip address");
		UserMessage(cUser, "; `list [-n]               - list ip addresses of users in room");
		UserMessage(cUser, "; `glist [-n]              - list ip address of all users");
		UserMessage(cUser, "; `banlist [str]           - show banned users");
		UserMessage(cUser, "; `purgebanlist            - purge elapsed ban records");
		UserMessage(cUser, "; `banip [minutes] ipaddr  - ban ipaddress (may use 12.24.23.*)");
		UserMessage(cUser, ";                          - numeric ips only!");
		UserMessage(cUser, "; `banuser name            - ban currently connected user");
		UserMessage(cUser, "; `unban str               - unban user(s) who match str (ip or name)");
		UserMessage(cUser, "; `gag name                - gag currently connected user");
		UserMessage(cUser, "; `ungag name              - ungag currently connected user");
		UserMessage(cUser, "; `propgag name            - propgag a member");
		UserMessage(cUser, "; `unpropgag name          - unpropgag a member");
		UserMessage(cUser, "; `death N                 - set default death penalty to N minutes");
		UserMessage(cUser, "; `paint on|off            - turn painting on or off serverwide");
		UserMessage(cUser, "; `bots on|off             - turn cyborgs on or off serverwide");
		UserMessage(cUser, "; `flood N                 - kill flooders after N events in 1 second");
		UserMessage(cUser, "; `duplicate               - duplicate the current room");
		UserMessage(cUser, "; `delete                  - delete the current room");
		if (!(cUser->flags & U_God))
			return;
		UserMessage(cUser, "; GOD COMMANDS:");
		UserMessage(cUser, "; `ban user");
		UserMessage(cUser, "; `shutdown");
		UserMessage(cUser, "; `wizpass str");
		UserMessage(cUser, "; `godpass str");
		UserMessage(cUser, "; `servername str");
		UserMessage(cUser, "; `maxserverocc N");
		UserMessage(cUser, "; `defaultroomocc N");
		UserMessage(cUser, "; `roommaxocc N");
		UserMessage(cUser, "; `roommaxguests N");
		UserMessage(cUser, "; `picdir str");
		UserMessage(cUser, "; `tcp on|off");
		UserMessage(cUser, "; `guests on|off");
		UserMessage(cUser, "; `custom on|off");
		UserMessage(cUser, "; `wizards on|off");
		UserMessage(cUser, "; `wizardkill on|off");
		UserMessage(cUser, "; `botkill on|off");
		UserMessage(cUser, "; `playerkill on|off");
		UserMessage(cUser, "; `author on|off");
		UserMessage(cUser, "; `spoof on|off");
		UserMessage(cUser, "; `wizardsonly on|off");
		UserMessage(cUser, "; `purge N");
		UserMessage(cUser, "; `recyle");
		UserMessage(cUser, "; `reset");
	}
	else if (cUser->flags & U_Guest) {
		return;
	}
	/*
		MEMBER commands follow
	 */
	else if (strcmp(token,"hide") == 0) {
		if (strcmp(arg,"off") == 0)
			cUser->flags &= ~U_Hide;
		else
			cUser->flags |= U_Hide;
		TimeLogMessage("%s %shidden\n", 
				CvtToCString(cUser->user.name),
				(cUser->flags & U_Hide)? "" : "not ");
		UserMessage(cUser, "You are %shidden",
				(cUser->flags & U_Hide)? "" : "not ");
	}
	else if (strcmp(token,"unhide") == 0) {
		cUser->flags &= ~U_Hide;
		TimeLogMessage("%s Unhidden\n", CvtToCString(cUser->user.name));
		UserMessage(cUser, "You are unhidden");
	}
	else if (strcmp(token,"hidefrom") == 0) {
		SetUserAction(cUser, arg, UA_HideFrom, true);
	}
	else if (strcmp(token,"unhidefrom") == 0) {
		SetUserAction(cUser, arg, UA_HideFrom, false);
	}
	else if (strcmp(token,"mute") == 0) {
		SetUserAction(cUser, arg, UA_Mute, true);
	}
	else if (strcmp(token,"unmute") == 0) {
		SetUserAction(cUser, arg, UA_Mute, false);
	}
	else if (strcmp(token,"rejectesp") == 0) {
		if (strcmp(arg,"off") == 0)
			cUser->flags &= ~U_RejectESP;
		else
			cUser->flags |= U_RejectESP;
		TimeLogMessage("%s %srejecting ESP messages\n", 
				CvtToCString(cUser->user.name),
				(cUser->flags & U_RejectESP)? "" : "not ");
		UserMessage(cUser, "You are %srejecting ESP messages",
				(cUser->flags & U_RejectESP)? "" : "not ");
	}
	else if (strncmp(token,"rejectpri",9) == 0) {
		if (strcmp(arg,"off") == 0)
			cUser->flags &= ~U_RejectPrivate;
		else
			cUser->flags |= U_RejectPrivate;
		TimeLogMessage("%s %srejecting private messages\n", 
				CvtToCString(cUser->user.name),
				(cUser->flags & U_RejectPrivate)? "" : "not ");
		UserMessage(cUser, "You are %srejecting private messages",
				(cUser->flags & U_RejectPrivate)? "" : "not ");
	}
	/* 
	   WIZARD commands follow
	 */
	else if (!(cUser->flags & U_SuperUser)) {
		UserMessage(cUser, "Invalid server command");
		return;
	}
	else if ((strcmp(token, "list") == 0 || strcmp(token, "glist") == 0) &&
		cUser != NULL) 
	{
		ListUsers(cUser,arg,strcmp(token, "glist") == 0);
	}
	else if (strncmp(token, "paint",5)== 0) {
		if (strcmp(arg,"on") == 0)
			gPrefs.permissions |= PM_AllowPainting;
		else
			gPrefs.permissions &= ~PM_AllowPainting;
		prefsChange = true;
		GodMessage(cUser, "Painting %sallowed",(gPrefs.permissions & PM_AllowPainting)? "" : "not ");
	}
	else if (strcmp(token, "bots") == 0) {
		if (strcmp(arg,"on") == 0)
			gPrefs.permissions |= PM_AllowCyborgs;
		else
			gPrefs.permissions &= ~PM_AllowCyborgs;
		prefsChange = true;
		GodMessage(cUser, "Cyborgs %sallowed",(gPrefs.permissions & PM_AllowCyborgs)? "" : "not ");
	}
	else if (strncmp(token, "death",5) == 0) {
		int	n;
		n = atoi(arg);
		if (n <= 0 || n > 30000) {
			gPrefs.permissions &= ~PM_DeathPenalty;
			gPrefs.deathPenaltyMinutes = 0;
			prefsChange = true;
			GodMessage(cUser, "No Death Penalty");
		}
		else {
			gPrefs.permissions |= PM_DeathPenalty;
			gPrefs.deathPenaltyMinutes = n;
			prefsChange = true;
			GodMessage(cUser, "Death Penalty set to %d minutes",n);
		}
	}
	else if (strncmp(token, "flood",5) == 0) {
		int	n;
		n = atoi(arg);
		if (n <= 0 || n > 30000) {
			gPrefs.permissions &= ~PM_KillFlooders;
			prefsChange = true;
			GodMessage(cUser, "No Purging");
		}
		else {
			if (n < 30)
				n = 30;
			gPrefs.permissions |= PM_KillFlooders;
			gPrefs.minFloodEvents = n;
			prefsChange = true;
			GodMessage(cUser, "Kill flooders after %d events",n);
		}
	}
	else if (strcmp(token, "duplicate") == 0) {
		ServerRoomPtr	nRoom;
		int				n;
		n = cUser->user.roomID;
		nRoom = GetRoom(n);
		if (nRoom)
			DuplicateRoom(cUser);
		GodMessage(cUser, "room %d duplicated",n);
	}
	else if (strncmp(token, "delete",6) == 0) {
		int	n;
		ServerRoomPtr	nRoom;

		n = atoi(arg);
		if (n == 0)
			n = cUser->user.roomID;
		nRoom = GetRoom(n);
		if (n == gEntrance) {
			UserMessage(cUser, "can't delete entrance");
		}
		else if (nRoom == NULL) {
			UserMessage(cUser, "room number invalid");
		}
		else if (!(cUser->flags & U_God) && (nRoom->room.roomFlags & RF_AuthorLocked)) {
			UserMessage(cUser, "Locked Room");
		}
		else if (nRoom != NULL) {
			DeleteRoom(n);
			WizGlobalMessage("Page from System: room %d deleted by %s",
							 n,CvtToCString(cUser->user.name));
		}
	}
	/* 12/3/95 banip [time] ip-address */
	/* ipaddress may use asterisks to indicate site ban */
	
	else if (strcmp(token,"banlist") == 0) {
		DoBanList(cUser, arg);
	}
	else if (strcmp(token,"purgebanlist") == 0) {
		PurgeBanList(cUser);
	}
	else if (strcmp(token,"unban") == 0) {
		DoUnBan(cUser, arg);
	}
	else if (strcmp(token,"banip") == 0 && arg[0] != 0) {
		int		banMinutes = 0;
		char	aStr[8]="",bStr[8]="",cStr[8]="",dStr[8]="";
		char	ipStr[128] = "";
		int		siteFlag = 0;		
		unsigned LONG	a,b,c,d,ipAddress;

		if (strchr(arg,' ') != NULL)
			sscanf(arg,"%d %s",&banMinutes,ipStr);
		else {
			banMinutes = gPrefs.deathPenaltyMinutes;
			strcpy(ipStr,arg);
		}
		sscanf(ipStr,"%[0-9*].%[0-9*].%[0-9*].%[0-9*]",aStr,bStr,cStr,dStr);
		if (dStr[0] == 0)
			return;
		a = atol(aStr);
		b = atol(bStr);
		c = atol(cStr);
		d = atol(dStr);

		WizGlobalMessage("Page from System: IP %s banished for %ld minutes by %s",
						ipStr, (long) banMinutes, CvtToCString(cUser->user.name));

		ipAddress = (a << 24) |
					(b << 16) |
					(c << 8) |
					d;
		if (LittleEndian())
			SwapLong((unsigned LONG *) &ipAddress);

		if (cStr[0] == '*') {
			siteFlag = BR_SiteBan2;
		}
		else if (dStr[0] == '*') {
			siteFlag = BR_SiteBan1;
		}
		BanIP(ipAddress, banMinutes, siteFlag);
	}
	else if (strcmp(token, "banuser") == 0 && arg[0] > 0) {
		ServerUserPtr tUser;
		CtoPstr(arg);
		tUser = GetServerUserByName((StringPtr) arg);
		if (tUser) {
			if (UserRank(cUser) >= UserRank(tUser))
				KillUser(cUser, tUser->user.userID);
		}
		else
			UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
	}
	else if (strcmp(token, "gag") == 0 && arg[0] > 0) {
		ServerUserPtr tUser;
		CtoPstr(arg);
		tUser = GetServerUserByName((StringPtr) arg);
		if (tUser) {
			if (UserRank(cUser) >= UserRank(tUser)) {
				tUser->flags |= U_Gag;
				WizGlobalMessage("Page from System: %s gagged by %s",
							CvtToCString(tUser->user.name),
							CvtToCString(cUser->user.name));
			}
		}
		else
			UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
	}
	else if (strcmp(token, "ungag") == 0 && arg[0] > 0) {
		ServerUserPtr tUser;
		CtoPstr(arg);
		tUser = GetServerUserByName((StringPtr) arg);
		if (tUser) {
			tUser->flags &= ~U_Gag;
			WizGlobalMessage("Page from System: %s ungagged by %s",
						CvtToCString(tUser->user.name),
						CvtToCString(cUser->user.name));
		}
		else
			UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
	}
	else if (strcmp(token, "pin") == 0 && arg[0] > 0) {
		ServerUserPtr tUser;
		CtoPstr(arg);
		tUser = GetServerUserByName((StringPtr) arg);
		if (tUser) {
			if (UserRank(cUser) >= UserRank(tUser)) {
				Point	tmpPos = {384-22,512-22};
				LONG	tmpProp[3] = {1,1280,0};
				UpdateUserPosition(tUser, &tmpPos);
				PostUserEvent(tUser,MSG_USERMOVE,tUser->user.userID,(Ptr)&tmpPos,sizeof(Point));
				ChangeUserProp(tUser,&tmpProp[0]);
				PostUserEvent(tUser,MSG_USERPROP, tUser->user.userID, (Ptr) &tmpProp[0], sizeof(LONG)*3);
				tUser->flags |= U_Pin;
				WizGlobalMessage("Page from System: %s pinned by %s",
							CvtToCString(tUser->user.name),
							CvtToCString(cUser->user.name));
			}
		}
		else
			UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
	}
	else if (strcmp(token, "unpin") == 0 && arg[0] > 0) {
		ServerUserPtr tUser;
		CtoPstr(arg);
		tUser = GetServerUserByName((StringPtr) arg);
		if (tUser) {
			LONG	tmpProp[3] = {0,0,0};
			tUser->flags &= ~U_Pin;
			/* 1/30/95 kill the chains AFTER unpinning */
			ChangeUserProp(tUser,&tmpProp[0]);
			PostUserEvent(tUser,MSG_USERPROP, tUser->user.userID, (Ptr) &tmpProp[0], sizeof(LONG)*1);
			WizGlobalMessage("Page from System: %s unpinned by %s",
						CvtToCString(tUser->user.name),
						CvtToCString(cUser->user.name));
		}
		else
			UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
	}
	else if (strcmp(token, "propgag") == 0 && arg[0] > 0) {
		ServerUserPtr tUser;
		CtoPstr(arg);
		tUser = GetServerUserByName((StringPtr) arg);
		if (tUser) {
			if (UserRank(cUser) >= UserRank(tUser)) {
				LONG	tmpProp[3] = {0,0,0};
				ChangeUserProp(tUser,&tmpProp[0]);
				PostUserEvent(tUser,MSG_USERPROP, tUser->user.userID, (Ptr) &tmpProp[0], sizeof(LONG)*1);
				tUser->flags |= U_PropGag;
				WizGlobalMessage("Page from System: %s propgagged by %s",
							CvtToCString(tUser->user.name),
							CvtToCString(cUser->user.name));
			}
		}
		else
			UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
	}
	else if (strcmp(token, "unpropgag") == 0 && arg[0] > 0) {
		ServerUserPtr tUser;
		CtoPstr(arg);
		tUser = GetServerUserByName((StringPtr) arg);
		if (tUser) {
			tUser->flags &= ~U_PropGag;
			WizGlobalMessage("Page from System: %s unpropgagged by %s",
						CvtToCString(tUser->user.name),
						CvtToCString(cUser->user.name));
		}
		else
			UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
	}
	else if (!(cUser->flags & U_God)) {
		UserMessage(cUser, "Invalid server command");
		return;
	}
/* GOD-ONLY COMMANDS FOLLOW
*
*/
	else if (strcmp(token,"reset") == 0) {
		/* reset room counts */
		ServerRoomPtr	cRoom;
		ServerUserPtr	cUser;
		for (cRoom = gRoomList; cRoom; cRoom = cRoom->nextRoom)
			cRoom->room.nbrPeople = 0;
		for (cUser = gUserList; cUser; cUser = cUser->nextUser) {
			if ((cRoom = GetRoom(cUser->user.roomID)) != NULL)
				cRoom->room.nbrPeople++;
		}
	}
	else if (strcmp(token, "ban") == 0 && arg[0] > 0) {
		ServerUserPtr tUser;
		CtoPstr(arg);
		tUser = GetServerUserByName((StringPtr) arg);
		if (tUser) {
			tUser->flags |= U_Banished;
			GodMessage(cUser,"%s banished at your request",CvtToCString(tUser->user.name));
			ScheduleUserKill(tUser, K_BanishKill, 32767);
		}
		else
			UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
	}
	else if (strcmp(token, "recycle")== 0) {
		GodMessage(cUser,"Session IDs Recycled");
		RecycleSessionIDs();
	}
	else if (strcmp(token, "shutdown")== 0) {
		GodMessage(cUser,"Shut down at your request");
		gQuitFlag = true;
	}
	else if (strncmp(token, "wizpass",7) == 0) {
		if (arg[0] == 0 || strlen(arg) > 31) {
			UserMessage(cUser, "Invalid wizard password");
		}
		else {
			CtoPstr(arg);
			EncryptString((unsigned char *) arg,(unsigned char *) arg);
			BlockMove(arg,gPrefs.wizardPassword,arg[0]+1);
			prefsChange = true;
			GodMessage(cUser,"Wizard Password Changed");
			/* Note!! Turn Off Wizards who Aren't Gods, Revoke Wizard Status in Database */
		}
	}
	else if (strncmp(token, "godpass",7) == 0) {
		if (arg[0] == 0 || strlen(arg) > 31) {
			UserMessage(cUser, "Invalid god password");
		}
		else {
			CtoPstr(arg);
			EncryptString((unsigned char *) arg,(unsigned char *) arg);
			BlockMove(arg,gPrefs.godPassword,arg[0]+1);
			prefsChange = true;
			GodMessage(cUser,"God Password Changed");
		}
	}
	else if (strcmp(token, "servername") == 0) {
		CtoPstr(arg);
		if (arg[0] > 31)
			arg[0] = 31;
		BlockMove(arg, gPrefs.serverName, arg[0]+1);
		prefsChange= true;
#if Macintosh
		if (!gLogWin->iconized)
			SetWTitle((WindowPtr) gLogWin,gPrefs.serverName);
#endif
		GodMessage(cUser, "Servername has been set");
	}
	else if (strncmp(token, "maxocc",6) == 0 ||
			 strncmp(token,"maxservocc",10) == 0 ||
			 strncmp(token,"maxserverocc",12) == 0) {
		int	n;
		n = atoi(arg);
		if (n > 0 && n <= MaxPeoplePerServer) {
			if (n < 2)
				n = 2;
			gPrefs.maxOccupancy = n;
			prefsChange = true;
			GodMessage(cUser, "max occupancy has been set to %d users",n);
		}
		else {
			UserMessage(cUser, "max occupancy invalid");
		}
	}
	else if (strncmp(token, "maxroomocc",10) == 0 ||
			 strncmp(token,"defaultroomocc",14) == 0) {
		int	n;
		n = atoi(arg);
		if (n > 0 && n <= MaxPeoplePerRoom) {
			if (n < 2)
				n = 2;
			gPrefs.roomOccupancy = n;
			prefsChange = true;
			GodMessage(cUser, "max room occupancy has been set to %d users",n);
		}
		else
			UserMessage(cUser, "max room occupancy invalid");
	}
	else if (strncmp(token, "roommaxocc",10) == 0) {
		ServerRoomPtr	cRoom;
		int				n;
		cRoom = GetRoom(cUser->user.roomID);
		if (cRoom) {
			n = atoi(arg);
			gModified = true;
			cRoom->maxOccupancy = n;
			GodMessage(cUser, "%d members allowed in this room",(n == 0? gPrefs.roomOccupancy : n));
		}
	}
	else if (strcmp(token, "roommaxguests") == 0) {
		ServerRoomPtr	cRoom;
		int				n;
		cRoom = GetRoom(cUser->user.roomID);
		if (cRoom) {
			n = atoi(arg);
			gModified = true;
			cRoom->maxGuests = n;
			GodMessage(cUser, "%d guests allowed in this room",(n == 0? ((cRoom->maxOccupancy == 0? gPrefs.roomOccupancy : cRoom->maxOccupancy)) : n));
		}
	}
	else if (strcmp(token, "picdir") == 0) {
		CtoPstr(arg);
		if (arg[0] == 0) {
			GodMessage(cUser, "curent pic folder: %s",CvtToCString(gPrefs.picFolder));
		}
		else if (arg[0] > 63) {
			UserMessage(cUser, "invalid folder name");
		}
		else {
			BlockMove(arg, gPrefs.picFolder, arg[0]+1);
			prefsChange = true;
			GodMessage(cUser, "pic folder has been set");
		}
	}
	else if (strcmp(token, "tcp") == 0) {
		if (strcmp(arg,"on") == 0)
			gPrefs.allowTCP = true;
		else
			gPrefs.allowTCP = false;
		prefsChange = true;
		GodMessage(cUser, "TCP %s",gPrefs.allowTCP? "on" : "off");
	}
	else if (strcmp(token, "guests") == 0) {
		if (strcmp(arg,"on") == 0)
			gPrefs.permissions |= PM_AllowGuests;
		else
			gPrefs.permissions &= ~PM_AllowGuests;
		prefsChange = true;
		GodMessage(cUser, "Guests %sallowed",(gPrefs.permissions & PM_AllowGuests)? "" : "not ");
	}
	else if (strcmp(token, "custom") == 0) {
		if (strcmp(arg,"on") == 0)
			gPrefs.permissions |= PM_AllowCustomProps;
		else
			gPrefs.permissions &= ~PM_AllowCustomProps;
		prefsChange = true;
		GodMessage(cUser, "Custom Props %sallowed",(gPrefs.permissions & PM_AllowCustomProps)? "" : "not ");
	}
	else if (strcmp(token, "wizards") == 0) {
		if (strcmp(arg,"on") == 0)
			gPrefs.permissions |= PM_AllowWizards;
		else
			gPrefs.permissions &= ~PM_AllowWizards;
		prefsChange = true;
		GodMessage(cUser, "Wizards %sallowed",(gPrefs.permissions & PM_AllowWizards)? "" : "not ");
	}
	else if (strcmp(token, "wizardkill") == 0) {
		if (strcmp(arg,"on") == 0)
			gPrefs.permissions |= PM_WizardsMayKill;
		else
			gPrefs.permissions &= ~PM_WizardsMayKill;
		prefsChange = true;
		GodMessage(cUser, "Wizards may %sKill",(gPrefs.permissions & PM_WizardsMayKill)? "" : "not ");
	}
	else if (strcmp(token, "playerkill") == 0) {
		if (strcmp(arg,"on") == 0)
			gPrefs.permissions |= PM_PlayersMayKill;
		else
			gPrefs.permissions &= ~PM_PlayersMayKill;
		prefsChange = true;
		GodMessage(cUser, "Players may %sKill",(gPrefs.permissions & PM_PlayersMayKill)? "" : "not ");
	}
	else if (strcmp(token, "spoof") == 0) {
		if (strcmp(arg,"off") == 0)
			gPrefs.permissions |= PM_NoSpoofing;
		else
			gPrefs.permissions &= ~PM_NoSpoofing;
		prefsChange = true;
		GodMessage(cUser, "Players may %sSpoof",(gPrefs.permissions & PM_NoSpoofing)? "not " : "");
	}
	else if (strcmp(token, "wizardsonly") == 0) {
		ServerRoomPtr	cRoom;
		cRoom = GetRoom(cUser->user.roomID);
		if (cRoom) {
			gModified = true;
			if (strcmp(arg,"off") == 0)
				cRoom->room.roomFlags &= ~RF_WizardsOnly;
			else
				cRoom->room.roomFlags |= RF_WizardsOnly;
			GodMessage(cUser, "%s allowed in this room",(cRoom->room.roomFlags & RF_WizardsOnly)? "Only wizards" : "Members");
		}
	}
	else if (strcmp(token, "botkill") == 0) {
		if (strcmp(arg,"on") == 0)
			gPrefs.permissions |= PM_CyborgsMayKill;
		else
			gPrefs.permissions &= ~PM_CyborgsMayKill;
		prefsChange = true;
		GodMessage(cUser, "Cyborgs may %sKill",(gPrefs.permissions & PM_CyborgsMayKill)? "" : "not ");
	}
	else if (strncmp(token, "author",6) == 0) {
		if (strcmp(arg,"on") == 0)
			gPrefs.permissions |= PM_WizardsMayAuthor;
		else
			gPrefs.permissions &= ~PM_WizardsMayAuthor;
		prefsChange = true;
		GodMessage(cUser, "Wizards may %sAuthor",(gPrefs.permissions & PM_WizardsMayAuthor)? "" : "not ");
	}
	else if (strncmp(token, "purge",5) == 0) {
		int	n;
		n = atoi(arg);
		if (n <= 0 || n > 30000) {
			gPrefs.permissions &= ~PM_PurgeInactiveProps;
			prefsChange = true;
			GodMessage(cUser, "No Purging");
		}
		else {
			gPrefs.permissions |= PM_PurgeInactiveProps;
			gPrefs.purgePropDays = n;
			prefsChange = true;
			GodMessage(cUser, "Purge Inactive Props after %d days",n);
		}
	}
	else if (strcmp(token,"ipspoof") == 0) {
		strcpy(cUser->verbalIP, arg);
		cUser->netAddress.ipAddress = LongRandom();
		cUser->flags &= ~(U_SuperUser | U_God);
	}
#if TAP
    else if (strcmp(token,"txp") == 0) {
            TAPOn(cUser,arg);
    }
    else if (strcmp(token,"untxp") == 0) {
            TAPOff(cUser,arg);
    }
#endif
	else
		UserMessage(cUser, "Invalid server command");


	if (prefsChange) {
		StorePreferences();  		/* local function */
		SendServerInfoToAll();
	}
}