/************************************************************************************
 * LogMessage.c - only LogMessage(char *str,...) needs to be shared
 *
 ************************************************************************************/
#include "U-USER.H"
#include "AppMenus.h"

#ifdef THINK_C
#include <LoMem.h>
#endif
#define		MainWIND	128
#define		MaxLines	300

LogWindowPtr	gLogWin;
void LogProcessCommand(WindowPtr theWin, short theMenu, short theItem);
void LogAdjustMenus(WindowPtr theWin);
void LogWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent);


// Create a new main window using a 'WIND' template from the resource fork
//
void NewLogWindow(void)
{
	WindowPtr		theWindow;
	LogWindowPtr	logRec;
	TEWindowPtr		tw;

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 
	logRec = (LogWindowPtr) NewPtrClear(sizeof(LogWindowRec));
	// Switched to True
	theWindow = NewTEWindow(MainWIND, (TEWindowPtr) logRec, true);
	RestoreWindowPos(theWindow, &gMacPrefs.logPos);
	TEWindowResize(theWindow, gMacPrefs.logSize.h, gMacPrefs.logSize.v);

	tw = (TEWindowPtr) theWindow;
	TEAutoView(true,tw->teH);
	((ObjectWindowPtr) theWindow)->Dispose = DisposeLogWin;
	((ObjectWindowPtr) theWindow)->Close = CloseLogWin;
	((ObjectWindowPtr) theWindow)->HandleClick = LogWindowClick;
	((ObjectWindowPtr) theWindow)->ProcessKey = NULL;	// Don't allow typing
	((ObjectWindowPtr) theWindow)->growRect.left = 64;
	((ObjectWindowPtr) theWindow)->growRect.top = 64;
	((ObjectWindowPtr) theWindow)->AdjustMenus = LogAdjustMenus;
	((ObjectWindowPtr) theWindow)->ProcessCommand = LogProcessCommand;
	SetWTitle(theWindow,gPrefs.name);
	gLogWin = (LogWindowPtr) theWindow;
	gLogWin->inMiddle = false;
	ShowWindow(theWindow);
}


//
//

void LogString(char *str)
{
	GrafPtr	savePort;
	Point	units;
	TEWindowPtr	tw = (TEWindowPtr) gLogWin;
	ScrollWindowPtr	sWin = (ScrollWindowPtr) gLogWin;
	TextStyle	ts;
	short		lineHeight,ascent;
	short		oldStart,oldEnd;

	if (gLogWin == NULL)
		return;

	GetPort(&savePort);
	SetPort((WindowPtr) gLogWin);

	oldStart = (*tw->teH)->selStart;
	oldEnd = (*tw->teH)->selEnd;

	// Moved this block of code (if statement)
	if ((*tw->teH)->nLines > MaxLines) {
		// Note: this is causing us to get off by 1 in the display
		short	startPoint = (*tw->teH)->lineStarts[(*tw->teH)->nLines - MaxLines];
		TEAutoView(false,tw->teH);	// 10/11/95
		TESetSelect(0,startPoint,tw->teH);
		TEDelete(tw->teH);
		// Deleted these two lines 1/11/95
		// (*tw->teH)->clikStuff = 255;	// Fix SetSelect bug
		// TESetSelect(32767,32767,tw->teH);
		if (!gLogWin->inMiddle)
			TEAutoView(true,tw->teH);	// 10/11/95
		oldStart -= startPoint;	if (oldStart < 0)	oldStart = 0;
		oldEnd -= startPoint; if (oldEnd < 0)		oldEnd = 0;
	}

	TESetSelect(32767,32767,tw->teH);	// 6/21/95
	TEGetStyle((*tw->teH)->selStart,&ts,&lineHeight,&ascent,tw->teH);
	ts.tsColor = gLogWin->curColor;
	TESetStyle(doColor,&ts,false,tw->teH);

	TEInsert(str,strlen(str),tw->teH);

	// Was Here!!!
	units.h = 0;
	units.v = (*tw->teH)->nLines;
	SetScrollUnits((WindowPtr) tw,units);

	if (!gLogWin->inMiddle) {
		sWin->scrollOffset.v = sWin->nbrUnits.v - sWin->unitsPerScreen.v;
		AdjustScrollBars((WindowPtr) gLogWin, false);
	}

	if (gLogWin->logFile) {
		long	count;
		count = strlen(str);
		FSWrite(gLogWin->logFile,&count,str);
	}
	if (oldEnd > oldStart) {
		TEAutoView(false,tw->teH);
		TESetSelect(oldStart,oldEnd,tw->teH);
		if (!gLogWin->inMiddle)
			TEAutoView(true,tw->teH);
	}
	SetPort(savePort);
}

void LogMessage(char *str,...)
{
	char 	tbuf[256];
	va_list args;

	if (gLogWin == NULL)
		return;

	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	LogString(tbuf);
}


void DisposeLogWin(WindowPtr theWin)
{
	if (gLogWin) {
		SaveWindowPos(theWin, &gMacPrefs.logPos);
		SaveWindowSize(theWin, &gMacPrefs.logSize);

		if (gLogWin->logFile) {
			FSClose(gLogWin->logFile);
			gLogWin->logFile = 0;
		}
		DisposeTEWin(theWin);
		DisposePtr((Ptr) theWin);
		gLogWin = NULL;
	}
}

// If Close Box Hit...
void CloseLogWin(WindowPtr theWin)
{
	((ObjectWindowPtr) theWin)->floating = false;
	HideWindow(theWin);
	SendBehind(theWin,(WindowPtr) gRoomWin);
	// SelectDocument((WindowPtr) gRoomWin);
}

void ToggleLogFile(void)
{
	Str255				defName,dateStr;
	unsigned long		secs;
	StandardFileReply	reply;
	OSErr				oe;

	if (gLogWin->logFile) {
		LogMessage("Closing Log\r");
		FSClose(gLogWin->logFile);
		gLogWin->logFile = 0;
	}
	else {
		GetDateTime(&secs);
		IUDateString(secs,abbrevDate,dateStr);
		sprintf((char *) defName, "ULog %.*s",dateStr[0],&dateStr[1]);
		CtoPstr((char *) defName);
		StandardPutFile("\pLog File:",defName,&reply);
		if (!reply.sfGood)
			return;
		FSpDelete(&reply.sfFile);
		if ((oe = FSpCreate(&reply.sfFile,'ttxt','TEXT',reply.sfScript)) != noErr)
		{
			ReportError(oe,"FspCreate");
			return;
		}
		if ((oe = FSpOpenDF(&reply.sfFile,fsRdWrPerm,&gLogWin->logFile)) != noErr)
		{
			ReportError(oe,"FspOpenDF");
			return;
		}
		LogMessage("Opening Log\r");
	}
}

void LogProcessCommand(WindowPtr theWin, short theMenu, short theItem)
{
	TEWindowPtr	tw = (TEWindowPtr) gLogWin;

	switch (theMenu) {
	case EditMENU:
		switch (theItem) {
		case EM_Copy:
			TECopy(tw->teH);
			return;
		case EM_SelectAll:
			TESetSelect(0,32767,tw->teH);
			return;
		}
		break;
	}
	DefaultProcessCommand(theWin,theMenu,theItem);

}

void LogAdjustMenus(WindowPtr theWin)
{
	TEWindowPtr	tw = (TEWindowPtr) gLogWin;
	Boolean				hasSelection;

	DefaultAdjustMenus(theWin);
	hasSelection = (*tw->teH)->selEnd > (*tw->teH)->selStart;
	MyEnableMenuItem(gEditMenu, EM_Copy, hasSelection);
	MyEnableMenuItem(gEditMenu, EM_SelectAll, true);
}

void SetLogColor(RGBColor *color)
{
	if (gLogWin == NULL)
		return;
	gLogWin->curColor = *color;
}

void LogWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent)
{
	TEWindowPtr	tw = (TEWindowPtr) gLogWin;
	ScrollWindowPtr	sWin = (ScrollWindowPtr) gLogWin;
	TEWindowClick(theWin,where,theEvent);
	if (sWin->scrollOffset.v >= sWin->nbrUnits.v-sWin->unitsPerScreen.v) {
		gLogWin->inMiddle = false;
		TEAutoView(true,tw->teH);
	}
	else {
		gLogWin->inMiddle = true;
		TEAutoView(false,tw->teH);
	}
}
