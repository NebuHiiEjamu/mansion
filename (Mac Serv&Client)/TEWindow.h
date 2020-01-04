// TEWindow.h
#pragma once

#include "ObjectWindow.h"
#include "ScrollWin.h"

typedef struct {
	ScrollWindow		win;
	TEHandle			teH;
	Boolean				localStorage,dirty;
} TEWindowRec, *TEWindowPtr;

WindowPtr NewTEWindow(short winID, TEWindowPtr storage, Boolean floating);
void TEWindowDraw(WindowPtr theWin);
void TEWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent);
void TEWindowKey(WindowPtr theWin, EventRecord *theEvent);
void DisposeTEWin(WindowPtr theWin);
void TEWindowResize(WindowPtr theWin, short width, short height);
void TEWindowIdle(WindowPtr theWin, EventRecord *theEvent);
void TEWindowActivate(WindowPtr theWin, Boolean activeFlag);
void TEAdjustMenus(WindowPtr theWin);
void TEProcessCommand(WindowPtr theWin, short theMenu, short theItem);
void TEUpdateToScrollPosition(WindowPtr theWindow);
