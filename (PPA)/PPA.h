/******************************************************************************
 PPA.h

	Author - Jim Bumgardner
	©1991 Warner New Media
 ******************************************************************************/
#ifndef _H_PPA
#define _H_PPA	1

#include "m-events.h"
#include "m-cmds.h"
#include "ppalocal.h"

typedef struct {
	LONG		id;					 	//  ----->
	LONG		flags;					// <----->
	LONG		idleTime;				// <-----
	LONG		lastIdle;				// <----->
	LONG		eventMask;				// <-----
	LONG		maxDataNeeded;			// <-----
	LONG		palaceEventType;		// <----->
	LONG		palaceEventData;		// <----->
	LONG		networkEventType;		// <----->
	LONG		networkEventRefcon;		//  ----->
	LONG		networkEventLength;		//  ----->
	LONG		localOSError;			// <-----
	LONG		runFlags;				//  ----->
	PPAHandle	userData;				// <-----
	PPARect		viewRect;				// <----->
	PPARect		refreshRect;			//  ----->
	PPACallbackPtr	PalaceControlFunction;	//  ----->
	PPAEventRecord 	*localEvent;		// <----->
	PPARootObjectPtr	rootObject;		//  ----->
	PPAPixMapH	pixMapH;				//  ----->
	PPAPtr		networkEventPtr;		//  ----->
	char		palaceEventMessage[256];// <----->
	char		verbalResult[256];		// <-----
	char		reserved[64];			// n/a
} PPARecord;

#define PPAF_RUNNING			1
#define PPAF_CLONEABLE			2	// Unimplemented
#define PPAF_FOREGROUND			4
#define PPAF_SERVERWIDE			8	// Unimplemented
#define PPAF_PERMANENT			16	// Unimplemented
#define PPAF_NEEDSNETEVENTS		32

enum PPASelections {
	PPA_LAUNCHNOTIFY,			/* First Call - setup maxDataNeeded if needed */
								/* Don't allocate anything here!  You may not
									get called again. */
								/* This may get called more than once */
								
	PPA_BEGIN,					/* First Call - Allocate Data, Setup ViewRect and Go */
								/* If staying resident,
									setup idleTime 
												0-none,	
												-1-all the time,
												other-max ticks
									setup eventMask for events you want.									
									setup ViewRect to indicate where you are
								*/
	PPA_TERMINATE,				/* Last Call - Dispose your UserData,
									Close your window, etc. */
									
	PPA_IDLE,					/* PPA was called during Dawdle Routine */
	PPA_REFRESH,				/* PPA should redraw itself */
	PPA_LOCALEVENT,				/* PPA has intercepted a Mac/Windows Event,
									set localEvent->what to nullEvent if you wish
									to trap it */
	PPA_PALACEEVENT,				/* PPA was sent an Idaho Event
									receive palaceEventType, palaceEventData and palaceEventMessage
								*/
	PPA_NETWORKEVENT,
	PPA_BLOWTHRUEVENT,			/* receives a blowthru event in 
								   networkEventType,networkEventRefcon,networkEventLength filled */
	NbrPPASelectionTypes
};

enum ReturnValues	{			/* Negative Return Values denote Fatal Errors */
	PPA_OK,						/* Hook will stay resident */
								/* Return Message displayed immediatly */
	PPA_CANCEL,					/* Abort - you will receive a terminate */
								/* after this - Idaho will display any returned message */
								/* Return Message preserved */
	NbrPPAReturnTypes
};

#define DoCallback(cmd, arg, str)	\
			CallPPACallback(gCtl->PalaceControlFunction, cmd, arg, str)

#endif