// ScrollWin.cp

#include "ScrollWin.h"
#include <PPCToolBox.h>

// Descendent window must maintain lineHeight, nbrLines

WindowPtr NewScrollWindow(short winID, ScrollWindowPtr storage, Boolean vertScroll, Boolean horizScroll, Boolean floating)
{
	WindowPtr		theWindow;

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 
	if (storage == NULL)
		storage = (ScrollWindowPtr) NewPtrClear(sizeof(ScrollWindow));
	theWindow = InitObjectWindow(winID, (ObjectWindowPtr) storage,floating);

	((ObjectWindowPtr) theWindow)->Activate = ScrollActivate;
	((ObjectWindowPtr) theWindow)->Resize = ScrollResize;
	((ObjectWindowPtr) theWindow)->PrintWindow = PrintScrollWindow;
	((ScrollWindow *) theWindow)->ComputeUnitsPerScreen = DefaultComputeUnitsPerScreen;
	((ScrollWindow *) theWindow)->ComputeUnits = DefaultComputeUnits;
	((ScrollWindow *) theWindow)->UpdateToScrollPosition = SWUpdateToScrollPosition;

	SetupScrollBars(theWindow,horizScroll,vertScroll);

	// Show the window
	ShowWindow(theWindow);

	// Make it the current grafport
	SetPort(theWindow);
	return theWindow;
}


void SetupScrollBars(WindowPtr theWin, Boolean horiz, Boolean vert)
{
	Rect			sRect,r;
	ScrollWindowPtr	sWin = (ScrollWindowPtr) theWin;
	// !! Set up vertical scroll bar
	//
	if (vert) {
		r = theWin->portRect;
		sRect.left = r.right - (SBarSize - 1);
		sRect.top = r.top - 1;
		sRect.right = r.right + 1;
		sRect.bottom = r.bottom  - GrowIconSize;
	 	sWin->vScrollBar = NewControl(theWin,&sRect,"\p",true,
	 								0,0,sRect.bottom - sRect.top, scrollBarProc,1L);
	}
	if (horiz) {
		r = theWin->portRect;
		sRect.top = r.bottom - (SBarSize - 1);
		sRect.left = r.left - 1;
		sRect.bottom = r.bottom + 1;
		sRect.right = r.right  - GrowIconSize;
	 	sWin->vScrollBar = NewControl(theWin,&sRect,"\p",true,
	 								0,0,0, scrollBarProc,1L);
	}
	AdjustScrollBars(theWin, true);
}

void ScrollResize(WindowPtr theWin, short w, short h)
{
	ScrollWindowPtr	sWin = (ScrollWindowPtr) theWin;
	DefaultResize(theWin, w,h);
	sWin->ComputeUnitsPerScreen(theWin);
	sWin->ComputeUnits(theWin);
	AdjustScrollBars(theWin, true);
}

void ScrollActivate(WindowPtr theWin, Boolean activeFlag)
{
	ScrollWindowPtr	sWin = (ScrollWindowPtr) theWin;

	DefaultActivate(theWin, activeFlag);

	HiliteControl(sWin->vScrollBar, activeFlag? 0 : 255);
}

void DefaultComputeUnitsPerScreen(WindowPtr theWin)
{
	ScrollWindowPtr	sWin = (ScrollWindowPtr) theWin;
	
	if (sWin->unitHeight.v == 0)
		sWin->unitsPerScreen.v = 1;
	else
		sWin->unitsPerScreen.v = ((theWin->portRect.bottom - theWin->portRect.top) - SBarSize)/sWin->unitHeight.v;
	if (sWin->unitHeight.h == 0)
		sWin->unitsPerScreen.h = 1;
	else
		sWin->unitsPerScreen.h = ((theWin->portRect.right - theWin->portRect.left) - SBarSize)/sWin->unitHeight.h;
}

void DefaultComputeUnits(WindowPtr theWin)
{
	// Assume they are already correct
	AdjustScrollBars(theWin,false);
}


void SetScrollUnits(WindowPtr theWin, Point p)
{
	ScrollWindowPtr	sWin = (ScrollWindowPtr) theWin;
	sWin->nbrUnits.h = p.h;
	sWin->nbrUnits.v = p.v;
	AdjustScrollBars(theWin,false);
}


void AdjustScrollBars(WindowPtr theWin, Boolean resizeFlag)
{
	short			w,h;
	GrafPtr			savePort;
	ScrollWindowPtr	sWin = (ScrollWindowPtr) theWin;

	GetPort(&savePort);

	if (resizeFlag) {
		// Adjust Lines Per Page
		sWin->ComputeUnitsPerScreen(theWin);

		// Move sliders to new position
		//
		MoveControl(sWin->vScrollBar, theWin->portRect.right - (SBarSize - 1),
											theWin->portRect.top - 1);	
		// Change their sizes to fit new window dimensions
		//
		w = 16;
		h = theWin->portRect.bottom - theWin->portRect.top - (GrowIconSize - 1);
		SizeControl(sWin->vScrollBar,w,h);

		// Reset their maximum values
		//
		SetCtlMax(sWin->vScrollBar,h);
	}

	// Set the value of the sliders accordingly
	//
	h = theWin->portRect.bottom - theWin->portRect.top - (GrowIconSize - 1);
	if (sWin->nbrUnits.v > 0) {
		h = sWin->nbrUnits.v - sWin->unitsPerScreen.v;
		if (h < 0)
			h = 0;
		SetCtlMax(sWin->vScrollBar,h);
		if (sWin->scrollOffset.v > h)
			sWin->scrollOffset.v = h;
		SetCtlValue(sWin->vScrollBar,sWin->scrollOffset.v);
	}
	else {
		SetCtlMax(sWin->vScrollBar,0);
		SetCtlValue(sWin->vScrollBar,0);
	}
		
	SetPort(savePort);
}

//
// Callback routine to handle arrows and page up, page down
//
ScrollWindow		*gDWin;

pascal void MyScrollAction(ControlHandle theControl, short thePart);

pascal void MyScrollAction(ControlHandle theControl, short thePart)
{
	Point		curPos,newPos;
	short		pageWidth;
	Rect		myRect;
	WindowPtr	gp = (GrafPtr) gDWin;

	curPos = gDWin->scrollOffset;
	newPos = curPos;

	myRect = ((WindowPtr) gp)->portRect;
	myRect.right -= SBarSize-1;
	myRect.bottom -= SBarSize-1;

	pageWidth = (gDWin->unitsPerScreen.v-1);

	if (theControl == ((ScrollWindowPtr) gp)->vScrollBar) {
		switch (thePart) {
		  case inUpButton:		newPos.v = curPos.v - 1;			break;
		  case inDownButton:	newPos.v = curPos.v + 1;			break;
	  	  case inPageUp:		newPos.v = curPos.v - pageWidth;	break;
		  case inPageDown:		newPos.v = curPos.v + pageWidth;	break;
		}
	}
	else {
		switch (thePart) {
		  case inUpButton:		newPos.h = curPos.h - 1;			break;
		  case inDownButton:	newPos.h = curPos.h + 1;			break;
	  	  case inPageUp:		newPos.h = curPos.h - pageWidth;	break;
		  case inPageDown:		newPos.h = curPos.h + pageWidth;	break;
		}
	}

	if (newPos.h > gDWin->nbrUnits.h - gDWin->unitsPerScreen.h)
		newPos.h = gDWin->nbrUnits.h - gDWin->unitsPerScreen.h;
	if (newPos.h < 0)
		newPos.h = 0;
	if (newPos.v > gDWin->nbrUnits.v - gDWin->unitsPerScreen.v)
		newPos.v = gDWin->nbrUnits.v - gDWin->unitsPerScreen.v;
	if (newPos.v < 0)
		newPos.v = 0;

	if (!EqualPt(newPos,curPos)) {
		ScrollToPosition((WindowPtr) gDWin, newPos);
	}
}

// Intercept Handler for scroll bars
// Returns true if user clicked on scroll bar
//

Boolean ScrollHandleControlClick(WindowPtr theWin, Point where)
{
	short 			ctlPart;
	ControlHandle	whichControl;
	short			vPos;
	ProcPtr			trackActionProc;
	ScrollWindowPtr	sWin = (ScrollWindowPtr) theWin;
	ControlActionUPP clickProc;
	Boolean			trackResult;

	// Check if user clicked on scrollbar
	//
	if ((ctlPart = FindControl(where,theWin,&whichControl)) == 0)
		return false;

	// Identify window for callback procedure
	//
	gDWin = (ScrollWindowPtr) theWin;

	// Use default behavior for thumb
	// Program will crash if you don't!!
	//
	if (ctlPart == inThumb)
		trackActionProc = 0L;
	else
		trackActionProc = (ProcPtr) MyScrollAction;

	// Perform scrollbar tracking
	//
	clickProc = NewControlActionProc(trackActionProc);
	trackResult = ((ctlPart = TrackControl(whichControl, where, clickProc)) != 0);
	DisposeRoutineDescriptor(clickProc);
	if (!trackResult)
		return false;

	// if ((ctlPart = TrackControl(whichControl, where, trackActionProc)) == 0)
	//	return false;
	if (ctlPart == inThumb) {
		Point	newPos;
		vPos = GetCtlValue(sWin->vScrollBar);
		newPos = gDWin->scrollOffset;
		newPos.v = vPos;
		ScrollToPosition(theWin, newPos);
	}

	return true;
}

// Scroll to an explicit position
//
void ScrollToPosition(WindowPtr theWin, Point newPos)
{
	ScrollWindowPtr	sWin = (ScrollWindowPtr) theWin;
	
	SetPort(theWin);

	if (newPos.h > sWin->nbrUnits.h-sWin->unitsPerScreen.h)
		newPos.h = sWin->nbrUnits.h-sWin->unitsPerScreen.h;
	if (newPos.h < 0)
		newPos.h = 0;
	if (newPos.v > sWin->nbrUnits.v-sWin->unitsPerScreen.v)
		newPos.v = sWin->nbrUnits.v-sWin->unitsPerScreen.v;
	if (newPos.v < 0)
		newPos.v = 0;

	sWin->scrollOffset = newPos;
	
	// Adjust Scrollbars
	AdjustScrollBars(theWin, false);

	sWin->UpdateToScrollPosition(theWin);
}

void SWUpdateToScrollPosition(WindowPtr theWin)
{
	((ObjectWindowPtr) theWin)->Draw(theWin);
}

void AutoScroll(WindowPtr theWin, Point pos)
{
	Point	offset;
	offset = ((ScrollWindowPtr) theWin)->scrollOffset;
	if (pos.v < theWin->portRect.top)
		offset.v--;
	else if (pos.v >= theWin->portRect.bottom)
		offset.v++;
	if (pos.h < theWin->portRect.left)
		offset.h--;
	else if (pos.h >= theWin->portRect.right)
		offset.h++;
	if (!EqualPt(offset,((ScrollWindowPtr) theWin)->scrollOffset))
		ScrollToPosition(theWin, offset);
}

#define FooterHeight	72
#define TopMargin		0
#define HeaderHeight	72
#define BotMargin		0
#define DescendHeight	4

void PrintScrollWindow(WindowPtr dWin)
{
	TPPrPort	printPort;
	TPrStatus	prStatus;
	GrafPtr		savePort;
	Boolean		ok;
	Rect		r;
	short		pageNbr,startPage,endPage,nbrPages;
	short		linesPerPage;
	ScrollWindowPtr	sWin;

	sWin = (ScrollWindowPtr) dWin;

	GetPort(&savePort);
	PrOpen();

	ok = PrValidate(gHPrint);
	ok = PrJobDialog(gHPrint);
	if (ok) {
		printPort = PrOpenDoc(gHPrint,NULL,NULL);

		r = printPort->gPort.portRect;
		linesPerPage = ((r.bottom - FooterHeight) - TopMargin - BotMargin - HeaderHeight)/sWin->unitHeight.v;
		nbrPages = sWin->nbrUnits.v/linesPerPage + 1;

		startPage = (**gHPrint).prJob.iFstPage;
		endPage = (**gHPrint).prJob.iLstPage;
		if (startPage > nbrPages) {
			PrCloseDoc(printPort);
			// ErrorAlert(ES_Caution, "Selected pages are out of range 1-%d",nbrPages);
			goto ErrorExit;
		}
		if (endPage > nbrPages)
			endPage = nbrPages;

		gIsPrinting = true;

		for (pageNbr = 1; pageNbr <= nbrPages; ++pageNbr) {
			SetPort(&printPort->gPort);
			PrOpenPage(printPort, NULL);
	
			if (pageNbr >= startPage && pageNbr <= endPage) {
				r = printPort->gPort.portRect;
				// DrawHeader(dWin, &r);	
					
				r.top += (HeaderHeight+TopMargin+sWin->unitHeight.v-DescendHeight);
				r.bottom -= (FooterHeight + DescendHeight + BotMargin);
		
				((ObjectWindowPtr) dWin)->Draw(dWin);
	
				// r.top = r.bottom + DescendHeight + BotMargin;
				// r.bottom = r.top + FooterHeight;
				// DrawFooter(dWin, &r, pageNbr, nbrPages);
			}
			PrClosePage(printPort);
		}
		gIsPrinting = false;

		PrCloseDoc(printPort);
		if ((**gHPrint).prJob.bJDocLoop == bSpoolLoop && PrError() == noErr)
			PrPicFile(gHPrint, NULL, NULL, NULL, &prStatus);
	}
ErrorExit:
	PrClose();
	SetPort(savePort);
}
