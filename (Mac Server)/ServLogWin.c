/************************************************************************************
 * LogMessage.c - only LogMessage(char *str,...) needs to be shared
 *
 ************************************************************************************/
#include "S-SERVER.H"
#include "AppMenus.h"

//#include <LoMem.h>
#include <string.h>

#define		MainWIND	128
#define		MaxLines	200

LogWindowPtr	gLogWin;

void NewLogWindow(void);
void DisposeLogWin(WindowPtr theWin);
void LogWindowDraw(WindowPtr theWin);
void LogWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent);
void LogWindowActivate(WindowPtr theWindow, Boolean activeFlag);
void LogProcessCommand(WindowPtr theWin, short theMenu, short theItem);
void LogAdjustMenus(WindowPtr theWin);

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
	theWindow = NewTEWindow(MainWIND, (TEWindowPtr) logRec, false);
	tw = (TEWindowPtr) theWindow;
	TEAutoView(true,tw->teH);
	((ObjectWindowPtr) theWindow)->Dispose = DisposeLogWin;
	((ObjectWindowPtr) theWindow)->Draw = LogWindowDraw;
	((ObjectWindowPtr) theWindow)->HandleClick = LogWindowClick;
	((ObjectWindowPtr) theWindow)->Activate = LogWindowActivate;
	((ObjectWindowPtr) theWindow)->ProcessCommand = LogProcessCommand;
	((ObjectWindowPtr) theWindow)->AdjustMenus = LogAdjustMenus;
	
	((ObjectWindowPtr) theWindow)->ProcessKey = NULL;	// Don't allow typing
	((ObjectWindowPtr) theWindow)->growRect.left = 64;
	((ObjectWindowPtr) theWindow)->growRect.top = 64;
	logRec->cIcons[0] = GetCIcon(128);
	logRec->cIcons[1] = GetCIcon(129);

	SetWTitle(theWindow,gPrefs.serverName);
	gLogWin = (LogWindowPtr) theWindow;
}


void LogWindowDraw(WindowPtr theWin)
{
	LogWindowPtr	lWin = (LogWindowPtr) theWin;
	if (lWin->iconized) {
		Rect		r;
		SetRect(&r,0,0,64,64);
		lWin->iconState = gNbrUsers > 0? 1 : 0;
		PlotCIcon(&r,lWin->cIcons[lWin->iconState]);
	}
	else {
		TEWindowDraw(theWin);
	}
}

//
//

void LogString(char *str)
{
	GrafPtr	savePort;
	Point	units;
	TEWindowPtr	tw = (TEWindowPtr) gLogWin;
	ScrollWindowPtr	sWin = (ScrollWindowPtr) gLogWin;

	if (gLogWin == NULL)
		return;

	GetPort(&savePort);
	SetPort((WindowPtr) gLogWin);

	TESetSelect(32767,32767,tw->teH);	// 6/21/95
	TEInsert(str,strlen(str),tw->teH);

	if ((*tw->teH)->nLines > MaxLines) {
		TESetSelect(0,(*tw->teH)->lineStarts[(*tw->teH)->nLines - MaxLines],tw->teH);
		TEDelete(tw->teH);
		(*tw->teH)->clikStuff = 255;	// Fix SetSelect bug
		TESetSelect(32767,32767,tw->teH);
	}
	units.h = 0;
	units.v = (*tw->teH)->nLines;
	SetScrollUnits((WindowPtr) tw,units);
	sWin->scrollOffset.v = sWin->nbrUnits.v - sWin->unitsPerScreen.v;
	AdjustScrollBars((WindowPtr) gLogWin, false);

	if (gLogWin->iconized && gLogWin->iconState != (gNbrUsers > 0? 1 : 0)) {
		// Play Lightswitch Sound...
		LogWindowDraw((WindowPtr) gLogWin);
	}
	if (gLogWin->logFile) {
		long	count;
		count = strlen(str);
		FSWrite(gLogWin->logFile,&count,str);
	}
	SetPort(savePort);
}

void LogMessage(char *str,...)
{
	char 	tbuf[128];
	va_list args;

	if (gLogWin == NULL)
		return;

	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	LogString(tbuf);
}

void TimeLogMessage(char *str,...)
{
	va_list 		args;
	char 			tbuf[128],tbuf2[128];
	unsigned long 	secs;
	Str31			timeStr;

	if (gLogWin == NULL)
		return;

	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	GetDateTime(&secs);
	IUTimeString(secs,true,timeStr);
	sprintf(tbuf2,"%.*s %s",timeStr[0],&timeStr[1],tbuf);
	LogString(tbuf2);
}

void DisposeLogWin(WindowPtr theWin)
{
	if (gLogWin) {
		if (gLogWin->logFile) {
			FSClose(gLogWin->logFile);
			gLogWin->logFile = 0;
		}
		DisposeTEWin(theWin);
		DisposePtr((Ptr) theWin);
		gLogWin = NULL;
	}
}

void Iconize()
{
	WindowPtr	theWindow = (WindowPtr) gLogWin;
	TEWindowPtr	tw = (TEWindowPtr) gLogWin;
	LogWindowPtr lWin = (LogWindowPtr) gLogWin;

	if (lWin->iconized)
		return;

	SetPort(theWindow);
	lWin->saveRect = theWindow->portRect;
	SizeWindow(theWindow,64,64,false);
	OffsetRect(&(*tw->teH)->viewRect,64,64);
	TEDeactivate(tw->teH);
	// InvalRect(&theWindow->portRect);
	SetWTitle(theWindow,"\p");
	lWin->iconized = true;
	LogWindowDraw(theWindow);
}

void DeIconize()
{
	WindowPtr	theWindow = (WindowPtr) gLogWin;
	TEWindowPtr	tw = (TEWindowPtr) gLogWin;
	LogWindowPtr lWin = (LogWindowPtr) gLogWin;

	if (!lWin->iconized)
		return;
	lWin->iconized = false;
	SizeWindow(theWindow,lWin->saveRect.right-lWin->saveRect.left,
						lWin->saveRect.bottom-lWin->saveRect.top,false);
	OffsetRect(&(*tw->teH)->viewRect,-64,-64);
	TEDeactivate(tw->teH);
	SetWTitle(theWindow,gPrefs.serverName);
	TECalText(tw->teH);
	AdjustScrollBars((WindowPtr) gLogWin, false);
	LogWindowDraw(theWindow);
}

void LogWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent)
{
	LogWindowPtr	lWin = (LogWindowPtr) theWin;
	if (lWin->iconized) {
		DeIconize();
		return;
	}
	TEWindowClick(theWin,where,theEvent);
}


void LogWindowActivate(WindowPtr theWindow, Boolean activeFlag)
{
	extern Boolean 	gSuspended;
	TEWindowActivate(theWindow,activeFlag);
	if (!activeFlag && gSuspended)
		Iconize();
}

void ToggleLogFile()
{
	Str255				defName,dateStr;
	unsigned long		secs;
	StandardFileReply	reply;
	OSErr				oe;

	if (gLogWin->logFile) {
		TimeLogMessage("Closing Log\r");
		FSClose(gLogWin->logFile);
		gLogWin->logFile = 0;
	}
	else {
		GetDateTime(&secs);
		IUDateString(secs,abbrevDate,dateStr);
		sprintf((char *) defName, "Log %.*s",dateStr[0],&dateStr[1]);
		CtoPstr((char *) defName);
		StandardPutFile("\pLog File:",defName,&reply);
		if (!reply.sfGood)
			return;
		FSpDelete(&reply.sfFile);
		if ((oe = FSpCreate(&reply.sfFile,'KAHL','TEXT',reply.sfScript)) != noErr)
		{
			ReportError(oe,"FspCreate");
			return;
		}
		if ((oe = FSpOpenDF(&reply.sfFile,fsRdWrPerm,&gLogWin->logFile)) != noErr)
		{
			ReportError(oe,"FspOpenDF");
			return;
		}
		TimeLogMessage("Opening Log\r");
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
	hasSelection = (*tw->teH)->selEnd > (*tw->teH)->selStart;
	DefaultAdjustMenus(theWin);
	MyEnableMenuItem(gEditMenu, EM_Copy, hasSelection);
	MyEnableMenuItem(gEditMenu, EM_SelectAll, true);
}
