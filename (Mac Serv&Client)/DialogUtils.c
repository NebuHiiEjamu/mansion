/**************************************************************
 * DialogUtils.c							for Class 6
 *
 * A collection of useful functions for manipulating dialogs
 **************************************************************/

#include "DialogUtils.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <Dialogs.h>

// Set a control's state (such as a checkbox or a radio button)
//
void SetControl(DialogPtr dial, int item, int value)
{
	Handle handle;
	short type;
	Rect r;

	GetDItem(dial,item,&type,&handle,&r);
	SetCtlValue((ControlHandle) handle,value);
}

// Get a control's state (such as a checkbox or a radio button)
//
int GetControl(DialogPtr dial,register int item)
{
	Handle handle;
	short type;
	Rect r;

	GetDItem(dial,item,&type,&handle,&r);
	return GetCtlValue((ControlHandle) handle);
}

// Toggle a control's state (such as a checkbox or a radio button)
//
void ToggleControl(DialogPtr dial, int item)
{
	int value;
	value = GetControl(dial,item);
	SetControl(dial,item,!value);
}

// Copy a pascal string to a dialog's text item (static text or edit text)
//
void SetText(DialogPtr dial, int item, StringPtr text)
{
	Handle handle;
	short type;
	Rect r;

	GetDItem(dial,item,&type,&handle,&r);
	SetIText(handle,text);
}

// Copy a dialog's text item to a pascal string.
//
void GetText(DialogPtr dial, int item, StringPtr text)
{
	Handle handle;
	short type;
	Rect r;

	GetDItem(dial,item,&type,&handle,&r);
	GetIText(handle,(StringPtr) text);
}

// Draw an outline around a button
//
void OutlineButton(DialogPtr dp, int bid, Pattern *pat)
{
	Handle	h;
	short	t;
	Rect	r;
	GrafPtr	gp;

	GetPort(&gp);
	SetPort(dp);
	GetDItem(dp,bid,&t,&h,&r);
	PenSize(3,3);
	PenPat(pat);
	InsetRect(&r,-4,-4);
	FrameRoundRect(&r,16,16);
	PenNormal();
	SetPort(gp);
}

// Gray out a disabled button
//
void DisableButton(DialogPtr dp, int bid)
{
	Handle	h;
	short	t;
	Rect	r;
	GrafPtr	gp;
	
	OutlineButton(dp,bid,&qd.white);
	GetPort(&gp);
	SetPort(dp);
	GetDItem(dp,bid,&t,&h,&r);
	HiliteControl((ControlHandle) h,255);
	PenPat(&qd.gray);
	PenMode(patBic);
	PaintRect(&r);
	PenNormal();
	SetPort(gp);
}

// Restore a previously disabled button
//
void EnableButton(DialogPtr dp, int bid)
{
	Handle	h;
	short	t;
	Rect	r;
	GrafPtr	gp;
	
	GetPort(&gp);
	SetPort(dp);
	GetDItem(dp,bid,&t,&h,&r);
	HiliteControl((ControlHandle) h,0);
	InvalRect(&r);
	SetPort(gp);
}

// Simulate the user pressing a button.  This is used to give the user some
// visual feedback when they use the keyboard as a shortcut to press a dialog button.
//
void SimulateButtonPress(DialogPtr dp, int bid)
{
	Handle	h;
	short	t;
	Rect	r;
	GrafPtr	gp;
	long	dmy;

	GetPort(&gp);
	SetPort(dp);
	GetDItem(dp,bid,&t,&h,&r);
	InvertRoundRect(&r,4,4);
	Delay(10,&dmy);
	InvertRoundRect(&r,4,4);
	Delay(10,&dmy);
	SetPort(gp);
}

// Use the sprintf function to add text to a dialog's static or edit text item
//
// Example:  MyPrintfItem(dp, TimeItem, "%02d:%02d:%02d", hours,minutes,seconds);
//
// This function uses the <stdarg.h> functions to implemenet a
// variable length argument list.

void PrintfItem(DialogPtr dp, int item, char *template,...)
{
	va_list		args;
	char		msg[256];

	va_start(args,template);
	vsprintf(msg,template,args);
	va_end(args);
	CtoPstr(msg);
	SetText(dp,item,(StringPtr) msg);
}

// Use the sscanf function to parse text from a dialog's static or edit text item
//
// Example:  ScanfItem(dp, TimeItem, "%02d:%02d:%02d", &hours,&minutes,&seconds);
//
// This function uses the <stdarg.h> functions to implemenet a
// variable length argument list.

int ScanfItem(DialogPtr dp, int item, char *template,...)
{
	va_list		args;
	char		msg[256];
	int			n;

#if !THINK_C
	int vsscanf(char *, const char *, _Va_list);
#endif

	return 0;
	GetText(dp,item,(StringPtr) msg);
	PtoCstr((StringPtr) msg);

	va_start(args,template);
#if !THINK_C
	n = vsscanf(msg,template,args);
#endif
	va_end(args);
	return n;
}

int GetIntItem(DialogPtr dp, int item)
{
	Handle	h;
	short	t;
	Rect 	r;
	Str255	str;
	GetDItem(dp,item,&t,&h,&r);
	GetIText(h,str);
	PtoCstr(str);
	return atoi((char *) str);
}

void PutIntItem(DialogPtr dp, int item, int n)
{
	Handle h;
	short t;
	Rect r;
	Str255	str;
	sprintf((char *) str, "%d", n);
	CtoPstr((char *) str);
	GetDItem(dp,item,&t,&h,&r);
	SetIText(h,str);
}

Boolean CallFilterProc(DialogPtr dp, EventRecord *ep, short *itemHit)
{
	ModalFilterUPP	theProc;
	OSErr oe;
	oe = GetStdFilterProc(&theProc);
	if (oe == noErr) {
		return CallModalFilterProc(theProc, dp, ep, itemHit);
	}
	else
		return false;
}
