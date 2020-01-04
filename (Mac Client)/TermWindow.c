/************************************************************************************
 * CommWindow.c
 *
 * Dumb (VT-100) Terminal
 *
 ************************************************************************************/
#include "TermWindow.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

// Create a new main window using a 'WIND' template from the resource fork
//
void TermString(WindowPtr theWin, char *str);

WindowPtr OpenTermWindow(Ptr termRec, short resID)
{
	WindowPtr			theWindow;
	TermWindowPtr		dWin;
	// OSErr				oe;

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 
	if (termRec == NULL)
		dWin = (TermWindowPtr) NewPtrClear(sizeof(TermWindowRecord));
	else
		dWin = (TermWindowPtr) termRec;

	theWindow = InitObjectWindow(resID, (ObjectWindowPtr) dWin,false);
	MoveWindow(theWindow,10,qd.screenBits.bounds.bottom - (theWindow->portRect.bottom+12),true);
	SetWTitle(theWindow, "\pTerm");
	dWin->tx = 1;
	dWin->ty = 1;
	dWin->topScroll = 1;
	dWin->botScroll = 25;

	((ObjectWindowPtr) theWindow)->Draw = TermDraw;
	((ObjectWindowPtr) theWindow)->HandleClick = TermHandleClick;
	
	// Show the window
	ShowWindow(theWindow);

	// Make it the current grafport
	SetPort(theWindow);
	TextFont(monaco);
	TextSize(9);
	TextMode(srcCopy);
	TermInit(theWindow);
	return theWindow;
}


// Respond to an update event - BeginUpdate has already been called.
//

void TermDraw(WindowPtr theWindow)
{
	Point	penPt;
	Rect	r;
	// EraseRect(&theWindow->portRect);
	r = theWindow->portRect;
	r.left = r.right - 15;
	r.top = r.bottom - 15;
	ClipRect(&r);
	DrawGrowIcon(theWindow);
	SetRect(&r,-32767,-32767,32767,32767);
	ClipRect(&r);

	GetPen(&penPt);

	MoveTo(0,4+26*11);
	LineTo(theWindow->portRect.right,4+26*11);

	MoveTo(penPt.h,penPt.v);
	// ShowStatus();

	TermRefresh(theWindow);
}


// Respond to a mouse-click - highlight cells until the user releases the button
//
void TermHandleClick(WindowPtr theWin, Point where, EventRecord *theEvent)
{

}
void TermCursorBlink(WindowPtr theWin)
{
	Rect	r;
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	SetPort(theWin);
	if (!tWin->cursorInvisible) {
		SetRect(&r,LeftM+tWin->tx*CharW,TopM+tWin->ty*LineH,
					LeftM+(tWin->tx+1)*CharW,TopM+(tWin->ty+1)*LineH);
		InvertRect(&r);
		tWin->cursorState ^= 1;
	}
}

void TermCursorReset(WindowPtr theWin)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	if (tWin->cursorState) {
		TermCursorBlink(theWin);
	}
}

void TermShowCursor(WindowPtr theWin)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	tWin->cursorInvisible = false;
	TermCursorBlink(theWin);
}

void TermHideCursor(WindowPtr theWin)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	TermCursorReset(theWin);
	tWin->cursorInvisible = true;
}

void TermPosPen(WindowPtr theWin, short y, short x)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	SetPort(theWin);

	TermCursorReset(theWin);
	tWin->tx = (x > 0)? x : 1;
	tWin->ty = (y > 0)? y : 1;
	MoveTo(LeftM+x*CharW,TopM+(y+1)*LineH-2);
	if (tWin->ty > 23)
		TermCursorBlink(theWin);
}

void TermScrollRgn(WindowPtr theWin, short r1, short r2)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	tWin->topScroll = r1;
	tWin->botScroll = r2;
}

void TermScroll(WindowPtr theWin)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	Rect		r1,r2,r3;
	RgnHandle	clipRgn;
	short		y;

	SetPort(theWin);
	clipRgn = (theWin == FrontWindow())? NULL : theWin->visRgn;
	TermCursorReset(theWin);
	SetRect(&r1,LeftM,TopM+(tWin->topScroll*LineH),LeftM+81*CharW,TopM+(tWin->botScroll+1)*LineH);

	// Clip to Screens
	r3 = qd.screenBits.bounds;
	GlobalToLocal(&topLeft(r3));
	GlobalToLocal(&botRight(r3));
	SectRect(&r1,&r3,&r1);

	r2 = r1;
	r1.top += LineH;
	r2.bottom -= LineH;
	CopyBits(&theWin->portBits,&theWin->portBits,
			&r1,&r2,srcCopy,clipRgn);
	r1.top = r1.bottom - LineH;
	EraseRect(&r1);
	for (y = tWin->topScroll; y < tWin->botScroll; ++y)
		BlockMove(&tWin->rBuf[y+1][1],&tWin->rBuf[y][1],80);
	memset(&tWin->rBuf[tWin->botScroll][1],' ',80);
}


void TermVideo(WindowPtr theWin, short n)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	SetPort(theWin);
	switch (n) {
	case 7:	// Inverse
		TextMode(notSrcCopy);
		break;
	case 1:	// Bold
		TextFace(bold);
		break;
	case 4:	// Underline
		tWin->underlineMode = true;
		break;
	case 0:	// Normal
		tWin->underlineMode = false;
		TextFace(0);
		TextMode(srcCopy);
		break;
	}
}

void TermEraseLine(WindowPtr theWin, short n)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	Rect	r;
	SetPort(theWin);
	TermCursorReset(theWin);
	// lineFlag = true;
	// *cp = 0;
	// cp = lbuffer;
	switch (n) {
	case 1:	// Start of Line to Cursor
		SetRect(&r,LeftM,TopM+tWin->ty*LineH,LeftM+tWin->tx*CharW,TopM+(tWin->ty+1)*LineH);
		EraseRect(&r);
		memset(&tWin->rBuf[tWin->ty][1],' ',tWin->tx);
		break;
	case 2:	// Entire Line
		SetRect(&r,LeftM,TopM+tWin->ty*LineH,LeftM+81*CharW,TopM+(tWin->ty+1)*LineH);
		EraseRect(&r);
		memset(&tWin->rBuf[tWin->ty][1],' ',80);
		break;
	default:	// Cursor to end of Line
		SetRect(&r,LeftM+tWin->tx*CharW,TopM+tWin->ty*LineH,LeftM+81*CharW,TopM+(tWin->ty+1)*LineH);
		EraseRect(&r);
		memset(&tWin->rBuf[tWin->ty][tWin->tx],' ',81-tWin->tx);
		break;
	}
}

void TermCls(WindowPtr theWin, short n)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	Rect	r;
	SetPort(theWin);
	TermCursorReset(theWin);
	switch (n) {
	case 0:	// Erase cursor to end of screen
		SetRect(&r,LeftM+tWin->tx*CharW,TopM+tWin->ty*LineH,LeftM+81*CharW,TopM+(tWin->ty+1)*LineH);
		EraseRect(&r);
		memset(&tWin->rBuf[tWin->ty][tWin->tx],' ',81-tWin->tx);
		SetRect(&r,LeftM,TopM+(tWin->ty+1)*LineH,LeftM+81*CharW,TopM+26*LineH);
		EraseRect(&r);
		memset(&tWin->rBuf[tWin->ty+1][1],' ',(25-(tWin->ty+1))*82);
		break;
	case 1:
		// Erase Start of Display to Cursor
		SetRect(&r,LeftM,TopM,LeftM+81*CharW,TopM+(tWin->ty-1)*LineH);
		EraseRect(&r);
		memset(&tWin->rBuf[1][1],' ',82*(tWin->ty-1));
		SetRect(&r,LeftM,TopM+tWin->ty*LineH,LeftM+tWin->tx*CharW,TopM+(tWin->ty+1)*LineH);
		EraseRect(&r);
		memset(&tWin->rBuf[tWin->ty][1],' ',tWin->tx);
		break;
	case 2:	// Erase screen
		SetRect(&r,LeftM,TopM,LeftM+81*CharW,TopM+26*LineH);
		EraseRect(&r);
		memset(&tWin->rBuf[0][0],' ',82*26);
		break;
	}
}

void TermLinefeed(WindowPtr theWin)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	SetPort(theWin);
	TermCursorReset(theWin);
	++tWin->ty;
	if (tWin->ty > tWin->botScroll) {
		TermScroll(theWin);
		tWin->ty = tWin->botScroll;
	}
	TermPosPen(theWin,tWin->ty,tWin->tx);
}

void TermReturn(WindowPtr theWin)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	TermCursorReset(theWin);
	tWin->tx = 1;
	TermPosPen(theWin,tWin->ty,tWin->tx);
}

void TermBackspace(WindowPtr theWin)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	TermCursorReset(theWin);
	if (tWin->tx > 1)
		--tWin->tx;
	TermPosPen(theWin,tWin->ty,tWin->tx);
}

void TermUp(WindowPtr theWin, short n)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	TermCursorReset(theWin);
	if (n <= 0)
		n = 1;
	tWin->ty -= n;
	TermPosPen(theWin,tWin->ty,tWin->tx);
}

void TermDown(WindowPtr theWin, short n)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	TermCursorReset(theWin);
	if (n <= 0)
		n = 1;
	tWin->ty += n;
	TermPosPen(theWin,tWin->ty,tWin->tx);
}

void TermRight(WindowPtr theWin, short n)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	TermCursorReset(theWin);
	if (n <= 0)
		n = 1;
	tWin->tx += n;
	TermPosPen(theWin,tWin->ty,tWin->tx);
}

void TermLeft(WindowPtr theWin, short n)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	TermCursorReset(theWin);
	if (n <= 0)
		n = 1;
	tWin->tx -= n;
	TermPosPen(theWin,tWin->ty,tWin->tx);
}


void TermTab(WindowPtr theWin)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	TermCursorReset(theWin);
	tWin->tx = ((tWin->tx-1)/8+1)*8 + 1;
	if (tWin->tx > 80) {
		TermReturn(theWin);
		TermLinefeed(theWin);
	}
	else
		TermPosPen(theWin,tWin->ty,tWin->tx);
}

void TermRefresh(WindowPtr theWin)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	short	y;
	TermCursorReset(theWin);
	for (y = 1; y <= 25; ++y) {
		MoveTo(LeftM+CharW,TopM+(y+1)*LineH-2);
		DrawText(&tWin->rBuf[y][1],0,80);
	}
	TermPosPen(theWin,tWin->ty,tWin->tx);
}

void TermChar(WindowPtr theWin, short c)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	SetPort(theWin);
	TermCursorReset(theWin);
	DrawChar(c);
	tWin->rBuf[tWin->ty][tWin->tx] = c;
	if (tWin->underlineMode) {
		Line(-6,0);
		Move(6,0);
	}
	++tWin->tx;
	if (tWin->tx > 80) {
		tWin->tx = 1;
		++tWin->ty;
		if (tWin->ty > tWin->botScroll) {
			TermScroll(theWin);
			tWin->ty = tWin->botScroll;
		}
		TermPosPen(theWin,tWin->ty,tWin->tx);
	}
}

void TermString(WindowPtr theWin, char *str)
{
	while (*str) {
		TermChar(theWin, *str);
		++str;
	}
}

void TermInit(WindowPtr theWin)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	TermPosPen(theWin,1,1);
	TermVideo(theWin,0);
}

Boolean TermCharAvail(WindowPtr theWin, char *c)
{
	return false;
}

/***
Boolean TermPollChar(WindowPtr theWin)
{
	TermWindowPtr	tWin = (TermWindowPtr) theWin;
	short	i;
	char	c;

	if (!tWin->CharAvail(theWin, &c)) {
		if (TickCount() - tWin->lastBlink > 30) {
			TermCursorBlink(theWin);
			tWin->lastBlink = TickCount();
		}
		return false;
	}

Reparse:
	switch (tWin->parseMode) {
	case 0:
		switch (c) {
		case 27:
			if (tWin->showControls)
				TermChar(theWin, '¥');
			else
				tWin->parseMode = 1;
			break;
		case 13:
			if (tWin->showControls)
				TermChar(theWin, 0xa8);
			TermReturn(theWin);
			break;
		case 10:
			if (tWin->showControls)
				TermChar(theWin, 0xc2);
			TermLinefeed(theWin);
			break;
		case 9:
			TermTab(theWin);
			break;
		case 8:
			TermBackspace(theWin);
			break;
		case 7:
			SysBeep(1);
			break;
		default:
			TermChar(theWin,c);
		}
		break;
	case 1:
		if (c != '[') {
			tWin->parseMode = 0;
			goto Reparse;
		}
		else {
			tWin->qFlag = tWin->pFlag = 0;
			tWin->v = 0;
			tWin->args[0] = 0;
			tWin->args[1] = 0;
			tWin->nbrArgs = 0;
			tWin->parseMode = 2;
		}
		break;
	case 2:
		if (c == '?') {
			tWin->qFlag = 1;
		}
		else if (c == '{' || c == '}')
			tWin->pFlag = 1;
		else if (c == ';') {
			tWin->args[tWin->nbrArgs] = tWin->v;
			tWin->v = 0;
			++tWin->nbrArgs;
		}
		else if (isdigit(c)) {
			tWin->v = tWin->v*10 + c-'0';
		}
		else {
			tWin->args[tWin->nbrArgs] = tWin->v;
			++tWin->nbrArgs;
			switch (c) {
			case 'A':	TermUp(theWin,tWin->args[0]);				break;
			case 'B':	TermDown(theWin,tWin->args[0]);				break;
			case 'C':	TermRight(theWin,tWin->args[0]);				break;
			case 'D':	TermLeft(theWin,tWin->args[0]);				break;
			case 'K':	TermEraseLine(theWin,tWin->args[0]);			break;
			case 'r':	TermScrollRgn(theWin,tWin->args[0],tWin->args[1]);	break;
			case 'H':	TermPosPen(theWin,tWin->args[0],tWin->args[1]);	break;
			case 'm':	TermVideo(theWin,tWin->args[0]);				break;
			case 'J':	TermCls(theWin,tWin->args[0]);				break;
			case 'h':	if (tWin->qFlag && tWin->args[0] == 25)
							TermShowCursor(theWin);
						break;
			case 'l':	if (tWin->qFlag && tWin->args[0] == 25)
							TermHideCursor(theWin);
						break;
			}
			tWin->parseMode = 0;
		}
	}
	return true;
}

**/

