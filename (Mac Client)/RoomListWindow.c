// RoomListWindow.c
// Note: Bug!!
#include "U-USER.H"
#include "m-cmds.H"
#include "UserTools.h"

// checkbox to show occupied rooms only...

#define		RLWIND			137
#define     WinBackColor	gGrayAA

typedef struct RLEntry {
	long		roomID;
	short		flags;
	short		nbrUsers;
	char		name[1];
} RoomList;

typedef struct {
	ObjectWindowRecord	win;
	Rect			listRect;
	ListHandle		uList;
	Ptr				roomList;
	short			nbrRooms;
	Boolean			pendingList;
} RLWindowRecord, *RLWindowPtr;

WindowPtr	gRLWin;
RLWindowPtr	gRLPtr;

void RefreshRoomList()
{
	if (gRLPtr) {
		PostServerEvent(MSG_LISTOFALLROOMS,gRoomWin->meID,NULL,0L);
		gRLPtr->pendingList = true;
	}
}


// Create a new main window using a 'WIND' template from the resource fork
//
void NewRLWindow()
{
	WindowPtr		theWindow;
	RLWindowPtr		ulRec;
	Rect			rDataBounds;
	Point			cellSize = {0,0};
	Point			toolP;

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 

	// 10/17/95
	PostServerEvent(MSG_LISTOFALLROOMS,gRoomWin->meID,NULL,0L);

	ulRec = (RLWindowPtr) NewPtrClear(sizeof(RLWindowRecord));
	theWindow = InitObjectWindow(RLWIND, (ObjectWindowPtr) ulRec,true);
	RestoreWindowPos(theWindow, &gMacPrefs.roomListPos);

	// Show the window
	ShowWindow(theWindow);
	SelectFloating(theWindow);

	// Make it the current grafport

	SetPort(theWindow);

	TextFont(monaco);
	TextSize(9);
	RGBBackColor(&WinBackColor);

	gRLWin = theWindow;
	gRLPtr = (RLWindowPtr) theWindow;

	((ObjectWindowPtr) theWindow)->Draw = RLWindowDraw;
	((ObjectWindowPtr) theWindow)->Dispose = DisposeRLWin;
	((ObjectWindowPtr) theWindow)->HandleClick = RLWindowClick;
	((ObjectWindowPtr) theWindow)->AdjustCursor = RLAdjustCursor;
	
	// Create List Manager Record & Buttons
	gRLPtr->listRect = theWindow->portRect;
	InsetRect(&gRLPtr->listRect,4,4);
	gRLPtr->listRect.bottom -= 32;

	SetRect(&rDataBounds,0,0,1,0);

	gRLPtr->listRect.right -= SBarSize;
	gRLPtr->uList = LNew(&gRLPtr->listRect, &rDataBounds, cellSize, 0, theWindow, false, false, 
				false, true);
	if (gRLPtr->uList == NULL) {
		DisposeRLWin(theWindow);
		return;
	}
	gRLPtr->listRect.right += SBarSize;

	(*gRLPtr->uList)->selFlags |= (lOnlyOne);
	// (*gRLPtr->uList)->selFlags |= (lUseSense | lNoRect | lNoExtend);

	toolP.h = theWindow->portRect.right - (ToolWidth*3+1);
	toolP.v = theWindow->portRect.bottom - (ToolHeight+1);
	SetToolBG(&gGray44);
	AddTool(theWindow, CancelIcon, CancelHiIcon, 0, toolP.h, toolP.v);
	AddTool(theWindow, GoIcon, GoHiIcon, 0, toolP.h + ToolWidth+ToolWidth/2, toolP.v);

	gRLPtr->pendingList = true;
}

void RLWindowDraw(WindowPtr theWindow)
{
	Rect	r;

	ColorPaintRect(&theWindow->portRect,&gGray44);
	HiliteRect(&theWindow->portRect);

	r = gRLPtr->listRect;
	InsetRect(&r,-1,-1);
	RevHiliteRect(&r);
	InsetRect(&r,-1,-1);
	RevHiliteRect(&r);
	InsetRect(&r,2,2);
	EraseRect(&r);
	
	// Render the List & Buttons
	// Crashola on this routine!!
	if (gRLPtr->uList && !gRLPtr->pendingList)
		LUpdate(theWindow->visRgn,gRLPtr->uList);

	RefreshTools(theWindow);
}

Boolean GotoRoomInRoomList(short theLine)
{
	RoomListPtr	up;
	Ptr			p;
	short		i,n;
	p = gRLPtr->roomList;
	for (i = n = 0; i < gRLPtr->nbrRooms; ++i) {
		up = (RoomListPtr) p;
		if (!(up->flags & RF_Hidden) || (gRoomWin->userFlags & U_SuperUser)) {
			if (n == theLine && (!(up->flags & RF_Closed) || gRoomWin->expert)) {
				DoPalaceCommand(PC_GotoRoom,(long) up->roomID,NULL);
				return true;
			}
			++n;
		}
		p += 8 + up->name[0] + (4 - (up->name[0] & 3));	// 9/5/95
	}
	return false;
}

void RLWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent)
{
	short	toolNbr;
	Point	cell;

	// Check if clicked using list Manager
	SetPort(theWin);
	GlobalToLocal(&where);

	if (PtInRect(where,&gRLPtr->listRect)) {
		if (theEvent->modifiers & cmdKey) {
			RefreshRoomList();
			return;
		}
		if (LClick(where,theEvent->modifiers, gRLPtr->uList))
		{
			// Process Double Click
			cell = LLastClick(gRLPtr->uList);
			if (GotoRoomInRoomList(cell.v))
				DisposeRLWin(theWin);
		}
	}
	else if (ToolClick(where,&toolNbr)) {
		switch (toolNbr) {
		case 0:		DisposeRLWin(theWin);	break;
		case 1:		
			cell.h = cell.v = 0;
			LGetSelect(true,&cell,gRLPtr->uList);
			// DoPalaceCommand(PC_GotoRoom,(long) 104L,NULL);
			if (GotoRoomInRoomList(cell.v))
				DisposeRLWin(theWin);
			break;
		}
	}
}

void RLAdjustCursor(WindowPtr theWin, Point where, EventRecord *theEvent)
{
	if (gRLPtr->pendingList)
		SpinCursor();
	else
		SetCursor(&qd.arrow);
}

void DisposeRLWin(WindowPtr theWin)
{
	if (gRLPtr->uList)
		LDispose(gRLPtr->uList);
	if (gRLPtr->roomList)
		DisposePtr(gRLPtr->roomList);
	SaveWindowPos(theWin, &gMacPrefs.roomListPos);
	gRLWin = NULL;
	gRLPtr = NULL;
	DefaultDispose(theWin);
	DisposePtr((Ptr) theWin);
}

void RLWindowReceiveRoomList(long nbrRooms, Ptr	p, long len)
{
	short		i,rowNumber=0;
	RoomListPtr	up;
	Point		theCell;
	char		buffer[128],occupancy[32];

	if (gRLWin == NULL || gRLPtr == NULL)
		return;
	SetPort(gRLWin);
	gRLPtr->pendingList = 0;
	if (gRLPtr->roomList)
		DisposePtr(gRLPtr->roomList);
	gRLPtr->roomList = NewPtrClear(len);	/* 7/1/96 JAB Changed to NewPtrClear */
	if (gRLPtr->roomList == NULL)
		return;
	BlockMove(p,gRLPtr->roomList,len);
	LDoDraw(false,gRLPtr->uList);
	LDelRow(0,0,gRLPtr->uList);
	for (i = 0; i < nbrRooms; ++i) {
		up = (RoomListPtr) p;
		if (!(up->flags & RF_Hidden) || (gRoomWin->userFlags & U_SuperUser)) {
			rowNumber = LAddRow(1,rowNumber,gRLPtr->uList);
			theCell.h = 0;
			theCell.v = rowNumber;

			if (up->flags & RF_Closed)
				sprintf(occupancy,"(closed)");
			else if (up->flags & RF_Hidden)
				sprintf(occupancy,"(hidden)");
			else if (up->flags & RF_Private)
				sprintf(occupancy,"-");
			else
				sprintf(occupancy,"%2d",up->nbrUsers);

			sprintf(buffer,"%-31.*s %s",up->name[0],&up->name[1],occupancy);

			LSetCell(buffer, strlen(buffer), theCell, gRLPtr->uList);
			++rowNumber;
		}
		p += 8 + up->name[0] + (4 - (up->name[0] & 3));	// 9/5/95
	}
	LDoDraw(true,gRLPtr->uList);
	gRLPtr->nbrRooms = nbrRooms;
	// !! Process the list
	DefaultRefresh(gRLWin);
}

void ProcessRoomList(long nbrRooms, Ptr	roomList, long len)
{
	void ReceiveDoorDialogRoomList(long nbrRooms, Ptr p, long len);

	if (gRLWin)
		RLWindowReceiveRoomList(nbrRooms, roomList, len);
	if (gULWin)
		ULWindowReceiveRoomList(nbrRooms, roomList, len);

	// if (gDoorDialog)
	ReceiveDoorDialogRoomList(nbrRooms, roomList, len);
}

