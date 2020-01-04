
/******************************************************************************************/
/* esr - Changes: * Removed DisplayPort() function.
//                * Removed #include <LoMem.h>
//                * Replaced all 'evnt' with MSG_xxxx.
//								* Changed Ticks to GetTicks() in NewUser()
//                * Rewrote loop in LogoffUser() to fix borland bug.
//                * Added DisconectUser(ServerUserPtr) to LogoffUser()
//								  to give low level network handlers a chance to
//							    to cleanup any memory/resources.
 */
#include "s-server.h"
#include "s-secure.h"

/******************************************************************************************/

ServerUserPtr gUserList,gToBeKilled;
LONG          gNbrUsers;

/******************************************************************************************/
unsigned long gSessionIDCounter=1;

void RecycleSessionIDs()
{
	gSessionIDCounter = 1;
}

/* 12/30/95 modified to take no arguments - id numbers and default name
 *  handled internally now
 */
ServerUserPtr NewUser(void /* PersonID userID,StringPtr name */)
{
	ServerUserPtr	newUser;
	unsigned long	userID;

	do {
		/* 4/9/96 autorecycle session IDs */
		/* use gNbrUsers to make sure we don't get into an infinite loop */


		/* 7/27/96 Make sure session numbers don't go
		   higher than MaxUserID which is used to allocate
		   gUserArray */
		if ( gSessionIDCounter >= MaxUserID ||
			(gSessionIDCounter >= gPrefs.recycleLimit &&
			 gPrefs.recycleLimit > gNbrUsers+1))
			RecycleSessionIDs();
		userID = gSessionIDCounter++;
	} while (GetServerUser(userID) != NULL);

	newUser = (ServerUserPtr)NewPtrClear(sizeof(ServerUserRec));
	if (newUser == NULL)
    {
		SysBeep(1);
		LogMessage("Out of Memory! (NewUser)\r");
		return NULL;
	}
	newUser->nextUser       = gUserList;
	gUserList               = newUser;
	newUser->user.userID    = userID;
	newUser->user.faceNbr   = 5;
	newUser->user.nbrProps  = 0;	/* 6/8/95 */
	/* esr portFix - Tick -> GetTicks(); */
	newUser->lastActive     = time(NULL);
	newUser->signonTime     = newUser->lastActive;
	newUser->nbrPings       = 0;

	/* 7/27/96 Add user to gUserArray */
	gUserArray[userID] = newUser;

	sprintf((char *) newUser->user.name,"Guest %ld",(long) userID);
	CtoPstr((char *) newUser->user.name);

	/* BlockMove(name,newUser->user.name,name[0]+1); */
	newUser->user.colorNbr = MyRandom(NbrColors);

#if !BISERVER
	newUser->groupBuffer = NewPtr(InitLenGroupBuffer);		/* 6/24/95 */
	newUser->groupAlloc = InitLenGroupBuffer;
#else
	newUser->groupBuffer = NULL;
	newUser->groupAlloc = 0;
#endif
	newUser->groupLen = 0L;
	++gNbrUsers;
	RebuildUserDisplay();	/* 7/26/95 Local Function */
	return newUser;
}

/* 5/8/95 JAB */
void LogoffAllUsers()
{
	while (gUserList)
		LogoffUser(gUserList->user.userID, K_ServerDown);
#if BISERVER
	GlobalFrontEndEvent(0,0,bi_serverdown,NULL,0);
#endif
}

int CountWizards(void);

int CountWizards(void)
{
	ServerUserPtr	curUser;
	int		sum=0;

	for (curUser = gUserList; curUser; curUser = curUser->nextUser)
	{
		if (curUser->flags & U_SuperUser)
			++sum;
	}
	return sum;
}


/******************************************************************************************/

/* Note: Kill - means killed by another player (should invoke death penalty)
 		Terminate means killed by server (if for flooding, should invoke flood penalty)
 		Logged off - means player voluntarily logged off
 
 */
static 	char *whyMsg[] = {	/* 7/22 corresponds to K_ flags in S-SERVER.H */
		"terminated for unknown reasons",
		"logged off",
		"terminated due to comm error",
		"terminated because of flooding",
		"killed by player",
		"terminated - server going down",
		"terminated due to inactivity",
		"terminated by server operator",
		"terminated - server is full",
		"turned away - invalid serial number",
		"turned away - duplicate user",
		"turned away - death penalty active",
		"turned away - banished",
		"terminated - banished by sysop",
		"terminated - members only"};

void LogoffUser(PersonID	userID, short why)
{
	/* esr portFix - Rewrote loop for borland bug. */
	ServerUserPtr	curUser  = gUserList;
	ServerUserPtr prevUser = NULL;
	ServerRoomPtr	cRoom    = 0;

	while (curUser) {
		if (curUser->user.userID == userID)
			break;
		else
		{
			prevUser = curUser;
			curUser  = curUser->nextUser;
		}
	}

	if (curUser)
	{
		char	numBuf[64];
		char	keyBuf[64]="";

		UpdateUserDatabase(curUser, why);	/* 8/2/95 */

		if (why != K_CommError && why != K_LoggedOff && !(curUser->flags & U_CommError))
			PostUserEvent(curUser,MSG_SERVERDOWN, why, NULL, 0L);	/* 7/22 */
		cRoom = GetRoom(curUser->user.roomID);
		if (curUser == gUserList)
			gUserList = curUser->nextUser;
		else if (prevUser)
			prevUser->nextUser = curUser->nextUser;
		--gNbrUsers;

		/* 7/27/96 Remove from gUserArray, which is used
		   for table lookups */
		gUserArray[userID] = NULL;

		if (why == K_InvalidSerialNumber) {
			char	addrBuf[256],numBuf[64];
			ConvertNetAddressToString(curUser,addrBuf);
			ConvertNetAddressToNumericString(curUser,numBuf);
		    TimeLogMessage("%s [%s] %s ",CvtToCString(curUser->user.name),
		    							addrBuf,
		    							whyMsg[why]);

			if (strcmp(addrBuf,numBuf) == 0) {
				GodGlobalMessage("Page from System: Invalid Ser# Attempt from %s",addrBuf);
			}
			else {
				GodGlobalMessage("Page from System: Invalid Ser# Attempt from %s (%s)",addrBuf,numBuf);
			}
		}
		else
		    TimeLogMessage("%s %s ",CvtToCString(curUser->user.name),
		    							whyMsg[why]);
		/* 9/10/96 addition for wizard notification */
		if (curUser->flags & U_SuperUser) {
			int	cnt;
			cnt = CountWizards();
			if (cnt)
				WizGlobalMessage("Page from System: %s has logged off, %d %s",
					CvtToCString(curUser->user.name),
					cnt,cnt == 1? "wizard remains (you)" : "wizards remain (including you)");
		}

#if unix
		/* 1/17/97 JAB */
		if (gPrefs.serverOptions & SO_SaveSessionKeys) {
			void SeedToWizKey(ServerUserPtr cUser, char *seedStr);
			SeedToWizKey(curUser,keyBuf);
		}
#endif
		ConvertNetAddressToNumericString(curUser,numBuf);

		LogMessage("[%ld seconds elapsed] (%s) [%ld Users]%s\r",
					(long) time(NULL) - curUser->signonTime,
					numBuf,
					gNbrUsers,keyBuf);

		/* esr portFix - added to allow the network handlers a chance to cleanup. */
		/* DisconnectUser(curUser); */

		/* esr portFix - 'evnt' -> MSG_xxxx */
		/* 8/4/95 - don't notify if user is being rejected */
		if (curUser->user.roomID) {
			/* 7/26/96 Limit amount of global traffic */
			PostGlobalUserStatus(MSG_LOGOFF, curUser, userID, curUser->user.roomID);
			/* PostGlobalEvent(MSG_LOGOFF,userID,(Ptr)&gNbrUsers,sizeof(LONG)); */
		}

		/* DisposePtr(curUser->groupBuffer);	// 6/24/95 */
		/* DisposePtr((Ptr) curUser); */
		curUser->nextUser = gToBeKilled;
		gToBeKilled = curUser;
	}
	if (cRoom)
		RemovePersonFromRoom(cRoom,userID);
	if (curUser->lastOwnedRoom)
		ScheduleRoomDeletionCheck();
	RebuildUserDisplay();	/* 7/26/95 Local Function for updating user list window */
}

/******************************************************************************************/

LONG	UserRank(ServerUserPtr cUser)
{
	if (cUser->flags & U_Guest)
		return 0;
	else if (cUser->flags & U_SuperUser) {
		if (cUser->flags & U_God)
			return 3;
		else
			return 2;
	}
	else
		return 1;
}

ServerUserPtr	GetServerUser(LONG id)
{
/* 7/27/96 optimized to do array lookup
	ServerUserPtr	cUser;

	for (cUser = gUserList; cUser; cUser = cUser->nextUser)
		if (cUser->user.userID == id)
			return cUser;
	return NULL;
*/
	if (id <= 0 || id > MaxUserID)
		return NULL;
	else
		return gUserArray[id];
}

ServerUserPtr	GetServerUserByCounter(unsigned LONG ctr)	/* 5/8/96 */
{
	ServerUserPtr	cUser;
	for (cUser = gUserList; cUser; cUser = cUser->nextUser)
		if (cUser->counter == ctr)
			return cUser;
	return NULL;
}

ServerUserPtr	GetServerUserByName(StringPtr name)
{
	ServerUserPtr	cUser;
	for (cUser = gUserList; cUser; cUser = cUser->nextUser)
		if (EqualPString(cUser->user.name,name,false))
			return cUser;
	return NULL;
}

/******************************************************************************************/

void ChangeUserFace(ServerUserPtr	rUser, short faceNbr)
{
	if (rUser == NULL)
		return;
	/* ActiveUser(rUser); */
	if (UserBadAssert(rUser,faceNbr >= 0 && faceNbr < 16))
		return;
	rUser->user.faceNbr = faceNbr;
	/* esr portFix - 'evnt' -> MSG_xxxx */
	/* 6/25/95 New Post to neighbors of this user, but not this user */
	PostNeighborEvent(rUser,MSG_USERFACE, rUser->user.userID, (Ptr) &rUser->user.faceNbr, sizeof(short));
}

/******************************************************************************************/

void ChangeUserProp(ServerUserPtr	rUser, LONG *p)		/* 6/8/95 JAB modified to use AssetSpec List */
{
	LONG			nbrProps,i;
	AssetSpec		*as;
	if (rUser == NULL)
		return;
	/* ActiveUser(rUser); */
	nbrProps = *p;
	if (UserBadAssert(rUser,nbrProps >= 0 && nbrProps <= MaxUserProps))
		return;
	if (rUser->flags & U_Pin)
		return;
	as = (AssetSpec *) (p+1);
	rUser->user.nbrProps = nbrProps;
	BlockMove((Ptr)as,(Ptr)&rUser->user.propSpec[0],nbrProps*sizeof(AssetSpec));
	for (i = 0; i < nbrProps; ++i,++as)	{
		if ((rUser->flags & (U_Guest | U_PropGag)) && (as->id < MinReservedProp || as->id > MaxReservedProp)) {
			UserMessage(rUser, "Sorry, Members Only");
			rUser->user.nbrProps = 0;
			return;
		}
		CheckForProp(rUser,as->id,as->crc);	/* Bugfix 6/21/95 */
	}
	/* esr portFix - 'evnt' -> MSG_xxxx */
	/* 6/25/95 New Post to neighbors of this user, but not this user */
	PostNeighborEvent(rUser,MSG_USERPROP, rUser->user.userID, (Ptr) p, sizeof(LONG)+nbrProps*sizeof(AssetSpec));
}

void ChangeUserDesc(ServerUserPtr	rUser, Ptr p)		/* 6/8/95 JAB new function */
{
	LONG			nbrProps,i;
	short			faceNbr,colorNbr;
	AssetSpec		*as;
	if (rUser == NULL)
		return;
	if (rUser->flags & U_Pin)
		return;
	/* ActiveUser(rUser); */
	faceNbr = *((short *) p);
	colorNbr = *((short *) (p+2));
	nbrProps = *((LONG *) (p+4));

	
	if (UserBadAssert(rUser,nbrProps >= 0 && nbrProps <= MaxUserProps))
		return;
	if (UserBadAssert(rUser,colorNbr >= 0 && colorNbr < NbrColors))
		return;
	if (UserBadAssert(rUser,faceNbr >= 0 && faceNbr < 16))
		return;

	/* if ((rUser->flags & U_Guest) && colorNbr != GuestColor) {
	 *	UserMessage(rUser, "Sorry, Members Only");
	 *	return;
	 * }
     */

	as = (AssetSpec *) (p+8);
	rUser->user.faceNbr = faceNbr;
	if (rUser->flags & U_Guest)	{		
		/* 3/15/95 */
		rUser->user.colorNbr = GuestColor;
		/* 8/14/96 security fix - don't transmit color to other users */
		*((short *) (p+2)) = GuestColor;
	}
	else
		rUser->user.colorNbr = colorNbr;
	rUser->user.nbrProps = nbrProps;
	BlockMove((Ptr)as,(Ptr)&rUser->user.propSpec[0],nbrProps*sizeof(AssetSpec));
	for (i = 0; i < nbrProps; ++i,++as) {				/* 6/21/95 BugFix */
		if ((rUser->flags & (U_Guest | U_PropGag)) && (as->id < MinReservedProp || as->id > MaxReservedProp)) {
			UserMessage(rUser, "Sorry, Members Only");
			rUser->user.nbrProps = 0;
			return;
		}
		CheckForProp(rUser,as->id,as->crc);		/* 6/21/95 Bugfix */
	}
	/* 6/25/95 New Post to neighbors of this user, but not this user */
	PostNeighborEvent(rUser,MSG_USERDESC, rUser->user.userID, (Ptr) p, sizeof(short)*2+sizeof(LONG)+nbrProps*sizeof(AssetSpec));
}

/******************************************************************************************/

void ChangeUserColor(ServerUserPtr	cUser, short colorNbr)
{
	if (cUser == NULL)
		return;
	/* ActiveUser(cUser); */
	/* if ((cUser->flags & U_Guest) && colorNbr != GuestColor) {
	 *	UserMessage(cUser, "Sorry, Members Only");
	 *	return;
	 * }
	 */
	if (UserBadAssert(cUser,colorNbr >= 0 && colorNbr < NbrColors))
		return;

	if (cUser->flags & U_Guest)
		cUser->user.colorNbr = GuestColor;
	else
		cUser->user.colorNbr = colorNbr;
	/* esr portFix - 'evnt' -> MSG_xxxx */
	/* 6/25/95 New Post to neighbors of this user, but not this user */
	PostNeighborEvent(cUser, MSG_USERCOLOR, cUser->user.userID, (Ptr) &cUser->user.colorNbr, sizeof(short));
}

/******************************************************************************************/

Boolean IsLegalName(ServerUserPtr	rUser, StringPtr uName)
{
	Boolean			alphaFlag = false;
	short			i;
	if (rUser->flags & U_Guest)
		return false;

	/* Name must be at least 1 letter long,and less than 32 */
	if (uName[0] > 31 || uName[0] < 1)
		return false;

	for (i = 1; i <= uName[0]; ++i) {
		/* Name may contain no control characters */
		if (uName[i] < ' ')
			return false;

		/* 9/6/96 additional illegal checks */
		if (uName[i] == 127 || /* delete */
		    (uName[i] == '*' && !(rUser->flags & U_SuperUser)) ||
		    uName[i] == 0xCA || /* invisible space on mac */
		    uName[i] >= 0xDA)   /* unprintable on mac */
			return false;
		/* Name must contain at least one alphanumeric */
		if (isalnum(uName[i])) {
			alphaFlag = true;
			/* break; 9/6/96 - break screws us UP for illegal checks */
		}
	}
	if (!alphaFlag)
		return false;

	/* first character may not be a space */
	if (isspace(uName[1]))
		return false;
	/* first character may not be a hyphen 4/15/96 */
	if (uName[1] == '-')
		return false;
	/* JAB 6/21/96 first character may not be high-order ascii */
	if (uName[1] > '~')
		return false;

	/* last character may not be a space */
	if (isspace(uName[uName[0]]))
		return false;

	/* only wizards may use asterisk in first spot */
	if (uName[1] == '*' && !(rUser->flags & U_SuperUser))
		return false;
	/* 1/14/97 JAB only demo-users may use ':' in first spot */
	if (uName[1] == ':' && !NewbieUserSerialNumber(rUser->crc,rUser->counter))
		return false;

	/* "Guest" is not permitted */
	if (strncmp((char *) &uName[1],"Guest",5) == 0)
		return false;
	/* 6/21/96 "System" is not permitted */
	if (strncmp((char *) &uName[1],"System",6) == 0)
		return false;
	
	return true;
}

void ChangeUserName(ServerUserPtr	rUser, StringPtr uName)
{
	ServerUserPtr	aUser;
	char			*sorryMsg = "Sorry that name is taken";
	char			*illegalMsg = "Sorry that name is illegal";
	int				illegalFlag=0;
	Str63			tName;

	if (rUser == NULL)
		return;
	if (rUser->flags & U_Guest)
		return;

	/* 1/14/97 JAB Modify name if needed */
	BlockMove(uName,tName,uName[0]+1);
	if (tName[0] && tName[1] == '*' && !(rUser->flags & U_SuperUser)) {
		BlockMove(&tName[2],&tName[1],tName[0]-1);
		tName[0]--;
	}
	/* 1/14/97 JAB - fix demo member names */
	if (NewbieUserSerialNumber(rUser->crc,rUser->counter)) {
		/* make sure name begins with ":" */
		if (tName[0] && tName[1] != ':') {
			BlockMove(&tName[1],&tName[2],tName[0]);
			tName[1] = ':';
			tName[0]++;

		}
	}
	else {
		/* make sure name does not begin with ":" if the user isn't a wizard */
		if (!(rUser->flags & U_SuperUser)) {
			if (tName[0] && tName[1] == ':') {
				BlockMove(&tName[2],&tName[1],tName[0]-1);
				tName[0]--;
			}
		}
	}

	/* ActiveUser(rUser); */
	if (tName[0] > 31)	/* 9/20/95 */
		tName[0] = 31;

	aUser = GetServerUserByName(tName);
	if (aUser && aUser != rUser) {
		UserMessage(rUser,"%s",sorryMsg);
		PostUserEvent(rUser, MSG_USERNAME, rUser->user.userID, (Ptr) rUser->user.name, rUser->user.name[0]+1);
		LogMessage("%s tried to change name to %s\r",
					CvtToCString(rUser->user.name),
					CvtToCString(tName));
		return;
	}

	illegalFlag = !IsLegalName(rUser, tName);

	if (illegalFlag && !(rUser->flags & U_SuperUser)) {
		LogMessage("%s tried to change name illegally\r",
					CvtToCString(rUser->user.name));
		UserMessage(rUser, "%s",illegalMsg);
		PostUserEvent(rUser, MSG_USERNAME, rUser->user.userID, (Ptr) rUser->user.name, rUser->user.name[0]+1);
		return;
	}

	LogMessage("%s changed name to %s\r",
					CvtToCString(rUser->user.name),
					CvtToCString(tName));
	BlockMove(tName,rUser->user.name,tName[0]+1);
	/* esr portFix - 'evnt' -> MSG_xxxx */
	PostRoomEvent(rUser->user.roomID,MSG_USERNAME, rUser->user.userID, (Ptr) rUser->user.name, rUser->user.name[0]+1);
	UpdateUserDisplay(rUser->user.userID);	/* 7/26/95 Local Function for updating user list window */
}

/******************************************************************************************/

/* 5/10/96 - added ForcedEntry flag when navigation is critical, e.g.
   punting people from deleted rooms - ignores occupancy restrictions and such 
 */
void ChangeUserRoom(ServerUserPtr	rUser, short newRoom, Boolean forcedEntry)
{
	ServerRoomPtr	nRoom,oRoom;
	Point        	 p;
	Boolean		    gotDoor;
	short		      n;
	HotspotPtr		hs;
	short			roomOccupancy;

	if (rUser == NULL)
		return;
	/* ActiveUser(rUser); */
	nRoom = GetRoom(newRoom);
	if (nRoom == NULL) {
        PostUserEvent(rUser, MSG_NAVERROR, (LONG) SE_RoomUnknown, NULL, 0L);
		return;
	}

	/* the forcedEntry flag can be used to bypass most access restrictions,
	 * it is used for getting people out of deleted rooms - they are usually
	 * sent to the front gate, and we need to allow them there, even if
	 * the gate is full
	 */

	/* fail safe - there is a hardcoded room limit, regardless of forcedentry flag */
	if (nRoom->room.nbrPeople >= MaxPeoplePerRoom) {
        PostUserEvent(rUser, MSG_NAVERROR, (LONG) SE_RoomFull, NULL, 0L);
		ServerTextToUser(rUser, "Sorry, the room is full");
	}

	if (!forcedEntry) {
		/* Deny Access if room is closed and user is not a god. */
		/* */
		if ((nRoom->room.roomFlags & RF_Closed) > 0 &&
			(nRoom->room.roomFlags & RF_Private) > 0 &&
			!(rUser->flags & U_God) && !IsRoomOwner(rUser,nRoom)) {
			/* Notify user */
	        PostUserEvent(rUser, MSG_NAVERROR, (LONG) SE_RoomClosed, NULL, 0L);
	        /* phase this out */
			ServerTextToUser(rUser, "Sorry, the room is closed");
			return;
		}
	
		/* 1/27/95 */
		roomOccupancy = nRoom->maxOccupancy;
		if (roomOccupancy == 0)
			roomOccupancy = gPrefs.roomOccupancy;
	
		/* Deny Access if room is full 7/29/95 */
		/* */
		if ((nRoom->room.nbrPeople >= roomOccupancy ||
		    nRoom->room.nbrPeople >= MaxPeoplePerRoom) &&
			!(rUser->flags & U_SuperUser)) {
			/* Notify user */
	        PostUserEvent(rUser, MSG_NAVERROR, (LONG) SE_RoomFull, NULL, 0L);
			/* Notify user - phase this out */
			ServerTextToUser(rUser, "Sorry, the room is full");
			return;
		}
	
		/* Deny Access if NoGuest flag and user is a guest */
		if ((nRoom->room.roomFlags & RF_NoGuests) &&
			(rUser->flags & U_Guest)) {
	       PostUserEvent(rUser, MSG_NAVERROR, (LONG) SE_RoomClosed, NULL, 0L);
		   ServerTextToUser(rUser, "Sorry, guests aren't allowed in that room");
		   return;
		}

		/* Deny Access if room has met guest limit and user is a guest */
		if (nRoom->maxGuests && (rUser->flags & U_Guest) &&
			CountRoomGuests(nRoom) >= nRoom->maxGuests) {
	       PostUserEvent(rUser, MSG_NAVERROR, (LONG) SE_RoomFull, NULL, 0L);
		   ServerTextToUser(rUser, "Sorry, no more guests are allowed in that room");
		   return;
		}
	
		/* Deny Access if WizardsOnly flag and user is not a wizard */
		if ((nRoom->room.roomFlags & RF_WizardsOnly) &&
			!(rUser->flags & U_SuperUser)) {
	       PostUserEvent(rUser, MSG_NAVERROR, (LONG) SE_RoomClosed, NULL, 0L);
		   ServerTextToUser(rUser, "Sorry, members aren't allowed in that room");
		   return;
		}
	
		/* Deny Access if User is pinned, no notification */
		if ((rUser->flags & U_Pin)) {
	       PostUserEvent(rUser, MSG_NAVERROR, (LONG) SE_RoomClosed, NULL, 0L);
		   ServerTextToUser(rUser, "Sorry, you've been pinned.");
		   return;
		}
		if (nRoom->memberOwner) {
			ServerUserPtr	oUser;
			/* 5/8/96 Deny access if member room and password doesn't match */
			if (nRoom->roomPassword[0] && strcmp(nRoom->roomPassword,rUser->navPassword) &&
				!IsRoomOwner(rUser,nRoom)) {
				PostUserEvent(rUser, MSG_NAVERROR, (LONG) SE_RoomClosed, NULL, 0L);
				ServerTextToUser(rUser, "That room has a password.  Use 'password <text> to go there");
				rUser->lastPasswordRoom = nRoom->room.roomID;
				return;
			}
			/* 5/8/96 Deny access if member room and you've been kicked */
			oUser = GetServerUserByCounter(nRoom->memberOwner);
			if (oUser && ActionExists(oUser, rUser, UA_Kick) &&
				!IsRoomOwner(rUser,nRoom)) {
				PostUserEvent(rUser, MSG_NAVERROR, (LONG) SE_RoomClosed, NULL, 0L);
				ServerTextToUser(rUser, "Sorry - you've been kicked from that room");
				return;
			}
		}
	}
	
	oRoom = GetRoom(rUser->user.roomID);
	if (oRoom == NULL) {
        PostUserEvent(rUser, MSG_NAVERROR, (LONG) SE_InternalError, NULL, 0L);
		return;
	}
	RemovePersonFromRoom(oRoom, rUser->user.userID);
	rUser->user.roomID = 0;
	/* esr portFix - 'evnt' -> MSG_xxxx */
	BeginSendGroup(rUser);
	PostRoomEvent(oRoom->room.roomID,MSG_USEREXIT,rUser->user.userID,NULL,0L);
	SendRoomToUser(nRoom, rUser);
	gotDoor = false;
	hs = (HotspotPtr) &nRoom->room.varBuf[nRoom->room.hotspotOfst];
	for (n = 0; n < nRoom->room.nbrHotspots; ++n) {
		if (hs[n].dest == oRoom->room.roomID) {
			gotDoor = true;
			break;
		}
	}
	if (gotDoor) {
		hs = &hs[n];
		p = hs->loc;
	}
	else {
		p.h = RoomWidth - rUser->user.roomPos.h;
		p.v = RoomHeight - rUser->user.roomPos.v;
	}
	p.h += MyRandom(32)-16;
	p.v += MyRandom(32)-16;

	/* 5/10/96 - clip entry position */
	if (p.h < 22)	p.h = 22;
	if (p.h >= 490)	p.h = 490;
	if (p.v < 22)	p.v = 22;
	if (p.v >= 362)	p.v = 362;

	AddUserToRoom(nRoom, rUser, p);
	EndSendGroup(rUser);
}

/******************************************************************************************/

void UpdateUserPosition(ServerUserPtr cUser, Point *newpos)
{
	Point		      tmp;

	if (cUser)
    {
		/* ActiveUser(cUser); */
		if (!(cUser->flags & U_Pin)) {
			cUser->user.roomPos = *newpos;
			tmp = *newpos;
#if PALACE_3D
			PostRoomEvent(cUser->user.roomID,MSG_USERMOVE,cUser->user.userID,(Ptr)&tmp,sizeof(Point));
#else
			PostNeighborEvent(cUser,MSG_USERMOVE,cUser->user.userID,(Ptr)&tmp,sizeof(Point));
#endif
		}
		else {
			tmp = cUser->user.roomPos;
			PostUserEvent(cUser,MSG_USERMOVE,cUser->user.userID,(Ptr)&tmp,sizeof(Point));
		}
	}
}

/******************************************************************************************/

/* 6/27/95 Added GOD access */
/*			Disabling wizard access if password is wrong. */
/* 6/21/96 Added auto-warning for failed password attempts */

void ProcessSuperUser(ServerUserPtr	cUser, StringPtr str)
{
	short			    userFlags;
	if (cUser == NULL)
		return;

	if (!(gPrefs.permissions & PM_AllowWizards))		/* 7/22/95 */
		return;

	/* 8/2/95 Guests can't be wizards */
	if ((cUser->flags & U_Guest) > 0)	
		return;

	/* ActiveUser(cUser); */
	/* 6/14/95 - if no wizard password, don't allow superuser */
	if (gPrefs.wizardPassword[0] && EqualPString(str,gPrefs.wizardPassword,true))
    {
		UserMessage(cUser, "You now have wizard privileges.");
		TimeLogMessage("%s made wizard\r",CvtToCString(cUser->user.name));
		WizGlobalMessage("Page from System: %s made wizard",CvtToCString(cUser->user.name));
		cUser->flags |= U_SuperUser;	/* 6/14/95 */
		cUser->flags &= ~U_God;
		cUser->nbrFailedPasswordAttempts = 0;
	}
	else if (gPrefs.godPassword[0] && EqualPString(str,gPrefs.godPassword,true))
    {
		UserMessage(cUser, "You now have god privileges.");
		TimeLogMessage("%s made god\r",CvtToCString(cUser->user.name));
		WizGlobalMessage("Page from System: %s made god",CvtToCString(cUser->user.name));
		cUser->flags |= (U_SuperUser | U_God);
		cUser->nbrFailedPasswordAttempts = 0;
	}
	else {
		TimeLogMessage("%s denied wizard access\r",CvtToCString(cUser->user.name));
		cUser->flags &= ~(U_SuperUser | U_God);
		if (gPrefs.serverOptions & SO_PasswordSecurity) {
			++cUser->nbrFailedPasswordAttempts;
			if (cUser->nbrFailedPasswordAttempts < 3) {
				UserMessage(cUser, "That password is invalid.  If you continue to try invalid passwords you will be automatically kicked off by the system.");
				WizGlobalMessage("Page from System: %s denied wizard access (and auto-warned by system)",CvtToCString(cUser->user.name));
			}
			else {
				UserMessage(cUser, "That password is invalid.  You are being automatically kicked off by the system.");
				WizGlobalMessage("Page from System: %s denied wizard access (and auto-kicked by system)",CvtToCString(cUser->user.name));
				ScheduleUserKill(cUser, K_KilledBySysop, 1500);
			}
		}
		else {
			UserMessage(cUser, "That password is invalid.");
			WizGlobalMessage("Page from System: %s denied wizard access",CvtToCString(cUser->user.name));
		}
	}
	userFlags = cUser->flags;
#if BISERVER
	{
		LONG	longFlags;
		longFlags = userFlags;
		SendFrontEndEvent((BiFrontEndPtr) cUser->frontEndPtr,
							bi_userflags,
							cUser->feIPPort,
							cUser->feIPAddr,
							sizeof(LONG),
							(unsigned char *) &longFlags);
	}
#endif
	PostUserEvent(cUser,MSG_USERSTATUS,cUser->user.userID,(Ptr)&userFlags,sizeof(short));
	UpdateUserDisplay(cUser->user.userID);	/* 7/26/95 */
}

/******************************************************************************************/
/* One user kills another */
void KillUser(ServerUserPtr cUser, PersonID targetID, LONG minutes, Boolean trackFlag)
{
	char		addrBuf[256],numBuf[64];
	char		*desc;
    ServerUserPtr tUser = GetServerUser(targetID);

	desc = (trackFlag? "tracked" : "killed");

	if (cUser == NULL || tUser == NULL)
		return;
	if (!(gPrefs.permissions & PM_PlayersMayKill) &&
		!(cUser->flags & U_SuperUser))
		return;
	if (!(gPrefs.permissions & PM_WizardsMayKill) &&
		!(cUser->flags & U_God))
		return;
	/* 1/30/95 - simplification - users of lower rank can't kill higher ranks */
	if (UserRank(cUser) < UserRank(tUser))
		return;
	/* 8/2/95 Guests can't kill */
	if ((cUser->flags & U_Guest) > 0)
		return;

	ConvertNetAddressToString(tUser,addrBuf);
	ConvertNetAddressToNumericString(tUser,numBuf);
	TimeLogMessage("%s [%s (%s)] %s by %s\r",CvtToCString(tUser->user.name),
						addrBuf, numBuf, desc, CvtToCString(cUser->user.name));
	if (strcmp(addrBuf,numBuf) == 0) {
		WizGlobalMessage("Page from System: %s [%s] %s by %s",
						CvtToCString(tUser->user.name),addrBuf, desc,
						CvtToCString(cUser->user.name));
	}
	else {
		WizGlobalMessage("Page from System: %s [%s (%s)] %s by %s",
						CvtToCString(tUser->user.name),addrBuf, numBuf, desc,
						CvtToCString(cUser->user.name));
	}
	/* 4/15/96 */
	BanUserByIP(cUser,tUser,trackFlag? K_Unknown : K_KilledByPlayer, 
				minutes, trackFlag? BR_Tracking : 0);
	if (!trackFlag)
		ScheduleUserKill(tUser, K_KilledByPlayer, minutes);
	if (!(targetID == cUser->user.userID) &&
		!(cUser->flags & U_SuperUser))		/* not killed self and not wizard */
		UserMessage(cUser,"%s [%s] %s",CvtToCString(tUser->user.name),addrBuf,desc);
}



void ServerTextToUser(ServerUserPtr cUser, char *str)
{
	PostUserEvent(cUser,MSG_TALK,0,str,strlen(str)+1);
}

