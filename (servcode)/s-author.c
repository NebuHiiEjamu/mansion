/******************************************************************************************/
/* esr - Changes:	* Replaced all 'evnt' with MSG_xxxx.
                * Remove extra unused vars
                * Rewrote AddRoomPString(StringPtr name) so
                  I could use it too.
*/
/******************************************************************************************/

#include "s-server.h"
#include "m-cmds.h"	/* 6/26/96 JAB */
#include <ctype.h>
#include <stdarg.h>

/******************************************************************************************/

extern short           gNbrRooms;
extern RoomRecPtr      gRoom;
extern HotspotPtr      gHotspots;
/* extern EventHandlerPtr gSpotEvents; */
extern StateRecPtr     gSpotStates;
extern DrawRecPtr      gDrawCmds;
extern PictureRecPtr   gPictureRecs;
extern LPropPtr		   gLProps;
extern Point		   *gPoints;
/* extern LONG				    *gFProps;		6/8/95 */

#define LONGALIGN(x)	x += (4 - (x & 3)) & 3

/******************************************************************************************/

short AddRoomBuffer(void *buf, LONG len)
{
	short	retVal = gRoom->lenVars;
	if (len == 0)
		return 0;
	else if ((LONG) gRoom->lenVars + len > 32767L)
		LogMessage("Room Too Big!\r");
	else {
		BlockMove((Ptr) buf,(char *) &gRoom->varBuf[retVal], len);
		gRoom->lenVars += len;
	}
	LONGALIGN(gRoom->lenVars);
	return retVal;
}

/******************************************************************************************/

short AddRoomPString(StringPtr name)
{
	return AddRoomBuffer(name, (LONG) name[0]+1);
}


short AddRoomCtoPString(char *name)
{
  char str[128];

  str[0] = strlen(name);
  strcpy(&str[1],name);
	return AddRoomBuffer(str,(LONG)str[0]+1);
}

/******************************************************************************************/

/* This procedure is done in response to a user */
/* requesting a new room */
ServerRoomPtr EditRoomToNewRoom(void)
{
	ServerRoomPtr	newRoom;
	LONG			varBufAlloc;
	int				i,j,k;
	Boolean			isUsed;
	StateRecPtr		ssS;


	/* JAB 7/15/96 - Delete references to pictures which aren't being used */
	for (i = 0; i < gRoom->nbrPictures; ++i) {
		isUsed = false;
		for (j = 0; j < gRoom->nbrHotspots && !isUsed; ++j) {
			ssS = (StateRec *) &gRoom->varBuf[gHotspots[j].stateRecOfst];
			for (k = 0; k < gHotspots[j].nbrStates; ++k,++ssS) {
				if (ssS->pictID == gPictureRecs[i].picID) {
					isUsed = true;
					break;
				}
			}
		}
		if (!isUsed) {
			if (i < gRoom->nbrPictures-1)
				BlockMove(&gPictureRecs[i+1],&gPictureRecs[i],((gRoom->nbrPictures-i)-1)*sizeof(PictureRec));
			--i;
			--gRoom->nbrPictures;
		}
	}

	/* Incorporate Pictures */
	LONGALIGN(gRoom->lenVars);
	gRoom->pictureOfst = AddRoomBuffer(gPictureRecs,gRoom->nbrPictures*sizeof(PictureRec));

	/* Incorporate Hotspots */
	LONGALIGN(gRoom->lenVars);
	gRoom->hotspotOfst = AddRoomBuffer(gHotspots, sizeof(Hotspot)*gRoom->nbrHotspots);

	/* Incorporate DrawCmds 7/14/95 */
	LONGALIGN(gRoom->lenVars);
	gRoom->firstDrawCmd = 0;
	for (i = gRoom->nbrDrawCmds-1; i >= 0; --i) {
		gDrawCmds[i].link.nextOfst = gRoom->firstDrawCmd;
		gRoom->firstDrawCmd = AddRoomBuffer((Ptr) &gDrawCmds[i],sizeof(DrawRecord));
	}
	/* gRoom->drawCmdOfst = AddRoomBuffer(gDrawCmds,gRoom->nbrDrawCmds*sizeof(DrawRecord)); */

	/* Incorporate LProps   6/28/95 */
	LONGALIGN(gRoom->lenVars);
	gRoom->firstLProp = 0;
	for (i = 0; i < gRoom->nbrLProps; ++i) {
		gLProps[i].link.nextOfst = gRoom->firstLProp;
		gRoom->firstLProp = AddRoomBuffer(gLProps+i,sizeof(LPropRec));
	}
	/* gRoom->lPropOfst = AddRoomBuffer(gLProps,gRoom->nbrLProps*sizeof(LPropRec)); */

	/* 6/28/95 JAB */
	/* Allocate 4K of extra space for loose props and such... */
	/* */
	varBufAlloc = gRoom->lenVars + 4096L;
	newRoom = (ServerRoomPtr) NewPtrClear(sizeof(ServerRoomRec)+varBufAlloc);
	if (newRoom == NULL)
		ReportError(memFullErr,"EditRoomToNewRoom");
	else {
		BlockMove((Ptr) gRoom, (Ptr) &newRoom->room, sizeof(RoomRec)+gRoom->lenVars);
		newRoom->varBufAlloc = varBufAlloc;
	}
	return newRoom;
}

/******************************************************************************************/

void AttachCurrentRoom(short maxMembers, short maxGuests)
{
	ServerRoomPtr	newRoom,roomPtr;

	newRoom = EditRoomToNewRoom();

	newRoom->maxOccupancy = maxMembers;	/* 1/27/96 */
	newRoom->maxGuests = maxGuests;	/* 1/27/96 */
	newRoom->nextRoom = NULL;
	
	if (gRoomList == NULL)
		gRoomList = newRoom;
	else {
		roomPtr = gRoomList;
		while (roomPtr->nextRoom)
			roomPtr = roomPtr->nextRoom;
		roomPtr->nextRoom = newRoom;
	}
}

/******************************************************************************************/

/* 5/8/96 - modified to allow for member created rooms */

RoomID MakeNewRoom(ServerUserPtr cUser, Boolean memberFlag, char *name)
{
	RoomID			roomID;
	ServerRoomPtr	rp;
	Boolean			unique;
	char			roomName[32];

	if (cUser == NULL)
		return 0;

	if (memberFlag) {
		if ((cUser->flags & U_Guest) || !(gPrefs.permissions & PM_MemberCreatedRooms))
		{
	        PostUserEvent(cUser, MSG_NAVERROR, (LONG) SE_CantAuthor, NULL, 0L);
			return 0;
		}
	}
	else {
		if (!(cUser->flags & U_SuperUser) ||
		     (	!(gPrefs.permissions & PM_WizardsMayAuthor) &&
			    !(cUser->flags & U_God)
			 )
			)
		{
	        PostUserEvent(cUser, MSG_NAVERROR, (LONG) SE_CantAuthor, NULL, 0L);
			return 0;
		}
	}

	BeginRoomCreation();
	roomID = cUser->user.roomID;
	do {
		unique = true;
		for (rp = gRoomList; rp; rp = rp->nextRoom) {
			if (rp->room.roomID == roomID) {
				unique = false;
				++roomID;
				break;
			}
		}
	} while (!unique);
	++gNbrRooms;
	gRoom->roomID = roomID;
	if (memberFlag) {
		if (name && *name)
			strcpy(roomName,name);
		else
			sprintf(roomName,"%.24s\'s Room", CvtToCString(cUser->user.name));
	}
	else
		sprintf(roomName,"New Room %d",(int) roomID);
	gRoom->roomNameOfst = AddRoomCtoPString(roomName);
	gRoom->pictNameOfst = AddRoomCtoPString("Clouds.GIF");	/* 10/16/95 JBUM */
	AttachCurrentRoom(0,0);
	EndRoomCreation();
	ChangeUserRoom(cUser, roomID, true);
	gModified = true;
	WizGlobalMessage("Page from System: %s %s created a new room (%s)",
				(cUser->flags & U_SuperUser)? "wizard" : "member",
				CvtToCString(cUser->user.name), roomName);
	return roomID;
}

void DuplicateRoom(ServerUserPtr cUser)
{
	RoomID			roomID,oldRoomID;
	ServerRoomPtr	roomPtr,rp;
	Boolean			unique;
	char			roomName[32];
	if (cUser == NULL)
		return;
	if (!(cUser->flags & U_SuperUser) ||
	     (!(gPrefs.permissions & PM_WizardsMayAuthor) &&
		  !(cUser->flags & U_God))) {					/* 6/14/95 */
        PostUserEvent(cUser, MSG_NAVERROR, (LONG) SE_CantAuthor, NULL, 0L);
		return;
	}
	oldRoomID = cUser->user.roomID;
	roomPtr = GetRoom(oldRoomID);
	if (roomPtr == NULL)
		return;

	BeginRoomCreation();
	CopyRoom(&roomPtr->room);
	roomID = oldRoomID;
	do {
		unique = true;
		for (rp = gRoomList; rp; rp = rp->nextRoom) {
			if (rp->room.roomID == roomID) {
				unique = false;
				++roomID;
				break;
			}
		}
	} while (!unique);
	++gNbrRooms;
	gRoom->roomID = roomID;
	if (gRoom->roomNameOfst)
		sprintf(roomName,"%s %d",CvtToCString((StringPtr) &gRoom->varBuf[gRoom->roomNameOfst]),(int) roomID);
	else
		sprintf(roomName,"New Room %d",(int) roomID);
	gRoom->roomNameOfst = AddRoomCtoPString(roomName);
	AttachCurrentRoom(roomPtr->maxOccupancy, roomPtr->maxGuests);
	EndRoomCreation();
	ChangeUserRoom(cUser, roomID,true);
	gModified = true;
	WizGlobalMessage("Page from System: %s made new room %d (duplicate of %d)",CvtToCString(cUser->user.name),
						(int) roomID,(int) oldRoomID);
}


/******************************************************************************************/
/* Copy Existing Hotspot Structure into gRoom */
/* */
void CopyHotspot(RoomRecPtr	rp, HotspotPtr hhS)
{
	short			j;
	HotspotPtr		hhD;
	StateRecPtr		ssD;
	Point			*ptS,*ptD;
	StateRec		sRecTemp;
	char			*ssS;	/* 9/11/96 was previously a StateRecPtr */
							/* modified to avoid longword alignment error due
							   to a client bug */
	/* EventHandlerPtr	ehS,ehD; */


	hhD = &gHotspots[gRoom->nbrHotspots];
	memset((Ptr) hhD, 0, sizeof(Hotspot));

	hhD->scriptEventMask = hhS->scriptEventMask;		/* 4/11/95 (modified the next few lines) */
	hhD->flags = hhS->flags;							/* JBUM */
	hhD->secureInfo = hhS->secureInfo;
	hhD->loc = hhS->loc;
	hhD->id = hhS->id;
	hhD->dest = hhS->dest;
	hhD->type = hhS->type;
	hhD->groupID = hhS->groupID;
	hhD->state = hhS->state;
	/* hhD->visible = hhS->visible; */

	if (hhD->type != HS_Normal)
		hhD->scriptEventMask &= ~(1L << PE_Select);

	/* Copy Spot Name 8/23/95 */
	if (hhS->nameOfst == 0)
		hhD->nameOfst = 0;
	else {
		hhD->nameOfst = AddRoomPString((StringPtr) &rp->varBuf[hhS->nameOfst]);
		/* JAB changed 7/15/96 LONGALIGN(gRoom->lenVars); */
	}

	/* Copy Picture Records */
	/* JAB 7/15/96 Moved to here */
	LONGALIGN(gRoom->lenVars);
	ssS = (char *) &rp->varBuf[hhS->stateRecOfst];
	for (j = 0; j < hhS->nbrStates; ++j,ssS += sizeof(StateRec)) {
		BlockMove(ssS, &sRecTemp, sizeof(StateRec));
		ssD = &gSpotStates[hhD->nbrStates];
		*ssD = sRecTemp;
		++hhD->nbrStates;
	}
	hhD->stateRecOfst = AddRoomBuffer(gSpotStates,hhD->nbrStates*sizeof(StateRec));

	/* Copy Outline */
	ptS = (Point *) &rp->varBuf[hhS->ptsOfst];
	for (j = 0; j < hhS->nbrPts; ++j,++ptS) {
		ptD = &gPoints[hhD->nbrPts];
		ptD->h = ptS->h;
		ptD->v = ptS->v;
		++hhD->nbrPts;
	}
	hhD->ptsOfst = AddRoomBuffer(gPoints,hhD->nbrPts*sizeof(Point));

	/* Copy Custom Scripts */
	hhD->scriptRecOfst = 0;
	hhD->nbrScripts = 0;
	if (hhS->scriptTextOfst)
		hhD->scriptTextOfst = AddRoomBuffer(&rp->varBuf[hhS->scriptTextOfst],strlen(&rp->varBuf[hhS->scriptTextOfst])+1);
	else
		hhD->scriptTextOfst = 0;
	LONGALIGN(gRoom->lenVars);


	/* hhD->scriptRecOfst = AddRoomBuffer(gSpotEvents,hhD->nbrScripts*sizeof(EventHandlerRec)); */
	++gRoom->nbrHotspots;
}

/******************************************************************************************/
/* Copy Existing Room Structure into gRoom, Compressing Unused Strings as Needed */
/* Doesn't calculate hotspotofst or pictureOfst */
/* */
void CopyRoom(RoomRecPtr	rp)
{
	short			i;
	PictureRecPtr	ppD;
	HotspotPtr		hhS;
	DrawRecPtr		dcS,dcD;
	LPropPtr		lp;
	PictureRec		pRecTemp;
	char			*ppS;		/* 9/11/96 was previously a PictureRecPtr */

	memset(gRoom,0,sizeof(RoomRec));
	gRoom->lenVars = 2;
	gRoom->roomID = rp->roomID;
	gRoom->roomFlags = rp->roomFlags;
	gRoom->roomNameOfst = AddRoomPString((StringPtr) &rp->varBuf[rp->roomNameOfst]);
	gRoom->pictNameOfst = AddRoomPString((StringPtr) &rp->varBuf[rp->pictNameOfst]);
	if (rp->artistNameOfst)
		gRoom->artistNameOfst = AddRoomPString((StringPtr) &rp->varBuf[rp->artistNameOfst]);
	if (rp->passwordOfst)
		gRoom->passwordOfst = AddRoomPString((StringPtr) &rp->varBuf[rp->passwordOfst]);
	/* Add Pictures */
	LONGALIGN(gRoom->lenVars);

	/* 9/11/96 */
	/* potential bug - some clients aren't using long alignment for ppS */
	/* so we copy into a longaligned buffer */
	ppS = (char *) &rp->varBuf[rp->pictureOfst];
	for (i = 0; i < rp->nbrPictures; ++i,ppS += sizeof(PictureRec)) {
		/* 9/11/96 */
		BlockMove(ppS, &pRecTemp, sizeof(PictureRec));
		ppD = &gPictureRecs[gRoom->nbrPictures];
		memset((char *) ppD, 0, sizeof(PictureRec));
		ppD->picID = pRecTemp.picID;
		ppD->transColor = pRecTemp.transColor;
		ppD->picNameOfst = AddRoomPString((StringPtr) &rp->varBuf[pRecTemp.picNameOfst]);
		++gRoom->nbrPictures;
	}

	LONGALIGN(gRoom->lenVars);

	/* Add DrawCmds 7/14/95 */
	dcS = (DrawRecPtr) &rp->varBuf[rp->firstDrawCmd];
	dcD = &gDrawCmds[0];
	for (i = 0; i < rp->nbrDrawCmds; ++i,++dcD) {
		BlockMove((Ptr) dcS, (Ptr) dcD,sizeof(DrawRecord));
		dcD->link.nextOfst = 0;
		dcD->dataOfst = AddRoomBuffer((Ptr) &rp->varBuf[dcS->dataOfst],dcS->cmdLength);
		dcS = (DrawRecPtr) &rp->varBuf[dcS->link.nextOfst];
	}
	gRoom->nbrDrawCmds = rp->nbrDrawCmds;

	/* Add LProps 6/30 */
	lp = (LPropPtr) &rp->varBuf[rp->firstLProp];
	for (i = 0; i < rp->nbrLProps; ++i) {
		/* Copy in backwards because they are inserted back in backwards */
		BlockMove((Ptr)lp,(Ptr)&gLProps[rp->nbrLProps-(i+1)],sizeof(LPropRec));
		gLProps[i].link.nextOfst = 0;
		lp = (LPropPtr) &rp->varBuf[lp->link.nextOfst];
	}
	/* BlockMove((Ptr) &rp->varBuf[rp->lPropOfst],(Ptr)gLProps,rp->nbrLProps*sizeof(LPropRec)); */
	gRoom->nbrLProps = rp->nbrLProps;

	LONGALIGN(gRoom->lenVars);

	/* Add Hotspots */
	hhS = (HotspotPtr) &rp->varBuf[rp->hotspotOfst];
	for (i = 0; i < rp->nbrHotspots; ++i,++hhS) {
		CopyHotspot(rp, hhS);
	}
}

/******************************************************************************************/
/* Replace a room record with a new, modified version */
/* */
/* 6/28/95 Added Boolean Argument for Notification */
void ReplaceRoom(ServerRoomPtr oldRoom, ServerRoomPtr newRoom, Boolean notify)
{
	ServerRoomPtr	rp;
	ServerUserPtr	curUser;
	RoomID			oldRoomID;

	/* Copy in Current People Info */
	newRoom->room.nbrPeople = oldRoom->room.nbrPeople;
	BlockMove((Ptr)&oldRoom->personID[0],(Ptr)&newRoom->personID[0],sizeof(LONG) * MaxPeoplePerRoom);

	/* Copy in Other Server-specific vars */
	newRoom->lastPainter = oldRoom->lastPainter;
	newRoom->maxOccupancy = oldRoom->maxOccupancy;
	newRoom->maxGuests = oldRoom->maxGuests;
	newRoom->memberOwner = oldRoom->memberOwner;
	strcpy(newRoom->roomPassword, oldRoom->roomPassword);	

	oldRoomID = oldRoom->room.roomID;

	newRoom->nextRoom = oldRoom->nextRoom;
	if (gRoomList == oldRoom)
		gRoomList = newRoom;
	else {
		rp = gRoomList;
		while (rp && rp->nextRoom != oldRoom)
			rp = rp->nextRoom;
		if (rp)
			rp->nextRoom = newRoom;
	}
	DisposePtr((Ptr) oldRoom);
	if (notify) {
		for (curUser = gUserList; curUser; curUser = curUser->nextUser)
			if (curUser->user.roomID == oldRoomID)
				PostUserEvent(curUser,MSG_ROOMSETDESC,0L,(Ptr)&newRoom->room, sizeof(RoomRec)+newRoom->room.lenVars);
	}
	gModified = true;
}

/******************************************************************************************/

void SetRoomInfo(ServerUserPtr	cUser, RoomRecPtr room)
{
	RoomID			oldRoomID;
	ServerRoomPtr	roomPtr;
	ServerRoomPtr	newRoom;
	StringPtr		rName;

	if (cUser == NULL)
		return;
	if (!(cUser->flags & U_SuperUser))		/* 6/14/95 */
		return;
	if (!(gPrefs.permissions & PM_WizardsMayAuthor) &&
		!(cUser->flags & U_God))	/* 7/22/95 */
		return;

	oldRoomID = cUser->user.roomID;
	roomPtr = GetRoom(oldRoomID);
	if (roomPtr == NULL)
		return;

	/* 9/29/95 Resolve Room Number Conflicts */
	if (room->roomID != oldRoomID && GetRoom(room->roomID) != NULL)
		room->roomID = oldRoomID;

	/* 9/19/95 allow users to unlock rooms */
	if ((roomPtr->room.roomFlags & RF_AuthorLocked) &&
		(room->roomFlags & RF_AuthorLocked))	/* 2/28/95 Don't allow authoring if room is password locked JAB */
		return;
	if (roomPtr->room.roomFlags != room->roomFlags) { /* 9/11/95 */
		if (roomPtr->room.roomNameOfst)
			rName = (StringPtr) &roomPtr->room.varBuf[roomPtr->room.roomNameOfst];
		else
			rName = (StringPtr) "";

		LogMessage("%s changed room state [%s]\r",CvtToCString(cUser->user.name),
					CvtToCString(rName));
		/* 3/11/95 unlock room if private flag cleared */
		if (!(room->roomFlags & RF_Private))
			room->roomFlags &= ~RF_Closed;
	}
	/* Copy the Room, Compressing out wasted space */
	BeginRoomCreation();
	CopyRoom(room);
	newRoom = EditRoomToNewRoom();
	EndRoomCreation();

	ReplaceRoom(roomPtr,newRoom,true);
}

/******************************************************************************************/
/* Modify user's room to add a new hotspot */
void MakeNewSpot(ServerUserPtr	cUser)
{
	RoomID			roomID;
	ServerRoomPtr	rp,newRoom;
	HotspotPtr		hp;
	static char		*exceedStr = "!Too many hotspots in this room";
	short			spotID;
	if (cUser == NULL)
		return;
	if (!(cUser->flags & U_SuperUser))	/* 6/14/95 */
		return;
	if (!(gPrefs.permissions & PM_WizardsMayAuthor) &&
		!(cUser->flags & U_God))	/* 7/22/95 */
		return;

	roomID = cUser->user.roomID;

	rp = GetRoom(roomID);
	if (rp == NULL)
		return;

	if (rp->room.roomFlags & RF_AuthorLocked)	/* 2/28/95 Don't allow authoring if room is locked JAB */
		return;

	if (rp->room.nbrHotspots >= MaxHotspots) {
		RoomMessage(roomID, "%s", exceedStr);
		return;
	}

	/* find unused spotid */
	spotID = rp->room.nbrHotspots+1;
	while (GetHotspot(rp, spotID) != NULL)
		++spotID;

	BeginRoomCreation();

	CopyRoom(&rp->room);

	hp = &gHotspots[gRoom->nbrHotspots];
	InitHotspot(hp);
	hp->id = spotID;
	hp->type = HS_Door;			/* 8/3/95 - default to passage */
	gPoints[0].h = 0;	gPoints[0].v = 0;
	gPoints[1].h = 256;	gPoints[1].v = 0;
	gPoints[2].h = 256;	gPoints[2].v = 192;
	gPoints[3].h = 0;	gPoints[3].v = 192;
	hp->nbrPts = 4;
	hp->loc.h = 128;
	hp->loc.v = 96;
	hp->stateRecOfst = AddRoomBuffer(gSpotStates,hp->nbrStates*sizeof(StateRec));
	hp->ptsOfst = AddRoomBuffer(gPoints,hp->nbrPts*sizeof(Point));
	++gRoom->nbrHotspots;

	newRoom = EditRoomToNewRoom();
	EndRoomCreation();

	ReplaceRoom(rp,newRoom,true);
}

/******************************************************************************************/
/* Modify user's room to add a new hotspot */
void DeleteSpot(ServerUserPtr	cUser, short spotID)
{
	RoomID			roomID;
	ServerRoomPtr	srp,newRoom;
	HotspotPtr		hhS;
	short			i,oldNbrHotspots;
	RoomRecPtr		rp;

	if (cUser == NULL)
		return;


	if (!(cUser->flags & U_SuperUser)) {
		WizGlobalMessage("Page from System: %s attempted to delete a hotspot",
					CvtToCString(cUser->user.name));
		return;
	}
	
	TimeLogMessage("%s deleted a hotspot\r",
				CvtToCString(cUser->user.name));
	
	roomID = cUser->user.roomID;
	srp = GetRoom(roomID);
	if (srp == NULL)
		return;

	if (srp->room.roomFlags & RF_AuthorLocked)	/* 2/28/95 Don't allow authoring if room is locked JAB */
		return;

	BeginRoomCreation();

	rp = &srp->room;

	oldNbrHotspots = rp->nbrHotspots;
	rp->nbrHotspots = 0;
	CopyRoom(&srp->room);
	rp->nbrHotspots = oldNbrHotspots;

	/* Add Hotspots */
	hhS = (HotspotPtr) &rp->varBuf[rp->hotspotOfst];
	for (i = 0; i < rp->nbrHotspots; ++i,++hhS) {
		if (hhS->id == spotID)
			continue;
		CopyHotspot(rp,hhS);
	}

	newRoom = EditRoomToNewRoom();
	EndRoomCreation();
	ReplaceRoom(srp,newRoom,true);
}

/******************************************************************************************/
/* 7/14/95  Optimized to not send whole room description back... */
void AddDrawCommand(ServerUserPtr	cUser, DrawRecPtr drawCmd)
{
	RoomID			roomID;
	ServerRoomPtr	rp,newRoom;
	DrawRecPtr		dp;
	static char		*exceedStr = "!Tone down the art!";

	if (cUser == NULL)
		return;

	if (cUser->flags & U_Guest)
		return;

	if (!(gPrefs.permissions & PM_AllowPainting) && !(cUser->flags & U_SuperUser))		/* 7/22 global painting setting */
		return;

	roomID = cUser->user.roomID;
	rp = GetRoom(roomID);
	if (rp == NULL)
		return;

	if ((rp->room.roomFlags & RF_NoPainting) > 0 && !(cUser->flags & U_SuperUser))		/* 7/25 - Individual Painting */
		return;											/* setting for room */

	if (drawCmd->drawCmd != DC_Detonate && 
		((LONG) rp->room.lenVars + (LONG) drawCmd->cmdLength > 12000L ||
		 rp->room.nbrDrawCmds >= MaxDrawCmds)) {
		 RoomMessage(roomID, "%s", exceedStr);
		return;
	}

	BeginRoomCreation();

	/* Automatically detonate if too many draw cmds.. */
	if (drawCmd->drawCmd == DC_Detonate) {
		rp->room.nbrDrawCmds = 0;
		rp->room.firstDrawCmd = 0;
	}

	CopyRoom(&rp->room);


	switch (drawCmd->drawCmd) {
	case DC_Detonate:
		gRoom->nbrDrawCmds = 0;
		gRoom->firstDrawCmd = 0;	/* 7/14/95 */
		break;
	case DC_Delete:
		if (gRoom->nbrDrawCmds) {
			--gRoom->nbrDrawCmds;
			if (gRoom->nbrDrawCmds == 0)	/* 7/14/95 */
				gRoom->firstDrawCmd = 0;
		}
		break;
	default:
		dp = &gDrawCmds[gRoom->nbrDrawCmds];
		memset(dp,0,sizeof(DrawRecord));
		dp->drawCmd = drawCmd->drawCmd;
		dp->cmdLength = drawCmd->cmdLength;
		dp->dataOfst = AddRoomBuffer((Ptr) &drawCmd[1],dp->cmdLength);
		dp->link.nextOfst = 0;
		++gRoom->nbrDrawCmds;
		break;
	}

	newRoom = EditRoomToNewRoom();
	EndRoomCreation();
	ReplaceRoom(rp,newRoom,false);
	PostRoomEvent(roomID,MSG_DRAW,0,(Ptr) drawCmd,sizeof(DrawRecord)+drawCmd->cmdLength);
	gModified = true;	/* 7/5/95 */
	if (cUser && (cUser->user.userID != newRoom->lastPainter || drawCmd->drawCmd == DC_Detonate)) {
		newRoom->lastPainter = cUser->user.userID;
		RoomMessage(roomID,"; %s %s",CvtToCString(cUser->user.name),
						drawCmd->drawCmd == DC_Detonate? "is Erasing" : "is Painting");
	}
}

/******************************************************************************************/

void AddNewProp(ServerUserPtr cUser, LONG *cmd)	/* modified 6/8/95 */
{
	RoomID			roomID;
	ServerRoomPtr	rp,newRoom;
	LPropPtr		dp;
	static char		*exceedStr = "!Clean your room!";
	LONG			propID;
	unsigned LONG	crc;
	Point			propLoc;

	if (cUser == NULL)
		return;
	roomID = cUser->user.roomID;
	rp = GetRoom(roomID);
	if (rp == NULL)
		return;

	if ((LONG) rp->room.lenVars + (LONG) sizeof(LPropRec) > 20000L ||
		rp->room.nbrLProps >= MaxLProps) {
		RoomMessage(roomID, "%s", exceedStr);
		return;
	}

	propID = cmd[0];
	crc = cmd[1];						/* 6/8/95 */
	propLoc = *((Point *) &cmd[2]);

	if ((cUser->flags & U_Guest) && (propID < MinReservedProp || propID > MaxReservedProp)) {
		UserMessage(cUser, "Sorry, Members Only");
		return;
	}


	CheckForProp(cUser,propID,crc);

	if (rp->varBufAlloc - rp->room.lenVars >= sizeof(LPropRec)+4) {
		/* Easy Method */
		LPropRec	lNew;
		memset(&lNew,0,sizeof(LPropRec));
		lNew.propSpec.id = propID;
		lNew.propSpec.crc = crc;
		lNew.loc = propLoc;
		lNew.link.nextOfst = rp->room.firstLProp;
		LONGALIGN(rp->room.lenVars);
		rp->room.firstLProp = rp->room.lenVars;
		BlockMove((Ptr)&lNew,(Ptr)&rp->room.varBuf[rp->room.lenVars],sizeof(LPropRec));
		rp->room.lenVars += sizeof(LPropRec);
		rp->room.nbrLProps++;
	}
	else {
		BeginRoomCreation();
		CopyRoom(&rp->room);
	
		dp = &gLProps[gRoom->nbrLProps];
		memset(dp,0,sizeof(LPropRec));
		dp->propSpec.id = propID;
		dp->propSpec.crc = crc;
		dp->loc = propLoc;
		++gRoom->nbrLProps;
		newRoom = EditRoomToNewRoom();
		EndRoomCreation();
		ReplaceRoom(rp,newRoom,false);
	}
	PostRoomEvent(roomID,MSG_PROPNEW,0,(Ptr) cmd,sizeof(LONG)*3);
	gModified = true;	/* 7/5/95 */
}

/******************************************************************************************/

void MoveLooseProp(ServerUserPtr cUser, LONG *cmd)
{
	RoomID			roomID;
	ServerRoomPtr	rp;
	LPropPtr		lp;
	short			i,n;

	if (cUser == NULL)
		return;
	roomID = cUser->user.roomID;
	rp = GetRoom(roomID);
	if (rp == NULL)
		return;
	n = cmd[0];
	if (n < rp->room.nbrLProps) {
		lp = (LPropPtr) &rp->room.varBuf[rp->room.firstLProp];
		for (i = 0; i < n; ++i) {
			if (lp->link.nextOfst)
				lp = (LPropPtr) &rp->room.varBuf[lp->link.nextOfst];
		}
		if ((cUser->flags & U_Guest) && (lp->propSpec.id < MinReservedProp || lp->propSpec.id > MaxReservedProp)) {
			UserMessage(cUser, "Sorry, Members Only");
			return;
		}
		lp->loc = *((Point *) &cmd[1]);
		PostRoomEvent(roomID,MSG_PROPMOVE,0,(Ptr) cmd,sizeof(LONG)*2);
	}
	gModified = true;	/* 7/5/95 */
}

/******************************************************************************************/

void DeleteLooseProp(ServerUserPtr	cUser, LONG *cmd)
{
	RoomID			roomID;
	ServerRoomPtr	rp;
	LPropPtr		lp,lastLP;
	short			i,propDel;

	if (cUser == NULL)
		return;
	propDel = cmd[0];
	roomID = cUser->user.roomID;
	rp = GetRoom(roomID);
	if (rp == NULL)
		return;
	if (propDel != -1L && propDel >= rp->room.nbrLProps)
		return;

	if (propDel == -1L) {
		rp->room.nbrLProps = 0;
		rp->room.firstLProp = 0;
	}
	else {
		lp = (LPropPtr) &rp->room.varBuf[rp->room.firstLProp];
		lastLP = NULL;
		for (i = 0; i < propDel; ++i) {
			if (lp->link.nextOfst) {
				lastLP = lp;
				lp = (LPropPtr) &rp->room.varBuf[lp->link.nextOfst];
			}
		}
		if ((cUser->flags & U_Guest) && (lp->propSpec.id < MinReservedProp || lp->propSpec.id > MaxReservedProp)) {
			UserMessage(cUser, "Sorry, Members Only");
			return;
		}
		if (propDel == 0)
			rp->room.firstLProp = lp->link.nextOfst;
		else
			lastLP->link.nextOfst = lp->link.nextOfst;
		--rp->room.nbrLProps;
	}
	PostRoomEvent(roomID,MSG_PROPDEL,0,(Ptr) cmd,sizeof(LONG));
	gModified = true;	/* 7/5/95 */

}

/******************************************************************************************/


