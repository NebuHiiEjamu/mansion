// RoomListWindow.c
// Note: Bug!!
#include "U-USER.H"
#include "UserTools.h"

// checkbox to show occupied rooms only...

#define		RLWIND			137
#define     WinBackColor	gGrayAA

#define XXX	0

typedef struct {
	ObjectWindowRecord	win;
} RLWindowRecord, *RLWindowPtr;

WindowPtr	gRLWin;
RLWindowPtr	gRLPtr;

void RefreshRoomList()
{
}


// Create a new main window using a 'WIND' template from the resource fork
//
void NewRLWindow()
{
	WindowPtr		theWindow;
	RLWindowPtr		ulRec;
	Point			toolP;

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 

	ulRec = (RLWindowPtr) NewPtrClear(sizeof(RLWindowRecord));
	theWindow = InitObjectWindow(RLWIND, (ObjectWindowPtr) ulRec,true);

	// Show the window
	ShowWindow(theWindow);
	SelectFloating(theWindow);

	// Make it the current grafport

	SetPort(theWindow);

	gRLWin = theWindow;
	gRLPtr = (RLWindowPtr) theWindow;

	((ObjectWindowPtr) theWindow)->Draw = RLWindowDraw;
	((ObjectWindowPtr) theWindow)->Dispose = DisposeRLWin;
	((ObjectWindowPtr) theWindow)->HandleClick = RLWindowClick;
	
	toolP.h = theWindow->portRect.right - (ToolWidth*3+1);
	toolP.v = theWindow->portRect.bottom - (ToolHeight+1);

	AddTool(theWindow, CancelIcon, CancelHiIcon, 0, toolP.h, toolP.v);
	AddTool(theWindow, GoIcon, GoHiIcon, 0, toolP.h + ToolWidth+ToolWidth/2, toolP.v);
}

void RLWindowDraw(WindowPtr theWindow)
{
	RefreshTools(theWindow);
}

Boolean GotoRoomInRoomList(short theLine)
{
	return false;
}

void RLWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent)
{
	short	toolNbr;
	// Check if clicked using list Manager
	SetPort(theWin);
	GlobalToLocal(&where);
    if (ToolClick(where,&toolNbr)) {
		switch (toolNbr) {
		case 0:		DisposeRLWin(theWin);	break;
		case 1:		
			// DoPalaceCommand(PC_GotoRoom,(long) 104L,NULL);
			DisposeRLWin(theWin);
			break;
		}
	}
}

void RLAdjustCursor(WindowPtr theWin, Point where, EventRecord *theEvent)
{
}

void DisposeRLWin(WindowPtr theWin)
{
	gRLWin = NULL;
	gRLPtr = NULL;

	// DefaultDispose(theWin);
	CloseWindow(theWin);

	DisposePtr((Ptr) theWin); // 10/17 ruled out
}

void RLWindowReceiveRoomList(long nbrRooms, Ptr	p, long len)
{
}

void ProcessRoomList(long nbrRooms, Ptr	roomList, long len)
{
}
