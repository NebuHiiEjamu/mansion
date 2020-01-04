// Authoring

#include <Menus.h>

#include "U-USER.H"
#include "m-cmds.H"
#include "DialogUtils.h"
#include "AppMenus.h"
#include "UserTCP.h"

#define GotoDLOG			600
#define G_DestItem			3
#define G_DestMenuItem		4

#define RoomInfoDLOG		700
#define R_ResetItem			3
#define R_DeleteItem		4
#define R_LockItem			5
#define R_IDItem			6
#define R_NameItem			7
#define R_PictNameItem		8
#define R_PictButtonItem	9
#define R_ArtistItem		10
#define R_OptionsItem		11

/*
#define R_Private			11
#define R_NoPainting		12
#define R_NoCyborgs			13
#define R_Hidden			14
*/

#define DoorInfoDLOG		701
#define D_DeleteItem		4
#define D_IDItem			5
#define D_DestItem			6
#define D_NameItem			7
#define D_DestMenuItem		8
#define D_SpotTypeItem		9
#define D_SpotStateItem		10
#define D_PictsItem			11
#define D_ScriptsItem		12
#define D_OptionsItem		13
#define D_KeyPtsItem		14

#define LockIcon			200

short		gLastRoom;
DialogPtr	gRoomDialog,gSpotDialog;
MenuHandle	gRoomListMenu;
short		gNbrRoomListMenuItems;
Ptr			gRoomListPtr;

pascal	void	DrawLockItem(DialogPtr dp, short itemNbr);
		Boolean MyNonModalDialogFilter(DialogPtr dp, EventRecord *ep, short *itemHit);
		short	RoomListID(Ptr p, short idx);

Boolean	HandleDialogEvent(EventRecord *theEvent)
{
	DialogPtr	dp;
	short		itemHit=0;
	char		tempChar;

	switch (theEvent->what) 
	{
		case keyDown:
		case autoKey:
		{
			tempChar = theEvent->message & charCodeMask;
			dp = FrontWindow();
			
			if (tempChar==10 || tempChar==13 || tempChar==3) 
			{
				itemHit = OK;
				SimulateButtonPress(dp,itemHit);
				if (dp == gRoomDialog)
					RoomDialogHit(theEvent, itemHit);
				else if (dp == gSpotDialog)
					SpotDialogHit(theEvent, itemHit);
				return true;
			}
			else if (tempChar == 27) 
			{
				itemHit = Cancel;
				SimulateButtonPress(dp,itemHit);
				if (dp == gRoomDialog)
					RoomDialogHit(theEvent, itemHit);
				else if (dp == gSpotDialog)
					SpotDialogHit(theEvent, itemHit);
				return true;
			}
			break;
		}
	}

	if (DialogSelect(theEvent,&dp,&itemHit)) 
	{
		if (dp == gRoomDialog)
			RoomDialogHit(theEvent, itemHit);
		else if (dp == gSpotDialog)
			SpotDialogHit(theEvent, itemHit);
		else
			return false;
		return true;
	}
	else 
	{
		switch (theEvent->what) 
		{
			case updateEvt:
			{
				// OutlineButton(dp,Cancel,&qd.white);
				OutlineButton(dp,OK,&qd.black);
				break;
			}
		}
	}
	return false;
}

void UpdateMyWindows();

void UpdateMyWindows()
{
	WindowPeek	wp;
	wp = (WindowPeek) FrontWindow();
	while (wp) {
		if (wp->refCon == ObjectWindowID) {
			SetPort((WindowPtr) wp);
			((ObjectWindowPtr) wp)->Draw((WindowPtr) wp);
		}
		wp = wp->nextWindow;
	}
}
// This Dialog Filter function is used to change the default button states,
// and trap for keyboard events so we can assign them to a particular button
// Used as default filter for most dialogs.
//
pascal Boolean MyDialogFilter(DialogPtr dp, EventRecord *ep, short *itemHit);

pascal Boolean MyDialogFilter(DialogPtr dp, EventRecord *ep, short *itemHit)
{
	// char tempChar;
	short	part;
	WindowPtr	whichWin;
	switch (ep->what) {
	case mouseDown:
		part = FindWindow(ep->where,&whichWin);
		if (whichWin == dp) {
			switch (part) {
			case inDrag:		// Transvestite code here
				{
					Rect	dragRect;
					dragRect = qd.screenBits.bounds;
					DragWindow(whichWin, ep->where, &dragRect);
					// Update background windows
					UpdateMyWindows();
				}
				return true;
			}
		}
		break;
	case kHighLevelEvent:
		if (ep->message == ServerEventID) {
			ProcessAppleTalkEvent(ep);
		}
		break;
	case nullEvent:
		if (gTCPRecord)
			PalTCPIdle(ep)
				;

	default:
		break;

	}
	return CallFilterProc(dp, ep, itemHit);
}

/**
Boolean MyNonModalDialogFilter(DialogPtr dp, EventRecord *ep, short *itemHit)
{
	// char tempChar;
	short	part;
	WindowPtr	whichWin;
	switch (ep->what) {

	case keyDown:
	case autoKey:
		tempChar = ep->message & charCodeMask;
		if(tempChar==10 || tempChar==13 || tempChar==3) {
			*itemHit = OK;
			SimulateButtonPress(dp,*itemHit);
			return true;
		}
		else if (tempChar == 27) {
			*itemHit = Cancel;
			SimulateButtonPress(dp,*itemHit);
			return true;
		}
		break;
	case updateEvt:
		// OutlineButton(dp,Cancel,&qd.white);
		OutlineButton(dp,OK,&qd.black);
		break;
	default:
		break;

	}
	return false;
	// CallFilterProc(dp, ep, itemHit);
}
**/



pascal void DrawLockItem(DialogPtr dp, short itemNbr)
{
	CIconHandle	cIcon;
	short		iconNbr,t;
	Rect		r;
	GrafPtr		savePort;
	Handle		h;
	if (itemNbr == R_LockItem) {
		GetPort(&savePort);
		SetPort(dp);

		GetDItem(dp,itemNbr,&t,&h,&r);
		EraseRect(&r);

		iconNbr = LockIcon + ((gRoomWin->curRoom.roomFlags & RF_AuthorLocked)? 1 : 0);
		cIcon = GetCIcon(iconNbr);
		if (cIcon) {
			PlotCIcon(&r,cIcon);
			DisposeCIcon(cIcon);
		}
		SetPort(savePort);
	}
}

struct RoomInfoVars {
	Boolean 	changeFlag,inLockedFlag;
	short		oldFlags,flags;
	Str63		oldName,oldPictName,oldArtist;
} gRoomInfo;

void RoomInfoDialog()
{
	GrafPtr		savePort;
	DialogPtr 	dp;
	short		t;
	Handle		h;
	Rect		r;
	StringPtr	name,pictName,artist;
	RoomRecPtr	rp = &gRoomWin->curRoom;
	UserItemUPP		drawLockProc = NewUserItemProc(DrawLockItem);

	gRoomInfo.changeFlag = false;
	GetPort(&savePort);

	if ((dp = GetNewDialog (RoomInfoDLOG, NULL, (WindowPtr) -1)) == NULL)
		return;
	gRoomDialog = dp;

	GetDItem(dp, R_LockItem, &t, &h, &r);
	SetDItem(dp, R_LockItem, t, (Handle) drawLockProc, &r);

	name = (StringPtr) &rp->varBuf[rp->roomNameOfst];
	BlockMove(name,gRoomInfo.oldName,name[0]+1);

	pictName = (StringPtr) &rp->varBuf[rp->pictNameOfst];
	BlockMove(pictName,gRoomInfo.oldPictName,pictName[0]+1);


	PrintfItem(dp, R_IDItem, "%d", rp->roomID);
	SetText(dp,R_NameItem,name);
	SetText(dp,R_PictNameItem,pictName);
	if (rp->artistNameOfst) {
		artist = (StringPtr) &rp->varBuf[rp->artistNameOfst];
		BlockMove(artist, gRoomInfo.oldArtist, artist[0]+1);
		SetText(dp,R_ArtistItem,artist);
	}
	else  {
		SetText(dp,R_ArtistItem,"\p");
		gRoomInfo.oldArtist[0]= 0;
	}
	SelIText(dp,R_IDItem,0,32767);

	gRoomInfo.inLockedFlag = (rp->roomFlags & RF_AuthorLocked) > 0;
	gRoomInfo.oldFlags = rp->roomFlags;
	gRoomInfo.flags = rp->roomFlags;

	SetDialogDefaultItem(dp, OK);
	SetDialogCancelItem(dp, Cancel);
	SetDialogTracksCursor(dp,true);

	ShowWindow(dp);
	SetPort(savePort);
//	SetPort(dp);
//	SetCursor(&qd.arrow);
//	ShowCursor();
}

void RoomDialogHit(EventRecord *theEvent, short itemHit)
{
	DialogPtr	dp = gRoomDialog;
	StringPtr	tp;
	Str63		tPass,password,temp;
	RoomRecPtr	rp = &gRoomWin->curRoom;
	int			t;
	Boolean		deleteFlag = false;
	Boolean		resetFlag = false;

//		ModalDialog(filterProc, &itemHit);
	switch (itemHit) 
	{
		
		case R_DeleteItem:
		{
			deleteFlag = true;
			itemHit = Cancel;
			break;
		}
		
		case R_ResetItem:
		{
			resetFlag = true;
			itemHit = Cancel;
			break;
		}
		
		case R_LockItem:
		{
			if (PasswordDialog(password,"\pRoom Password:") == OK && password[0] > 0) 
			{
				EncryptString(password,tPass);
				if (rp->roomFlags & RF_AuthorLocked) 
				{
					tp = (StringPtr) &rp->varBuf[rp->passwordOfst];
					if (EqualPString(tPass,tp,false))
					{
						rp->roomFlags &= ~RF_AuthorLocked;
						gRoomInfo.changeFlag = true;
						gRoomInfo.inLockedFlag = false;
					}
				}
				else 
				{
					rp->roomFlags |= RF_AuthorLocked;
					rp->passwordOfst = AddRoomString(tPass);
					gRoomInfo.changeFlag = true;
				}
				DrawLockItem(dp, R_LockItem);
			}
			break;
		}
		
		case R_OptionsItem:
		{
			short	t;
			Handle	h;
			Rect	r;
			Point	p;
			long	menuSelect;
			GrafPtr	savePort;
			
			GetPort(&savePort);
			SetPort(dp);
			GetDItem(dp,R_OptionsItem,&t,&h,&r);
			p = topLeft(r);
			LocalToGlobal(&p);
			gRoomOptionsMenu = GetMenu(RoomOptionsMENU);
			InsertMenu(gRoomOptionsMenu, hierMenu);
			
			if (gRoomOptionsMenu) 
			{
				CheckItem(gRoomOptionsMenu, RO_Private, (gRoomInfo.flags & RF_Private) > 0);
				CheckItem(gRoomOptionsMenu, RO_CyborgFreeZone, (gRoomInfo.flags & RF_CyborgFreeZone) > 0);
				CheckItem(gRoomOptionsMenu, RO_NoPainting, (gRoomInfo.flags & RF_NoPainting) > 0);
				CheckItem(gRoomOptionsMenu, RO_Hidden, (gRoomInfo.flags & RF_Hidden) > 0);
				CheckItem(gRoomOptionsMenu, RO_NoGuests, (gRoomInfo.flags & RF_NoGuests) > 0);
				menuSelect = PopUpMenuSelect(gRoomOptionsMenu,p.v+7,p.h+22,1);
				
				switch (LoWord(menuSelect)) 
				{
					case RO_Private:			gRoomInfo.flags ^= RF_Private;			break;
					case RO_CyborgFreeZone:		gRoomInfo.flags ^= RF_CyborgFreeZone;	break;
					case RO_NoPainting:			gRoomInfo.flags ^= RF_NoPainting;		break;
					case RO_Hidden:				gRoomInfo.flags ^= RF_Hidden;			break;
					case RO_NoGuests:			gRoomInfo.flags ^= RF_NoGuests;			break;
				}
				DeleteMenu(RoomOptionsMENU);
				DisposeMenu(gRoomOptionsMenu);
			}
			SetPort(savePort);
			break;
		}
		
		case R_PictButtonItem:
		{
			StandardFileReply	sfReply;
			SFTypeList			typeList = {'PICT', 'DIB ', 'BMP ', 'GIFf'};
			StandardGetFile(NULL,4,typeList,&sfReply);
			if (sfReply.sfGood) {
				SetText(dp, R_PictNameItem, sfReply.sfFile.name);
			}
			// The standard file dialog grays these buttons out...
			SetPort(dp);
			SelectWindow(dp);
			EnableButton(dp, OK);
			EnableButton(dp, Cancel);
			break;
		}
	}
	
	if (itemHit == OK && gConnectionType != C_None) 
	{
		GetText(dp,R_NameItem,temp);
		if (!EqualString(temp,gRoomInfo.oldName,true,true)) 
		{
			if (!gRoomInfo.inLockedFlag)
				rp->roomNameOfst = AddRoomString(temp);
			gRoomInfo.changeFlag = true;
		}
		
		GetText(dp,R_PictNameItem,temp);
		if (!EqualString(temp,gRoomInfo.oldPictName,true,true)) 
		{
			if (!gRoomInfo.inLockedFlag)
				rp->pictNameOfst = AddRoomString(temp);
			gRoomInfo.changeFlag = true;
		}
		
		if (gRoomInfo.flags != gRoomInfo.oldFlags) 
		{
			if (!gRoomInfo.inLockedFlag)
				rp->roomFlags = gRoomInfo.flags;
			gRoomInfo.changeFlag = true;
		}
		
		GetText(dp,R_ArtistItem,temp);
		if (!EqualString(temp,gRoomInfo.oldArtist,true,true)) 
		{
			if (!gRoomInfo.inLockedFlag) 
			{
				if (temp[0])
					rp->artistNameOfst = AddRoomString(temp);
				else
					rp->artistNameOfst = 0;
			}
			gRoomInfo.changeFlag = true;
		}
		
		t = GetIntItem(dp, R_IDItem);
		if (t > 0 && rp->roomID != t) 
		{
			if (!gRoomInfo.inLockedFlag)
				rp->roomID = t;
			gRoomInfo.changeFlag = true;
		}
		
		if (gRoomInfo.changeFlag) 
		{
			if (gRoomInfo.inLockedFlag)
				ErrorMessage("Sorry, the room is locked");
			else
				DoPalaceCommand(PC_SetRoomInfo, (long) &gRoomWin->curRoom, NULL);
		}
	}
	
	if (itemHit == OK || itemHit == Cancel) 
	{
		DisposDialog(dp);
		gRoomDialog = NULL;
		if (deleteFlag)
			DoPalaceCommand(PC_Chat, 0L, "`delete");
	}
}

/**
pascal Boolean DoorInfoFilter(DialogPtr dp, EventRecord *ep, short *itemHit);

pascal Boolean DoorInfoFilter(DialogPtr dp, EventRecord *ep, short *itemHit)
{
	char tempChar;
	switch (ep->what) {
	  case keyDown:
	  case autoKey:
		tempChar = ep->message & charCodeMask;
		if(tempChar==10 || tempChar==13 || tempChar==3) {
			if (gValidFlag)
				*itemHit = OK;
			else
				*itemHit = Cancel;
			SimulateButtonPress(dp,*itemHit);
			return true;
		}
		else if (tempChar == 27) {
			*itemHit = Cancel;
			SimulateButtonPress(dp,*itemHit);
			return true;
		}
		break;
	  case updateEvt:
		if (GetControl(dp, D_SpotTypeItem) != 1)
			DisableButton(dp, D_ScriptsItem);
		else
			EnableButton(dp, D_ScriptsItem);
		if (gValidFlag) {
			EnableButton(dp,OK);
			OutlineButton(dp,Cancel,&qd.white);
			OutlineButton(dp,OK,&qd.black);
		}
		else {
			DisableButton(dp,OK);
			OutlineButton(dp,OK,&qd.white);
			OutlineButton(dp,Cancel,&qd.black);
		}
		break;
	  default:
	  	break;

	}
	return false;
}
**/

short RoomListID(Ptr p, short idx)
{
	RoomListPtr	up;
	short		i;
	if (p == NULL)
		return 0;
	for (i = 0; i < gNbrRoomListMenuItems; ++i) {
		up = (RoomListPtr) p;
		if (i == idx)
			return up->roomID;
		p += 8 + up->name[0] + (4 - (up->name[0] & 3));		/* 9/6/95 */
	}
	return 0;
}

short RoomListIdx(Ptr p, short id);
short RoomListIdx(Ptr p, short id)
{
	RoomListPtr	up;
	short		i;
	if (p == NULL)
		return 0;
	for (i = 0; i < gNbrRoomListMenuItems; ++i) {
		up = (RoomListPtr) p;
		if (up->roomID == id)
			return i;
		p += 8 + up->name[0] + (4 - (up->name[0] & 3));	/* 9/6/95 */
	}
	return 0;
}

void ReceiveDoorDialogRoomList(long nbrRooms, Ptr p, long len);
void ReceiveDoorDialogRoomList(long nbrRooms, Ptr p, long len)
{
	RoomListPtr	up;
	short	i,n;
	if (gRoomListMenu == NULL) {
		gRoomListMenu = NewMenu(RoomListMENU,"\p");
		InsertMenu(gRoomListMenu,hierMenu);
	}
	else {
		// Clear all entries
		n = CountMItems(gRoomListMenu);
		for (i = 0; i < n; ++i) {
			DelMenuItem(gRoomListMenu, 1);
		}
		gNbrRoomListMenuItems = 0;
	}
	if (gRoomListPtr)
		DisposePtr(gRoomListPtr);
	gRoomListPtr = NewPtrClear(len);		/* 7/1/96 JAB Changed to NewPtrClear */
	if (gRoomListPtr) {
		BlockMove(p,gRoomListPtr,len);
		gNbrRoomListMenuItems = nbrRooms;
		// Append Room Names to menu
		for (i = 0; i < nbrRooms; ++i) {
			up = (RoomListPtr) p;
			AppendMenu(gRoomListMenu, (StringPtr) up->name);
			p += 8 + up->name[0] + (4 - (up->name[0] & 3));
		}
	}
}

struct DoorInfoVars {
	HotspotPtr	door;
	Str255	oldName;
	short	oldFlags,flags;
	Boolean	changeFlag,stateChange,inLockedFlag;
} gDoorInfo;

void DoorInfoDialog()
{
	GrafPtr		savePort;
	DialogPtr 	dp;
	HotspotPtr hsl = (HotspotPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];

	if (gRoomWin->curHotspot == 0)
		return;

	gDoorInfo.changeFlag = false;
	gDoorInfo.stateChange = false;

	if (gRoomListMenu == NULL)
		PostServerEvent(MSG_LISTOFALLROOMS,gRoomWin->meID,NULL,0L);

	gDoorInfo.door = &hsl[gRoomWin->curHotspot-1];

	GetPort(&savePort);

	if ((dp = GetNewDialog (DoorInfoDLOG, NULL, (WindowPtr) -1)) == NULL)
		return;
	gSpotDialog = dp;
	
	GetSpotName(gDoorInfo.door, gDoorInfo.oldName);
	SetText(dp, D_NameItem, gDoorInfo.oldName);

	PrintfItem(dp, D_IDItem, "%d", gDoorInfo.door->id);
	PrintfItem(dp, D_DestItem, "%d", gDoorInfo.door->dest);
	PrintfItem(dp, D_KeyPtsItem, "%d", gDoorInfo.door->nbrPts);
	gDoorInfo.oldFlags = gDoorInfo.door->flags;
	gDoorInfo.flags = gDoorInfo.door->flags;
	SetControl(dp, D_SpotTypeItem, gDoorInfo.door->type+1);
	SetControl(dp, D_SpotStateItem, gDoorInfo.door->state+1);
	// SetControl(dp, D_DontMoveHere, (flags & HS_DontMoveHere) > 0);
	// SetControl(dp, D_Draggable, (flags & HS_Draggable) > 0);
		
	SelIText(dp,D_IDItem,0,32767);
	gDoorInfo.inLockedFlag = (gRoomWin->curRoom.roomFlags & RF_AuthorLocked) > 0;

	SetDialogDefaultItem(dp, OK);
	SetDialogCancelItem(dp, Cancel);
	SetDialogTracksCursor(dp,true);

	ShowWindow(dp);
//	SetPort(dp);
//	SetCursor(&qd.arrow);
//	ShowCursor();
	SetPort(savePort);
}

void ChangeDoorKeyPts(HotspotPtr door, short t);
void ChangeDoorKeyPts(HotspotPtr door, short t)
{
	Point	*pPtr,ptList[12],lp,fp,dp;
	short	i,n;

	if (t == door->nbrPts)
		return;
	if (t < door->nbrPts) {
		door->nbrPts = t;
		return;
	}
	pPtr = (Point *) &gRoomWin->curRoom.varBuf[door->ptsOfst];
	BlockMove(pPtr, &ptList[0], door->nbrPts * sizeof(Point));
	n = t - door->nbrPts;
	lp = ptList[door->nbrPts-1];
	fp = ptList[0];
	dp.h = (fp.h -  lp.h) / (n + 1);
	dp.v = (fp.v -  lp.v) / (n + 1);
	for (i = door->nbrPts; i < t; ++i) {
		ptList[i].h = lp.h + dp.h * (1 + (i-door->nbrPts));
		ptList[i].v = lp.v + dp.v * (1 + (i-door->nbrPts));
	}
	door->ptsOfst = AddRoomBuffer((Ptr) &ptList[0],sizeof(Point)*t);
	door->nbrPts = t;
}


void SpotDialogHit(EventRecord *theEvent, short itemHit)
{
		// ModalDialog(filterProc, &itemHit);
	DialogPtr	dp = gSpotDialog;
	Str255		newName;
	short		t;
	Boolean		scriptFlag = false,pictFlag = false,deleteFlag=false;
	
	switch (itemHit) {
	case D_DeleteItem:
		if (!((gRoomWin->curRoom.roomFlags & RF_AuthorLocked) > 0)) {
			deleteFlag = true;
			itemHit = Cancel;
		}
		break;
	case D_PictsItem:
		itemHit = OK;
		pictFlag = true;
		break;
	case D_ScriptsItem:
		itemHit = OK;
		scriptFlag = true;
		break;
	case D_SpotTypeItem:
	case D_SpotStateItem:
		InvalRect(&dp->portRect);
		break;
	case D_DestMenuItem:
		if (gRoomListMenu && gNbrRoomListMenuItems) {
			short	t;
			Handle	h;
			Rect	r;
			long	menuSelect;
			short	roomIdx;
			short	defIdx=0,destRoomID=0;
			Point	p;
			GrafPtr	savePort;
			GetPort(&savePort);
			SetPort(dp);
			destRoomID = GetIntItem(dp, D_DestItem);
			defIdx = RoomListIdx(gRoomListPtr, destRoomID)+1;
			if (defIdx > gNbrRoomListMenuItems)
				defIdx = gNbrRoomListMenuItems;
			else if (defIdx < 1)
				defIdx = 1;
			GetDItem(dp,D_DestMenuItem,&t,&h,&r);
			p = topLeft(r);
			LocalToGlobal(&p);
			menuSelect = PopUpMenuSelect(gRoomListMenu,p.v+7,p.h+22,defIdx);
			roomIdx = LoWord(menuSelect);
			if (roomIdx > 0) {
				PutIntItem(dp, D_DestItem, RoomListID(gRoomListPtr, roomIdx-1));
			}
			SetPort(savePort);
		}
		break;
	case D_OptionsItem:
		{
			short	t;
			Handle	h;
			Rect	r;
			Point	p;
			long	menuSelect;
			GrafPtr	savePort;
			GetPort(&savePort);
			SetPort(dp);
			GetDItem(dp,D_OptionsItem,&t,&h,&r);
			p = topLeft(r);
			LocalToGlobal(&p);
			gSpotOptionsMenu = GetMenu(SpotOptionsMENU);
			InsertMenu(gSpotOptionsMenu, hierMenu);
			if (gSpotOptionsMenu) {
				CheckItem(gSpotOptionsMenu, SO_DontMoveHere, (gDoorInfo.flags & HS_DontMoveHere) > 0);
				CheckItem(gSpotOptionsMenu, SO_Draggable, (gDoorInfo.flags & HS_Draggable) > 0);
				CheckItem(gSpotOptionsMenu, SO_ShowName, (gDoorInfo.flags & HS_ShowName) > 0);
				CheckItem(gSpotOptionsMenu, SO_ShowFrame, (gDoorInfo.flags & HS_ShowFrame) > 0);
				CheckItem(gSpotOptionsMenu, SO_Shadow, (gDoorInfo.flags & HS_Shadow) > 0);
				CheckItem(gSpotOptionsMenu, SO_Fill, (gDoorInfo.flags & HS_Fill) > 0);
				menuSelect = PopUpMenuSelect(gSpotOptionsMenu,p.v+7,p.h+22,1);
				switch (LoWord(menuSelect)) {
				case SO_DontMoveHere:	gDoorInfo.flags ^= HS_DontMoveHere;	break;
				case SO_Draggable:		gDoorInfo.flags ^= HS_Draggable;		break;
				case SO_ShowName:		gDoorInfo.flags ^= HS_ShowName;		break;
				case SO_ShowFrame:		gDoorInfo.flags ^= HS_ShowFrame;		break;
				case SO_Shadow:			gDoorInfo.flags ^= HS_Shadow;			break;
				case SO_Fill:			gDoorInfo.flags ^= HS_Fill;			break;
				}
				DeleteMenu(SpotOptionsMENU);
				DisposeMenu(gSpotOptionsMenu);
			}
			SetPort(savePort);
		}
	}
	if (itemHit == OK && gConnectionType != C_None) {
		t = GetIntItem(dp, D_KeyPtsItem);
		if (t != gDoorInfo.door->nbrPts) {
			if (t >= 3 && t < 10) {
				ChangeDoorKeyPts(gDoorInfo.door, t);
				gDoorInfo.changeFlag = true;
			}
		}

		t = GetIntItem(dp, D_IDItem);
		if (t != gDoorInfo.door->id) {
			gDoorInfo.door->id = t;
			gDoorInfo.changeFlag = true;
		}
		t = GetIntItem(dp, D_DestItem);
		if (t != gDoorInfo.door->dest) {
			gDoorInfo.door->dest = t;
			gDoorInfo.changeFlag = true;
		}
		if (GetControl(dp, D_SpotTypeItem)-1 != gDoorInfo.door->type) {
			gDoorInfo.door->type = GetControl(dp, D_SpotTypeItem) - 1;
			gDoorInfo.changeFlag = true;
		}
		GetText(dp, D_NameItem, newName);
		if (!EqualString(gDoorInfo.oldName,newName,true,true)) {
			gDoorInfo.changeFlag = true;
			if (newName[0] == 0)
				gDoorInfo.door->nameOfst = 0;
			else
				gDoorInfo.door->nameOfst = AddRoomString(newName);
		}
		if (gDoorInfo.flags != gDoorInfo.oldFlags) {
			gDoorInfo.door->flags = gDoorInfo.flags;
			gDoorInfo.changeFlag = true;
		}
		gDoorInfo.stateChange = GetControl(dp, D_SpotStateItem)-1 != gDoorInfo.door->state;
		if (gDoorInfo.inLockedFlag && (gDoorInfo.changeFlag || gDoorInfo.stateChange))
			ErrorMessage("Sorry, the room is locked");
		else {
			if (gDoorInfo.changeFlag) {
				DoPalaceCommand(PC_SetRoomInfo, (long) &gRoomWin->curRoom, NULL);
			}
			if (gDoorInfo.stateChange) {
				short	args[3];
				args[0] = gRoomWin->curRoom.roomID;
				args[1] = gDoorInfo.door->id;
				args[2] = GetControl(dp, D_SpotStateItem) - 1;
				DoPalaceCommand(PC_SetSpotState,(long) &args[0],NULL);
			}
		}
	}
	if (itemHit == OK || itemHit == Cancel) {
		gSpotDialog = NULL;
		DisposDialog(dp);
		if (scriptFlag && !gDoorInfo.inLockedFlag)
			NewScriptEditor();	// Load Correct Hotspot...
		if (pictFlag && !gDoorInfo.inLockedFlag) {
			void NewPLWindow(void);
			NewPLWindow();
		}
		if (deleteFlag) {
			DoPalaceCommand(PC_DeleteSpot, (long) gDoorInfo.door->id, NULL);
			gRoomWin->curHotspot = 0;
			RefreshRoom(&gOffscreenRect);
		}
	}
}

void SwitchModes(short mode)
{
	short	lastMode = gMode;
	gMode = mode;
	switch (gMode) {
	case M_Authoring:
		RefreshRoom(&gOffscreenRect);
		break;
	case M_Normal:
		if (lastMode == M_Authoring) {
			KillAuthoring();
			gRoomWin->curHotspot = 0;
			DefaultRefresh((WindowPtr) gRoomWin);
		}
		RefreshRoom(&gOffscreenRect);
		break;
	}
}

#define PolyHandleRadius	3

void DrawPolyHandle(Point p)
{
	Rect	r;
	topLeft(r) = p;
	botRight(r) = p;
	InsetRect(&r,-PolyHandleRadius,-PolyHandleRadius);
	PaintRect(&r);
}

void DrawSpotFrame(HotspotPtr hs, short drawMode, Boolean showHandles)
{
	short		i;
	Point		*pPtr,p;
	short		nbrPts = hs->nbrPts;

	// Make hotspot accessible if you it's bugged...

	PenMode(drawMode);

	if (hs->nbrPts <= 2) {
		Point	pAry[] = {0,0,0,100,100,100,100,0};
		pPtr = &pAry[0];
		hs->loc.h = 50;
		hs->loc.v = 50;
		nbrPts = 4;
		// return;
	}
	else
		pPtr = (Point *) &gRoomWin->curRoom.varBuf[hs->ptsOfst];

	if (showHandles) {
		p = pPtr[0];
		AddPt(hs->loc,&p);
		DrawPolyHandle(p);
	}
	for (i = 1; i < nbrPts; ++i) {
		if (showHandles) {
			p = pPtr[i];
			AddPt(hs->loc,&p);
			DrawPolyHandle(p);
		}
		MoveTo(pPtr[i-1].h+hs->loc.h,pPtr[i-1].v+hs->loc.v);
		LineTo(pPtr[i].h+hs->loc.h,pPtr[i].v+hs->loc.v);
	}
	LineTo(pPtr[0].h+hs->loc.h,pPtr[0].v+hs->loc.v);
	PenNormal();
}

void DrawSpotFrameOnscreen(HotspotPtr hs, Boolean showHandles)
{
	short	i;
	Point		*pPtr,p;

	PenMode(patXor);
	pPtr = (Point *) &gRoomWin->curRoom.varBuf[hs->ptsOfst];
	if (showHandles) {
		p = pPtr[0];
		AddPt(hs->loc,&p);
		AddPt(topLeft(gVideoRoomRect),&p);
		DrawPolyHandle(p);
	}
	for (i = 1; i < hs->nbrPts; ++i) {
		if (showHandles) {
			p = pPtr[i];
			AddPt(hs->loc,&p);
			AddPt(topLeft(gVideoRoomRect),&p);
			DrawPolyHandle(p);
		}
		MoveTo(pPtr[i-1].h+hs->loc.h+gVideoRoomRect.left,pPtr[i-1].v+hs->loc.v+gVideoRoomRect.top);
		LineTo(pPtr[i].h+hs->loc.h+gVideoRoomRect.left,pPtr[i].v+hs->loc.v+gVideoRoomRect.top);
	}
	LineTo(pPtr[0].h+hs->loc.h+gVideoRoomRect.left,pPtr[0].v+hs->loc.v+gVideoRoomRect.top);
	PenNormal();
}


void RefreshSpotFrames()
{
	short		i;
	HotspotPtr	hsl;
	Boolean		allSpots = (gMode == M_Authoring);
	hsl = (HotspotPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];
	for (i = 0; i < gRoomWin->curRoom.nbrHotspots; ++i,++hsl) {
		if (allSpots || hsl->type != HS_Normal)
			DrawSpotFrame(hsl,patXor,i+1 == gRoomWin->curHotspot);
	}
}

// 6/11/95
// Recompute loc of hotspot as average of points...
//
void ReCenterHotspot(HotspotPtr hs)
{
	Point	p={0,0},*pPtr;
	short	i;
	pPtr = (Point *) &gRoomWin->curRoom.varBuf[hs->ptsOfst];
    for (i = 0; i < hs->nbrPts; ++i,++pPtr) {
		pPtr->h += hs->loc.h;
		pPtr->v += hs->loc.v;
        p.h += pPtr->h;
        p.v += pPtr->v;
     }
     p.h /= hs->nbrPts;
     p.v /= hs->nbrPts;
     hs->loc = p;
	 pPtr = (Point *) &gRoomWin->curRoom.varBuf[hs->ptsOfst];
     for (i = 0; i < hs->nbrPts; ++i,++pPtr) {
		pPtr->h -= hs->loc.h;
		pPtr->v -= hs->loc.v;
     }
}

void ClickAuthoring(Point mp, Boolean shiftFlag)
{
	short	i,j;
	Point	p,np,*pPtr,tp;
	Rect	r;
	HotspotPtr hsl = (HotspotPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];
	HotspotPtr	door = NULL;
	StateRecPtr	sr;
	Boolean		inLockedFlag = (gRoomWin->curRoom.roomFlags & RF_AuthorLocked) > 0;
	short		spotIdx;

	Boolean	changeFlag = false;
	static long	lastClick;

	if (gRoomDialog || gSpotDialog)
		return;

	if (PtInRect(mp,&gVideoRoomRect)) {
		mp.h -= gVideoRoomRect.left - gOffscreenRect.left;
		mp.v -= gVideoRoomRect.top - gOffscreenRect.top;

		// First check if dragging an anchor pt of current spot
		if (gRoomWin->curHotspot && !inLockedFlag) {
			door = &hsl[gRoomWin->curHotspot-1];
			pPtr = (Point *) &gRoomWin->curRoom.varBuf[door->ptsOfst];
			for (j = 0; j < door->nbrPts; ++j) {
				p = pPtr[j];
				AddPt(door->loc,&p);
				topLeft(r) = p;
				botRight(r) = p;
				InsetRect(&r,-PolyHandleRadius,-PolyHandleRadius);
				if (PtInRect(mp,&r)) {
					BeginPaintClipping();
					while (WaitMouseUp()) {
						GetMouse(&np);
						np.h -= gVideoRoomRect.left - gOffscreenRect.left;
						np.v -= gVideoRoomRect.top - gOffscreenRect.top;
						if (!EqualPt(np,mp)) {
							changeFlag = true;
							DrawSpotFrameOnscreen(door, true);
							pPtr[j].h += np.h - mp.h;
							pPtr[j].v += np.v - mp.v;
							if (pPtr[j].h+door->loc.h > gOffscreenRect.right)
								pPtr[j].h -= pPtr[j].h+door->loc.h - gOffscreenRect.right;
							if (pPtr[j].v+door->loc.v > gOffscreenRect.bottom)
								pPtr[j].v -= pPtr[j].v+door->loc.v - gOffscreenRect.bottom;
							if (pPtr[j].h+door->loc.h < gOffscreenRect.left)
								pPtr[j].h += gOffscreenRect.left - (pPtr[j].h+door->loc.h);
							if (pPtr[j].v+door->loc.v < gOffscreenRect.top)
								pPtr[j].v += gOffscreenRect.top - (pPtr[j].v+door->loc.v);
							DrawSpotFrameOnscreen(door, true);
							mp = np;
						}
					}
					EndPaintClipping();
					if (changeFlag) {
						// Recompute Spot Points and Loc so that
						// loc is within hotspot...
						ReCenterHotspot(door);
						DoPalaceCommand(PC_SetRoomInfo, (long) &gRoomWin->curRoom, NULL);
					}
					// !! Tell Server About New Point
					return;
				}
			}
		}
	
		// Find which spot
		door = NULL;
		spotIdx = 0;
		for (i = 0; i < gRoomWin->curRoom.nbrHotspots; ++i) {
			if (PtInHotspot(mp,&hsl[i])) {
				// User wandered into hotspot
				door = &hsl[i];
				spotIdx = i;
				// break;			7/24/95 - get foremost hotspot
			}
		}
		if (door == NULL) {
			if (gRoomWin->curHotspot) {
				gRoomWin->curHotspot = 0;
				RefreshRoom(&gOffscreenRect);
			}
			return;
		}		
		else {
			sr = (StateRecPtr) &gRoomWin->curRoom.varBuf[door->stateRecOfst];
			if (spotIdx+1 != gRoomWin->curHotspot) {
				gRoomWin->curHotspot = spotIdx+1;
				RefreshRoom(&gOffscreenRect);
				lastClick = TickCount();
			}
			else {
				Rect	br;
				if (TickCount() - lastClick < GetDblTime()) {
					DoorInfoDialog();
					return;
				}
				lastClick = TickCount();

				if (inLockedFlag)
					return;
				br = (**((RgnHandle) door->refCon)).rgnBBox;
				OffsetRect(&br,-door->loc.h,-door->loc.v);
				// Drag Spot
				BeginPaintClipping();
				while (WaitMouseUp()) {
					GetMouse(&np);
					np.h -= gVideoRoomRect.left - gOffscreenRect.left;
					np.v -= gVideoRoomRect.top - gOffscreenRect.top;
					if (!EqualPt(np,mp)) {
						changeFlag = true;
						DrawSpotFrameOnscreen(door, true);
						tp = door->loc;
						tp.h += np.h - mp.h;
						tp.v += np.v - mp.v;
						if (tp.h+br.right > gOffscreenRect.right)
							tp.h -= tp.h+br.right - gOffscreenRect.right;
						if (tp.v+br.bottom > gOffscreenRect.bottom)
							tp.v -= tp.v+br.bottom - gOffscreenRect.bottom;
						if (tp.h+br.left < gOffscreenRect.left)
							tp.h += gOffscreenRect.left - (tp.h+br.left);
						if (tp.v+br.top < gOffscreenRect.top)
							tp.v += gOffscreenRect.top - (tp.v+br.top);
/*						if (sr[door->state].pictID) {*/
/*							sr[door->state].picLoc.h -= tp.h - door->loc.h;*/
/*							sr[door->state].picLoc.v -= tp.v - door->loc.v;*/
/*						}*/
						door->loc = tp;
						DrawSpotFrameOnscreen(door, true);
						mp = np;
					}
				}
				EndPaintClipping();
				if (changeFlag) {
					short	changeRec[4];
					changeRec[0] = gRoomWin->curRoom.roomID;
					changeRec[1] = door->id;
					changeRec[2] = door->loc.v;
					changeRec[3] = door->loc.h;
					DoPalaceCommand(PC_SetLoc, (long) &changeRec[0], NULL);
				}
			}
			return;
		}
	}
}

short KeyAuthoring(short code)
{
	HotspotPtr 	hsl;
	HotspotPtr	door;
	Boolean		inLockedFlag = (gRoomWin->curRoom.roomFlags & RF_AuthorLocked) > 0;

	switch (code) {
	case 0x33:	// delete
	case 0x75:
		if (!inLockedFlag && gRoomWin->curHotspot && !gRoomWin->msgActive) {
			hsl = (HotspotPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];
			door = &hsl[gRoomWin->curHotspot-1];
			if (door) {
				DoPalaceCommand(PC_DeleteSpot, (long) door->id, NULL);
				gRoomWin->curHotspot = 0;
				RefreshRoom(&gOffscreenRect);
			}
			return true;
		}
		break;
	}
	return 0;
}

void LayerSpot(short cmd)
{
	Boolean		inLockedFlag = (gRoomWin->curRoom.roomFlags & RF_AuthorLocked) > 0;
	HotspotPtr	hsl,spot;
	Hotspot		temp;
	short		n;
	Boolean		changeFlag=false;
	RoomRecPtr	rp = &gRoomWin->curRoom;
	
	if (inLockedFlag || gRoomWin->curHotspot == 0)
		return;

	hsl = (HotspotPtr) &rp->varBuf[rp->hotspotOfst];
	n = gRoomWin->curHotspot-1;
	spot = &hsl[n];
	
	switch (cmd) {
	case LM_MoveToBottom:
		if (n > 0) {
			temp = *spot;
			BlockMove(&hsl[0],&hsl[1],n*sizeof(Hotspot));
			hsl[0] = temp;
			changeFlag = true;
		}
		break;
	case LM_MoveBackward:
		if (n > 0) {
			temp = hsl[n];
			hsl[n] = hsl[n-1];
			hsl[n-1] = temp;
			changeFlag = true;
		}
		break;
	case LM_MoveForward:
		if (n < rp->nbrHotspots-1) {
			temp = hsl[n];
			hsl[n] = hsl[n+1];
			hsl[n+1] = temp;
			changeFlag = true;
		}
		break;
	case LM_MoveToTop:
		if (n < rp->nbrHotspots-1) {
			temp = *spot;
			BlockMove(&hsl[n+1],&hsl[n],(rp->nbrHotspots-(n+1))*sizeof(Hotspot));
			hsl[rp->nbrHotspots-1] = temp;
			changeFlag = true;
		}
		break;
	}
	if (changeFlag)
		DoPalaceCommand(PC_SetRoomInfo, (long) &gRoomWin->curRoom, NULL);
}

void KillAuthoring()
{

	if (gRoomDialog) {
		DisposeDialog(gRoomDialog);
		gRoomDialog = NULL;
	}
	if (gSpotDialog) {
		DisposeDialog(gSpotDialog);
		gSpotDialog= NULL;
	}
}