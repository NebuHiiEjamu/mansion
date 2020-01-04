/************************************************************************************
 * ManServ.c
 *
 *
 ************************************************************************************/
#include "S-SERVER.H"
#include "UserTools.h"
#include "ServRegistration.h"

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
void 	InitAppleEvents(void);
void 	CheckEnvironment();

Boolean		gSuspended,gQuitFlag,gModified,gDebugFlag;
SysEnvRec	gSysEnv;
long		gIdleTime=TIMER_FASTEST;
int			gMaxPeoplePerServer;

// Typical Macintosh Initialization Code


// Main Entry Point

void main()
{
	long	nbrMembers;
	unsigned long t;
	// Standard Mac Initialization

	MyInitMacintosh();

	// Check System
	CheckEnvironment();
	

	// Set up the menu bar
	MySetUpMenus();

	// InitAppleEvents
	InitAppleEvents();


	LoadSecureInfo();
	if (CheckServerSecurity()) 
	{
		(void) RegistrationDialog();
		
		if (CheckServerSecurity()) 
			ErrorExit("Sorry!  You need to register to use this program.  Please read the READ.ME for more information.");
	}

	// 12/11/95
	if (AllocateServerBuffers())
		ErrorExit("Sorry!  There isn't enough memory to run the server.");

	// Load Preferences
	LoadPreferences();

	// Open Graphics window
	NewLogWindow();



	// Init TCP
	if (gPrefs.allowTCP) {
		InitServerTCP();		//  5/8/95 JAB
		if (gPrefs.allowTCP == false)	// operator disconnected... - remember this
			StorePreferences();
	}		

	LogMessage("Reading Mansion Script...\r");

	if (ReadScriptFile("\pMansion Script"))
		ErrorExit("There is an unrecoverable error in the Mansion Script file.");

	SetWTitle((WindowPtr) gLogWin,gPrefs.serverName);


	LogMessage("Opening Assets...\r");

	InitServerAssets();

	InitTools();

	nbrMembers= CountAssets(RT_USERBASE);
	if (nbrMembers > 2)
		LogMessage("Over %ld members served!\r", nbrMembers-1);

	// Register on TCP Yellow Pages
	RegisterOnYellowPages(false, true);

	// Init LocalTalk
	LogMessage("Registering on local area network...\r");
	LogMessage("This may take a few seconds...\r");
	InitPPCStuff();

	LogMessage("Server Active\r");
	
	gDebugFlag = 0;				// 5/8/95


#if THINK_C
#if __option(profile)			// 6/15 Optional profiling support
	freopen("PServer.log","w",stdout);		// If console isn't wanted
	InitProfile(200,200);
	_profile = 0;
#endif
#else
#if __profile__
	ProfilerInit(collectDetailed,bestTimeBase,20,5);
	ProfilerSetStatus(false);
#endif
#endif
	// main event loop
	while (!gQuitFlag)					// Till the End of Time...
		MyHandleEvent();		// Get an Event, do something about it


#if !THINK_C
#if __profile__
	ProfilerDump("\pPServer.Profile");
	ProfilerTerm();
#endif
#endif
	LogMessage("Shutting down\r");

	// PostGlobalEventShort(MSG_SERVERDOWN);		// 7/22 LogoffUser does this now
	LogoffAllUsers();		//  5/8/95 JAB
	t = TickCount();
	while (gToBeKilled && TickCount() - t < 5*60L)
		ServerIdle();
	// Force kill of all users...
	while (gToBeKilled) {
		gToBeKilled->whyKilled = K_ServerDown;
		ServerIdle();
	}

	SaveScript();
	CleanupServerTCP();		//  5/8/95 JAB
	CloseServerAssets();
	CloseAllWindows();
	ClosePPCStuff();		// 6/8/95
}

void CloseAllWindows()
{
	WindowPtr	theWin;
	
	while ((theWin = FrontWindow()) != NULL)
		((ObjectWindowPtr) theWin)->Dispose(theWin);
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
}

// The Main Event Dispatcher - this routine should be called repeatedly

void MyHandleEvent(void)

{
	EventRecord	theEvent;
	Boolean		ok;

	// If the more modern WaitNextEvent is implemented, use it 

	ok = WaitNextEvent(everyEvent,&theEvent,gIdleTime,NULL);

	if (ok) {
		// Handle the Event
		MyDoEvent(&theEvent);
	}
	else {
		// Nothing happened, kick back...
		IdleObjects(&theEvent);
		ServerIdle();
	}
}


void MyDoEvent(EventRecord *theEvent)
{
	short 		windowCode;
	WindowPtr	theWindow;


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
			if (theWindow != FrontWindow()) {
				// Make it so...
				SelectWindow(theWindow);
				if (((WindowPeek) theWindow)->refCon != ObjectWindowID ||
					((ObjectWindowPtr) theWindow)->activeClick)
					return;
			}
			else {	

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
							((ObjectWindowPtr) theWindow)->Dispose(theWindow);
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
				}
			}
			break;
		}
		break;
		
	// Was a key pressed?
	case keyDown: 
	case autoKey:
		// Was the cmd-key being held down?  If so, process menu bar short cuts.
	    if ((theEvent->modifiers & cmdKey) != 0) {
	      MyAdjustMenus();
		  MyHandleMenu(MenuKey((char) (theEvent->message & charCodeMask)));
		}
		else {
			theWindow = FrontWindow();
			if (((WindowPeek) theWindow)->refCon == ObjectWindowID &&
				((ObjectWindowPtr) theWindow)->ProcessKey != NULL)
				((ObjectWindowPtr) theWindow)->ProcessKey(theWindow, theEvent);
		}
		break;

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
			if (theWindow && ((WindowPeek) theWindow)->refCon == ObjectWindowID)
				((ObjectWindowPtr) theWindow)->Activate(theWindow,(theEvent->message & resumeFlag) > 0);
			break;
		}
		break;		
	case kHighLevelEvent:
		if (gSysEnv.systemVersion >= 0x0700) {
			if (theEvent->message == UserEventID) {
#if THINK_C
#if __option(profile)
				_profile = 1;
#endif
#else
#if __profile__
				ProfilerSetStatus(true);
#endif
#endif
				ProcessLocalTalkEvent(theEvent);
#if THINK_C
#if __option(profile)
				_profile = 0;
#endif
#else
#if __profile__
				ProfilerSetStatus(false);
#endif
#endif
			}
			else
				AEProcessAppleEvent(theEvent);
		}
		break;
    }
}	    

// Do Idle Time Processing

void IdleObjects(EventRecord *theEvent)
{
	WindowPeek	theWin;
	theWin = (WindowPeek) FrontWindow();
	while (theWin) {
		if (theWin->refCon == ObjectWindowID &&
			((ObjectWindowPtr) theWin)->Idle)
			((ObjectWindowPtr) theWin)->Idle((WindowPtr) theWin,theEvent);
		theWin = theWin->nextWindow;
	}
	ServerTCPIdle();		// 5/8/95
}

void CheckEnvironment()
{
	OSErr	oe;
	short	eID = 0;
	static		char *eStr[7] = 
				{"This Mac is ancient history - sorry!",
				 "This Program needs a Color Macintosh - sorry!",
				 "This Program needs 32-bit Quickdraw - sorry!"};
	long		gResp,gestaltAnswer;

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

	// if (gSysEnv.processor >= 1 && gSysEnv.processor <= 4)
	//	gSlowMachine = true;


	return;
ErrorExit:
	ErrorExit(eStr[eID-1]);
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
		DebugStr("\pAEGetParamDesc");
		return;
	}


	if (!GotRequiredParams(theEvent)) {
		DebugStr("\pGotRequiredParams");
		return;
	}

	if ((oe = AECountItems( &theList, &itemCount)) != noErr) {
		DebugStr("\pAECountItems");
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
		
	return noErr;
}


AEEventHandlerUPP	aeEventHandler;

void InitAppleEvents(void)
{
	if (gSysEnv.systemVersion >= 0x0700) {
		aeEventHandler = NewAEEventHandlerProc(AppleEventHandler);
		AEInstallEventHandler(typeWildCard, typeWildCard,
								aeEventHandler,
								0,FALSE);
	}
}

void SetIdleTime(long idle)
{
	gIdleTime = idle;
}

/* end Evtlab.c */