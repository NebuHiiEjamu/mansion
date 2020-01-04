// PPAMgr.c
/* TODO

	Update Callbacks in Docs

	Provide a way for the iptscrae callback to return a value (make DoUserScript
	return a value if something is left on the stack).

   Consider allowing PPAs to modify/censor network events.
*/

#include "u-user.h"
#include "m-cmds.h"
#include "PPAMgr.h"
#include "AppMenus.h"

#if PPASUPPORT

PPAMgr	gPPAMgr;

/* PPA Manager Routines */

// This is called at by main to initialize the PPA manager.  
// It searches the "PlugIns" directory for PPAs, and creates a heirarchical menu of named
// PPAs, which is added to the Options menu.  Users can turn plug-ins on and off
// by using this menu - when a PPA is active, a checkmark appears next to it on
// the menu.

void PPAMgrInit(void)
{
	DirInfo		pbRec;
	HFileInfo	pbRec2;
	Str255		dirName="\p:plugins",fileName="\p";
	
	LONG	dirID;
	OSErr	oe;
	LONG	n,i;
	// Add support for built-in plug-ins
	StatusMessage("Searching for Plug-ins...",0);
	AddResMenu(gPlugInsMenu, 'PPA ');

	// !!! Scan plug-in folder and add PPA Menu bar if appropriate
	pbRec.ioCompletion = NULL;
	pbRec.ioNamePtr = dirName;
	pbRec.ioFDirIndex = 0;	// !!!
	pbRec.ioVRefNum = 0;
	pbRec.ioDrDirID = 0;
	oe = PBGetCatInfo((CInfoPBRec *) &pbRec,false);
	if (oe != noErr) {
		LogMessage("Plugin Folder not found (%d)",oe);
		return;
	}
	dirID = pbRec.ioDrDirID;
	n = pbRec.ioDrNmFls;

	for (i = 1; i <= n; ++i) {
		pbRec2.ioCompletion = NULL;
		pbRec2.ioNamePtr = fileName;
		pbRec2.ioFDirIndex = i;
		pbRec2.ioVRefNum = 0;
		pbRec2.ioDirID = dirID;
		oe = PBGetCatInfo((CInfoPBRec *) &pbRec2,false);
		if (oe != noErr) {
			LogMessage("Plugin Folder file search error (%d)",oe);
			return;
		}
		if (pbRec2.ioFlFndrInfo.fdType == 'PPAF') {
			AppendMenu(gPlugInsMenu, pbRec2.ioNamePtr);
		}
	}
}

// This is called by the Menu Manager when the PPA menu needs to be drawn - it provides
// checkmarks next to the active PPAs on the menu.
void PPAMgrPreparePlugInsMenu(void)
{
	short	n,i;
	Str63	pStr;
	Boolean	needsCheck;

	n = CountMItems(gPlugInsMenu);
	for (i = 1; i <= n; ++i) 
	{
		GetItem(gPlugInsMenu, i, pStr);
		PtoCstr(pStr);
		needsCheck = (PPAMgrFindNonClonablePPA((char *) pStr) != NULL);
		CheckItem(gPlugInsMenu, i, needsCheck);
	}
}

// This is called by the Menu Manager when a user has selected a PPA on the menu.
// It either turns off the PPA (if it is active) or launches it.
void PPAMgrSelectPlugInsMenuItem(short menuItem)
{
	PPAObject	*ppa;
	Str63		pStr;
	GetItem(gPlugInsMenu, menuItem, pStr);
	PtoCstr(pStr);
	ppa = PPAMgrFindNonClonablePPA((char *) pStr);
	if (ppa) {
		PPAMgrKillPPA(ppa, true);			// Kill it
	}
	else {
		PPAMgrPPACommand((char *) pStr);	// Launch It
	}
}

// This is called by the main module when the program is exiting.  It Kills
// all active PPAs.
void PPAMgrCleanup(void)
{
	PPAMgrKillAllPPAs(PPAKILL_ALL);
}

// This is the routine for killing a PPA.  A boolean flag "refreshScreen"
// allows the caller to specify whether the screen should be
// redrawn after killing it (for example, when killing all the
// active PPAs when quitting, we don't need to redraw the
// screen, otherwise, we usually do

void PPAMgrKillPPA(PPAObject *ppa, Boolean refreshScreen)
{
	PPAObject	*a,*lastPPA;
	Rect		ppRect;
	if (ppa) {
		PPAObjectSendMessage(ppa, PPA_TERMINATE);
		ppRect = ppa->prec.viewRect;

		// Unlink from the list
		lastPPA = NULL;
		for (a = gPPAMgr.firstPPA; a; a = a->nextPPA) {
			if (a == ppa) {
				if (lastPPA == NULL)
					gPPAMgr.firstPPA = ppa->nextPPA;
				else
					lastPPA->nextPPA = ppa->nextPPA;
				break;
			}
			lastPPA = a;
		}

		PPAObjectDispose(ppa);

		// Refresh the area of the screen left behind
		if (refreshScreen && !EmptyRect(&ppRect))
			RefreshRoom(&ppRect);

	}
}

// This allows all PPAs of a particular type to be killed.
// Available types are:
//		PPAKILL_ALL		Kill all of them
//		PPAKILL_SERVER	Kill server-local PPAs
//		PPAKILL_ROOM	Kill room-local PPAs
//
// This routine is called from:
//  PPAMgrCleanUp()			(kills all of them)
//	DisconnectUser()		(kills server-local PPAs)
//  ClearRoom() (u-rooms.c)	(kills room-specific PPAs)

void PPAMgrKillAllPPAs(LONG killType)
{
	PPAObject	*a,*b;
	for (a = gPPAMgr.firstPPA; a; a = b) {
		b = a->nextPPA;
		switch (killType) {
		case PPAKILL_SERVER:
			if (a->prec.flags & PPAF_PERMANENT)
				continue;
			break;
		case PPAKILL_ROOM:
			if (a->prec.flags & (PPAF_PERMANENT | PPAF_SERVERWIDE))
				continue;
			break;
		}
		PPAMgrKillPPA(a, false);
	}
}

// This is called from U-CMDS.c to kill a PPA by name, in response
// to the PC_KillPPA command.

void PPAMgrPPAKillByName(char *ppaName)
{
	PPAObject	*ppa;
	
	// If an active IPA that matches the name is launched, send it
	// the message
	if ((ppa = PPAMgrFindNonClonablePPA(ppaName)) != NULL) {
		PPAMgrKillPPA(ppa, true);
	}
}

// This is the procedure for launching a PPA.
// It can be used to either launch a ppa, or send an active PPA
// a message.
//
// Called from SelectPlugsInMenuItem()
// Called from DoPalaceCommand (u-cmds.c) in response to PC_LaunchPPA
// Called from SF_LAUNCHPPA (u-sfuncs.c) in response to any of the
// following iptscrae commands:
//
// 	"PPANAME" LAUNCHPPA
// 	"PPANAME(PARAM)" LAUNCHPPA
// 	"PPANAME" TALKPPA
// 	"PPANAME(PARAM)" TALKPPA
//

void PPAMgrPPACommand(char *ppaMsg)
{
	char		ppaName[256],ppaParams[256],*p,*p2;
	PPAObject	*ppa;
	long		result;
	
	// Parse PPA Name & Param
	strcpy(ppaName,ppaMsg);
	if ((p = strchr(ppaName,'(')) != NULL) {
		// Trim off whitespace before parens
		p2 = p - 1;
		while (p2 > &ppaName[0] && isspace(*p2)) {
			*p2 = 0;
			--p2;
		}

		// Extract Params
		*p = 0;
		strcpy(ppaParams,p+1);
		if ((p = strrchr(ppaParams,')')) != NULL)
			*p = 0;
	}
	else {
		ppaParams[0] = 0;
	}			

	// If an active IPA that matches the name is launched, send it
	// the message
	if ((ppa = PPAMgrFindNonClonablePPA(ppaName)) != NULL) {
		ppa->prec.palaceEventType = PE_PPAMessage;
		ppa->prec.palaceEventData = 0;
		strcpy(ppa->prec.palaceEventMessage,ppaParams);
		result = PPAObjectSendMessage(ppa,PPA_PALACEEVENT);
		if (result != PPA_OK) {
			PPAMgrKillPPA(ppa, true);
		}
	}
	else {
		Handle	ppaCode;
		Str255	ppaPName,ppaCName;
		short	fRefNum = 0;

		Boolean	targetWasFolder,wasAliased;
		
		strcpy((char *) ppaCName, ppaName);
		CtoPstr((char *) ppaCName);

		ppaCode = GetNamedResource('PPA ',ppaCName);
		if (ppaCode == NULL) {
			FSSpec	fSpec;
			// Look for PPA File in Plug-ins dir and open it
			strcpy((char *) ppaPName,":plugins:");
			strcat((char *) ppaPName,ppaName);
			CtoPstr((char *) ppaPName);
			FSMakeFSSpec(0,0L,ppaPName,&fSpec);
			ResolveAliasFile(&fSpec,true,&targetWasFolder,&wasAliased);
			fRefNum = FSpOpenResFile(&fSpec,fsRdWrPerm);
			if (fRefNum == -1)
				return;
			ppaCode = GetNamedResource('PPA ',ppaCName);
			if (ppaCode == NULL) {
				ppaCode = GetResource('PPA ',60);
				if (ppaCode == NULL) {
					CloseResFile(fRefNum);
					return;
				}
			}
		}
		DetachResource(ppaCode);
		PPAMgrLaunchPPA(ppaCode, fRefNum, ppaName, ppaParams);
	}
}

// Called from PPAMgrPPACommand - it takes a code resource that has
// been loaded into memory, and converts it into a PPAObject,
// and launches the PPA.
//
void PPAMgrLaunchPPA(Handle ppaCode, FileHandle fRefNum, char *name, char *params)
{
	PPAObject	*newPPA;

	newPPA = (PPAObject *) NewPtrClear(sizeof(PPAObject));
	if (newPPA == NULL)
		return;

	PPAObjectInit(newPPA, ppaCode, fRefNum, name);

	// Link it in
	newPPA->nextPPA = gPPAMgr.firstPPA;
	gPPAMgr.firstPPA = newPPA;

	++gPPAMgr.idCtr;
	newPPA->prec.id = gPPAMgr.idCtr;

	PPAObjectSendMessage(newPPA, PPA_LAUNCHNOTIFY);

	/*
	 * Fail if you don't have enough memory + 32K padding
	 */
	if (FreeMem() < newPPA->prec.maxDataNeeded + 32000) {
		PPAMgrKillPPA(newPPA, false);
		return;
	}

	newPPA->prec.flags |= PPAF_RUNNING;

	strcpy(newPPA->prec.palaceEventMessage,params);
	if (PPAObjectSendMessage(newPPA,PPA_BEGIN) != PPA_OK) {
		PPAMgrKillPPA(newPPA, true);
	}
	else {
		if (!EmptyRect(&newPPA->prec.viewRect)) {
			RefreshRoom(&newPPA->prec.viewRect);
		}
	}
}

// This is called from AlarmsIdle() (u-alarms.c) to give PPA routines 
// idle time
//
void PPAMgrIdle(void)
{
	PPAObject	*a,*b;
	register long	tc;
	for (a = gPPAMgr.firstPPA; a; a = b) {
		b = a->nextPPA; 	/* Make a copy of next IPA in list, in case this
						        one gets terminated */
		if (a->prec.idleTime == 0)
			continue;
		if (a->prec.idleTime == -1L || (tc = TickCount()) - a->prec.lastIdle >= a->prec.idleTime) {
			a->prec.lastIdle = tc;
			if (PPAObjectSendMessage(a, PPA_IDLE) != PPA_OK) {
				PPAMgrKillPPA(a, true);
			}
		}
	}
}

// Called from the Macintosh code to provide Macintosh (keyboard+mouse)
// events to PPAs.
void PPAMgrEventHook(EventRecord *lastEvent)
{
	PPAObject	*a,*b;
	long		result;

	// --	ignore non-window messages to IPAs when an IPA window is present
	//		(assumption for now: IPA windows are modal)
	
	/* if (ModalPPAWindowIsActive())
		return;
	 */
	//

	for (a = gPPAMgr.firstPPA; a && lastEvent->what != nullEvent; a = b) {
		b = a->nextPPA;
		if ((a->prec.eventMask & (1 << lastEvent->what)) == 0)
			continue;
		a->prec.localEvent = lastEvent;
		result = PPAObjectSendMessage(a, PPA_LOCALEVENT);
		if (result != PPA_OK) {
			PPAMgrKillPPA(a, true);
		}
	}
}

// This routine passes Palace Events to Active PPAs.
//
// This is called from TriggerHotspotEvent() (u-rgns.c) (which generates
// high level events for iptscrae, and from ProcessMacro() (u-cmds.c)
// which processes local macros which begin with ~
//
void PPAMgrPropagatePalaceEvent(long eventType, long eventData, char *eventMessage)
{
	PPAObject	*a,*b;
	short		result;

	//	ignore non-window messages to IPAs when an IPA window is present
	//	(assumption for now: IPA windows are modal)
	
	/* if (ModalIPAWindowIsActive())
		return;
	 */
	//

	for (a = gPPAMgr.firstPPA; a; a = b) {
		b = a->nextPPA;
		a->prec.palaceEventType = eventType;
		a->prec.palaceEventData = eventData;
		if (eventMessage)
			strcpy(a->prec.palaceEventMessage, eventMessage);
		else
			a->prec.palaceEventMessage[0] = 0;
		result = PPAObjectSendMessage(a, PPA_PALACEEVENT);
		if (eventType == PE_InChat || eventType == PE_OutChat) {
			// Chat events modify the string..
			strcpy(eventMessage, a->prec.palaceEventMessage);
		}
		if (result != PPA_OK) {
			PPAMgrKillPPA(a, true);
		}
	}
}

// This routine passes Network events to PPAs
//
// Called from ProcessMansionEvent() (u-events.c)
// 
void PPAMgrPropagateNetworkEvent(long eventType, long eventRefcon, long eventLength, char *eventBuffer)
{
	PPAObject	*a,*b;
	LONG		result;

	//	ignore non-window messages to IPAs when an IPA window is present
	//	(assumption for now: IPA windows are modal)
	
	/* if (ModalIPAWindowIsActive())
		return;
	 */
	//

	for (a = gPPAMgr.firstPPA; a; a = b) {
		b = a->nextPPA;
		if (a->prec.flags & PPAF_NEEDSNETEVENTS) {
			a->prec.networkEventType = eventType;
			a->prec.networkEventRefcon = eventRefcon;
			a->prec.networkEventLength = eventLength;
			a->prec.networkEventPtr = eventBuffer;
			result = PPAObjectSendMessage(a, PPA_NETWORKEVENT);
			if (result != PPA_OK) {
				PPAMgrKillPPA(a, true);
			}
		}
	}
}

typedef struct tagBlowThruEvent {
	long	flag ;
	long	users ;
	long	signature ;
} BlowThruEvent ;

// Probably change the network ids to a blowthru id. 
//@@@ PPAF_NEEDSBLOWTHRUEVENT
//@@@ PPA_BLOWTHRUEVENT 

void PPAMgrPropagateBlowThruEvent(long eventType, long eventRefcon, long eventLength, char *eventBuffer)
{
	BlowThruEvent *bte	=	(BlowThruEvent *)eventBuffer ;
	PPAObject	*a,*b;
	LONG		result;

	//@@@ TEMP@@ Currently checks for PPAF_NEEDSNETEVENTS before sending
	// the blowthru event.

	for (a = gPPAMgr.firstPPA; a; a = b)
	{
		b = a->nextPPA;
		if (a->prec.flags & PPAF_NEEDSNETEVENTS)
		{
			// Check for the plugins signature. if same then send event.

			a->prec.networkEventType = eventType;
			a->prec.networkEventRefcon = eventRefcon;
			a->prec.networkEventLength = eventLength;
			a->prec.networkEventPtr = eventBuffer;
			result = PPAObjectSendMessage(a, PPA_BLOWTHRUEVENT);
			if (result != PPA_OK) 
			{
				PPAMgrKillPPA(a, true);
			}
		}
	}
}

// This routine refreshes active PPAs.  PPAs should draw into the
// active graphics world (which will be an offscreen pixel map).
//
// Called from RefreshRoom() (RoomGraphics.c) twice, once for background 
// layer and once  for foreground layer.  
// PPAs use the PPAF_FOREGROUND flag to specify
// which layer they should be drawn in.
//
void PPAMgrRefreshPPAs(Rect *area, Boolean foregroundFlag)
{
	PPAObject	*a,*b;
	long		result;
	Rect		t;
	for (a = gPPAMgr.firstPPA; a; a = b) {
		b = a->nextPPA;
		if ((a->prec.flags & PPAF_FOREGROUND) > 0 ==
			foregroundFlag > 0 &&
			SectRect(area,&a->prec.viewRect,&t)) {
			a->prec.refreshRect = *area;
			result = PPAObjectSendMessage(a, PPA_REFRESH);
			if (result != PPA_OK) {
				PPAMgrKillPPA(a, false);
			}
			TextFont(gPrefs.fontID);
			TextSize(gPrefs.fontSize);
			TextMode(srcOr);
			// Restore Font STuff
		}
	}
}

// Used internally by the PPAMgr to find active PPAs by name.
//
PPAObject *PPAMgrFindNonClonablePPA(char *name)
{
	PPAObject	*a;
	for (a = gPPAMgr.firstPPA; a; a = a->nextPPA)
		if (stricmp(name,a->name) == 0 && !(a->prec.flags & PPAF_CLONEABLE))
			return a;
	return NULL;
}

// Used internally by the PPAMgr to initialize a PPAObject
//
void PPAObjectInit(PPAObject *newPPA, Handle ppaCode, FileHandle fRefNum, char *name)
{
	newPPA->itsPPA = ppaCode;
	HLockHi(ppaCode);

	// 68k client calls 68k plug-ins,
	// PPC client calls PPC or 68k plug-ins
	newPPA->hookPtr = (PPAProcPtr) (*ppaCode);
	strcpy(newPPA->name, name);
	newPPA->prec.rootObject = (void *) gRoomWin;

	// Setting up Callback function
	newPPA->prec.PalaceControlFunction = 
				NewRoutineDescriptor((ProcPtr) DoPalaceCommand,
									uppPPACallbackInfo,
									GetCurrentISA());
	newPPA->prec.pixMapH = gRoomWin->offPixMap;
}

// Used internally by the PPAMgr to send a message to a PPA
//
// This is the main entry point for actually calling a PPA.
//
LONG PPAObjectSendMessage(PPAObject *ppa, LONG message)
{
	LONG		tmp = PPA_OK;
	GrafPtr		gp1,gp2;				// 1/29/92

	GetPort(&gp1);

	// !!! implement flags
	// ppa->irec.gRunFlags = gRunFlags;			

	CallPPAProc(ppa->hookPtr, message,&ppa->prec, &tmp);

	// 2/13/92  Add Verbal Result Code to IPA
	if (tmp != PPA_OK && ppa->prec.verbalResult[0]) {
		if (message != PPA_REFRESH) {
			// Don't print log messages while offscreen
			LogMessage("%s\r",ppa->prec.verbalResult);
			// Throw up a dialog if msg doesn't start with ';'
			if (ppa->prec.verbalResult[0] != ';')
				ErrorMessage("%s",ppa->prec.verbalResult);
		}
		ppa->prec.verbalResult[0] = 0;
	}

	GetPort(&gp2);
	if (gp1 != gp2) {
		SetPort(gp1);
		// gAppDir->itsMainPane->ForceNextPrepare();
	}
	return	tmp;
}

// Used internally by the PPAMgr to dispose of a PPA Object
//
void PPAObjectDispose(PPAObject *a)
{
	// Dispose of routine descriptor, code handle, close file
	if (a) {
		if (a->prec.PalaceControlFunction)
			DisposeRoutineDescriptor(a->prec.PalaceControlFunction);
		if (a->itsPPA)
			DisposeHandle(a->itsPPA);
		if (a->fRefNum)
			CloseResFile(a->fRefNum);
		DisposePtr((Ptr) a);
	}
}

#endif
