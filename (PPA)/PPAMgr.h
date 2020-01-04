// ppamgr.h
//
#if PPASUPPORT
#ifndef _H_PPAMgr
#define _H_PPAMgr	1

#include "PPA.h"


#if USESROUTINEDESCRIPTORS

enum {
	uppPPAProcInfo	= kThinkCStackBased |
				STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long))) |
				STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(PPARecord *))) |
				STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long *)))
};

#define CallPPAProc(userRoutine, cmd, ppaRec, retCode)	\
		CallUniversalProc((UniversalProcPtr) (userRoutine), uppPPAProcInfo,	\
							cmd, ppaRec, retCode)
#else
#define CallPPAProc(userRoutine, cmd, ppaRec, retCode)	\
		(*(userRoutine))(cmd,ppaRec,retCode)
#endif

typedef struct PPAObject {
	struct PPAObject *nextPPA;
	PPARecord	prec;		// PPA Control Structure
	FileHandle	fRefNum;	// It's file handle, if any
	Handle		itsPPA;		// Handle to block of code
	PPAProcPtr	hookPtr;
	char		name[32];
	
} PPAObject;

typedef struct {
	PPAObject	*firstPPA;
	LONG		idCtr;
} PPAMgr;

// Codes for PPAMgrKillAllPPAs()
enum { PPAKILL_ROOM,			// Kill All
		PPAKILL_SERVER,		// Kill non-permanent ones
		PPAKILL_ALL };		// Kill non-serverwide/permenent ones

// PPAObject Methods
long PPAObjectSendMessage(PPAObject *a, LONG message);
void PPAObjectDispose(PPAObject *a);
void PPAObjectInit(PPAObject *newPPA, Handle ppaCode, FileHandle fRefNum, char *name);

/*
	void	InitIPAObject(Handle ipa,StringPtr name);
	void	Dispose(void);
	short	SendMessage(short message);
*/

// PPAManager Methods
void PPAMgrInit();
void PPAMgrCleanup();
void PPAMgrKillAllPPAs(LONG kind);
void PPAMgrLaunchPPA(Handle ppaCode, FileHandle fRefNum, char *ppaName, char *ppaParams);

void PPAMgrUnlink(PPAObject *ppa);
void PPAMgrKillPPA(PPAObject *a, Boolean refreshScreen);
void PPAMgrPPACommand(char *ppaMsg);
void PPAMgrPPAKillByName(char *name);

PPAObject *PPAMgrFindNonClonablePPA(char *name);
void PPAMgrIdle(void);
void PPAMgrEventHook(EventRecord *lastEvent);
void PPAMgrPropagatePalaceEvent(long eventType, long eventData, char *eventMessage);
void PPAMgrPropagateNetworkEvent(long eventType, long eventRefCon, long eventLength, char *eventBuffer);
void PPAMgrPropagateBlowThruEvent(long eventType, long eventRefcon, long eventLength, char *eventBuffer);
void PPAMgrRefreshPPAs(Rect *area, Boolean foregroundFlag);
void PPAMgrPreparePlugInsMenu(void);
void PPAMgrSelectPlugInsMenuItem(short menuItem);

#endif
#endif


