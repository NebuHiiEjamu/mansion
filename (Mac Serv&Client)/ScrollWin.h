// ScrollWin.h - inherits from ObjectWin
#ifndef _H_SCROLLWIN
#define _H_SCROLLWIN	1

#include "ObjectWindow.h"

#define SBarSize		16
#define GrowIconSize	14

typedef struct
{
	ObjectWindowRecord	objWin;
	// Scroll Stuff
	ControlHandle	vScrollBar,hScrollBar;
	Point			scrollOffset;
	Point			nbrUnits,unitsPerScreen,unitHeight;
	void			(*ComputeUnitsPerScreen)(WindowPtr theWin);
	void			(*ComputeUnits)(WindowPtr theWin);
	void			(*UpdateToScrollPosition)(WindowPtr theWin);
} ScrollWindow, *ScrollWindowPtr;

WindowPtr	NewScrollWindow(short winID, ScrollWindowPtr storage, Boolean vertScroll, Boolean horizScroll, Boolean floating);
void		SetupScrollBars(WindowPtr theWin, Boolean horiz, Boolean vert);
void		AdjustScrollBars(WindowPtr theWin, Boolean resizeFlag);
Boolean		ScrollHandleControlClick(WindowPtr theWin, Point where);
void		ScrollToPosition(WindowPtr theWin, Point newPos);
void		AutoScroll(WindowPtr theWin, Point pos);
void		DefaultComputeUnitsPerScreen(WindowPtr theWin);
void		DefaultComputeUnits(WindowPtr theWin);
void 		SetScrollUnits(WindowPtr theWin, Point p);
void 		SWUpdateToScrollPosition(WindowPtr theWin);

// Overrides
void 		ScrollResize(WindowPtr theWin, short w, short h);
void		ScrollActivate(WindowPtr theWin, Boolean activeFlag);
void 		PrintScrollWindow(WindowPtr dWin);


#endif