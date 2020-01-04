// AboutBox.c
#include "U-USER.H"

#define		ABWIND			139
#define		ABPICT			130
#define		ABStrings		130

#define ScrollTime	2
#define ScrollLines	2


enum {AB_NextStr, AB_Scroll, AB_Done};

typedef struct {
	ObjectWindowRecord	win;
	short				curFace,curColor;
	GWorldPtr			offWorld,scrollWorld;
	PixMapHandle		offMap,scrollMap;
	short				state;
	short				scrollCtr,scrollLimit;
	short				strCtr;
	unsigned long		nextTicks,lastTicks;
	Str255				tBuf;
} ABWindowRecord, *ABWindowPtr;

WindowPtr	gABWin;
ABWindowPtr	gABPtr;

void NewABWindow(void);
void ABWindowDraw(WindowPtr theWindow);
void DisposeABWin(WindowPtr theWin);
void ABIdle(WindowPtr theWin, EventRecord *theEvent);


// Create a new main window using a 'WIND' template from the resource fork
//
void NewABWindow(void)
{
	WindowPtr		theWindow;
	ABWindowPtr		abRec;
	PicHandle		bgPic;
	CGrafPtr		curPort;
	GDHandle		curDevice;
	OSErr			oe;

	SetPort((GrafPtr) gRoomWin);

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 
	abRec = (ABWindowPtr) NewPtrClear(sizeof(ABWindowRecord));
	theWindow = InitObjectWindow(ABWIND, (ObjectWindowPtr) abRec,true);
	RestoreWindowPos(theWindow, &gMacPrefs.aboutBoxPos);

	gABWin = theWindow;
	gABPtr = (ABWindowPtr) theWindow;

	((ObjectWindowPtr) theWindow)->Draw = ABWindowDraw;
	((ObjectWindowPtr) theWindow)->Dispose = DisposeABWin;
	((ObjectWindowPtr) theWindow)->Idle = ABIdle;
	
	bgPic = (PicHandle) GetResource('PICT',ABPICT);
	if (bgPic == NULL)
		return;

	// Allocate GWorld
	if ((oe = NewGWorld(&gABPtr->offWorld,8,&gABWin->portRect,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"NewABWindow");
		return;
	}
	gABPtr->offMap = GetGWorldPixMap(gABPtr->offWorld);
	LockPixels(gABPtr->offMap);

	if ((oe = NewGWorld(&gABPtr->scrollWorld,8,&gABWin->portRect,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"NewABWindow");
		return;
	}
	gABPtr->scrollMap = GetGWorldPixMap(gABPtr->scrollWorld);
	LockPixels(gABPtr->scrollMap);


	GetGWorld(&curPort,&curDevice);

	SetGWorld(gABPtr->offWorld,NULL);
	DrawPicture(bgPic,&gABPtr->offWorld->portRect);
	ReleaseResource((Handle) bgPic);

	SetGWorld(gABPtr->scrollWorld,NULL);
	PaintRect(&gABPtr->scrollWorld->portRect);

	SetGWorld(curPort,curDevice);



	// Show the window
	ShowWindow(theWindow);
	SelectFloating(theWindow);

	// Make it the current grafport
	SetPort(theWindow);
}

void DisposeABWin(WindowPtr theWin)
{
	if (gABPtr->offWorld)
		DisposeGWorld(gABPtr->offWorld);
	if (gABPtr->scrollWorld)
		DisposeGWorld(gABPtr->scrollWorld);
	SaveWindowPos(theWin, &gMacPrefs.aboutBoxPos);
	gABWin = NULL;
	gABPtr = NULL;
	DefaultDispose(theWin);
	DisposePtr((Ptr) theWin);
}

void ABWindowDraw(WindowPtr theWindow)
{
	CopyBits((BitMap *) *gABPtr->offMap, (BitMap *) &theWindow->portBits, 
				&gABPtr->offWorld->portRect,&theWindow->portRect,ditherCopy,NULL);
	gABPtr->state = AB_NextStr;
	gABPtr->strCtr = 0;
}

void ABIdle(WindowPtr theWin, EventRecord *theEvent)
{
	static long lastTicks;
	long		t;
	Rect		r1,r2;
	CGrafPtr		curPort;
	GDHandle		curDevice;

	if ((t = TickCount())  < gABPtr->nextTicks)
		return;
		
	if (gABPtr->state == AB_Done) {
		DisposeABWin(theWin);
		return;
	}
	gABPtr->nextTicks = t + ScrollTime;

	switch (gABPtr->state) {
	case AB_NextStr:
		++gABPtr->strCtr;
		GetIndString(gABPtr->tBuf,ABStrings,gABPtr->strCtr);
		if (gABPtr->tBuf[0] == 0) {
			gABPtr->state = AB_Done;
			gABPtr->nextTicks = t + 180;
			return;
		}
		GetGWorld(&curPort,&curDevice);
		SetGWorld(gABPtr->scrollWorld,NULL);
		RGBBackColor(&gBlackColor);
		RGBForeColor(&gWhiteColor);
		gABPtr->scrollLimit = 18;
		if (gABPtr->tBuf[1] == '.') {
			switch (gABPtr->tBuf[2]) {
			case 't':
			case 'T':
				TextFont(times);			
				TextSize(24);
				gABPtr->scrollLimit = 48;
				break;
			case 's':
				gABPtr->scrollLimit = gABPtr->scrollWorld->portRect.bottom/8;
				break;
			case 'S':
				gABPtr->scrollLimit = gABPtr->scrollWorld->portRect.bottom/4;
				break;
			case 'l':
			case 'L':
				gABPtr->scrollLimit = gABPtr->scrollWorld->portRect.bottom/2;
				break;
			case 'b':
			case 'B':
				TextFont(times);			
				TextSize(18);
				RGBForeColor(&gCyanColor);
				gABPtr->scrollLimit = 24;
				break;
			case 'j':
			case 'J':
				TextFont(times);			
				TextSize(18);
				RGBForeColor(&gPurpleColor);
				gABPtr->scrollLimit = 24;
				break;
			}
			gABPtr->tBuf[0] -= 2;
			BlockMove(&gABPtr->tBuf[3],&gABPtr->tBuf[1],gABPtr->tBuf[0]);
		}
		else {
			TextFont(times);			
			TextSize(12);
		}
		r1 = gABPtr->scrollWorld->portRect;
		r1.bottom = gABPtr->scrollLimit;

		EraseRect(&gABPtr->scrollWorld->portRect);
		if (gABPtr->tBuf[0]) {
			MoveTo((gABPtr->scrollWorld->portRect.right - StringWidth(gABPtr->tBuf))/2,
					(gABPtr->scrollLimit*2)/3);
			DrawString(gABPtr->tBuf);
		}
		gABPtr->scrollCtr = 0;
		gABPtr->state = AB_Scroll;
		RGBBackColor(&gWhiteColor);
		RGBForeColor(&gBlackColor);
		SetGWorld(curPort,curDevice);
		gABPtr->nextTicks = gABPtr->lastTicks + ScrollTime;
		break;
	case AB_Scroll:
		SetPort(theWin);
		r1 = theWin->portRect;
		r2 = r1;
		r1.top += ScrollLines;
		r2.bottom -= ScrollLines;
		CopyBits(&theWin->portBits,&theWin->portBits,&r1,&r2,srcCopy,NULL);
		r1.top = gABPtr->scrollCtr;
		r1.bottom = r1.top+ScrollLines;
		r2.top = r2.bottom;
		r2.bottom = r2.top+ScrollLines;
		CopyBits((BitMap *) *gABPtr->scrollMap,&theWin->portBits,&r1,&r2,srcCopy,NULL);
		gABPtr->scrollCtr += ScrollLines;
		if (gABPtr->scrollCtr >= gABPtr->scrollLimit) {
			gABPtr->state = AB_NextStr;
			gABPtr->nextTicks = t;
		}
		
		break;
	}
	gABPtr->lastTicks = t;
}
