/* S-Events.c */

/* 1/27/95 JAB Added response to MSG_SUPERUSER and MSG_KILLUSER */

/******************************************************************************************/

#include "s-server.h"
#include "m-assets.h"
#include "m-events.h"
#include <time.h>
/******************************************************************************************/

time_t gLastCorpseCheck;
time_t gLastDirtyCheck;

/* This determines how fast subsequent events have to be to be */
/* counted as a flood */
#if macintosh
#define FLOOD_INTERVAL	1					/* 1/6 of a second  */
#else
#define FLOOD_INTERVAL	1					/* 1/6 of a second */
#endif

/* How slow subsequent events have to be to reset flood counter to zero */
#define FLOOD_RELEASE		2
/* How LONG we wait for a response to a PING before logging the person off */
#define PING_RESPONSE		120
/* How much inactivity must occur before we PING someone */
#define INACTIVE_CHECK		480		/* 8 minutes */
/* How often we check for Users for inactivity */
#define USER_CHECK			5
/* How often we save the script (if it has changed) */
#define SAVESCRIPT_CHECK	60

/******************************************************************************************/

/* !!!! This should be called as frequently as possible (e.g. more than once per second)
*/
void ServerIdle(void)
{
	FileSendIdle();		/* 8/14/95 */
	YellowPagesIdle();	/* 11/27/95 */

	/* removed label 1/17/95 */
	if (gToBeKilled) {
		ServerUserPtr	curUser;
		ServerUserPtr	prevUser = NULL,nextUser;
		for (curUser = gToBeKilled; curUser; curUser = nextUser) {
			nextUser= curUser->nextUser;
			if (DisconnectUser(curUser) == C_None) {
				/* Remove from list */
				if (prevUser == NULL)
					gToBeKilled = curUser->nextUser;
				else
					prevUser->nextUser = curUser->nextUser;
				/* Dispose */
				if (curUser->groupBuffer) {
					DisposePtr(curUser->groupBuffer);	/* 6/24/95 */
					curUser->groupBuffer = NULL;
				}
				DeleteAllUserActionRecords(curUser);	/* 1/30/95 */
				DisposePtr((Ptr) curUser);
				/* removed goto */
			}
			else	/* added else 1/17/95 */
				prevUser = curUser;
		}
	}
	if (time(NULL) - gLastCorpseCheck >= USER_CHECK)
	{
		ServerUserPtr	curUser;

		gLastCorpseCheck = time(NULL);

	Restart:
		for (curUser = gUserList; curUser; curUser = curUser->nextUser) {
			if (curUser->flags & U_Kill) {		/* 5/25/95 */
				LogoffUser(curUser->user.userID, curUser->whyKilled);
				goto Restart;
			}
			/* 7/22/95 Reenabled PINGS for TCP Users
			 * 7/23/95 If user doesn't send _LOGON message (roomID == 0) by 20 seconds, then
			 *         kill them
			 */
			if (!gDebugFlag) {
				if (time(NULL) - curUser->lastActive > PING_RESPONSE && 
					(curUser->nbrPings || curUser->user.roomID == 0))
	      		{
					LogoffUser(curUser->user.userID, K_Unresponsive);
					goto Restart;
				}
				else if (time(NULL) - curUser->lastActive > INACTIVE_CHECK)
	      		{
					curUser->lastActive = time(NULL);
					curUser->nbrPings++;
					PostUserEvent(curUser,MSG_PING,curUser->user.userID, NULL,0L);
				}
			}
		}
	}

	if (time(NULL) - gLastDirtyCheck > SAVESCRIPT_CHECK)
    {
		gLastDirtyCheck = time(NULL);
		if (gModified)
    {
			SaveScript();
			gModified = 0;
		}
		UpdateAssetFile();
	}
}

/* Removed ActiveUser() */

/******************************************************************************************/

/* Returns true if assertion failed */
Boolean UserBadAssert(ServerUserPtr cUser, Boolean condition)
{
	if (!condition) {
        TimeLogMessage("Assertion Failed for User %s\r",CvtToCString(cUser->user.name));
		ScheduleUserKill(cUser, K_CommError, 0);
		return true;
	}
	return false;
}

void ScheduleUserKill(ServerUserPtr cUser, short why, unsigned LONG deathPenalty)
{
	if (why == K_CommError)
		cUser->flags |= U_CommError;
	if (!(cUser->flags & U_Kill)) {
		cUser->flags |= U_Kill;
		cUser->whyKilled = why;
		cUser->deathPenalty = deathPenalty;
		gLastCorpseCheck = time(NULL) - USER_CHECK;	/* Force Kill on Idle */
	}
}

void FloodControl(ServerUserPtr cUser)
{
	++cUser->nbrFloodEvents;
	if (gPrefs.permissions & PM_KillFlooders && cUser->nbrFloodEvents >= gPrefs.minFloodEvents)
		ScheduleUserKill(cUser, K_Flood, 5);
}

/******************************************************************************************/

Boolean IllegalText(ServerUserPtr cUser, char *buffer, Boolean encrypted);
Boolean IllegalText(ServerUserPtr cUser, char *buffer, Boolean encrypted)
{
	char	tbuf[256]="",*p;
	short	len;

	if (cUser == NULL)
		return true;
	if (!(gPrefs.permissions & PM_NoSpoofing) && !(cUser->flags & U_Guest))
		return false;
	if (encrypted) {
		len = *((short *) buffer) - 3;
		if (len > 0 && len < 256)
			DecryptCString((unsigned char*) buffer+2, (unsigned char *) tbuf, len);
	}
	else
		strcpy(tbuf,buffer);
	p = strchr(tbuf,'@');
	if (p && isdigit(p[1]))
		return true;
	return false;
}

/* return TRUE if this is an ESP msg */
Boolean ESPMessage(ServerUserPtr cUser, ServerUserPtr tUser, char *buffer, Boolean encrypted);
Boolean ESPMessage(ServerUserPtr cUser, ServerUserPtr tUser, char *buffer, Boolean encrypted)
{
	char	tbuf[256];
	short	len;

	if (tUser == NULL || cUser->user.roomID == tUser->user.roomID)
		return false;
	if (cUser->flags & U_Guest)	/* guests may not esp */
		return false;

	if (cUser->flags & U_Hide) {	/* hiders may not esp */
		UserMessage(cUser,"You may not ESP while hiding");
		return false;
	}
	if (ActionExists(cUser,tUser, UA_HideFrom))	{ /* hiders may not esp */
		UserMessage(cUser,"You are hiding from %s, no ESP allowed.",CvtToCString(tUser->user.name));
		return false;
	}
								/* if target is rejecting ESP or Privates msgs */
	if ((tUser->flags & U_RejectESP) && !(cUser->flags & U_SuperUser)) {
		UserMessage(cUser,"%s is rejecting ESP messages",CvtToCString(tUser->user.name));
		return false;
	}
	if (ActionExists(tUser, cUser, UA_Mute) ||
		((tUser->flags & U_RejectPrivate) && !(cUser->flags & U_SuperUser))) {
		UserMessage(cUser,"%s is rejecting private messages",CvtToCString(tUser->user.name));
		return false;
	}
	if (encrypted) {
		len = *((short *) buffer) - 3;
		if (len > 0 && len < 256)
			DecryptCString((unsigned char*) buffer+2, (unsigned char *) tbuf, len);
	}
	else
		strcpy(tbuf,buffer);
	UserPrivateMessage(cUser,"Message to %s: %.200s",CvtToCString(tUser->user.name),tbuf);
	UserPrivateMessage(tUser,"Message from %s: %.200s",CvtToCString(cUser->user.name),tbuf);
	return true;
}

void ProcessBlowthrough(ServerUserPtr	cUser, LONG refCon, char *buffer, LONG length)
{
	char	*p = buffer;
	LONG	flags,nbrUsers,userID,headerLen;
	flags = *((LONG *) p);		p += sizeof(LONG);
	nbrUsers = *((LONG *) p);	p += sizeof(LONG);
	headerLen = sizeof(LONG)*2;
	/* -1 = everybody, 0 = everyone in room, > 0 means user list */
	if (nbrUsers < 0) {
		PostGlobalEvent(MSG_BLOWTHRU, refCon, p, length - headerLen);
	}
	else if (nbrUsers == 0L) {
		PostRoomEvent(cUser->user.roomID, MSG_BLOWTHRU, refCon, p, length - headerLen);
	}
	else {
		LONG			*userPtr;
		ServerUserPtr	tUser;
		headerLen += sizeof(LONG) * nbrUsers;
		userPtr = (LONG *) p;
		p += nbrUsers*sizeof(LONG);
		while (nbrUsers--) {
			userID = *((LONG *) userPtr);	userPtr += sizeof(LONG);
			tUser = GetServerUser(userID);
			if (tUser)
				PostUserEvent(tUser, MSG_BLOWTHRU, refCon, p, length - headerLen);
		}
	}
}

void ProcessMansionEvent(ServerUserPtr	cUser, LONG cmd, LONG msgRefCon, char *buffer, LONG len)
{
	unsigned LONG	curTicks,tDelta;
	/* 7/22/95 Flood Control & Activation */
	if (cUser == NULL)
		return;

	curTicks = time(NULL);
	tDelta = curTicks - cUser->lastActive;

	if (tDelta < FLOOD_INTERVAL) {				/* 1 second */
		FloodControl(cUser);
		if (cUser->flags & U_Kill)
			return;
	}
	else if (tDelta > FLOOD_RELEASE)			/* Clear Flood Events... */
		cUser->nbrFloodEvents = 0;

	cUser->lastActive = curTicks;
	cUser->nbrPings = 0;

	switch (cmd)
	{
	case MSG_GMSG:
		if (UserBadAssert(cUser,len > 0 && len < 256 && buffer[len-1] == 0))
			return;
		if (cUser->flags & U_SuperUser)				/* 6/14/95 */
			PostGlobalEvent(MSG_TALK,0,buffer,len);
		break;
	case MSG_SMSG:
		if (UserBadAssert(cUser,len > 0 && len < 256 && buffer[len-1] == 0))
			return;
		/* if (cUser->flags & U_SuperUser)				 */
		{
#if !unix
			SysBeep(1);									/* 7/26/95 Page Server Operator */
#endif
			 											/* 7/26/95 Put message in server log */
			WizGlobalMessage("Page from %s: %.200s",CvtToCString(cUser->user.name), (char *) buffer);
		}
		break;
	case MSG_RMSG:
		if (UserBadAssert(cUser,len > 0 && len < 256 && buffer[len-1] == 0))
			return;
		if (cUser) {
			PostRoomEvent(cUser->user.roomID,MSG_TALK,0,buffer,len);
		}
		break;
  	case MSG_LOGON:
		if (UserBadAssert(cUser,len >= sizeof(LogonInfoPtr)))
			return;
		if (cUser)
	    {
	    	EnterMansion((LogonInfoPtr) buffer, cUser);
		}
	    else
	    	LogMessage("Unknown user trying to logon.\r");
	    break;
	case MSG_LOGOFF:
		ScheduleUserKill(cUser, K_LoggedOff, 0);
		/* LogoffUser(msgRefCon, K_LoggedOff); */
		break;
	case MSG_USERMOVE:
		if (UserBadAssert(cUser,len == sizeof(Point)))
			return;
		UpdateUserPosition(cUser, (Point *) buffer);	/* 6/23/95 Update Arg Passing */
		break;
	case MSG_USERCOLOR:
		ChangeUserColor(cUser, *((short *) buffer));	/* 6/23 */
		break;
	case MSG_USERFACE:
		ChangeUserFace(cUser, *((short *) buffer));		/* 6/23 */
		break;
	case MSG_USERPROP:
		ChangeUserProp(cUser, (LONG *) buffer);		/* 6/23 */
		break;
	case MSG_USERDESC:
		ChangeUserDesc(cUser, (Ptr) buffer);		/* 6/8/95 new command, 6/23 */
		break;
	case MSG_USERNAME:
		if (cUser->flags & U_Pin)
			return;
		ChangeUserName(cUser, (StringPtr) buffer);
		break;
	case MSG_TALK:
		if (UserBadAssert(cUser,len > 0 && len < 256 && buffer[len-1] == 0))
			return;
		if (cUser)
      	{
			if (IllegalText(cUser, buffer, false)) {
				return;
			}
#if TAP
            TAPMessage(cUser,NULL,buffer,false);
#endif
			if (!GodMacro(cUser,buffer,false) && 
				!(cUser->flags & U_Gag) )
				PostRoomEvent(cUser->user.roomID,MSG_TALK,msgRefCon,buffer,strlen(buffer)+1);
		}
		break;
	case MSG_XTALK:
		if (UserBadAssert(cUser,len > 2 && len < 258 && len == (LONG) *((short *) buffer)))
			return;
		if (cUser) 
		{
			if (IllegalText(cUser, buffer, true)) {
				return;
			}
#if TAP
            TAPMessage(cUser,NULL,buffer,true);
#endif
			if (!GodMacro(cUser,buffer,true) && 
				!(cUser->flags & U_Gag))	/* 9/21/95 */
				PostRoomEvent(cUser->user.roomID,MSG_XTALK,msgRefCon,buffer,(LONG) *((short *) buffer));
		}
    	break;
	case MSG_WHISPER:
		if (UserBadAssert(cUser,len > 4 && len < 260 && buffer[len-1] == 0))
			return;
		{
			ServerUserPtr	tUser;
			LONG		targetID;
			targetID  = *((LONG *) &buffer[0]);
			tUser = GetServerUser(targetID);
			buffer += 4;

			if (IllegalText(cUser, buffer, false)) {
				return;
			}
#if TAP
            TAPMessage(cUser,tUser,buffer,false);
#endif
			if (cUser && tUser && 
			    !GodMacro(cUser,buffer,false) && 
				!ESPMessage(cUser,tUser,buffer,false) &&
			    !(cUser->flags & U_Gag) &&
			    cUser->user.roomID == tUser->user.roomID)
      		{
				if (ActionExists(tUser, cUser, UA_Mute) ||
					((tUser->flags & U_RejectPrivate) && !(cUser->flags & U_SuperUser))) {
					UserMessage(cUser,"%s is rejecting private messages",CvtToCString(tUser->user.name));
				}
				else {
					PostUserEvent(tUser,MSG_WHISPER,msgRefCon,buffer,strlen(buffer)+1);
					PostUserEvent(cUser,MSG_WHISPER,msgRefCon,buffer,strlen(buffer)+1);
				}
			}
		}
		break;
	case MSG_XWHISPER:
		if (UserBadAssert(cUser,len > 6 && len < 262 && 
			len-4 == (LONG) *((short *) &buffer[4]) ))
			return;
		{
			ServerUserPtr	tUser;
			LONG		targetID;
			targetID  = *((LONG *) &buffer[0]);
			tUser = GetServerUser(targetID);
			buffer += 4;

			if (IllegalText(cUser, buffer, true)) {
				return;
			}
#if TAP
            TAPMessage(cUser,tUser,buffer,true);
#endif

			if (cUser && tUser && 
			    !GodMacro(cUser,buffer,true) &&
				!ESPMessage(cUser,tUser,buffer,true) &&
				!(cUser->flags & U_Gag) &&
			    cUser->user.roomID == tUser->user.roomID) 
			{
				if (ActionExists(tUser, cUser, UA_Mute) ||
					((tUser->flags & U_RejectPrivate) && !(cUser->flags & U_SuperUser))) 
				{
					UserMessage(cUser,"%s is rejecting private messages",CvtToCString(tUser->user.name));
				}
				else {
					PostUserEvent(tUser,MSG_XWHISPER,msgRefCon,buffer,(LONG) *((short *) buffer));
					PostUserEvent(cUser,MSG_XWHISPER,msgRefCon,buffer,(LONG) *((short *) buffer));
				}
			}
		}
		break;
	case MSG_ROOMGOTO:
		if (UserBadAssert(cUser,len == 2))
			return;
		ChangeUserRoom(cUser, *((short *) buffer));	/* 6/23 */
		break;
	case MSG_PING:
		if (cUser) {
			PostUserEvent(cUser,MSG_PONG,msgRefCon,NULL,0L);
		}
		break;
	case MSG_PONG:
		break;
	case MSG_DOORLOCK:
		if (UserBadAssert(cUser,len == 4))
			return;
		LockDoor(cUser, (short *) buffer, true);
		break;
	case MSG_DOORUNLOCK:
		if (UserBadAssert(cUser,len == 4))
			return;
		LockDoor(cUser, (short *) buffer, false);
		break;
	case MSG_PICTMOVE:
		if (UserBadAssert(cUser,len == 8))
			return;
		UpdatePictureLocation(cUser, (short *) buffer);
		break;
	case MSG_SPOTMOVE:
		if (UserBadAssert(cUser,len == 8))
			return;
		UpdateSpotLocation(cUser, (short *) buffer);
		break;
	case MSG_SPOTSTATE:
		if (UserBadAssert(cUser,len == 6))
			return;
		UpdateSpotState(cUser, (short *) buffer);
		break;
	case MSG_ROOMNEW:
		if (UserBadAssert(cUser,len == 0))
			return;
		MakeNewRoom(cUser);
		break;
	case MSG_SPOTNEW:
		if (UserBadAssert(cUser,len == 0))
			return;
		MakeNewSpot(cUser);
		break;
	case MSG_SPOTDEL:
		if (UserBadAssert(cUser,len == 2))
			return;
		DeleteSpot(cUser,*((short *) buffer));
		break;
	case MSG_ROOMSETDESC:
		if (UserBadAssert(cUser,len >= sizeof(RoomRec)))
			return;
		SetRoomInfo(cUser, (RoomRecPtr) buffer);
		break;
	case MSG_DRAW:
		if (cUser->flags & U_Gag)
			return;
		if (UserBadAssert(cUser,len >= sizeof(DrawRecord)))
			return;
		AddDrawCommand(cUser, (DrawRecPtr) buffer);
		break;
	case MSG_ASSETREGI:
		if (UserBadAssert(cUser,len >= sizeof(AssetBlockHeader)))
			return;
		ReceiveAsset(cUser, buffer);
		break;
	case MSG_ASSETQUERY:
		if (UserBadAssert(cUser,len == 12))
			return;
		UserAssetQuery(cUser, (LONG *) buffer);		/* 6/23/95 changed to cUser */
		break;
	case MSG_PROPNEW:
		if (UserBadAssert(cUser,len == 12))
			return;
		if (cUser->flags & (U_Gag | U_Pin | U_PropGag))
			return;
		AddNewProp(cUser,(LONG *) buffer);			/* 6/23/95 ditto */
		break;
	case MSG_PROPMOVE:
		if (UserBadAssert(cUser,len == 8))
			return;
		if (cUser->flags & (U_Gag | U_Pin | U_PropGag))
			return;
		MoveLooseProp(cUser,(LONG *) buffer);		/* 6/23/95 ditto */
		break;
	case MSG_PROPDEL:
		if (UserBadAssert(cUser,len == 4))
			return;
		if (cUser->flags & (U_Gag | U_Pin | U_PropGag))
			return;
		DeleteLooseProp(cUser,(LONG *) buffer);		/* 6/23/95 */
		break;
	case MSG_SUPERUSER:		/* 1/27/95 JAB */
		if (UserBadAssert(cUser,len >= buffer[0]))
			return;
		ProcessSuperUser(cUser, (StringPtr) buffer);	/* 6/23/95 */
		break;
	case MSG_KILLUSER:		/* 1/27/95 JAB */
		if (UserBadAssert(cUser,len == 4))
			return;
		KillUser(cUser, *((LONG *) buffer));		/* 6/23/95 */
		break;
	case MSG_LISTOFALLUSERS:	/* 6/9/95 */
		if (UserBadAssert(cUser,len == 0))
			return;
		GenerateListOfAllUsers(cUser);				/* 6/23/95 */
		break;
	case MSG_LISTOFALLROOMS:	/* 6/9/95 */
		if (UserBadAssert(cUser,len == 0))
			return;
		GenerateListOfAllRooms(cUser);				/* 6/23/95 */
		break;
	case MSG_FILEQUERY:
		if (UserBadAssert(cUser,len >= buffer[0]))
			return;
		ProcessFileSend(cUser, (StringPtr) buffer);
		break;
	case MSG_BLOWTHRU:
		ProcessBlowthrough(cUser, msgRefCon, buffer, len);
		break;
	default:
		ScheduleUserKill(cUser, K_CommError, 0);
		break;
	}
}

/******************************************************************************************/

void PostRoomEvent(short roomID,unsigned LONG eventType,unsigned LONG refNum,Ptr bufferContents,LONG bufferLength)
{
	ServerUserPtr	curUser;
	Boolean			muteEvent = false;
	ServerUserPtr	srcUser = NULL;

	switch (eventType) {
	case MSG_TALK:
	case MSG_XTALK:
		muteEvent = true;
		break;
	}
	if (muteEvent) {
		srcUser = GetServerUser(refNum);
		if (srcUser == NULL)
			muteEvent = false;
	}
	for (curUser = gUserList; curUser; curUser = curUser->nextUser) {
		/* 8/4/95 Don't post global events to killed users */
		if (curUser->user.roomID == roomID && !(curUser->flags & U_Kill)) {
			if (muteEvent && 		
				ActionExists(curUser, srcUser, UA_Mute))
				continue;
			PostUserEvent(curUser,eventType, refNum, bufferContents, bufferLength);
		}
	}
}

/******************************************************************************************/

/* 6/25/95 New - post to neighbor's of this user in the same room, but
 * not this user (who has already updated his own status)
 */
void PostNeighborEvent(ServerUserPtr cUser,unsigned LONG eventType,unsigned LONG refNum,Ptr bufferContents,LONG bufferLength)
{
	ServerUserPtr	curUser;
	RoomID			roomID = cUser->user.roomID;
	Boolean			muteEvent = false;
	ServerUserPtr	srcUser = NULL;

	switch (eventType) {
	case MSG_TALK:
	case MSG_XTALK:
		muteEvent = true;
		break;
	}
	if (muteEvent) {
		srcUser = GetServerUser(refNum);
		if (srcUser == NULL)
			muteEvent = false;
	}

	for (curUser = gUserList; curUser; curUser = curUser->nextUser)
		/* 8/4/95 Don't post global events to killed users */
		if (curUser != cUser && curUser->user.roomID == roomID && 
			!(curUser->flags & U_Kill))
		{
			if (muteEvent && 		
				ActionExists(curUser, srcUser, UA_Mute))
					continue;
			PostUserEvent(curUser,eventType, refNum, bufferContents, bufferLength);
		}
}

/******************************************************************************************/

void PostGlobalEvent(unsigned LONG eventType,unsigned LONG refNum,Ptr bufferContents,LONG bufferLength)
{
	ServerUserPtr	curUser;

	for (curUser = gUserList; curUser; curUser = curUser->nextUser)
		/* 8/4/95 Don't post global events to killed users */
		if (!(curUser->flags & U_Kill))
			PostUserEvent(curUser,eventType, refNum, bufferContents, bufferLength);
}

/******************************************************************************************/

void PostGlobalEventShort(unsigned LONG eventType)
{
	PostGlobalEvent(eventType,0L,NULL,0L);
}

/******************************************************************************************/

void BeginSendGroup(ServerUserPtr cUser)
{
	cUser->groupFlag = true;
}

void EndSendGroup(ServerUserPtr cUser)
{
	if (cUser->groupLen)
		SendGroupBuffer(cUser);
	cUser->groupFlag = false;
}
