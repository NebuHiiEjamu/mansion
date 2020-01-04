/************************************************************************************
 * Skel.c
 *
 *
 ************************************************************************************/
#include "U-USER.H"
#include "UserTCP.h"
#include "u-secure.h"

#if PPASUPPORT
#include "PPAMgr.h"
#endif
#include "AppMenus.h"
#include <OSEvents.h>

#if THINK_C
#if __option(profile)
#include <Console.h>
#include <Profile.h>
#endif
#else
#if __profile__
#include <profiler.h>
#endif
#endif


void 	MyInitMacintosh(void);
void 	MyHandleEvent(void);
void 	MyDoEvent(EventRecord *theEvent);
void	IdleObjects(EventRecord *theEvent);
void	InitAppleEvents();
void 	CheckEnvironment();

#if M5SUPPORT
void	M5MovieIdle(EventRecord *theEvent);
#endif

char		*copyRightString = "Copyright 1995 Time Warner Interactive";
Boolean		gSuspended,gQuitFlag,gCancel;
SysEnvRec	gSysEnv;
long		gTimeSlice = 30;
Boolean		gLogMessages = true, gRecordCoords, gDragDoors, gDebugFlag, gExpert, gSublaunch;
short		gMode = M_Normal;
EventRecord	gTheEvent;
EventRecord *gLastEvent = &gTheEvent;
void CleanUp();

// Typical Macintosh Initialization Code



// Main Entry Point
void CloseAllWindows();
void CloseAllWindows()
{
	WindowPtr	theWin;
	
	while ((theWin = FrontWindow()) != NULL)
		((ObjectWindowPtr) theWin)->Dispose(theWin);
}


void main()
{
	void InitSpeech();

	// Standard Mac Initialization
	MyInitMacintosh();

	// Check if Legal System
	CheckEnvironment();

	// Set up the menu bar
	MySetUpMenus();

	InitAppleEvents();

	InitAppleTalkBuffers();

	Randomize();

	InitUserAssets();

	LoadPreferences();
	LoadSecureInfo();
	NetscapeConfigureCheck();

	if (CheckSecurity())
		ExitToShell();

	if (!(gPrefs.userPrefsFlags & UPF_ClubMode)) {	// 5/15/96
	 	if (ChildLock())
			ErrorExit("Sorry - Access Denied");
	}

	// Open Graphics window
	NewLogWindow();
	
	InitSounds();

	NewRoomWindow();

	// RefreshRoom(&gOffscreenRect);	// 7/2/96 - tried adding, but causes crash - check it out

	SetWindowParent((WindowPtr) gLogWin,(WindowPtr) gRoomWin);	// 6/21/95

	StatusMessage("Loading scripts...",0);

	InitScriptMgr();	// 7/20 must be called after window is setup
	InitDrawTools();
	InitCursors();

	// LogMessage("User Active\r");

	gDebugFlag = 0;				// 5/8/95 JAB

#if THINK_C
#if __option(profile)			// 6/15 Optional profiling support
	freopen("PServer.log","w",stdout);		// If console isn't wanted
	InitProfile(200,200);
	_profile = 0;
#endif
#else
#if __profile__
	ProfilerInit(collectDetailed,bestTimeBase,20,5);
	ProfilerSetStatus(true);
#endif
#endif
	// _atexit(CleanUp);

	StatusMessage("Initializing Speech...",0);

	InitSpeech();
	SpeakChat(0,"The Palace");

	if (gMacPrefs.propPickerVisible) {
		TogglePropPicker();
	}

	if (gMacPrefs.facePickerVisible) {
		ToggleFacePicker();
	}


#if PPASUPPORT
	PPAMgrInit();
#endif

	StatusMessage("Ready",0);

	// main event loop
	while (!gQuitFlag)					// Till the End of Time...
		MyHandleEvent();		// Get an Event, do something about it

#if !THINK_C
#if __profile__
	ProfilerDump("\pPalace.Profile");
	ProfilerTerm();
#endif
#endif

	EndSounds();

	CleanUp();
}

void CleanUp()
{
	gMacPrefs.logVisible = (gLogWin && ((ObjectWindowPtr) gLogWin)->floating == true);
	gMacPrefs.propPickerVisible = (gPropWin != NULL);
	gMacPrefs.facePickerVisible = (gFPWin != NULL);

#if PPASUPPORT
	PPAMgrCleanup();
#endif

	SignOff();

	ClosePalTCP();	// 8/10 swapped order with preceding function

	CloseUserAssets();

	CloseAllWindows();

	StorePreferences();
	// SetEventMask(LMGetSysEvtMask());
}

// Standard Macintosh Initialization

void MyInitMacintosh(void)
{
	MaxApplZone();
	
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	
	// SetEventMask(everyEvent);
}

// The Main Event Dispatcher - this routine should be called repeatedly


void MyHandleEvent(void)

{
	Boolean		ok;

	// If the more modern WaitNextEvent is implemented, use it 

	ok = WaitNextEvent(everyEvent,&gTheEvent,gTimeSlice,NULL);


#if M5SUPPORT
	M5MovieIdle(&gTheEvent);
#endif

	if (ok) {
		// Handle the Event
		MyDoEvent(&gTheEvent);
	}
	else {
		// Nothing happened, kick back...
		if (IsDialogEvent(&gTheEvent))
			HandleDialogEvent(&gTheEvent);
		IdleObjects(&gTheEvent);
	}
}


void MyDoEvent(EventRecord *theEvent)
{
	short 		windowCode;
	WindowPtr	theWindow;


	if (IsDialogEvent(theEvent)){
		HandleDialogEvent(theEvent);
		return;
	}
	
	// 9/23/96 Allow IPAs first crack at events...
	PPAMgrEventHook(theEvent);

	switch (theEvent->what) {
	//
	// Was the mouse button pressed?
	case mouseDown:
		// Find out where the mouse went down
		windowCode = FindWindow (theEvent->where, &theWindow);

	  	switch (windowCode) {
		case inSysWindow: 	// Desk Accessory?
		    SystemClick (theEvent, theWindow);
		    break;
		    
		case inMenuBar:		// Menu Bar?
		  	MyAdjustMenus();
		    MyHandleMenu(MenuSelect(theEvent->where));
		    break;

		default:			// Cursor was inside our window
			// If the window isn't in the front
			if (theWindow == NULL ||
				((WindowPeek) theWindow)->refCon != ObjectWindowID) {
				SysBeep(1);
				return;
			}
			if (theWindow != FrontWindow()) {
				// Make it so...
				if (((ObjectWindowPtr) theWindow)->floating) {
					if (theWindow != FrontFloating())
						SelectFloating(theWindow);
				}
				else {
					if (theWindow != FrontDocument())
						SelectDocument(theWindow);
				}
				if (((WindowPeek) theWindow)->refCon != ObjectWindowID ||
					((ObjectWindowPtr) theWindow)->activeClick)
					return;
			}
			// else {	

				// Window is already in the front, handle the click
				switch (windowCode) {

				case inContent:		// Content area?
					if (((WindowPeek) theWindow)->refCon == ObjectWindowID)
						((ObjectWindowPtr) theWindow)->HandleClick(theWindow, theEvent->where, theEvent);
					break;

				case inDrag:		// Dragbar?
					{
						Rect	dragRect;
						dragRect = qd.screenBits.bounds;
						// Handle the dragging of the window
						DragWindow(theWindow, theEvent->where, &dragRect);
					}
					break;

				 case inGoAway:						// close box?
					// Only Grid Windows can be closed
				  	if (((WindowPeek) theWindow)->refCon == ObjectWindowID) {

						// Handle the mouse tracking for the close box
				  		if (TrackGoAway(theWindow, theEvent->where))
							// If mouse is released inside the close box
							// Hide or close the window
							((ObjectWindowPtr) theWindow)->Close(theWindow);
					}
				  	break;

				case inGrow:						// Grow box?
					{
						long	growResult;

						// Handle the mouse tracking for the resizing
						growResult = GrowWindow(theWindow,theEvent->where,&((ObjectWindowPtr) theWindow)->growRect);

					  	if (((WindowPeek) theWindow)->refCon == ObjectWindowID)
							((ObjectWindowPtr) theWindow)->Resize(theWindow, LoWord(growResult),HiWord(growResult));
					}
					break;
				case inZoomIn:
				case inZoomOut:
					if (theWindow == (WindowPtr) gRoomWin) {
						if (gIconized)
							DeIconize();
						else
							Iconize();
					}
					break;
				}
			// }
			break;
		}
		break;
		
	// Was a key pressed?
	case keyDown: 
	case autoKey:
		// Was the cmd-key being held down?  If so, process menu bar short cuts.
	    if ((theEvent->modifiers & cmdKey) != 0) {
			long	menuResult;
			// pascal	long MDEF_MenuKey (long theMessage, short theModifiers, MenuHandle menu);
	      	MyAdjustMenus();
	      	menuResult = MenuKey((char) (theEvent->message & charCodeMask));
			// menuResult = MDEF_MenuKey(theEvent->message, theEvent->modifiers, gFileMenu);
		  	MyHandleMenu(menuResult);
		}
		else {
			WindowPtr	theWin;
			theWin = FrontWindow();

			while (theWin) {
				if (((WindowPeek) theWin)->refCon == ObjectWindowID &&
					((ObjectWindowPtr) theWin)->ProcessKey != NULL) 
				{
					((ObjectWindowPtr) theWin)->ProcessKey(theWin, theEvent);
					break;
				}
				theWin = (WindowPtr) ((WindowPeek) theWin)->nextWindow;
			}
		}
		break;
/**
	case keyUp:
		{
			WindowPtr	theWin;
			theWin = FrontWindow();

			while (theWin) {
				if (((WindowPeek) theWin)->refCon == ObjectWindowID &&
					((ObjectWindowPtr) theWin)->ProcessKeyUp != NULL) 
				{
					((ObjectWindowPtr) theWin)->ProcessKeyUp(theWin, theEvent);
					break;
				}
				theWin = (WindowPtr) ((WindowPeek) theWin)->nextWindow;
			}
		}
		break;
**/
	// Does a window need to be redrawn?
	case updateEvt:
		theWindow = (WindowPtr) theEvent->message;
		if (((WindowPeek) theWindow)->refCon == ObjectWindowID)
			((ObjectWindowPtr) theWindow)->Update(theWindow);
	    break;

	// Has a window been activated or deactivated?
	case activateEvt:
		theWindow = (WindowPtr) theEvent->message;

		// Force it to be redrawn
		if (((WindowPeek) theWindow)->refCon == ObjectWindowID)
			((ObjectWindowPtr) theWindow)->Activate(theWindow, theEvent->modifiers & 1);
		break;
	case osEvt:
		// Force it to be redrawn
		switch (theEvent->message >> 24) {
		case suspendResumeMessage:
			theWindow = FrontWindow();
			gSuspended = !((theEvent->message & resumeFlag) > 0);
			ProcessSuspend();
			if (theWindow && ((WindowPeek) theWindow)->refCon == ObjectWindowID)
				((ObjectWindowPtr) theWindow)->Activate(theWindow,(theEvent->message & resumeFlag) > 0);
			break;
		}
		break;		
	case kHighLevelEvent:
		if (gSysEnv.systemVersion >= 0x0700) {
			if (theEvent->message == ServerEventID) {
#if THINK_C
#if __option(profile)
				_profile = 1;
#endif
#else
#if __profile__
//				ProfilerSetStatus(true);
#endif
#endif
				ProcessAppleTalkEvent(theEvent);
#if THINK_C
#if __option(profile)
				_profile = 0;
#endif
#else
#if __profile__
//				ProfilerSetStatus(false);
#endif
#endif
			}
			else
				AEProcessAppleEvent(theEvent);
		}
		break;
    }
}	    

void ProcessSuspend()
{
	if (!gSuspended) {
		TEFromScrap();
		gSublaunch = false;
	}
	else {
		// ZeroScrap();
		// TEToScrap();
		if (gSuspended && gFullScreen && !gIconized && !gSublaunch)
			PartialScreen(true);
	}
}

// Do Idle Time Processing

void IdleObjects(EventRecord *theEvent)
{
	WindowPtr	theWin,nextWin;
	Point		p;
	Boolean		gotIt = false;
	theWin = FrontWindow();

#if THINK_C
#if __option(profile)
				_profile = 1;
#endif
#else
#if __profile__
//				ProfilerSetStatus(true);
#endif
#endif
	if (gTCPRecord)
		if (PalTCPIdle(theEvent))
			return;

	while (theWin) {
		nextWin = (WindowPtr) ((WindowPeek) theWin)->nextWindow;
		SetPort(theWin);
		p = theEvent->where;
		GlobalToLocal(&p);
		if (PtInRect(p,&theWin->portRect)) {
			if (((WindowPeek) theWin)->refCon == ObjectWindowID)
				((ObjectWindowPtr) theWin)->AdjustCursor((WindowPtr) theWin,p,theEvent);
			gotIt = true;
			break;
		}
		theWin = nextWin;
	}
	if (!gotIt)
		SetCursor(&qd.arrow);
			
	theWin = FrontWindow();
	while (theWin) {
		nextWin = (WindowPtr) ((WindowPeek) theWin)->nextWindow;
		if (((WindowPeek) theWin)->refCon == ObjectWindowID &&
			((ObjectWindowPtr) theWin)->Idle)
			((ObjectWindowPtr) theWin)->Idle((WindowPtr) theWin,theEvent);
		theWin = nextWin;
	}
#if THINK_C
#if __option(profile)
				_profile = 0;
#endif
#else
#if __profile__
//				ProfilerSetStatus(false);
#endif
#endif
}

#if TIMEBOMB
short TimeBomb(void)
{
	DateTimeRec	dtr;
	unsigned long deadLine,curSecs;
	
	
	dtr.year = 1995;
	dtr.month = 12;
	dtr.day = 1;
	dtr.hour = 0;
	dtr.minute = 0;
	dtr.second = 0;
	dtr.dayOfWeek = 0;
	Date2Secs(&dtr,&deadLine);
	GetDateTime(&curSecs);

	// If clock is over deadline
	if (curSecs > deadLine)
		return -1;
	else
		return 0;
}
#endif

void CheckEnvironment()
{
	OSErr	oe;
	short	eID = 0;
	static		char *eStr[] = 
				{"Please upgrade your Palace software",
				 "The Palace needs a Color Macintosh - sorry!",
				 "The Palace needs 32-bit Quickdraw - sorry!",
				 "Warning: There is currently a bug which causes the Palace to crash occasionally if you aren't in 256 colors.\r\rWe recommend that you quit the program and reset your monitor to 256 colors."};
	long		gResp,gestaltAnswer;
	int			GetPixelDepth();

#if TIMEBOMB
	if (TimeBomb()) {
		ReportMessage(RE_OldVersion);
		ExitToShell();
	}
#endif

	oe = SysEnvirons(curSysEnvVers,&gSysEnv);
	if (eID != noErr) {
		eID = 1;
		goto ErrorExit;
	}
	if (!gSysEnv.hasColorQD) {
		eID = 2;
		goto ErrorExit;
	}
	oe = Gestalt(gestaltVersion, &gestaltAnswer);
	if (!gestaltAnswer)
		goto ErrorExit;
	oe = Gestalt(gestaltQuickdrawVersion,&gResp);
	if (gResp < gestalt32BitQD) {
		eID = 3;
		goto ErrorExit;
	}
#if TIMEBOMB
	if (TimeBomb()) {
		eID = 0;
		ReportMessage(RE_OldVersion);
		goto ErrorExit;
	}
#endif
	// Finally fixed the 256 colors bug
	// if (GetPixelDepth() != 8)
	// {
	//	ErrorMessage(eStr[3]);
	// }
	return;
ErrorExit:
	ErrorExit(eStr[eID-1]);
}

int	GetPixelDepth()
{
	GDHandle	curDevice;
	curDevice = GetGDevice();
	if (curDevice)
		return (*(*curDevice)->gdPMap)->pixelSize;
	else
		return 1;
}

Boolean GotRequiredParams(AppleEvent *theEvent)
{
   DescType returnedType;
   Size 	actualSize;
   OSErr	err;
   err = AEGetAttributePtr ( theEvent, keyMissedKeywordAttr, 
						typeWildCard, &returnedType, NULL, 0, 
						&actualSize);
   
   return err == errAEDescNotFound;
   
 }	/* CAppleEvent::GotRequiredParams */


void DoOpenEvent(AppleEvent *theEvent)
{
	Handle		docList = NULL;
	long		i;
	FSSpec		myFSS;
	AEDescList	theList;
	AEKeyword	aeKeyword=keyDirectObject;
	long		itemCount;
	DescType	actualType;
	Size		actualSize;
	OSErr		oe;

	if ((oe = AEGetParamDesc( theEvent, keyDirectObject, typeAEList, &theList)) != noErr) {
		// DebugStr("\pAEGetParamDesc");
		return;
	}


	if (!GotRequiredParams(theEvent)) {
		// DebugStr("\pGotRequiredParams");
		return;
	}

	if ((oe = AECountItems( &theList, &itemCount)) != noErr) {
		// DebugStr("\pAECountItems");
		return;
	}


	for (i = 1; i <= itemCount; i++)
	{
		oe = AEGetNthPtr( &theList, i, typeFSS, &aeKeyword, &actualType,
						(Ptr) &myFSS, sizeof( FSSpec), &actualSize);

		if (oe == noErr) {
			// Open file based on myFSS
		}
	}
	AEDisposeDesc(&theList);
	// event was handled successfully
}

pascal OSErr AppleEventHandler(AppleEvent *theEvent,AppleEvent *reply, long refCon);
pascal OSErr AppleEventHandler(AppleEvent *theEvent,AppleEvent *reply, long refCon)
{	
	DescType	actualType;
	Size		actualSize;
	DescType	eventClass, eventID;
	OSErr		oe;

	if ((oe = AEGetAttributePtr( (AppleEvent*) theEvent, keyEventClassAttr,
					typeType, &actualType, (Ptr) &eventClass, 
					sizeof(eventClass), &actualSize)) != noErr)
			return oe;
							
	
	if ((oe = AEGetAttributePtr(  (AppleEvent*) theEvent, keyEventIDAttr,
					typeType, &actualType, (Ptr) &eventID, 
					sizeof(eventID), &actualSize)) != noErr)
			return oe;
									
	if (eventClass == kCoreEventClass)
	{
		switch (eventID)
		{
		case kAEOpenApplication:
			if (GotRequiredParams(theEvent))
			{
				// gGopher->DoCommand( cmdNew);
				// anAppleEvent->SetErrorResult( noErr);
				// AskEditWindow();
			}
			break;
				
		case kAEOpenDocuments:
			DoOpenEvent( theEvent);
			break;
				
		case kAEPrintDocuments:
			break;
			
		case kAEQuitApplication:
			if (GotRequiredParams(theEvent))
			{
				gQuitFlag = true;
			}
			break;
		}		
	}
#define NS_NetscapeSuiteOut		'WWW?'
#define NS_NetscapeSuite		'WWW!'
#define NS_OpenURL				'OURL'
#define NS_WebActivate			'ACTV'
#define NS_URLSpec				'----'

#define SURL_SuiteID			'GURL'
#define SURL_GetURL				'GURL'
#define SURL_Text				'TEXT'
	else if (eventClass == NS_NetscapeSuiteOut ||
			eventClass == NS_NetscapeSuite)	{		// Netscape	
		switch (eventID) {
		case NS_OpenURL: 
			{						// Open URL
				DescType	actualType;
				char		urlStr[512];
				Size		actualSize;
				oe = AEGetParamPtr((AppleEvent*) theEvent, NS_URLSpec, typeChar, 
								&actualType, urlStr, 512, &actualSize);
				if (oe != noErr)
					return oe;
				urlStr[actualSize] = 0;
				if (GotRequiredParams(theEvent)) {
					ProcessSerialNumber psn;
					GetCurrentProcess(&psn);
					SetFrontProcess(&psn);
					GotoURL(urlStr);
				}
			}
			break;		
		}
	}
	else if (eventClass == SURL_SuiteID) {
		switch (eventID) {
		case SURL_GetURL: 
			{						// Open URL
				DescType	actualType;
				char		urlStr[512];
				Size		actualSize;
				oe = AEGetParamPtr((AppleEvent*) theEvent, SURL_Text, typeChar, 
								&actualType, urlStr, 512, &actualSize);
				if (oe != noErr)
					return oe;
				urlStr[actualSize] = 0;
				if (GotRequiredParams(theEvent))
					GotoURL(urlStr);
			}
			break;		
		}
	}
	return noErr;
}


static AEEventHandlerUPP	aeEventHandler;

void InitAppleEvents(void)
{
	if (gSysEnv.systemVersion >= 0x0700) {
	aeEventHandler = NewAEEventHandlerProc(AppleEventHandler);
	AEInstallEventHandler(typeWildCard, typeWildCard,
								aeEventHandler,
								0,FALSE);
	}
}

Boolean GiveTime(short sleepTime)
{
	// SpinCursor(1);
	return true;
}

void SetIdleTimer(long x)
{
	if (gConnectionType == C_PalaceTCP /*  && !gSuspended */)
		gTimeSlice = TIMER_FAST;
	else
		gTimeSlice = x;
}

/* end Evtlab.c */