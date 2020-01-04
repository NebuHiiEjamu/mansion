#include "TEWindow.h"
#if THINK_C
#include <LoMem.h>
#endif
#include "AppMenus.h"

// Create a new main window using a 'WIND' template from the resource fork
//
WindowPtr NewTEWindow(short winID, TEWindowPtr storage, Boolean floating)
{
	WindowPtr		theWindow;
	TEWindowPtr		logRec;
	Rect			r;
	TextStyle		ts;
	short			lineHeight,ascent;
	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 

	if (storage) {
		logRec = storage;
		logRec->localStorage = false;
	}
	else {
		logRec = (TEWindowPtr) NewPtrClear(sizeof(TEWindowRec));
		logRec->localStorage = true;
	}
	theWindow = NewScrollWindow(winID, (ScrollWindowPtr) logRec,true,false,floating);
	((ObjectWindowPtr) theWindow)->Draw = TEWindowDraw;
	((ObjectWindowPtr) theWindow)->Dispose = DisposeTEWin;
	((ObjectWindowPtr) theWindow)->HandleClick = TEWindowClick;
	((ObjectWindowPtr) theWindow)->ProcessKey = TEWindowKey;
	((ObjectWindowPtr) theWindow)->Resize = TEWindowResize;
	((ObjectWindowPtr) theWindow)->Idle = TEWindowIdle;
	((ObjectWindowPtr) theWindow)->Activate = TEWindowActivate;
	((ObjectWindowPtr) theWindow)->AdjustMenus = TEAdjustMenus;
	((ObjectWindowPtr) theWindow)->ProcessCommand = TEProcessCommand;
	((ScrollWindow *) theWindow)->UpdateToScrollPosition = TEUpdateToScrollPosition;

	// Show the window
	ShowWindow(theWindow);

	// Make it the current grafport
	SetPort(theWindow);

	TextFont(monaco);
	TextSize(9);

	r = theWindow->portRect;
	r.right -= 15;
	r.bottom -= 15;
	InsetRect(&r,2,2);
	logRec->teH = TEStylNew(&r,&r);
	TECalText(logRec->teH);
	TEAutoView(true,logRec->teH);
	TEGetStyle(0,&ts,&lineHeight,&ascent,logRec->teH);
	((ScrollWindowPtr) theWindow)->unitHeight.v = lineHeight;
	((ScrollWindowPtr) theWindow)->unitHeight.h = 0;
	((ScrollWindowPtr) theWindow)->ComputeUnitsPerScreen(theWindow);
	return theWindow;
}


void TEUpdateToScrollPosition(WindowPtr theWindow)
{
	TEWindowPtr		lw;
	Rect			r;

	lw = (TEWindowPtr) theWindow;

	r = (*lw->teH)->viewRect;
	r.bottom -= (r.bottom - r.top) % ((ScrollWindowPtr) theWindow)->unitHeight.v;
	(*lw->teH)->viewRect = r;

	r.bottom = r.top + ((ScrollWindowPtr) theWindow)->unitHeight.v * (*lw->teH)->nLines;
	OffsetRect(&r,0,-((ScrollWindowPtr) theWindow)->unitHeight.v * ((ScrollWindowPtr) theWindow)->scrollOffset.v);
	(*lw->teH)->destRect = r;

	((ObjectWindowPtr) theWindow)->Draw(theWindow);
}

// Respond to an update event - BeginUpdate has already been called.
//
void TEWindowDraw(WindowPtr theWindow)
{
	TEWindowPtr		lw;

	lw = (TEWindowPtr) theWindow;

	// Erase the content area
	EraseRect(&theWindow->portRect);

	// Draw the grow icon in the corner
	DrawGrowIcon(theWindow);
	UpdtControl(theWindow,theWindow->visRgn);

	TEUpdate(&theWindow->portRect, lw->teH);
}


// Respond to a mouse-click - highlight cells until the user releases the button
//
void TEWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent)
{
	Point	p = where;
	// Rect	r;
	TEWindowPtr		lw = (TEWindowPtr) theWin;


	SetPort(theWin);
	GlobalToLocal(&p);

	if (ScrollHandleControlClick(theWin,p)) {
		return;
	}
	else
		TEClick(p,(theEvent->modifiers & shiftKey) > 0,lw->teH);
}

void TEWindowKey(WindowPtr theWin, EventRecord *theEvent)
{
	TEWindowPtr		lw = (TEWindowPtr) theWin;
	char	theChar,theCode;
	SetPort(theWin);
	theChar = theEvent->message & charCodeMask;
	theCode = (theEvent->message & keyCodeMask) >> 8;
	switch (theCode) {
	case 0x73:		// Home
			TESetSelect(0,0,lw->teH);
			break;
	case 0x77:		// End
			TESetSelect(32767,32767,lw->teH);
			break;
	case 0x75:		// KP Del
		if ((*lw->teH)->selEnd > (*lw->teH)->selStart) {
			TEKey('\b',lw->teH);
		}
		else {
			TESetSelect((*lw->teH)->selStart,(*lw->teH)->selStart+1,lw->teH);
			if ((*lw->teH)->selEnd > (*lw->teH)->selStart)
				TEKey('\b',lw->teH);
		}
		lw->dirty = true;
		break;
	default:
		TEKey(theChar,lw->teH);
		lw->dirty = true;
	}
}


void TEWindowIdle(WindowPtr theWin, EventRecord *theEvent)
{
	TEWindowPtr		lw = (TEWindowPtr) theWin;
	TEIdle(lw->teH);
}

void TEWindowActivate(WindowPtr theWin, Boolean activeFlag)
{
	TEWindowPtr		lw = (TEWindowPtr) theWin;
	if (activeFlag) {
		TEActivate(lw->teH);
	}
	else {
		TEDeactivate(lw->teH);
	}
	ScrollActivate(theWin,activeFlag);
}

void TEWindowResize(WindowPtr theWin, short width, short height)
{
	TEWindowPtr		lw = (TEWindowPtr) theWin;
	Rect	r;
	DefaultResize(theWin, width, height);
	r = theWin->portRect;
	r.right -= 15;
	r.bottom -= 15;
	InsetRect(&r,2,2);
	(*lw->teH)->destRect = r;
	(*lw->teH)->viewRect = r;
	TECalText(lw->teH);
	AdjustScrollBars(theWin, true);
}

void TEProcessCommand(WindowPtr theWin, short theMenu, short theItem)
{
	TEWindowPtr	tw = (TEWindowPtr) theWin;

	switch (theMenu) {
	case FileMENU:
		switch (theItem) {
		case FM_Save:
			if (((ObjectWindowPtr) theWin)->Save) {
				((ObjectWindowPtr) theWin)->Save(theWin);
				tw->dirty = false;
				return;
			}
			break;
		}
		break;
	case EditMENU:
		switch (theItem) {
		case EM_Copy:
			TECopy(tw->teH);
			return;
		case EM_Cut:
			TECut(tw->teH);
			tw->dirty = true;
			return;
		case EM_Clear:
			TEDelete(tw->teH);
			tw->dirty = true;
			return;
		case EM_Paste:
			TEPaste(tw->teH);
			tw->dirty = true;
			return;
		case EM_SelectAll:
			TESetSelect(0,32767,tw->teH);
			return;
		}
		break;
	}
	DefaultProcessCommand(theWin,theMenu,theItem);

}

void TEAdjustMenus(WindowPtr theWin)
{
	TEWindowPtr	tw = (TEWindowPtr) theWin;
	Boolean				hasSelection;
	Boolean				hasClip;
	hasSelection = (*tw->teH)->selEnd > (*tw->teH)->selStart;
	hasClip = CanPaste('TEXT');
	DefaultAdjustMenus(theWin);

	MyEnableMenuItem(gFileMenu, FM_Save, 
		((ObjectWindowPtr) theWin)->Save && tw->dirty);

	MyEnableMenuItem(gEditMenu, EM_Copy, hasSelection);
	MyEnableMenuItem(gEditMenu, EM_Cut, hasSelection);
	MyEnableMenuItem(gEditMenu, EM_Clear, hasSelection);
	MyEnableMenuItem(gEditMenu, EM_Paste, hasClip);
	MyEnableMenuItem(gEditMenu, EM_SelectAll, true);
}

void DisposeTEWin(WindowPtr theWin)
{
	TEWindowPtr		lw = (TEWindowPtr) theWin;
	Boolean			localStorage;
	localStorage = lw->localStorage;
	TEDispose(lw->teH);
	DefaultDispose(theWin);
	if (localStorage)
		DisposePtr((Ptr) lw);
}
