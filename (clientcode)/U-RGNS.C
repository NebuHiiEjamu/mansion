////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "u-User.h"
#include "m-cmds.H"
#include "u-Snds.h"
#include "u-Script.h"
#if PPASUPPORT
#include "ppamgr.h"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HotspotPtr GetHotspot(short n)
{
	short		   i;
	HotspotPtr hs = (HotspotPtr)&gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];

	for (i=0;i<gRoomWin->curRoom.nbrHotspots;i++)
		if (hs[i].id == n)
			return &hs[i];
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PictureRecPtr GetPictureRec(short n)
{
	short		      i;
	PictureRecPtr	ps = (PictureRecPtr)&gRoomWin->curRoom.varBuf[gRoomWin->curRoom.pictureOfst];

	for (i=0;i<gRoomWin->curRoom.nbrPictures;i++)
		if (ps[i].picID == n)
			return &ps[i]; 
	return NULL;
}

PictureRecPtr GetPictureRecByName(StringPtr name)
{
	short		    i;
	RoomRec			*rp = &gRoomWin->curRoom;
	PictureRecPtr	ps = (PictureRecPtr)&gRoomWin->curRoom.varBuf[gRoomWin->curRoom.pictureOfst];

	for (i=0;i<gRoomWin->curRoom.nbrPictures;i++)
		if (EqualPString((StringPtr) &rp->varBuf[ps[i].picNameOfst],name,false))
			return &ps[i]; 
	return NULL;
}

// 8/24/95
void GetSpotName(HotspotPtr hs, StringPtr name)
{
	name[0] = 0;
	if (hs && hs->nameOfst) {
		StringPtr	str;
		str = (StringPtr) &gRoomWin->curRoom.varBuf[hs->nameOfst];
		BlockMove(str,name,str[0]+1);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HotspotPtr GetNavArea(void)
{
	short		   i;
	HotspotPtr hs = (HotspotPtr)&gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];

	for (i=0;i<gRoomWin->curRoom.nbrHotspots;i++)
		if (hs[i].type == HS_NavArea)
			return &hs[i];
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HotspotPtr GetDoor(short doorID)
{
	short		   i;
	HotspotPtr hs = (HotspotPtr)&gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];

	for (i=0;i<gRoomWin->curRoom.nbrHotspots;i++)
    switch (hs[i].type)
    {
      case HS_Door:
      case HS_ShutableDoor:
      case HS_LockableDoor:
			  if (hs[i].id == doorID)
    			return &hs[i];
        break;
    }
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Boolean PtInHotspot(Point p,Hotspot *hs)
{
	if (hs->refCon)
		if (PtInRgnMac(p,(RgnHandle)hs->refCon))
      return TRUE;
    else
      return FALSE;
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *GetEventHandler(HotspotPtr hs, short type)
{
	EventHandlerPtr	ehp;
	short			      i;

	if (type < PE_NbrEvents && hs->scriptEventMask & (1L << type))
	{
		ehp = (EventHandlerPtr) &gRoomWin->curRoom.varBuf[hs->scriptRecOfst];
		for (i = 0; i < hs->nbrScripts; ++i,++ehp)
			if (ehp->eventType == type)
				return &gRoomWin->curRoom.varBuf[ehp->scriptTextOfst];
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PassHotspotEvent(HotspotPtr hs, short type)
{
	char	*eh;
	if ((eh = GetEventHandler(hs,type)) != NULL)
		DoHotspotScript(eh, hs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserClickHotspot(Point np, short n)
{
	HotspotPtr	hsl = (HotspotPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst],hs;
	hs = &hsl[n];

	if (!(hs->flags & HS_DontMoveHere)) {
		PostServerEvent(MSG_USERMOVE,gRoomWin->meID,(Ptr) &np,sizeof(Point));
		// 6/25 - server won't reply to the above msg anymore...
		UpdateUserPosition(gRoomWin->meID,&np);
	}
	PassHotspotEvent(hs,PE_Select);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TriggerHotspotEvents(short eventType)
{
	short	i;
	long	eventMask = (1L << eventType);

	HotspotPtr hsl = (HotspotPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];

	if (gRoomWin->mePtr == NULL)
		return;

	//  7/20/95 User Script Handling
	GenerateUserEvent(eventType);

	for (i = 0; i < gRoomWin->curRoom.nbrHotspots; ++i,++hsl) {
		if (hsl->scriptEventMask & eventMask)
			PassHotspotEvent(hsl,eventType);
	}

#if PPASUPPORT
	// 6/27/96 Plugin handling
	{
		char	textBuf[256]="";		// was char eventData...
		LONG	eventData;				// changed to long 11/20/96 JAB
		// Support modifying of chat strings
		if (eventType == PE_InChat || eventType == PE_OutChat) {
			eventData = gWhoChat;
			GetExternStringGlobal(textBuf, GS_ChatString);
		}
		PPAMgrPropagatePalaceEvent(eventType,eventData,textBuf);
		if (eventType == PE_InChat || eventType == PE_OutChat) {
			SetExternStringGlobal(textBuf, GS_ChatString);
		}
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Boolean DoorIsLocked(short doorID)
{
	HotspotPtr door;

	door = GetDoor(doorID);
	if (door)
		return (door->state == HS_Lock);
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetDoorLock(short *lockRec, Boolean lockFlag)
{
	short		    roomID;
  short       doorID;
	HotspotPtr	door;

	roomID = lockRec[0];
	doorID = lockRec[1];
	if (roomID && roomID == gRoomWin->curRoom.roomID)
  {
		door = GetDoor(doorID);
		if (door)
    {
			if (lockFlag)
      {
				door->state = HS_Lock;
				PassHotspotEvent(door,PE_Lock);
				PlaySnd(S_DoorClose, 1);
			}
			else
      {
				door->state = HS_Unlock;
				PassHotspotEvent(door,PE_Unlock);
				PlaySnd(S_DoorOpen, 1);
			}
			RefreshRoom(&gOffscreenRect);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetPictureOffset(short *setRec)
{ 
	short		    roomID,spotID,h,v;
	HotspotPtr	spot;
	StateRecPtr	sr;

	roomID = setRec[0];
	spotID = setRec[1];
	v = setRec[2];
	h = setRec[3];
	if (roomID && roomID == gRoomWin->curRoom.roomID)
  {
		spot = GetHotspot(spotID);
		if (spot)
    {
			sr = (StateRecPtr) &gRoomWin->curRoom.varBuf[spot->stateRecOfst];
			sr[spot->state].picLoc.v = v;
			sr[spot->state].picLoc.h = h;
			RefreshRoom(&gOffscreenRect);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetSpotLoc(short *setRec)
{
	short		    roomID,spotID,h,v;
	HotspotPtr	spot;

	roomID = setRec[0];
	spotID = setRec[1];
	v      = setRec[2];
	h      = setRec[3];
	if (roomID && roomID == gRoomWin->curRoom.roomID)
  {
		spot = GetHotspot(spotID);
		if (spot)
    {
			CollapseHotspot(spot);
			spot->loc.v = v;
			spot->loc.h = h;
			ExpandHotspot(spot);
			RefreshRoom(&gOffscreenRect);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetSpotState(short *setRec)
{
	short		    roomID,spotID,state;
	HotspotPtr	spot;

	roomID = setRec[0];
	spotID = setRec[1];
	state = setRec[2];
	if (roomID && roomID == gRoomWin->curRoom.roomID)
    {
		spot = GetHotspot(spotID);
		if (spot && spot->state != state)
    {
			spot->state = state;
			RefreshRoom(&gOffscreenRect);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


short GetSpotState(short spotID)	// 6/27/95
{
	HotspotPtr spot;

	spot = GetHotspot(spotID);
	if (spot)
		return spot->state;
	return 0;
}
