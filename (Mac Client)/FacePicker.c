// FacePicker.c
#include "U-USER.H"

#define		FPWIND			138
#define		FPPICT			129
#define		FaceTileHeight	22
#define		FaceTileWidth	22
#define		ColorTileHeight	11
#define		ColorTileWidth	11

// tile order: normal,smile,winkleft,winkright,
//				up,down,left,right,
//				closed,talk,sad,angry,
//				blotto,res,res,res

// face order:	closed,smile,down,talk,
//				winkleft,normal,winkright,left,
//				up,right,sad,blotto,
//				angry,res,res,res
static char	face2tile[] = {8,1,5,9,  2,0,3,6,  4,7,10,12,  11,0,0,0};
static char tile2face[] = {5,1,4,6,  8,2,7,9,  0,3,10,12,  11,0,0,0};

typedef struct {
	ObjectWindowRecord	win;
	PicHandle			facePic;
	short				curFace,curColor;
} FPWindowRecord, *FPWindowPtr;

WindowPtr	gFPWin;
FPWindowPtr	gFPPtr;


// Create a new main window using a 'WIND' template from the resource fork
//
void NewFPWindow(void)
{
	WindowPtr		theWindow;
	FPWindowPtr		fpRec;
	Point			origin;

	SetPort((GrafPtr) gRoomWin);
	origin = topLeft(gRoomWin->toolRect);
	origin.h += ToolWidth;
	origin.v += ToolHeight;
	origin.h -= FaceTileWidth*4;
	origin.v -= FaceTileHeight*5;
	LocalToGlobal(&origin);

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 
	fpRec = (FPWindowPtr) NewPtrClear(sizeof(FPWindowRecord));
	theWindow = InitObjectWindow(FPWIND, (ObjectWindowPtr) fpRec,true);
	MoveWindow(theWindow, origin.h,origin.v,false);
	RestoreWindowPos(theWindow, &gMacPrefs.facePickerPos);

	gFPWin = theWindow;
	gFPPtr = (FPWindowPtr) theWindow;

	((ObjectWindowPtr) theWindow)->Draw = FPWindowDraw;
	((ObjectWindowPtr) theWindow)->Dispose = DisposeFPWin;
	((ObjectWindowPtr) theWindow)->HandleClick = FPWindowClick;
	
	gFPPtr->facePic = (PicHandle) GetResource('PICT',FPPICT);
	if (gRoomWin->mePtr) {
		gFPPtr->curFace = gRoomWin->mePtr->user.faceNbr;
		gFPPtr->curColor = gRoomWin->mePtr->user.colorNbr;
	}
	// Show the window
	ShowWindow(theWindow);
	SelectFloating(theWindow);

	// Make it the current grafport
	SetPort(theWindow);
}

void FPWindowDraw(WindowPtr theWindow)
{
	short	n,h,v;
	Rect	r;
	DrawPicture(gFPPtr->facePic,&theWindow->portRect);
	// !!! Do a RevHilite of face and color
	if (gRoomWin->mePtr) {
		n = face2tile[gFPPtr->curFace];
		h = n%4;
		v = n/4;
		SetRect(&r,0,0,FaceTileWidth,FaceTileHeight);
		OffsetRect(&r,h*FaceTileWidth,v*FaceTileHeight);
		RevHiliteRect(&r);
		n = gFPPtr->curColor;
		h = n%8;
		v = n/8;
		SetRect(&r,0,0,ColorTileWidth,ColorTileHeight);
		OffsetRect(&r,h*ColorTileWidth,v*ColorTileHeight+FaceTileHeight*4);
		RevHiliteRect(&r);
	}
}

void FPWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent)
{
	// Check if clicked using list Manager
	short	n;
	static long lastClick=0L;
	SetPort(theWin);
	GlobalToLocal(&where);
	if (where.v/FaceTileHeight < 4) {
		// Pick Face
		n = (where.v/FaceTileHeight)*4+(where.h/FaceTileWidth);
		if (gFPPtr->curFace == tile2face[n] && theEvent->when - lastClick < GetDblTime()) {
			DisposeFPWin(theWin);
			return;
		}
		gFPPtr->curFace = tile2face[n];
		ChangeFace(tile2face[n]);
	}
	else {
		// Pick Color
		n = (where.v - FaceTileHeight*4)/ColorTileHeight * 8 + (where.h/ColorTileWidth);
		if (!MembersOnly(n != 3)) {
			if (gFPPtr->curColor == n && theEvent->when - lastClick < GetDblTime()) {
				DisposeFPWin(theWin);
				return;
			}
			gFPPtr->curColor = n;
			ChangeColor(n);
		}
	}
	lastClick = theEvent->when;
	FPWindowDraw(theWin);
}

void DisposeFPWin(WindowPtr theWin)
{
	if (gFPPtr->facePic)
		ReleaseResource((Handle) gFPPtr->facePic);
	SaveWindowPos(theWin, &gMacPrefs.facePickerPos);
	gFPWin = NULL;
	gFPPtr = NULL;
	DefaultDispose(theWin);
	DisposePtr((Ptr) theWin);
}

void ToggleFacePicker(void)
{
	GrafPtr	savePort;		// 6/10/95
	GetPort(&savePort);	
	if (gFPWin)
		DisposeFPWin(gFPWin);
	else
		NewFPWindow();
	SetPort(savePort);
}
