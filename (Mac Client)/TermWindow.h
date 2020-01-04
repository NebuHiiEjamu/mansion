// TermWindow.h
#ifndef _H_TermWindow
#define _H_TermWindow	1

#include "ObjectWindow.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>


#define TopM	0
#define LeftM	4
#define CharW	6
#define LineH	11

typedef struct {
	ObjectWindowRecord	oWin;
	short	topScroll, botScroll;
	short	tx,ty;
	short	cursorState;
	Boolean	cursorInvisible,underlineMode,showControls;
	char	rBuf[26][82];
	short	args[4];
	short	parseMode,v,nbrArgs;
	long	lastBlink,qFlag,pFlag;
} TermWindowRecord, *TermWindowPtr;

WindowPtr OpenTermWindow(Ptr termRec, short resID);

void TermCursorBlink(WindowPtr theWin);
void TermCursorReset(WindowPtr theWin);
void TermShowCursor(WindowPtr theWin);
void TermHideCursor(WindowPtr theWin);
void TermPosPen(WindowPtr theWin, short y, short x);
void TermScrollRgn(WindowPtr theWin, short r1, short r2);
void TermScroll(WindowPtr theWin);
void TermVideo(WindowPtr theWin, short n);
void TermEraseLine(WindowPtr theWin, short n);
void TermCls(WindowPtr theWin, short n);
void TermLinefeed(WindowPtr theWin);
void TermReturn(WindowPtr theWin);
void TermBackspace(WindowPtr theWin);
void TermUp(WindowPtr theWin, short n);
void TermDown(WindowPtr theWin, short n);
void TermRight(WindowPtr theWin, short n);
void TermLeft(WindowPtr theWin, short n);
void TermTab(WindowPtr theWin);
void TermRefresh(WindowPtr theWin);
void TermInit(WindowPtr theWin);
Boolean TermPollChar(WindowPtr theWin);
void TermChar(WindowPtr theWin, short c);

void TermRefresh(WindowPtr theWin);
void TermDraw(WindowPtr theWindow);
void TermHandleClick(WindowPtr theWin, Point where, EventRecord *theEvent);
void TermKey(WindowPtr theWin, EventRecord *theEvent);

// Term Overrides
void	TermRecordChar(WindowPtr theWin, char c);
void	TermRecordLine(WindowPtr theWin);
Boolean TermCharAvail(WindowPtr theWin, char *c);

#endif