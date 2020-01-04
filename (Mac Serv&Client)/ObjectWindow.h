// ObjectWindow.h

#pragma once
#ifdef THINK_C
#include <PrintTraps.h>
#endif
#define ObjectWindowID	1000

typedef struct ObjectWindowRecord {
	CWindowRecord 	theWin;
	struct ObjectWindowRecord *theParent;
	Boolean			ownStorage;
	Boolean			floating;
	Boolean			active;
	Boolean			dirty;
	Boolean			activeClick;
	Rect			growRect;
	Handle			toolH;			// Tool List 6/10/95
	void		(*Dispose)(WindowPtr theWin);
	void		(*Update)(WindowPtr theWin);
	void		(*Activate)(WindowPtr theWin, Boolean activateFlag);
	void		(*HandleClick)(WindowPtr theWin, Point where, EventRecord *theEvent);
	void		(*Draw)(WindowPtr theWin);
	void		(*Idle)(WindowPtr theWin, EventRecord *theEvent);
	void		(*AdjustCursor)(WindowPtr theWin, Point where, EventRecord *theEvent);
	void		(*ProcessKey)(WindowPtr theWin, EventRecord *theEvent);
	void		(*ProcessKeyUp)(WindowPtr theWin, EventRecord *theEvent);
	void		(*Save)(WindowPtr theWin);
	void		(*SaveAs)(WindowPtr theWin);
	void		(*Resize)(WindowPtr theWin, short w, short h);
	void		(*Refresh)(WindowPtr theWin);
	void		(*PrintWindow)(WindowPtr theWin);
	void		(*Close)(WindowPtr theWin);	// Close Box hit
	void		(*ProcessCommand)(WindowPtr theWin, short theMenu, short theCommand);
	void		(*AdjustMenus)(WindowPtr theWin);
} ObjectWindowRecord, *ObjectWindowPtr;

WindowPtr InitObjectWindow(short resID, ObjectWindowPtr theStorage, Boolean isFloating);

extern THPrint		gHPrint;
extern Boolean		gIsPrinting;


void DisposeObjectWindow(WindowPtr theWin, Boolean disposeFlag);
void DefaultUpdate(WindowPtr theWin);
void DefaultActivate(WindowPtr theWin, Boolean activeFlag);
void DefaultHandleClick(WindowPtr theWin, Point where, EventRecord *theEvent);
void DefaultAdjustCursor(WindowPtr theWin, Point where, EventRecord *theEvent);
void DefaultDispose(WindowPtr theWin);
void DefaultDraw(WindowPtr theWin);
void DefaultResize(WindowPtr theWin, short w, short h);
void DefaultRefresh(WindowPtr theWin);
void DefaultPrintWindow(WindowPtr theWin);
void DefaultProcessCommand(WindowPtr theWin,short theMenu, short theCommand);
void DefaultClose(WindowPtr theWin);
void DefaultAdjustMenus(WindowPtr theWin);

void SetWindowParent(WindowPtr theWin, WindowPtr theParent);

void InitPrinting(void);
WindowPtr FrontDocument(void);
WindowPtr FrontFloating(void);
void SelectDocument(WindowPtr theWindow);
void SelectFloating(WindowPtr theWindow);
void LocateWindows(void);
