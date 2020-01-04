// UserListWindow.c
/***
	Controls for User List:
		[ ] All users checkbox, otherwise, only show room users
		Target for Private Message
		Update (shouldn't need - just update every 30 seconds or so...)
		Go

	todo: add room translation (ask for the roomlist, and use it)
***/
#include "U-USER.H"
#include "m-cmds.H"

#include "UserTools.h"	// 6/10/95

#define		ULWIND			136
#define		WinBackColor	gGrayAA

#define MAXUSERSORTS	3
short		gUserWindowSortType;	/* 0 = user, 1 = room, 2 = chrono */

typedef struct {
	ObjectWindowRecord	win;
	Rect			listRect;
	ListHandle		uList;
	Ptr				roomList,userList;
	UserListPtr		*sortIdx;
	short			nbrUsers,nbrRooms;
	Boolean			pendingList;
} ULWindowRecord, *ULWindowPtr;

WindowPtr	gULWin;
ULWindowPtr	gULPtr;

void ULResetUserList(void);

// Rebuild User List
void RefreshUserList()
{
	if (gULPtr) {
		PostServerEvent(MSG_LISTOFALLUSERS,gRoomWin->meID,NULL,0L);
		gULPtr->pendingList = true;
	}
}

// Create a new main window using a 'WIND' template from the resource fork
//
void NewULWindow()
{
	WindowPtr		theWindow;
	ULWindowPtr		ulRec;
	Rect			rDataBounds;
	Point			cellSize = {0,0};
	Point			toolP;
	short			nbrTools;

	PostServerEvent(MSG_LISTOFALLROOMS,gRoomWin->meID,NULL,0L);
	PostServerEvent(MSG_LISTOFALLUSERS,gRoomWin->meID,NULL,0L);

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 
	ulRec = (ULWindowPtr) NewPtrClear(sizeof(ULWindowRecord));
	theWindow = InitObjectWindow(ULWIND, (ObjectWindowPtr) ulRec,true);
	RestoreWindowPos(theWindow, &gMacPrefs.userListPos);
	gULWin = theWindow;
	gULPtr = (ULWindowPtr) theWindow;

	// Show the window
	ShowWindow(theWindow);
	SelectFloating(theWindow);

	// Make it the current grafport
	SetPort(theWindow);
	TextFont(monaco);
	TextSize(9);
	RGBBackColor(&WinBackColor);

	((ObjectWindowPtr) theWindow)->Draw = ULWindowDraw;
	((ObjectWindowPtr) theWindow)->Dispose = DisposeULWin;
	((ObjectWindowPtr) theWindow)->HandleClick = ULWindowClick;
	((ObjectWindowPtr) theWindow)->AdjustCursor = ULAdjustCursor;
	
	// Create List Manager Record & Buttons
	gULPtr->listRect = theWindow->portRect;
	InsetRect(&gULPtr->listRect,4,4);
	gULPtr->listRect.bottom -= 32;

	SetRect(&rDataBounds,0,0,1,0);

	gULPtr->listRect.right -= SBarSize;
	gULPtr->uList = LNew(&gULPtr->listRect, &rDataBounds, cellSize, 0, theWindow, false, false, 
				false, true);
	if (gULPtr->uList == NULL) {
		DisposeULWin(theWindow);
		return;
	}
	gULPtr->listRect.right += SBarSize;

	(*gULPtr->uList)->selFlags |= (lOnlyOne);

	SetToolBG(&gGray44);

	if ((gRoomWin->userFlags & U_God) > 0 ||
		(gRoomWin->serverInfo.serverPermissions & PM_PlayersMayKill) ||
		((gRoomWin->serverInfo.serverPermissions & PM_WizardsMayKill) && gExpert))
		nbrTools = 3;
	else
		nbrTools = 2;

	toolP.h = theWindow->portRect.right - ((ToolWidth+ToolWidth/2)*(nbrTools)+1);
	toolP.v = theWindow->portRect.bottom - (ToolHeight+1);
	AddTool(theWindow, CancelIcon, CancelHiIcon, 0,toolP.h, toolP.v);
	toolP.h += ToolWidth + ToolWidth/2;
	AddTool(theWindow, GoIcon, GoHiIcon, 0,toolP.h, toolP.v);
	if (nbrTools == 3) {
		toolP.h += ToolWidth + ToolWidth/2;
		AddTool(theWindow, KillIcon, KillHiIcon, 0, toolP.h,toolP.v);
	}


	gULPtr->pendingList = true;
}

void ULWindowDraw(WindowPtr theWindow)
{
	Rect	r;
	RGBForeColor(&gGray44);
	PaintRect(&theWindow->portRect);
	HiliteRect(&theWindow->portRect);

	r = gULPtr->listRect;
	InsetRect(&r,-1,-1);
	RevHiliteRect(&r);
	InsetRect(&r,-1,-1);
	RevHiliteRect(&r);
	InsetRect(&r,2,2);
	EraseRect(&r);
	
	// Render the List & Buttons
	if (gULPtr->uList && !gULPtr->pendingList)
		LUpdate(theWindow->visRgn,gULPtr->uList);
	RefreshTools(theWindow);
}

RoomListPtr GetRoomListEntry(long roomID);

RoomListPtr GetRoomListEntry(long roomID)
{
	RoomListPtr	rp;
	Ptr			p;
	short		i;
	p = gULPtr->roomList;
	if (p == NULL)
		return NULL;
	for (i = 0; i < gULPtr->nbrRooms; ++i) {
		rp = (RoomListPtr) p;
		if (rp->roomID == roomID) {
			return rp;
		}
		p += 8 + rp->name[0] + (4 - (rp->name[0] & 3));
	}
	return NULL;
}

long GetUserListID(short theLine);
long GetUserListID(short theLine)
{
	if (theLine < gULPtr->nbrUsers && theLine >= 0)
		return gULPtr->sortIdx[theLine]->userID;
	else
		return 0L;
}

Boolean GotoRoomInUserList(short theLine)
{
	UserListPtr	up;
	RoomListPtr	rp;

	if (theLine < 0 || theLine >= gULPtr->nbrUsers)
		return false;

	up = gULPtr->sortIdx[theLine];
	rp = GetRoomListEntry(up->roomID);

	if (rp && (!(rp->flags & (RF_Hidden | RF_Closed)) || gRoomWin->expert)) {
		DoPalaceCommand(PC_GotoRoom,(long) up->roomID,NULL);
		return true;
	}
	else
		return false;
}

LONG GetUserInList(short theLine, unsigned char *name);
LONG GetUserInList(short theLine, unsigned char *name)
{
	UserListPtr	up;
	if (theLine < gULPtr->nbrUsers && theLine >= 0) {
		up = gULPtr->sortIdx[theLine];
		BlockMove(up->name,name,up->name[0]+1);
		return up->userID;
	}
	else {
		name[0] = 0;
		return 0L;
	}
}


void ULWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent)
{
	short	toolNbr;
	Point	cell;
	// Check if clicked using list Manager
	SetPort(theWin);
	GlobalToLocal(&where);
	if (PtInRect(where,&gULPtr->listRect)) {
		if (theEvent->modifiers & cmdKey) {
			gUserWindowSortType++;
			if (gUserWindowSortType >= MAXUSERSORTS)
				gUserWindowSortType = 0;
			ULResetUserList();
			return;
		}
		if (LClick(where,theEvent->modifiers, gULPtr->uList))
		{
			// Process Double Click
			cell = LLastClick(gULPtr->uList);
			if (GotoRoomInUserList(cell.v))
				DisposeULWin(theWin);
		}
		else {
			LONG			userID=0;
			unsigned char	name[32]="";
			// Target user for private chat...
			cell = LLastClick(gULPtr->uList);
			userID = GetUserInList(cell.v, name);
			if (gRoomWin->userFlags & U_Guest)
				PrivateChatSelect(0, "\p");
			else
				PrivateChatSelect(userID, name);
		}
	}
	else if (ToolClick(where,&toolNbr)) {
		switch (toolNbr) {
		case 0:		DisposeULWin(theWin);	break;
		case 1:		
			cell.h = cell.v = 0;
			LGetSelect(true,&cell,gULPtr->uList);
			if (GotoRoomInUserList(cell.v))
				DisposeULWin(theWin);
			break;
		case 2:
			// Kill!!!
			{
				long	userID=0;
				cell.h = cell.v = 0;
				LGetSelect(true,&cell,gULPtr->uList);
				userID = GetUserListID(cell.v);
				if (userID) {
					DoPalaceCommand(PC_KillUser, userID, NULL);
					DisposeULWin(theWin);
				}
			}
			break;
		}
	}
}

void ULAdjustCursor(WindowPtr theWin, Point where, EventRecord *theEvent)
{
	if (gULPtr->pendingList)
		SpinCursor();
	else
		SetCursor(&qd.arrow);

}

void DisposeULWin(WindowPtr theWin)
{
	if (gULPtr->uList)
		LDispose(gULPtr->uList);
	if (gULPtr->roomList)
		DisposePtr(gULPtr->roomList);
	if (gULPtr->userList)
		DisposePtr(gULPtr->userList);
	if (gULPtr->sortIdx)
		DisposePtr((Ptr) gULPtr->sortIdx);
	SaveWindowPos(theWin, &gMacPrefs.userListPos);
	gULWin = NULL;
	gULPtr = NULL;
	DefaultDispose(theWin);
	DisposePtr((Ptr) theWin);
}

int UserListCompare(const void *e1, const void *e2);
int UserListCompare(const void *e1, const void *e2)
{
	UserListPtr	up1,up2;
	RoomListPtr	rp1,rp2;
	Boolean	isPrivate1,isPrivate2;

	up1 = *((UserListPtr *) e1);
	up2 = *((UserListPtr *) e2);
	switch (gUserWindowSortType) {
	case 0:		if ((up1->flags & U_Guest) == (up2->flags & U_Guest)) {
					if (up1->flags & U_Guest)
						return up1->userID - up2->userID;
					else
						return RelString((StringPtr) up1->name,(StringPtr) up2->name,false,false);
				}
				else
					return (up1->flags & U_Guest) - (up2->flags & U_Guest);
				break;
    case 1:		
				rp1 = GetRoomListEntry(up1->roomID);
				rp2 = GetRoomListEntry(up2->roomID);
				isPrivate1 = (rp1 == NULL || (rp1->flags & (RF_Closed | RF_Hidden)));
				isPrivate2 = (rp2 == NULL || (rp2->flags & (RF_Closed | RF_Hidden)));
				if (isPrivate2 && isPrivate1) {
					return RelString((StringPtr) up1->name,(StringPtr) up2->name,false,false);
				}
				else if (isPrivate1)
					return 1;
				else if (isPrivate2)
					return -1;
				else if (up1->roomID == up2->roomID)
					return RelString((StringPtr) up1->name,(StringPtr) up2->name,false,false);
				else
					return up1->roomID - up2->roomID;
    default:	return up1->userID - up2->userID;
	}
	// Return *e1 - *e2;
}

void SortUserList();
void SortUserList()
{
	if (gULWin == NULL || gULPtr == NULL)
		return;
	qsort(gULPtr->sortIdx, gULPtr->nbrUsers, sizeof(UserListPtr), UserListCompare);
}

void ULResetUserList(void)
{
	Ptr			pp;
	Point		theCell;
	char		buffer[128];
	short		i;
	UserListPtr	up;
	RoomListPtr	rp;

	LDoDraw(false,gULPtr->uList);
	LDelRow(0,0,gULPtr->uList);

	LDoDraw(false,gULPtr->uList);
	LDelRow(0,0,gULPtr->uList);

	pp = gULPtr->userList;
	for (i = 0; i < gULPtr->nbrUsers; ++i) {
		up = (UserListPtr) pp;
		gULPtr->sortIdx[i] = up;
		pp += 8 + up->name[0] + (4 - (up->name[0] & 3));
	}

	SortUserList();
	for (i = 0; i < gULPtr->nbrUsers; ++i) {
		up = gULPtr->sortIdx[i];
		LAddRow(1,i,gULPtr->uList);
		theCell.h = 0;
		theCell.v = i;
		rp = GetRoomListEntry(up->roomID);
		if (rp) {
			if (rp->flags & RF_Closed)
				sprintf(buffer,"%-22.*s (closed)",up->name[0],&up->name[1]);
			// 11/27/95 Private Rooms don't list as "Private"
			else if (rp->flags & (RF_Hidden))
				sprintf(buffer,"%-22.*s (private)",up->name[0],&up->name[1]);
			else
				sprintf(buffer,"%-22.*s %.*s",up->name[0],&up->name[1],rp->name[0],&rp->name[1]);
		}
		else {
			if (up->roomID)
				sprintf(buffer,"%-22.*s (private)",up->name[0],&up->name[1]);
			else
				sprintf(buffer,"%-22.*s (arriving)",up->name[0],&up->name[1]);
		}
		LSetCell(buffer, strlen(buffer), theCell, gULPtr->uList);
	}
	LDoDraw(true,gULPtr->uList);
	DefaultRefresh(gULWin);
}

void ULWindowReceiveUserList(long nbrUsers, Ptr	p, long len)
{

	if (gULWin == NULL || gULPtr == NULL)
		return;
	gULPtr->pendingList = false;
	if (gULPtr->userList)
		DisposePtr(gULPtr->userList);
	if (gULPtr->sortIdx)
		DisposePtr((Ptr) gULPtr->sortIdx);
	gULPtr->sortIdx = (UserListPtr *) NewPtrClear(sizeof(UserListPtr) * nbrUsers);
	if (gULPtr->sortIdx == NULL)
		return;
	gULPtr->userList = NewPtrClear(len);		/* 7/1/96 JAB Changed to NewPtrClear */
	if (gULPtr->userList == NULL)
		return;
	BlockMove(p,gULPtr->userList,len);
	gULPtr->nbrUsers = nbrUsers;
	ULResetUserList();
}

// !! What happens when we recieve list multiple times??
void ULWindowReceiveRoomList(long nbrRooms, Ptr	p, long len)
{
	if (gULWin == NULL || gULPtr == NULL)
		return;
	if (gULPtr->roomList)
		DisposePtr(gULPtr->roomList);
	gULPtr->roomList = NewPtrClear(len);		/* 7/1/96 JAB Changed to NewPtrClear */
	if (gULPtr->roomList == NULL)
		return;
	BlockMove(p,gULPtr->roomList,len);
	gULPtr->nbrRooms = nbrRooms;
	// Process the list, if need be...
}


void ProcessUserList(long nbrUsers, Ptr	userList, long len)
{
	if (gULWin)
		ULWindowReceiveUserList(nbrUsers, userList, len);
}

