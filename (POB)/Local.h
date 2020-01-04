// Mac Local.h
#ifndef __LOCAL__
#define __LOCAL__

#ifndef macintosh
#define macintosh 1
#endif

#include "ObjectWindow.h"
#include "TEWindow.h"
#include <EPPC.h>
#include <AppleEvents.h>
#include <GestaltEqu.h>
#include <QDOffscreen.h> // esr 5-19
#include <Picker.h>      // esr 5-19
#include <stdio.h>

// #define ServerEventID		'mSrv'		moved to events
// #define UserEventID			'mUsr'

#define LONG	long
#define HugePtr Ptr
#define huge	
#define FileHandle short

#define MEMDEBUG		1
#define PALACE_IN_A_BOX	1

#if MEMDEBUG

void MyDisposeHandle(Handle *h);
void MyDisposePtr(Ptr *p);
void MyReleaseResource(Handle *h);
OSErr MyFSClose(FileHandle *f);
Handle MyNewHandleClear(Size size);
Ptr MyNewPtrClear(Size size);

#define DisposeHandle(h)	MyDisposeHandle(&(h))
#define DisposePtr(p)		MyDisposePtr(&(p))
#define FSClose(f)			MyFSClose(&(f))
#define ReleaseResource(h)	MyReleaseResource(&(h))
#define NewHandle(s)		MyNewHandleClear(s)
#define NewHandleClear(s)	MyNewHandleClear(s)
#define NewPtr(s)			MyNewPtrClear(s)
#define NewPtrClear(s)		MyNewPtrClear(s)

#endif

////////////////////////////////////////////////////////////////////
// esr 5-19
#define	UnionRectMac(r1,r2,r3)  	UnionRect(r1,r2,r3)
#define SetRectMac(r,x1,y1,x2,y2)	SetRect(r,x1,y1,x2,y2)
#define OffsetRectMac(r,x,y)		OffsetRect(r,x,y)
#define SectRectMac(r1,r2,r3)  		SectRect(r1,r2,r3)
#define IsRectEmptyMac(r)  			EmptyRect(r)
#define PtInRgnMac					PtInRgn
#define PtInRectMac					PtInRect
#define InsetRectMac				InsetRect
#define LPSTR						char *
#define BOOL						Boolean
#define TIMER_FASTEST	0L
#define TIMER_FAST 		1L
#define TIMER_SLOW		20L
#define TIMER_SLEEP		-1L
////////////////////////////////////////////////////////



#define GetTicks()			TickCount()
#define TICK_SECONDS		60L

// Prototypes should be in S-FUNC.H


/*void ReportError(short code, char *name);*/
/*void ErrorExit(char *str,...);*/
/*void ProcessAppleTalkEvent(EventRecord *er);*/
/*void ProcessMansionEvent(long cmd, long msgRefCon, char *buffer, long len);*/
/*PicHandle	GetPictureFromFile(StringPtr fName);*/
/**/
/*// Random Number Routines*/
/*void MySRand(long s);*/
/*long GetSRand();*/
/*void Randomize();*/
/*long LongRandom(void);*/
/*double DoubleRandom(void);*/
/*short MyRandom(short max);*/
/*void EncryptString(StringPtr inStr, StringPtr outStr);*/
/*Boolean EqualPString(StringPtr inStr, StringPtr outStr, Boolean caseSens);*/

// Client

#ifdef powerc

#define BreakToLowLevelDebugger_() 		SysBreak()
#define BreakStrToLowLevelDebugger_(s) 	SysBreakStr(s)
#define BreakToSourceDebugger_()		 Debugger()
#define BreakStrToSourceDebugger_(s)	 DebugStr(s)

#else // 68K

#define BreakToLowLevelDebugger_()		 Debugger()
#define BreakStrToLowLevelDebugger_(s) DebugStr(s)
#define BreakToSourceDebugger_()		 SysBreak()
#define BreakStrToSourceDebugger_(s) 	SysBreakStr(s)

#endif

typedef struct {
	long	versID;
	Point	logPos,roomPos,propEditorPos, drawPos, propPickerPos, scriptPos, userListPos,
			roomListPos, facePickerPos, aboutBoxPos, pictListPos;
	Boolean	partialScreen,logVisible,propPickerVisible,facePickerVisible;
	Point	logSize;
} MacPrefs;
extern MacPrefs	gMacPrefs;


#endif __LOCAL__