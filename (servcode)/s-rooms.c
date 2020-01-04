/******************************************************************************************/

#include "s-server.h"
#include "s-secure.h"	/* 1/14/97 JAB */
#include "s-timout.h"	/* 1/14/97 JAB */

/******************************************************************************************/

ServerRoomPtr	gRoomList;
short			    gNbrRooms;
short			    gEntrance;

/******************************************************************************************/

ServerRoomPtr	GetRoom(short id)
{
	ServerRoomPtr	cRoom;
	for (cRoom = gRoomList; cRoom; cRoom = cRoom->nextRoom) {
		if (cRoom->room.roomID == id)
			return cRoom;
	}
	return NULL;
}

short CountRoomGuests(ServerRoomPtr cRoom)
{
	ServerUserPtr	curUser;
	short			sum = 0;
	for (curUser = gUserList; curUser; curUser = curUser->nextUser) {
		if ((curUser->flags & U_Guest) && curUser->user.roomID == cRoom->room.roomID)
			++sum;
	}
	return sum;
}

/* 8/28/96 New function to change order of a room in
  room list -N means move UP in list, +N means move
  DOWN in list */
void PushRoom(short id, int n)
{
	if (gNbrRooms == 1)
		return;
	if (n < 0) 		/* push up */
	{
		ServerRoomPtr	prevRoom,prev1Room,cRoom;
		n = -n;
		while (n-- && gRoomList->room.roomID != id) {
			prevRoom = NULL;
			prev1Room = NULL;
			for (cRoom = gRoomList; cRoom; cRoom = cRoom->nextRoom) {
				if (cRoom->room.roomID == id) {
					if (prev1Room)
						prev1Room->nextRoom = cRoom;
					else
						gRoomList = cRoom;
					if (prevRoom)
						prevRoom->nextRoom = cRoom->nextRoom;					
					cRoom->nextRoom = prevRoom;
					break;
				}
				prev1Room = prevRoom;
				prevRoom = cRoom;
			}
		}
	}
	else 			/* push down */
	{
		ServerRoomPtr	nextRoom,prevRoom,cRoom;
		while (n--) {
			prevRoom = NULL;
			for (cRoom = gRoomList; cRoom; cRoom = cRoom->nextRoom) {
				if (cRoom->room.roomID == id) {
					if (cRoom->nextRoom == NULL)
						break;
					if (prevRoom)
						prevRoom->nextRoom = cRoom->nextRoom;
					else
						gRoomList = cRoom->nextRoom;
					nextRoom = cRoom->nextRoom;
					cRoom->nextRoom = cRoom->nextRoom->nextRoom;
					nextRoom->nextRoom = cRoom;
					break;
				}
				prevRoom = cRoom;
			}
			if (cRoom && cRoom->nextRoom == NULL)
				break;
		}
	}	
}

void DeleteRoom(short roomNumber)
{
	ServerRoomPtr	cRoom,lastRoom = NULL;
	ServerUserPtr	who;
	short			i;

	if (gNbrRooms == 1 || roomNumber == gEntrance)
		return;
	for (cRoom = gRoomList; cRoom; cRoom = cRoom->nextRoom) {
		if (cRoom->room.roomID == roomNumber) {
			TimeLogMessage("Room %d deleted\r",(int) cRoom->room.roomID);
			/* Move every user in this room to the entrance... */
			/* 5/10/96 made loop backwards to prevent bugs */
			for (i = cRoom->room.nbrPeople-1; i >= 0; --i) {
				who = GetServerUser(cRoom->personID[i]);
				if (who)
		        {
		        	ChangeUserRoom(who,gEntrance,true);
				}
			}
			if (lastRoom)
				lastRoom->nextRoom = cRoom->nextRoom;
			if (gRoomList == cRoom)
				gRoomList = cRoom->nextRoom;
			--gNbrRooms;
			DisposePtr((Ptr) cRoom);
			break;
		}
		lastRoom = cRoom;
	}
}

/******************************************************************************************/

HotspotPtr GetHotspot(ServerRoomPtr room, short id)
{
	short		i;
	HotspotPtr	hs;
	hs = (HotspotPtr) &room->room.varBuf[room->room.hotspotOfst];
	for (i = 0; i < room->room.nbrHotspots; ++i)
		if (hs[i].id == id)
			return &hs[i];
	return NULL;
}

/******************************************************************************************/

void SendRoomToUser(ServerRoomPtr cRoom, ServerUserPtr cUser)
{
	/* esr portFix - Made sure NewPtr is passed > 0 */
	short			    i,n;
	ServerUserPtr	who;
	UserRecPtr		up,uList;

	/* Generate appropriate events to initiate new room for client */
	/* esr portFix - 'evnt' -> MSG_xxxx */
						/*  - this hints that a packet group is to follow */
	PostUserEvent(cUser,MSG_ROOMDESC,0L,(Ptr)&cRoom->room,sizeof(RoomRec)+cRoom->room.lenVars);


	/* 12/11/95 uList = (UserRecPtr) NewPtr(sizeof(UserRec) * (cRoom->room.nbrPeople + 1)); */
	uList = (UserRecPtr) gBigBuffer;
	if (uList == NULL)  {
        PostUserEvent(cUser, MSG_NAVERROR, (LONG) SE_InternalError, NULL, 0L);
		LogMessage("Not enough Mem to send room\r");
		return;
	}
	up = uList;
	n  = 0;
	for (i = 0; i < cRoom->room.nbrPeople; ++i) {
		who = GetServerUser(cRoom->personID[i]);
		if (who)
        {
			*up = who->user;
			++up;
			++n;
		}
	}
	PostUserEvent(cUser,MSG_USERLIST, (LONG) n, (Ptr) uList, sizeof(UserRec) * n);
	/* DisposePtr((Ptr) uList); */
	/* esr portFix - 'evnt' -> MSG_xxxx */
	PostUserEvent(cUser,MSG_ROOMDESCEND, 0L, NULL, 0L);
}

void GenerateListOfAllUsers(ServerUserPtr cUser)
{
	UserListPtr	uList;
	char	*p,*uListBuffer;
	ServerUserPtr	curUser;
	LONG			count;
	LONG			nbrUsers;
	Boolean			hideFlag;
	/* 12/11/95 uListBuffer = NewPtrClear((sizeof(UserListRec)+32)*gNbrUsers); */
	uListBuffer = gBigBuffer;
	if (uListBuffer == NULL) {
		LogMessage("Not enough Mem to send user list\r");
		return;
	}
	p = uListBuffer;
	nbrUsers = 0;
	for (curUser = gUserList; curUser; curUser = curUser->nextUser) {
		hideFlag = (((curUser->flags & U_Hide) || 
					ActionExists(curUser, cUser, UA_HideFrom)) &&
					!(cUser->flags & U_SuperUser) &&
					!(curUser->flags & U_Guest));
		if (curUser->user.name[0] > 0 && curUser->user.name[0] < 32 && !hideFlag) {
			uList = (UserListRec *) p;		
			uList->userID = curUser->user.userID;
			uList->flags = curUser->flags;
			uList->roomID = curUser->user.roomID;
			BlockMove(curUser->user.name,uList->name,curUser->user.name[0]+1);
			/* JAB 7/16/96 add null for buggy PC's */
			uList->name[curUser->user.name[0]+1] = 0;
			p += 8 + uList->name[0] + (4 - (uList->name[0] & 3));	/* 9/5/95 */
			++nbrUsers;
		}
	}
	count = (long) p - (long) uListBuffer;
	PostUserEvent(cUser,MSG_LISTOFALLUSERS,(LONG) nbrUsers,(Ptr) uListBuffer,count);
	/* DisposePtr(uListBuffer); */
}

void GenerateListOfAllRooms(ServerUserPtr cUser)
{
	RoomListPtr	rList;
	char	*p,*rListBuffer,*rName;
	ServerRoomPtr	curRoom;
	LONG			count;
	long			roomCount = 0;
	if (cUser == NULL)
		return;
	/* 12/11/95 rListBuffer = NewPtrClear((sizeof(RoomListRec)+32)*gNbrRooms);	*/ /* 6/11/95 */
	rListBuffer = gBigBuffer;
	if (rListBuffer == NULL) {
		LogMessage("Not enough Mem to send room list\r");
		return;
	}
	p = rListBuffer;
	for (curRoom = gRoomList; curRoom; curRoom = curRoom->nextRoom) {
		rList = (RoomListRec *) p;		
		/* 11/18 Don't send hidden rooms to non-wizards */
		if (!(curRoom->room.roomFlags & RF_Hidden) ||
			 (cUser->flags & U_SuperUser)) {
			rList->roomID = curRoom->room.roomID;
			rList->flags = curRoom->room.roomFlags & 0x0000FFFF;
			rList->nbrUsers = curRoom->room.nbrPeople;
			if (curRoom->room.roomNameOfst)
				rName = &curRoom->room.varBuf[curRoom->room.roomNameOfst];
			else
				rName = "";
			BlockMove(rName,rList->name,rName[0]+1);
			/* JAB 7/16/96 Add null terminator at end of string - */
			/* Old buggy PC's versions are needing it... */
			rList->name[rName[0]+1] = 0;
			p += 8 + rList->name[0] + (4 -  (rList->name[0] & 3));	/* 9/5/95 */
			++roomCount;
		}
	}
	count = (long) p - (long) rListBuffer;
	PostUserEvent(cUser,MSG_LISTOFALLROOMS,(LONG) roomCount,(Ptr) rListBuffer,count);
	/* DisposePtr(rListBuffer); */
}


/******************************************************************************************/

void AddUserToRoom(ServerRoomPtr cRoom, ServerUserPtr cUser, Point where)
{
	/* Add user to room data structure, Send appropriate events */
	if (cRoom->room.nbrPeople < MaxPeoplePerRoom) {
#if BISERVER
		{
			LONG	roomID;
			roomID = cRoom->room.roomID;
			SendFrontEndEvent((BiFrontEndPtr) cUser->frontEndPtr,
								bi_assoc,
								cUser->feIPPort,
								cUser->feIPAddr,
								sizeof(LONG),
								(unsigned char *) &roomID);
		}
#endif
		cUser->user.roomID = cRoom->room.roomID;
		cUser->user.roomPos = where;
		cRoom->personID[cRoom->room.nbrPeople] = cUser->user.userID;
		cRoom->room.nbrPeople++;
		/* esr portFix - 'evnt' -> MSG_xxxx */
		PostRoomEvent(cRoom->room.roomID,MSG_USERNEW, cUser->user.userID, (Ptr) &cUser->user, sizeof(UserRec));
	}
    else /* 9/12/95 */
        PostUserEvent(cUser, MSG_NAVERROR, (LONG) SE_RoomFull, NULL, 0L);
	UpdateUserDisplay(cUser->user.userID);	/* 7/26/95 Local Function for updating user list window */
}

/******************************************************************************************/

/* 7/25/95  */
void SendServerInfo(ServerUserPtr user)
{
	ServerInfo		sInfo;
	BlockMove(gPrefs.serverName, sInfo.serverName, gPrefs.serverName[0]+1);
	sInfo.serverPermissions = gPrefs.permissions;
	/* 1/27/97 JAB New Style Server Info */
#define NEW_STYLE	0
#if NEW_STYLE
	sInfo.serverOptions = gPrefs.serverOptions;
	sInfo.ulUploadCaps = LI_ULCAPS_ASSETS_PALACE;
	sInfo.ulDownloadCaps = LI_DLCAPS_ASSETS_PALACE | LI_DLCAPS_FILES_PALACE;
	PostUserEvent(user,MSG_SERVERINFO, user->user.userID, (Ptr) &sInfo, sizeof(ServerInfo));
#else
	PostUserEvent(user,MSG_SERVERINFO, user->user.userID, (Ptr) &sInfo, sizeof(LONG) + gPrefs.serverName[0]+1);
#endif
}

/* 7/25/95 Update Server Info */
void SendServerInfoToAll(void)
{
	ServerUserPtr	curUser;
	for (curUser = gUserList; curUser; curUser = curUser->nextUser)
		SendServerInfo(curUser);
}

/* 8/2/95 Modified */
/* 1/14/97 JAB - added support for newStyle AuxRegistrationRec */
void EnterMansion(AuxRegistrationRec *lPtr, ServerUserPtr	user, Boolean newStyle)	
{
	ServerRoomPtr			cRoom = GetRoom(gEntrance);
	Point					p;
	short					userFlags;
	Boolean					badNameFlag=0;
	char					numBuf[64];
	AuxRegistrationRec		tempRec;

	/* 1/14/97 JAB */
	if (!newStyle)
		lPtr = TranslateToAlternateSignonRecord((LogonInfoPtr) lPtr, &tempRec);

	if (UserSecurityCheck(lPtr, user))
		return;

	if (!(user->flags & U_Guest)) {
		ServerUserPtr	aUser;

		sprintf((char *) user->user.name,"Member %ld",(long) user->user.userID);
		CtoPstr((char *) user->user.name);

		/* shorten bad names */
		if (lPtr->userName[0] > 31)
			lPtr->userName[0] = 31;
		/* fix wizard names */
		if (lPtr->userName[0] && lPtr->userName[1] == '*') {
			BlockMove(&lPtr->userName[2],&lPtr->userName[1],lPtr->userName[0]-1);
			lPtr->userName[0]--;
		}
		/* 1/14/97 JAB - fix demo member names */
		if (NewbieUserSerialNumber(user->crc,user->counter)) {
			/* make sure name begins with ":" */
			if (lPtr->userName[0] && lPtr->userName[1] != ':') {
				BlockMove(&lPtr->userName[1],&lPtr->userName[2],lPtr->userName[0]);
				lPtr->userName[1] = ':';
				if (lPtr->userName[0] < 31)
					lPtr->userName[0]++;

			}
		}
		else {
			/* make sure name does not begin with ":" */
			if (lPtr->userName[0] && lPtr->userName[1] == ':') {
				BlockMove(&lPtr->userName[2],&lPtr->userName[1],lPtr->userName[0]-1);
				lPtr->userName[0]--;
			}
		}


		/* 11/5/95 don't allow copycat names on log-in */
		if (IsLegalName(user, lPtr->userName)) {	/* 1/15/95 */
			aUser = GetServerUserByName(lPtr->userName);
			if (aUser && aUser->counter != user->counter) {
				char	tbuf[64];
				int		n=2;
				/* make an attempt to fix the name */
				do {
					sprintf(tbuf,"%s %d",CvtToCString(lPtr->userName),n);
					n++;
					CtoPstr(tbuf);
				} while ((aUser = GetServerUserByName((StringPtr) tbuf)) != NULL);
			   	BlockMove(tbuf,user->user.name,tbuf[0]+1);
				badNameFlag = 1;
			}
			else
			   	BlockMove(lPtr->userName,user->user.name,lPtr->userName[0]+1);
		}
	}

	ConvertNetAddressToNumericString(user,numBuf);

	TrackCheck(user);

	TimeLogMessage("%s signed on from (%s) ",CvtToCString(user->user.name),numBuf);
	LogMessage("[%ld Users]\r",gNbrUsers);

	/* 1/14/97 JAB Allow user to go to requested room */
	if (lPtr->desiredRoom) {
		cRoom = GetRoom(lPtr->desiredRoom);
		if (cRoom == NULL) {
			cRoom = GetRoom(gEntrance);
		}
	}
	else
		cRoom = GetRoom(gEntrance);

	/* 8/28/96 Search Drop Zones First... */
	if (cRoom == NULL || cRoom->room.nbrPeople > gPrefs.roomOccupancy) 
	{
		cRoom = gRoomList;
		/* skip to next drop zone */
		while (cRoom && (!(cRoom->room.roomFlags & RF_DropZone) || 
						  (cRoom->room.roomFlags & RF_Private) ||
			 			 cRoom->room.nbrPeople > gPrefs.roomOccupancy))
		{
			cRoom = cRoom->nextRoom;
		}
	}

	/* Drop Zones are Full, use available rooms */
	/* 7/29/95 Room is Full or non-existant - find a room that's ok */
	if (cRoom == NULL || cRoom->room.nbrPeople > gPrefs.roomOccupancy) {
		cRoom = gRoomList;
		while (cRoom && ((cRoom->room.roomFlags & RF_Private) ||
			   cRoom->room.nbrPeople > gPrefs.roomOccupancy))
			cRoom = cRoom->nextRoom;
	}

	if (cRoom == NULL) {
		LogMessage("%s Can't Enter Mansion (rooms full?) \r",CvtToCString(user->user.name));
		ScheduleUserKill(user, K_ServerFull, 0);
		return;
	}

	/* esr portFix - 'evnt' -> MSG_xxxx */
	PostUserEvent(user,MSG_VERSION, MansionVersion, NULL, 0L);
  /* esr - move to net code. Not the right place. I need
   *       a function that sends a TIYID but waits for
   *       the user to explicitly send a 'regi' event.
   *
   */
	userFlags = user->flags;
	BeginSendGroup(user);	/* jab 6/24/95 - Locally implemented function
							 * for hinting about message groups
							 * can be ignored, but use for better performance
								*/
	/* New - Send Server Info Packet
	 * 7/25/95
     */
	SendServerInfo(user);

	PostUserEvent(user,MSG_USERSTATUS, user->user.userID, (Ptr) &userFlags, sizeof(short));


	/* 7/26/96 Limit amount of global traffic */
	PostGlobalUserStatus(MSG_USERLOG, user, user->user.userID, cRoom->room.roomID);
	/* PostGlobalEvent(MSG_USERLOG,user->user.userID,(Ptr) &gNbrUsers,sizeof(LONG)); */


	SendRoomToUser(cRoom, user);
	p.h = RoomWidth/2 + MyRandom(256)-128;	/* 10/18/95 increased random location */
	p.v = RoomHeight/2 + MyRandom(256)-128;	/* 10/18/95 increased randomness */
	AddUserToRoom(cRoom, user, p);

	if (gPrefs.autoAnnounce[0]) {	/* 6/21/96 */
		UserMessage(user,"%s",gPrefs.autoAnnounce);
	}
	/* if (gPrefs.serverOptions & SO_ChatLogging)
	 *	UserMessage(user,"Notice: ")
     */

	if (badNameFlag)
		UserMessage(user,"Sorry - that name was taken");

	EndSendGroup(user);	/*jab  6/24/95 Send Entire Group as a single packet. */
	
	/* !!! This can be taken out after 12/1/95 */
	{
		void UserOldSerialCheck(ServerUserPtr user);
		UserOldSerialCheck(user);
	}
	/* 1/14/97 JAB allow for automatic wiz access */
	if (lPtr->wizPassword[0] && EqualPString(lPtr->wizPassword,gPrefs.wizardPassword,true)) 
	{
		ProcessSuperUser(user,lPtr->wizPassword);
	}
}

/******************************************************************************************/
/* Add security here later using userID */
/* if cUser is NULL, door is being locked/unlocked by server. */
void LockDoor(ServerUserPtr cUser, short *lockRec, Boolean lockFlag)
{
	short			roomID,doorID;
	ServerRoomPtr	srcRoom,dstRoom;
	HotspotPtr		srcDoor,dstDoor;
	short			i;
	short			destLockRec[2];
	HotspotPtr		hs;

	roomID = lockRec[0];
	doorID = lockRec[1];
	srcRoom = GetRoom(roomID);
	/* 6/30/95 */
	if (srcRoom == NULL)
		return;
	/* 6/30/95 */
	if (cUser && cUser->user.roomID != roomID && !(cUser->flags & U_God))
		return;
	srcDoor = GetHotspot(srcRoom, doorID);
	if (srcDoor == NULL)
		return;
	if (srcDoor->type != HS_ShutableDoor && srcDoor->type != HS_LockableDoor)
		return;
	if (lockFlag) {
		if (srcRoom->room.roomFlags & RF_Private)
			srcRoom->room.roomFlags |= RF_Closed;
		srcDoor->state = HS_Lock;
		/* esr portFix - 'evnt' -> MSG_xxxx */
		PostRoomEvent(srcRoom->room.roomID,MSG_DOORLOCK,0L,(Ptr) lockRec,sizeof(short)*2);
	}
	else {
		srcRoom->room.roomFlags &= ~RF_Closed;
		srcDoor->state = HS_Unlock;
		/* esr portFix - 'evnt' -> MSG_xxxx */
		PostRoomEvent(srcRoom->room.roomID,MSG_DOORUNLOCK,0L,(Ptr) lockRec,sizeof(short)*2);
	}
	dstRoom = GetRoom(srcDoor->dest);
	if (dstRoom == NULL)
		return;
	destLockRec[0] = srcDoor->dest;
	hs = (HotspotPtr) &dstRoom->room.varBuf[dstRoom->room.hotspotOfst];
	for (i = 0; i < dstRoom->room.nbrHotspots; ++i) {
		if (hs[i].type == HS_ShutableDoor &&
			hs[i].dest == roomID) {
			dstDoor = &hs[i];
			destLockRec[1] = dstDoor->id;
			if (lockFlag) {
				dstDoor->state = HS_Lock;
				/* esr portFix - 'evnt' -> MSG_xxxx */
				PostRoomEvent(dstRoom->room.roomID,MSG_DOORLOCK,0L,(Ptr) destLockRec,sizeof(short)*2);
			}
			else {
				dstDoor->state = HS_Unlock;
				/* esr portFix - 'evnt' -> MSG_xxxx */
				PostRoomEvent(dstRoom->room.roomID,MSG_DOORUNLOCK,0L,(Ptr) destLockRec,sizeof(short)*2);
			}
		}
	}
}

/******************************************************************************************/
/* !! Add security here later using userID */
void UpdatePictureLocation(ServerUserPtr cUser, short *uRec)
{
	short			roomID,spotID;
	Point			picLoc;
	ServerRoomPtr	srcRoom;
	HotspotPtr		srcSpot;
	StateRecPtr		sr;

	if (cUser == NULL)
		return;
	if (!(cUser->flags & U_SuperUser))		/* 6/14/95 */
		return;
	if (!(gPrefs.permissions & PM_WizardsMayAuthor) &&
		!(cUser->flags & U_God))	/* 7/22/95 */
		return;

	roomID = uRec[0];
	spotID = uRec[1];
	picLoc.v = uRec[2];
	picLoc.h = uRec[3];
	srcRoom = GetRoom(roomID);
	if (srcRoom == NULL)
		return;
	srcSpot = GetHotspot(srcRoom, spotID);
	if (srcSpot == NULL)
		return;
	sr = (StateRecPtr) &srcRoom->room.varBuf[srcSpot->stateRecOfst];
	sr += srcSpot->state;
	if (!EqualPt(sr->picLoc,picLoc)) {
		sr->picLoc = picLoc;
		/* esr portFix - 'evnt' -> MSG_xxxx */
		PostRoomEvent(srcRoom->room.roomID,MSG_PICTMOVE,0L,(Ptr) uRec,sizeof(short)*4);
	}
}

/******************************************************************************************/
/* !! Add security here later using userID */
void UpdateSpotLocation(ServerUserPtr cUser, short *uRec)
{
	short			roomID,spotID;
	Point			loc;
	ServerRoomPtr	srcRoom;
	HotspotPtr		srcSpot;

	if (cUser == NULL)
		return;
	if (!(cUser->flags & U_SuperUser))		/* 6/14/95 */
		return;
	if (!(gPrefs.permissions & PM_WizardsMayAuthor) &&
		!(cUser->flags & U_God))	/* 7/22/95 */
		return;

	roomID = uRec[0];
	spotID = uRec[1];
	loc.v = uRec[2];
	loc.h = uRec[3];
	srcRoom = GetRoom(roomID);
	if (srcRoom == NULL)
		return;
	srcSpot = GetHotspot(srcRoom, spotID);
	if (srcSpot == NULL)
		return;
	if (!EqualPt(srcSpot->loc,loc)) {
		srcSpot->loc = loc;
		/* esr portFix - 'evnt' -> MSG_xxxx */
		PostRoomEvent(srcRoom->room.roomID,MSG_SPOTMOVE,0L,(Ptr) uRec,sizeof(short)*4);
	}
}

/******************************************************************************************/

void UpdateSpotState(ServerUserPtr cUser, short *uRec)
{
	short			roomID,spotID,state;
	ServerRoomPtr	srcRoom;
	HotspotPtr		srcSpot;

	roomID = uRec[0];
	spotID = uRec[1];
	state = uRec[2];
	srcRoom = GetRoom(roomID);
	if (srcRoom == NULL)
		return;
	if (cUser && cUser->user.roomID != roomID && !(cUser->flags & U_God))
		return;
	srcSpot = GetHotspot(srcRoom, spotID);
	if (srcSpot == NULL)
		return;
	if (srcSpot->state != state) {
		srcSpot->state = state;
		/* esr portFix - 'evnt' -> MSG_xxxx */
		PostRoomEvent(srcRoom->room.roomID,MSG_SPOTSTATE,0L,(Ptr) uRec,sizeof(short)*3);
	}
}

/******************************************************************************************/

void RemovePersonFromRoom(ServerRoomPtr cRoom, PersonID userID )
{
	short	i;
	for (i = 0; i < cRoom->room.nbrPeople; ++i) {
		if (cRoom->personID[i] == userID) {
			cRoom->personID[i] = cRoom->personID[cRoom->room.nbrPeople-1];
			--cRoom->room.nbrPeople;
			--i;
		}
	}
	if (cRoom->room.nbrPeople == 0) {
		short	lockRec[2];
		HotspotPtr	hs;
		lockRec[0] = cRoom->room.roomID;
		hs = (HotspotPtr) &cRoom->room.varBuf[cRoom->room.hotspotOfst];
		for (i = 0; i < cRoom->room.nbrHotspots; ++i) {
			if ((hs[i].type == HS_LockableDoor) && hs[i].state == HS_Lock) {
				lockRec[1] = hs[i].id;
				LockDoor(0L,lockRec,false);
			}
		}
		cRoom->room.roomFlags &= ~RF_Closed;

		/* 5/8/96 member owned rooms are deleted if emptied and owner
		   has logged off */
		if (cRoom->memberOwner != 0 && GetServerUserByCounter(cRoom->memberOwner) == NULL)
			ScheduleRoomDeletionCheck();

	}
}

/******************************************************************************************/


