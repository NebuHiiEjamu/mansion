/* God Macros */
#include "s-server.h"
#include "s-secure.h"
#include "s-gmacro.h"

LONG	gLastMemberPager;

static char	*gPageHeader = "Page from System:";

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
	TimeLogMessage("%s\r",tbuf);
}

void WizNeighborMessage(ServerUserPtr cUser, char *tmp,...)
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
		if (curUser != cUser && !(curUser->flags & U_Kill) && 
			(curUser->flags & U_SuperUser))
			PostUserEvent(curUser,MSG_TALK, 0L,tbuf,len);
	TimeLogMessage("%s\r",tbuf);
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

	TimeLogMessage("%s\r",tbuf);
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


Boolean GodMacro(ServerUserPtr cUser, ServerUserPtr tUser,
				 char *buffer, Boolean encrypted)
{
	char	tbuf[256]="";
	short	len;
	/* 4/9/96 added support for forward as well as backquotes */
	if ((buffer[0] == '`' || buffer[0] == '\'') && !encrypted)	/* 9/29/95 process unencrypted god commands too */
		strcpy(tbuf,buffer);
	else if (encrypted) {
		len = *((short *) buffer) - 3;
		if (len > 0 && len < 256)
			DecryptCString((unsigned char*) buffer+2, (unsigned char *) tbuf, len);
	}
	else
		return false;
	if (tbuf[0] == '`' || tbuf[0] == '\'') {
		ProcessGodCommand(cUser, tUser, &tbuf[1]);
		return true;
	}
	else
		return false;
}

/* produce an alpha key which is difficult to convert to
   original registration number */
void SeedToWizKey(ServerUserPtr cUser, char *seedStr);

/* 1/14/97 JAB Changed to user cUser as input, and to use
   a base 13, instead of base 10 */
#define WIZKEYBASE	13

void SeedToWizKey(ServerUserPtr cUser, char *seedStr)
{
	unsigned LONG seedCounter;

	/* 1/14/97 JAB */
	/* 1/27/97 JAB always use puidCtr if it is availble */
	if (cUser->puidCtr)
		seedCounter = cUser->puidCtr ^ cUser->puidCRC;
	else
		seedCounter = cUser->counter ^ MAGIC_LONG ^ cUser->crc;

	*(seedStr++) = '{';
	if (cUser->puidCtr)
		*(seedStr++) = 'Z';

	while (seedCounter) {
		*(seedStr++) = 'A' + ((seedCounter % WIZKEYBASE) ^ 4);
		seedCounter /= WIZKEYBASE;
	}
	*(seedStr++) = '}';
	*seedStr = 0;
}

LONG WizKeytoSeed(char *seedStr)
{
    char    *str = seedStr;
    LONG 	ctr = 0,mag=1;
    while (*str) {
			if (*str < 'A' || *str > 'Q')
				return -1;
            ctr += ((*str - 'A') ^ 4) * mag;
            mag *= WIZKEYBASE;
            ++str;
    }
    return ctr;
}

void ListUsers(ServerUserPtr cUser, char *arg, Boolean globalFlag)
{
	ServerUserPtr	curUser,tUser=NULL;
	int				numericFlag = false,keyFlag = false,litkeyFlag = false;
	LONG			usrCnt = 0;
	char			seedStr[32]="";
	unsigned LONG	seedCounter;
	
	while (*arg == '-') {
		++arg;
		switch (*arg) {
		case 'n':	numericFlag = true;	break;
		case 'k':	keyFlag = true;		break;
		case 'l':	litkeyFlag = true;	break;
		}
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
			/* 6/25/96 Added support for list -k KEY */
			/* 1/14/97 Changed calling conventions */
			SeedToWizKey(curUser,seedStr);
			if (strstr(numBuf, arg) == NULL && (strstr(seedStr,arg) == NULL || !keyFlag))
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

		/* 1/17/97 JAB changed calling conventions */
		/* 1/27/97 JAB Always use puidCounter if present */
		if (keyFlag) {
			if (curUser->puidCtr)
				seedCounter = curUser->puidCtr ^ curUser->puidCRC;
			else
				seedCounter = curUser->counter ^ MAGIC_LONG ^ curUser->crc;
			if (!litkeyFlag)
				SeedToWizKey(curUser,seedStr);
			else
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

Boolean WizardsExist()
{
	ServerUserPtr	cUser;
	for (cUser = gUserList; cUser; cUser = cUser->nextUser)
		if (cUser->flags & U_SuperUser)
			return true;
	return false;
}

char *GetUserRoomName(ServerUserPtr cUser)
{
	ServerRoomPtr	rp;
	StringPtr		rName;
	static			char nulllReturn[] = "";

	rp = GetRoom(cUser->user.roomID);
	if (rp) {
		rName = (StringPtr) &rp->room.varBuf[rp->room.roomNameOfst];
		return CvtToCString(rName);
	}
	else
		return nulllReturn;
}

void PerformPage(ServerUserPtr cUser, char *msg)
{
#if !unix
	SysBeep(1);
#endif
	if (cUser->flags & U_SuperUser) {
		WizGlobalMessage("Page from %s: %.200s",
				CvtToCString(cUser->user.name),msg);
	}
	else {
		WizGlobalMessage("Page from %s in %s: %.168s",
				CvtToCString(cUser->user.name), 
				GetUserRoomName(cUser),
				msg);

		gLastMemberPager = cUser->user.userID;
		UserMessage(cUser, "The wizards have been paged");
	}
}

void PerformER(ServerUserPtr cUser)
{
	ServerUserPtr	tUser;
	if (gLastMemberPager) {
		tUser = GetServerUser(gLastMemberPager);
		if (tUser) {
			ChangeUserRoom(cUser, tUser->user.roomID, false);	/* 6/23 */
			WizNeighborMessage(cUser, "%s %s has responded to %s's page via 'er",
							gPageHeader,
							CvtToCString(cUser->user.name),
							CvtToCString(tUser->user.name));
		}
		else {
			gLastMemberPager = 0;
			UserMessage(cUser, "The sender of the last page is no longer present.");
		}
	}
	else {
		UserMessage(cUser, "No member pages have been sent");
	}
}

void PerformResPage(ServerUserPtr cUser, char *msg)
{
	ServerUserPtr	tUser;
	if (gLastMemberPager) {
		tUser = GetServerUser(gLastMemberPager);
		if (tUser) {
			ESPMessage(cUser,tUser, msg, false);
			WizNeighborMessage(cUser, "%s %s has responded to %s's page via 'repage",
							gPageHeader,
							CvtToCString(cUser->user.name),
							CvtToCString(tUser->user.name));
		}
		else {
			gLastMemberPager = 0;
			UserMessage(cUser, "The sender of the last page is no longer present.");
		}
	}
	else {
		UserMessage(cUser, "No member pages have been sent");
	}
}

void PerformRespond(ServerUserPtr cUser, char *msg)
{
	ServerUserPtr	tUser;

	if (cUser->lastESPsender) {
		tUser = GetServerUser(cUser->lastESPsender);
		if (tUser) {
			ESPMessage(cUser,tUser,msg,false);
		}
		else {
			cUser->lastESPsender = 0;
			UserMessage(cUser, "The sender is no longer present");
		}
	}
	else {
		UserMessage(cUser, "You have not received an ESP");
	}
}

int ParseGodCommand(char *token);

int ParseGodCommand(char *token)
{
	int	i;
	if (token[0] == 0)
		return -1;
	for (i = 0; gPTable[i].keyword != NULL; ++i) {
		if (gPTable[i].keyword[0] == '*') {
			if (token[0] == gPTable[i].keyword[1] && 
				strncmp(gPTable[i].keyword+1,token,strlen(gPTable[i].keyword)-1) == 0)
				return i;
		}
		else {
			if (token[0] == gPTable[i].keyword[0] && strcmp(gPTable[i].keyword,token) == 0)
				return i;
		}			
	}
	return -1;
}


void ProcessGodCommand( ServerUserPtr cUser, 
						ServerUserPtr targetUser, 
						char *p)
{
	char	token[256],*arg;
	Boolean	prefsChange = false;

	int		commandIdx,command=0,argIdx,argCommand=0;

	p = GetGodToken(token, p);
	arg = p;

	commandIdx = ParseGodCommand(token);	
	if (commandIdx == -1) {
		UserMessage(cUser, "Invalid server command");
		return;
	}
	if (UserRank(cUser) < gPTable[commandIdx].rank) {
		UserMessage(cUser, "Sorry - you don't have the privileges to use that command");
		return;
	}

	command = gPTable[commandIdx].cmd;
	if (*arg) {
		argIdx = ParseGodCommand(arg);
		if (argIdx != -1)
			argCommand = gPTable[argIdx].cmd;
	}

	/* add user argument, if provided, and command is appropriate */
	if (targetUser && gPTable[commandIdx].userArg) {
		if (arg[0] != 0)
			strcat(arg," ");
		strcat(arg, CvtToCString(targetUser->user.name));
	}

/* Wizard/God Commands First */
	switch (command) {
	case GC_help:
		{
			ServerRoomPtr	cRoom;
			cRoom = GetRoom(cUser->user.roomID);

			if (cUser->flags & U_Guest)
				return;

			UserMessage(cUser, "; `mute <name>             - mute another user");
			UserMessage(cUser, "; `hidefrom <name>         - hide from another user");
			UserMessage(cUser, "; `hide on|off             - hide from all other users");
			UserMessage(cUser, "; `rejectesp on|off        - reject ESP messages");
			UserMessage(cUser, "; `rejectprivate on|off    - reject private messages");
			UserMessage(cUser, "; `page <msg>              - summon help from a wizard");
			UserMessage(cUser, "; `re <msg>                - reply to the last sender of an ESP message");
			if (gPrefs.permissions & PM_MemberCreatedRooms) 
			{
				UserMessage(cUser, "; `password <password>     - Specify a navigation password");
				UserMessage(cUser, "; `newroom [name]          - Create a new room (one allowed per user)");
				/* only if the current room is owned */
				if (cRoom && cRoom->memberOwner == cUser->counter) 
				{
					UserMessage(cUser, "; _ROOM OWNER COMMANDS_");
					UserMessage(cUser, "; `rowner <name>           - Transfer room ownership to someone else");
					UserMessage(cUser, "; `rdelete                 - Delete the room");
					UserMessage(cUser, "; `rname <title>           - Rename the room");
					UserMessage(cUser, "; `rpicture <filename>     - Specify a new room background image");
					UserMessage(cUser, "; `rkick <name>            - Kick someone out of the room");
					UserMessage(cUser, "; `runkick <name>          - Cancel a kick");
					UserMessage(cUser, "; `rpassword <password>    - Make the room password protected");
					UserMessage(cUser, "; `rclose                  - Close the room to further visitors");
					UserMessage(cUser, "; `ropen                   - Open the room to visitors");
					UserMessage(cUser, "; `rscripts on|off         - Enable/Disable scripts");
					UserMessage(cUser, "; `rpainting on|off        - Enable/Disable painting");
					UserMessage(cUser, "; `rguests on|off          - Allow/Disallow guests to enter");
					UserMessage(cUser, "; `rhide on|off            - Make the room hidden / unhidden");
				}
			}

			if (!(cUser->flags & U_SuperUser))
				return;

			UserMessage(cUser, "; _WIZARD COMMANDS_");
			UserMessage(cUser, "; `list [-n] <name>        - list ip address of specific user");
			UserMessage(cUser, "; `list [-n] <ip>          - list users matching ip address");
			UserMessage(cUser, "; `list [-n]               - list ip addresses of users in room");
			UserMessage(cUser, "; `glist [-n]              - list ip address of all users");
			UserMessage(cUser, "; `banlist [str]           - show banned users");
			UserMessage(cUser, "; `purgebanlist            - purge elapsed ban records");
			UserMessage(cUser, "; `banip [minutes] <ip>    - ban ipaddress (may use 12.24.23.*)");
			UserMessage(cUser, ";                          - numeric ips only!");
			UserMessage(cUser, "; `kill [minutes] <name>   - kill currently connected user");
			UserMessage(cUser, "; `unban <str>             - unban user(s) who match str (ip or name)");
			UserMessage(cUser, "; `gag <name>              - gag currently connected user");
			UserMessage(cUser, "; `ungag <name>            - ungag currently connected user");
			UserMessage(cUser, "; `propgag <name>          - propgag a member");
			UserMessage(cUser, "; `unpropgag <name>        - unpropgag a member");
			UserMessage(cUser, "; `death <N>               - set default death penalty to N minutes");
			UserMessage(cUser, "; `paint on|off            - turn painting on or off serverwide");
			UserMessage(cUser, "; `bots on|off             - turn cyborgs on or off serverwide");
			UserMessage(cUser, "; `flood <N>               - kill flooders after N events in 1 second");
			UserMessage(cUser, "; `duplicate               - duplicate the current room");
			UserMessage(cUser, "; `delete                  - delete the current room");
			UserMessage(cUser, "; `purgeprops <N>          - purge props older than N days");
			UserMessage(cUser, "; `repage <msg>            - respond to the last (non-wizard) page sender");
			UserMessage(cUser, "; `er                      - go to the room of last (non-wizard) page sender");
			UserMessage(cUser, "; `comment <ip> <reason>   - add a comment to a banlist record");
			UserMessage(cUser, "; `trackip [minutes] <ip>  - track sign-ons from ipaddress (may use 12.24.23.*)");
			UserMessage(cUser, "; `track [minutes] <name>  - track sign-ons from currently connected user");
			UserMessage(cUser, "; `untrack <str>           - untrack user(s) who match str (ip or name)");
			UserMessage(cUser, "; `extend <str> <minutes>  - extend death penalty on a ban record");
			UserMessage(cUser, "; `roommaxocc <N>          - set the max occupancy for the current room");
			UserMessage(cUser, "; `roommaxguests <N>       - set max guest occupancy for the current room");
			UserMessage(cUser, "; `autoannounce <msg>      - set the server greeting");

			if (!(cUser->flags & U_God))
				return;

			UserMessage(cUser, "; _GOD COMMANDS_");
			UserMessage(cUser, "; `ban user                - banish a user from the server");
			UserMessage(cUser, "; `shutdown                - shut down the server");
			UserMessage(cUser, "; `wizpass <password>      - set the wizard password");
			UserMessage(cUser, "; `godpass <password>      - set the god password");
			UserMessage(cUser, "; `servername <name>       - set the server name");
			UserMessage(cUser, "; `maxserverocc <N>        - set the maximum server occupancy");
			UserMessage(cUser, "; `defaultroomocc <N>      - set the default room occupancy");
			UserMessage(cUser, "; `picdir <pathname>       - identify the pictures folder");
			UserMessage(cUser, "; `tcp on|off              - enable TCP connections");
			UserMessage(cUser, "; `guests on|off           - enable guest access to the server (on by default)");
			UserMessage(cUser, "; `custom on|off           - enable the user of custom props (on by default)");
			UserMessage(cUser, "; `wizards on|off          - enable support for wizards (on by default)");
			UserMessage(cUser, "; `wizardkill on|off       - wizards may kill (on by default)");
			UserMessage(cUser, "; `botkill on|off          - cyborgs may kill (off by default)");
			UserMessage(cUser, "; `playerkill on|off       - members may kill (off by default)");
			UserMessage(cUser, "; `author on|off           - wizards may author (on by default)");
			UserMessage(cUser, "; `spoof on|off            - members may spoof using @x,y (on by default)");
			UserMessage(cUser, "; `memberrooms on|off      - members may create their own rooms");
			UserMessage(cUser, "; `wizardsonly on|off      - set current room as wizard only");
			UserMessage(cUser, "; `recycle <N>             - automatically recycle guest ID numbers at N");
			UserMessage(cUser, "; `recycle                 - recycle guest ID numbers now");
			UserMessage(cUser, "; `reset                   - reset room counts");
			UserMessage(cUser, "; `uplist N                - move the current room N slots up in the room list");
			UserMessage(cUser, "; `downlist N              - move the current room N slots down in the room list");
			UserMessage(cUser, "; `dropzone                - make the current room a drop zone (front gate)");
			UserMessage(cUser, "; `passwordsecurity        - enable password security (auto-kicks after 3 attempts)");
		}
		break;
	/*
		MEMBER commands follow
	 */
	case GC_hide:
		if (argCommand == GC_off)
			cUser->flags &= ~U_Hide;
		else
			cUser->flags |= U_Hide;
		TimeLogMessage("%s %shidden\r", 
				CvtToCString(cUser->user.name),
				(cUser->flags & U_Hide)? "" : "not ");
		UserMessage(cUser, "You are %shidden",
				(cUser->flags & U_Hide)? "" : "not ");
		break;
	case GC_unhide:
		cUser->flags &= ~U_Hide;
		TimeLogMessage("%s Unhidden\r", CvtToCString(cUser->user.name));
		UserMessage(cUser, "You are unhidden");
		break;
	case GC_hidefrom: 
		SetUserAction(cUser, arg, UA_HideFrom, true);
		break;
	case GC_page:
		if (*arg)
			PerformPage(cUser, arg);
		break;
	case GC_respond:
		if (*arg)
			PerformRespond(cUser, arg);
		break;
	case GC_unhidefrom: 
		SetUserAction(cUser, arg, UA_HideFrom, false);
		break;
	case GC_mute: 
		SetUserAction(cUser, arg, UA_Mute, true);
		break;
	case GC_unmute: 
		SetUserAction(cUser, arg, UA_Mute, false);
		break;
	case GC_rejectesp: 
		if (argCommand == GC_off)
			cUser->flags &= ~U_RejectESP;
		else
			cUser->flags |= U_RejectESP;
		TimeLogMessage("%s %srejecting ESP messages\r", 
				CvtToCString(cUser->user.name),
				(cUser->flags & U_RejectESP)? "" : "not ");
		UserMessage(cUser, "You are %srejecting ESP messages",
				(cUser->flags & U_RejectESP)? "" : "not ");
		break;
	case GC_rejectprivate: 
		if (argCommand == GC_off)
			cUser->flags &= ~U_RejectPrivate;
		else
			cUser->flags |= U_RejectPrivate;
		TimeLogMessage("%s %srejecting private messages\r", 
				CvtToCString(cUser->user.name),
				(cUser->flags & U_RejectPrivate)? "" : "not ");
		UserMessage(cUser, "You are %srejecting private messages",
				(cUser->flags & U_RejectPrivate)? "" : "not ");
		break;
		
	case GC_newroom:
		if ((gPrefs.permissions & PM_MemberCreatedRooms) > 0)
			NewMemberRoom(cUser,arg);
		break;
	case GC_rname: 
		MemberRoomModification(cUser, MR_Title, arg);
		break;
	case GC_rdelete: 
		MemberRoomModification(cUser, MR_Delete, arg);
		break;
	case GC_rpicture: 
		MemberRoomModification(cUser, MR_Picture, arg);
		break;
	case GC_rowner: 
		MemberRoomModification(cUser, MR_Owner, arg);
		break;
	case GC_rkick: 
		MemberRoomModification(cUser, MR_Kick, arg);
		break;
	case GC_runkick: 
		MemberRoomModification(cUser, MR_Unkick, arg);
		break;
	case GC_rpassword: 
		MemberRoomModification(cUser, MR_Password, arg);
		break;
	case GC_rclose: 
		MemberRoomModification(cUser, MR_Close, arg);
		break;
	case GC_ropen: 
		MemberRoomModification(cUser, MR_Open, arg);
		break;
	case GC_rscripts: 
		MemberRoomModification(cUser, MR_Scripts, arg);
		break;
	case GC_rpainting: 
		MemberRoomModification(cUser, MR_Painting, arg);
		break;
	case GC_rguests: 
		MemberRoomModification(cUser, MR_Guests, arg);
		break;
	case GC_rhide: 
		MemberRoomModification(cUser, MR_Hide, arg);
		break;
	case GC_runhide: 
		MemberRoomModification(cUser, MR_Unhide, arg);
		break;
	case GC_password: 
		SetUserNavPassword(cUser, arg);
		break;
	/* 
	   WIZARD commands follow
	 */
	case GC_repage: 
		PerformResPage(cUser, arg);
		break;
	case GC_er: 
		PerformER(cUser);
		break;
	case GC_list:
	case GC_glist:
		if (cUser)
			ListUsers(cUser,arg,command == GC_glist);
		break;
	case GC_painting:
		if (argCommand == GC_on)
			gPrefs.permissions |= PM_AllowPainting;
		else
			gPrefs.permissions &= ~PM_AllowPainting;
		prefsChange = true;
		GodMessage(cUser, "Painting %sallowed on entire server",(gPrefs.permissions & PM_AllowPainting)? "" : "not ");
		break;
	case GC_bots:
		if (argCommand == GC_on)
			gPrefs.permissions |= PM_AllowCyborgs;
		else
			gPrefs.permissions &= ~PM_AllowCyborgs;
		prefsChange = true;
		GodMessage(cUser, "Cyborgs %sallowed on entire server",(gPrefs.permissions & PM_AllowCyborgs)? "" : "not ");
		break;
	case GC_deathpenalty:
		{
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
		break;
	case GC_floodlimit:
		{
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
		break;
	case GC_duplicate:
		{
			ServerRoomPtr	nRoom;
			int				n;
			n = cUser->user.roomID;
			nRoom = GetRoom(n);
			if (nRoom)
				DuplicateRoom(cUser);
			GodMessage(cUser, "room %d duplicated",n);
		}
		break;
	case GC_deleteroom:
		if (targetUser == NULL) {
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
				WizGlobalMessage("%s room %d deleted by %s",
								gPageHeader,
								 n,CvtToCString(cUser->user.name));
			}
		}
		break;
	/* 12/3/95 banip [time] ip-address */
	/* ipaddress may use asterisks to indicate site ban */
	
	case GC_banlist: 
		DoBanList(cUser, arg);
		break;
	case GC_purgebanlist: 
		PurgeBanList(cUser);
		break;
	case GC_unban: 
		DoUnBan(cUser, arg, 0);
		break;
	case GC_untrack: 
		DoUnBan(cUser, arg, 1);
		break;
	case GC_banip:
	case GC_trackip:
		if (*arg) {
			int		banMinutes = 0;
			char	aStr[8]="",bStr[8]="",cStr[8]="",dStr[8]="";
			char	ipStr[128] = "";
			int		siteFlag = 0;		
			unsigned LONG	a,b,c,d,ipAddress;
			Boolean trackFlag = (command == GC_trackip);

			if (strchr(arg,' ') != NULL)
				sscanf(arg,"%d %s",&banMinutes,ipStr);
			else {
				banMinutes = gPrefs.deathPenaltyMinutes;
				strcpy(ipStr,arg);
			}
			sscanf(ipStr,"%[0-9*].%[0-9*].%[0-9*].%[0-9*]",aStr,bStr,cStr,dStr);
			if (bStr[0] == 0)
				return;
			if (cStr[0] == 0)
				strcpy(cStr,"*");
			if (dStr[0] == 0)
				strcpy(dStr,"*");

			a = atol(aStr);
			b = atol(bStr);
			c = atol(cStr);
			d = atol(dStr);

			WizGlobalMessage("%s IP %s %s for %ld minutes by %s",
							gPageHeader,
							ipStr, 
							trackFlag? "being tracked" : "banned",
							(long) banMinutes, CvtToCString(cUser->user.name));

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
			if (trackFlag)
				siteFlag |= BR_Tracking;
			BanIP(cUser,ipAddress, banMinutes, siteFlag);
		}
		break;
	case GC_banuser:
	case GC_kill:
	case GC_track:
		if (*arg) {
			ServerUserPtr tUser;
			int				minutes = gPrefs.deathPenaltyMinutes;
			Boolean			trackFlag;

			trackFlag = (command == GC_track);
			CtoPstr(arg);
			tUser = GetServerUserByName((StringPtr) arg);
			PtoCstr((StringPtr) arg);
			/* 6/14/96 JAB - added parsing for Keys */
			if (tUser == NULL && isdigit(arg[0]) && strchr(arg,' ') != NULL) {
				minutes = atoi(arg);
				while (*arg && isdigit(*arg))
					++arg;
				while (*arg && isspace(*arg))
					++arg;
				CtoPstr(arg);
				tUser = GetServerUserByName((StringPtr) arg);
				PtoCstr((StringPtr) arg);
			}
			if (tUser) {
				if (UserRank(cUser) >= UserRank(tUser))
					KillUser(cUser, tUser->user.userID, (LONG) minutes, trackFlag);
			}
			else {
				/* 6/14/96 Allow user to be identified by KEY */
				if ((cUser->flags & U_God) != 0) {
					if (BanUserByKey(cUser, arg, trackFlag? K_Unknown : K_KilledByPlayer, 
							minutes, trackFlag? BR_Tracking : 0) == 0)
						WizGlobalMessage("%s key %s %s for %d minutes by %s",
								gPageHeader,
								arg,trackFlag? "tracked" : "killed",(int) minutes,
								CvtToCString(cUser->user.name));
					else
						UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
				}
				else
					UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
			}
		}
		break;
	case GC_comment:
		if (*arg)
			CommentBanRec(cUser, arg);
		break;
	case GC_extend:
		if (*arg)
			ExtendBanRec(cUser, arg);
		break;
	case GC_gag:
		if (*arg) {
			ServerUserPtr tUser;
			CtoPstr(arg);
			tUser = GetServerUserByName((StringPtr) arg);
			if (tUser) {
				if (UserRank(cUser) >= UserRank(tUser)) {
					tUser->flags |= U_Gag;
					WizGlobalMessage("%s %s gagged by %s",
								gPageHeader,
								CvtToCString(tUser->user.name),
								CvtToCString(cUser->user.name));
				}
			}
			else
				UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
		}
		break;
	case GC_ungag:
		if (*arg)
		{
			ServerUserPtr tUser;
			CtoPstr(arg);
			tUser = GetServerUserByName((StringPtr) arg);
			if (tUser) {
				tUser->flags &= ~U_Gag;
				WizGlobalMessage("%s %s ungagged by %s",
							gPageHeader,
							CvtToCString(tUser->user.name),
							CvtToCString(cUser->user.name));
			}
			else
				UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
		}
		break;
	case GC_pin:
		if (*arg) {
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
					WizGlobalMessage("%s %s pinned by %s",
								gPageHeader,
								CvtToCString(tUser->user.name),
								CvtToCString(cUser->user.name));
				}
			}
			else
				UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
		}
		break;
		
	case GC_unpin:
		if (*arg) {
			ServerUserPtr tUser;
			CtoPstr(arg);
			tUser = GetServerUserByName((StringPtr) arg);
			if (tUser) {
				LONG	tmpProp[3] = {0,0,0};
				tUser->flags &= ~U_Pin;
				/* 1/30/95 kill the chains AFTER unpinning */
				ChangeUserProp(tUser,&tmpProp[0]);
				PostUserEvent(tUser,MSG_USERPROP, tUser->user.userID, (Ptr) &tmpProp[0], sizeof(LONG)*1);
				WizGlobalMessage("%s %s unpinned by %s",
							gPageHeader,
							CvtToCString(tUser->user.name),
							CvtToCString(cUser->user.name));
			}
			else
				UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
		}
		break;
	case GC_propgag:
		if (*arg) {
			ServerUserPtr tUser;
			CtoPstr(arg);
			tUser = GetServerUserByName((StringPtr) arg);
			if (tUser) {
				if (UserRank(cUser) >= UserRank(tUser)) {
					LONG	tmpProp[3] = {0,0,0};
					ChangeUserProp(tUser,&tmpProp[0]);
					PostUserEvent(tUser,MSG_USERPROP, tUser->user.userID, (Ptr) &tmpProp[0], sizeof(LONG)*1);
					tUser->flags |= U_PropGag;
					WizGlobalMessage("%s %s propgagged by %s",
								gPageHeader,
								CvtToCString(tUser->user.name),
								CvtToCString(cUser->user.name));
				}
			}
			else
				UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
		}
		break;
	case GC_unpropgag:
		if (*arg) {
			ServerUserPtr tUser;
			CtoPstr(arg);
			tUser = GetServerUserByName((StringPtr) arg);
			if (tUser) {
				tUser->flags &= ~U_PropGag;
				WizGlobalMessage("%s %s unpropgagged by %s",
							gPageHeader,
							CvtToCString(tUser->user.name),
							CvtToCString(cUser->user.name));
			}
			else
				UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
		}
		break;
	case GC_sortprops: 
		SortAssetsByUsage();
		UserMessage(cUser,"Props Sorted");
		break;
	case GC_purgeprops: 
		{
			LONG	n;
			int		days;
			days = atoi(arg);
			n = PurgeServerProps(days);
			UserMessage(cUser,"%ld Props Purged",(long) n);
		}
		break;
		
/* Most God commands below here */
	case GC_reset: 
		/* reset room counts */
		{
			ServerRoomPtr	cRoom;
			ServerUserPtr	cUser;
			for (cRoom = gRoomList; cRoom; cRoom = cRoom->nextRoom)
				cRoom->room.nbrPeople = 0;
			for (cUser = gUserList; cUser; cUser = cUser->nextUser) {
				if ((cRoom = GetRoom(cUser->user.roomID)) != NULL)
					cRoom->room.nbrPeople++;
			}
		}
		break;
	case GC_ban:
		if (*arg) {
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
		break;
	case GC_recycle:
		if (*arg) {
			if (atol(arg) > gNbrUsers) {
				gPrefs.recycleLimit = atol(arg);
				if (gPrefs.recycleLimit > MaxUserID)
					gPrefs.recycleLimit = MaxUserID;
				GodMessage(cUser,"Session IDs will recycle at %ld",(long) gPrefs.recycleLimit);
			}
			else
				GodMessage(cUser,"Recycle Limit is too low (1000 or greater is suggested)");
		}
		else {
			GodMessage(cUser,"Session IDs Recycled");
			RecycleSessionIDs();
		}
		break;

	case GC_shutdown:
		GodMessage(cUser,"Shut down at your request");
		gQuitFlag = true;
		break;
		
	case GC_wizpassword:
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
		
		break;

	case GC_godpassword:
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
		break;
	case GC_servername:
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
		break;
	case GC_maxoccupancy:
		{
			int	n;
			n = atoi(arg);
			if (n > 0 && n <= gMaxPeoplePerServer) {
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
		break;
	case GC_defaultroomocc:
		{
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
		break;
	case GC_roommaxocc:
		{
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
		break;
	case GC_roommaxguests:
		{
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
		break;
	case GC_picdir:
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
		break;
	case GC_tcp:
		if (argCommand == GC_on)
			gPrefs.allowTCP = true;
		else
			gPrefs.allowTCP = false;
		prefsChange = true;
		GodMessage(cUser, "TCP %s",gPrefs.allowTCP? "on" : "off");
		break;
	case GC_guestaccess:
		if (argCommand == GC_on)
			gPrefs.permissions |= PM_AllowGuests;
		else
			gPrefs.permissions &= ~PM_AllowGuests;
		prefsChange = true;
		GodMessage(cUser, "Guests %sallowed",(gPrefs.permissions & PM_AllowGuests)? "" : "not ");
		break;
	case GC_customprops:
		if (argCommand == GC_on)
			gPrefs.permissions |= PM_AllowCustomProps;
		else
			gPrefs.permissions &= ~PM_AllowCustomProps;
		prefsChange = true;
		GodMessage(cUser, "Custom Props %sallowed",(gPrefs.permissions & PM_AllowCustomProps)? "" : "not ");
		break;
	case GC_allowwizards:
		if (argCommand == GC_on)
			gPrefs.permissions |= PM_AllowWizards;
		else
			gPrefs.permissions &= ~PM_AllowWizards;
		prefsChange = true;
		GodMessage(cUser, "Wizards %sallowed",(gPrefs.permissions & PM_AllowWizards)? "" : "not ");
		break;
	case GC_wizardkill:
		if (argCommand == GC_on)
			gPrefs.permissions |= PM_WizardsMayKill;
		else
			gPrefs.permissions &= ~PM_WizardsMayKill;
		prefsChange = true;
		GodMessage(cUser, "Wizards may %sKill",(gPrefs.permissions & PM_WizardsMayKill)? "" : "not ");
		break;
	case GC_playerkill:
		if (argCommand == GC_on)
			gPrefs.permissions |= PM_PlayersMayKill;
		else
			gPrefs.permissions &= ~PM_PlayersMayKill;
		prefsChange = true;
		GodMessage(cUser, "Players may %sKill",(gPrefs.permissions & PM_PlayersMayKill)? "" : "not ");
		break;
	case GC_spoof:
		if (argCommand == GC_off)
			gPrefs.permissions |= PM_NoSpoofing;
		else
			gPrefs.permissions &= ~PM_NoSpoofing;
		prefsChange = true;
		GodMessage(cUser, "Players may %sSpoof",(gPrefs.permissions & PM_NoSpoofing)? "not " : "");
		break;
	case GC_memberrooms:
		if (argCommand == GC_off)
			gPrefs.permissions &= ~PM_MemberCreatedRooms;
		else
			gPrefs.permissions |= PM_MemberCreatedRooms;
		prefsChange = true;
		GodMessage(cUser, "Members may %screate rooms",(gPrefs.permissions & PM_MemberCreatedRooms)? "" : "not ");
		break;
	case GC_wizardsonly:
		{
			ServerRoomPtr	cRoom;
			cRoom = GetRoom(cUser->user.roomID);
			if (cRoom) {
				gModified = true;
				if (argCommand == GC_off)
					cRoom->room.roomFlags &= ~RF_WizardsOnly;
				else
					cRoom->room.roomFlags |= RF_WizardsOnly;
				GodMessage(cUser, "%s allowed in this room",(cRoom->room.roomFlags & RF_WizardsOnly)? "Only wizards" : "Members");
			}
		}
		break;
	case GC_uplist: 
		{
			int 	n;
			ServerRoomPtr	cRoom;
			cRoom = GetRoom(cUser->user.roomID);
			if (cRoom) {
				gModified = true;
				n  = atoi(arg);
				if (n == 0)
					n = 1;
				PushRoom(cUser->user.roomID,-n);
				GodMessage(cUser, "Room %s moved up %d places",GetUserRoomName(cUser),
							(int) n);
			}
		}
		break;
	case GC_downlist: 
		{
			int 	n;
			ServerRoomPtr	cRoom;
			cRoom = GetRoom(cUser->user.roomID);
			if (cRoom) {
				gModified = true;
				n  = atoi(arg);
				if (n == 0)
					n = 1;
				PushRoom(cUser->user.roomID,n);
				GodMessage(cUser, "Room %s moved down %d places",GetUserRoomName(cUser),
							(int) n);
			}
		}
		break;
	case GC_dropzone:
		{
			ServerRoomPtr	cRoom;
			cRoom = GetRoom(cUser->user.roomID);
			if (cRoom) {
				gModified = true;
				if (argCommand == GC_off)
					cRoom->room.roomFlags &= ~RF_DropZone;
				else
					cRoom->room.roomFlags |= RF_DropZone;
				GodMessage(cUser, "Room %s  is %s a drop zone",
							GetUserRoomName(cUser),
							(cRoom->room.roomFlags & RF_DropZone)? "now" : "no longer");
			}
		}
		break;
	case GC_botkill:
		if (argCommand == GC_on)
			gPrefs.permissions |= PM_CyborgsMayKill;
		else
			gPrefs.permissions &= ~PM_CyborgsMayKill;
		prefsChange = true;
		GodMessage(cUser, "Cyborgs may %sKill",(gPrefs.permissions & PM_CyborgsMayKill)? "" : "not ");
		break;
	case GC_authoring:
		if (argCommand == GC_on)
			gPrefs.permissions |= PM_WizardsMayAuthor;
		else
			gPrefs.permissions &= ~PM_WizardsMayAuthor;
		prefsChange = true;
		GodMessage(cUser, "Wizards may %sAuthor",(gPrefs.permissions & PM_WizardsMayAuthor)? "" : "not ");
		break;
	case GC_purgelimit:
		{
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
		break;
	/* 6/21/96 JAB */	
	case GC_passwordsecurity: 
		if (argCommand == GC_off)
			gPrefs.serverOptions &= ~SO_PasswordSecurity;
		else
			gPrefs.serverOptions |= SO_PasswordSecurity;
		prefsChange = true;
		GodMessage(cUser, "Password Security %s", (gPrefs.serverOptions & SO_PasswordSecurity)? "On" : "Off");
		break;
	case GC_autoannounce: 
		strcpy(gPrefs.autoAnnounce, arg);
		UserMessage(cUser, "Auto Announcement Set");
		prefsChange = true;
		break;
	case GC_killprop: 
		/* a hidden god command for removing a forged guest prop from the
		  the prop file */
		{
			Handle	h;
			struct {
				long		nbrProps;
				AssetSpec	props[MaxUserProps];
			} ups;
			if (cUser->user.nbrProps) {
				while ((h = GetAssetWithCRC(RT_PROP, cUser->user.propSpec[cUser->user.nbrProps-1].id,
													 cUser->user.propSpec[cUser->user.nbrProps-1].crc)) != NULL)
				{
					RmveAsset(h);
					DisposeHandle(h);
				}
				cUser->user.nbrProps--;
				ups.nbrProps = cUser->user.nbrProps;
				BlockMove(&cUser->user.propSpec[0],&ups.props[0],ups.nbrProps*sizeof(AssetSpec));
				PostRoomEvent(cUser->user.roomID,MSG_USERPROP, cUser->user.userID, (Ptr) &ups, sizeof(LONG)+ups.nbrProps*sizeof(AssetSpec));
			}
			else {
				UserMessage(cUser, "No prop to kill");
			}
		}
		break;
#if unix
	case GC_savesessionkeys: 
		if (argCommand == GC_off)
			gPrefs.serverOptions &= ~SO_SaveSessionKeys;
		else
			gPrefs.serverOptions |= SO_SaveSessionKeys;
		prefsChange = true;
		GodMessage(cUser, "%s session keys", (gPrefs.serverOptions & SO_SaveSessionKeys)? "Saving" : "Not saving");
		break;
#endif
	/* 1/27/97 JAB */
	case GC_allowdemomembers: 
		if (argCommand == GC_off)
			gPrefs.serverOptions &= ~SO_AllowDemoMembers;
		else
			gPrefs.serverOptions |= SO_AllowDemoMembers;
		prefsChange = true;
		GodMessage(cUser, "Demo Members %s", (gPrefs.serverOptions & SO_AllowDemoMembers)? "Allowed" : "Not Allowed");
		break;
	default:
		UserMessage(cUser, "Invalid server command");
		break;
	}

	if (prefsChange) {
		gModified = true;			/* 8/28/96 JAB - save the script too */
		StorePreferences();  		/* local function */
		SendServerInfoToAll();
	}
}
