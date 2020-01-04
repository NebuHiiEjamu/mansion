/************************************************************************************
 * ObjectWindow.c						
 *
 * Grid Window Routines
 ************************************************************************************/
#include "ObjectWindow.h"

#ifdef THINK_C
#include <PrintTraps.h>
#endif

THPrint		gHPrint;
Boolean		gIsPrinting;


WindowPtr InitObjectWindow(short resID, ObjectWindowPtr theWindow, Boolean isFloating)
{
	WindowPtr wp;
	if (theWindow == NULL) {
		theWindow = (ObjectWindowPtr) NewPtrClear(sizeof(ObjectWindowRecord));
		theWindow->ownStorage = true;
	}
	else
		theWindow->ownStorage =false;
	theWindow->floating = false;
	wp = GetNewCWindow(resID, (WindowPtr) theWindow, (WindowPtr) -1L);
	((WindowPeek) wp)->refCon = ObjectWindowID;
	theWindow->Update = DefaultUpdate;
	theWindow->Activate = DefaultActivate;
	theWindow->HandleClick = DefaultHandleClick;
	theWindow->Dispose = DefaultDispose;
	theWindow->Draw = DefaultDraw;
	theWindow->Idle = NULL;
	theWindow->AdjustCursor = DefaultAdjustCursor;
	theWindow->Resize = DefaultResize;
	theWindow->Refresh = DefaultRefresh;
	theWindow->PrintWindow = DefaultPrintWindow;
	theWindow->Close = DefaultClose;
	theWindow->ProcessCommand = DefaultProcessCommand;
	theWindow->AdjustMenus = DefaultAdjustMenus;

	SetRect(&theWindow->growRect,20,20,
			qd.screenBits.bounds.right,qd.screenBits.bounds.bottom);

	((ObjectWindowPtr) theWindow)->floating = isFloating;

	return wp;
}

void DefaultDispose(WindowPtr theWin)
{
	CloseWindow(theWin);
	if (((ObjectWindowPtr) theWin)->toolH)
		DisposeHandle(((ObjectWindowPtr) theWin)->toolH);
	if (((ObjectWindowPtr) theWin)->ownStorage)
		DisposPtr((Ptr) theWin);
}

void DefaultClose(WindowPtr theWin)
{
	((ObjectWindowPtr) theWin)->Dispose(theWin);
}

void DefaultUpdate(WindowPtr theWin)
{
	GrafPtr	savePort;
	GetPort(&savePort);
	SetPort(theWin);
	BeginUpdate(theWin);
	((ObjectWindowPtr) theWin)->Draw(theWin);
	EndUpdate(theWin);
	SetPort(savePort);
}

void DefaultActivate(WindowPtr theWin, Boolean activateFlag)
{
	// GrafPtr	savePort;
	// GetPort(&savePort);
	// SetPort(theWin);
	// InvalRect(&theWin->portRect);	6/10/95 - this line was causing flashing.
	//									leaving it out allows normal clipping to take place
	// SetPort(savePort);
	((ObjectWindowPtr) theWin)->active = activateFlag;
}

void DefaultHandleClick(WindowPtr theWin, Point where, EventRecord *theEvent)
{
}

void DefaultAdjustCursor(WindowPtr theWin, Point where, EventRecord *theEvent)
{
	SetCursor(&qd.arrow);
}


void DefaultDraw(WindowPtr theWin)
{
}

void DefaultResize(WindowPtr theWin, short w, short h)
{
	// Change the size of the window
	SizeWindow(theWin,w,h,true);
	// AdjustScrollBars(true);
	// Draw Page??
	// Redraw the window
	SetPort(theWin);
  	InvalRect(&theWin->portRect);
}

void DefaultRefresh(WindowPtr theWin)
{
	GrafPtr	savePort;
	GetPort(&savePort);
	SetPort(theWin);
	InvalRect(&theWin->portRect);
	SetPort(savePort);
}

/************************************************************************************
 * Routines for maintaining floating windows.						
 *
 ************************************************************************************/

WindowPtr FrontDocument(void)
{
	WindowPeek	theWin;
	theWin = (WindowPeek) FrontWindow();
	while (theWin && ((ObjectWindowPtr) theWin)->floating)
		theWin = theWin->nextWindow;
	if (theWin && !((ObjectWindowPtr) theWin)->floating)
		return (WindowPtr) theWin;
	else
		return NULL;
}

WindowPtr FrontFloating(void)
{
	WindowPeek	theWin;
	theWin = (WindowPeek) FrontWindow();
	while (theWin && !((ObjectWindowPtr) theWin)->floating)
		theWin =  theWin->nextWindow;
	if (theWin && ((ObjectWindowPtr) theWin)->floating)
		return (WindowPtr) theWin;
	else
		return NULL;
}

void SelectDocument(WindowPtr theWindow)
{
	WindowPeek	frontFloat;
	frontFloat = (WindowPeek) FrontFloating();
	if (frontFloat == NULL)
		SelectWindow(theWindow);
	else {
		while ( frontFloat->nextWindow &&
				((ObjectWindowPtr) frontFloat->nextWindow)->floating)
			frontFloat = frontFloat->nextWindow;
		SendBehind(theWindow,(WindowPtr) frontFloat);
		HiliteWindow(theWindow,true);
		CalcVisBehind((WindowRef) FrontWindow(),((WindowPeek) theWindow)->strucRgn);
		PaintOne((WindowRef)theWindow,((WindowPeek) theWindow)->strucRgn);
	}
}

void SelectFloating(WindowPtr theWindow)
{
	WindowPtr	frontDoc;
	SelectWindow(theWindow);
	frontDoc = FrontDocument();
	if (frontDoc)
		HiliteWindow(frontDoc,true);
}

void LocateWindows(void)
{
//	WindowPtr	frontDoc;
//	if ((frontDoc = FrontDocument()) != NULL) {
//		SelectDocument(frontDoc);
//	}
}

#define FooterHeight	72
#define TopMargin		0
#define HeaderHeight	72
#define BotMargin		0
#define DescendHeight	4

void DefaultPrintWindow(WindowPtr dWin)
{
	TPPrPort	printPort;
	TPrStatus	prStatus;
	GrafPtr		savePort;
	Boolean		ok;
	Rect		r;
	short		pageNbr,startPage,endPage,nbrPages;
	extern 		Boolean	gIsPrinting;

	GetPort(&savePort);
	PrOpen();

	ok = PrValidate(gHPrint);
	ok = PrJobDialog(gHPrint);
	if (ok) {
		printPort = PrOpenDoc(gHPrint,NULL,NULL);

		r = printPort->gPort.portRect;
		nbrPages = 1;

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
		
				((ObjectWindowPtr) dWin)->Draw(dWin);
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

void InitPrinting()
{
	PrOpen();
	/* 7/1/96 JAB Changed to NewHandleClear */
	gHPrint = (THPrint) NewHandleClear(sizeof(TPrint));
	if (gHPrint == NULL) {
		// ErrorAlert(ES_Stop,"Not enough memory");
	}
	PrintDefault(gHPrint);
	PrClose();
}

// 6/12/95 Default Menu Processing
void DefaultProcessCommand(WindowPtr theWin,short theMenu, short theCommand)
{
	void AppProcessCommand (short menuID, short menuItem);

	if (((ObjectWindowPtr) theWin)->theParent)
		((ObjectWindowPtr) theWin)->theParent->ProcessCommand((WindowPtr) ((ObjectWindowPtr) theWin)->theParent,theMenu,theCommand);
	else
		AppProcessCommand(theMenu, theCommand);
}

void DefaultAdjustMenus(WindowPtr theWin)
{
	void AppAdjustMenus(void);
	if (((ObjectWindowPtr) theWin)->theParent)
		((ObjectWindowPtr) theWin)->theParent->AdjustMenus((WindowPtr) ((ObjectWindowPtr) theWin)->theParent);
	else {
		AppAdjustMenus();
	}
}

void SetWindowParent(WindowPtr theWin, WindowPtr theParent)
{
	((ObjectWindowPtr) theWin)->theParent = (ObjectWindowPtr) theParent;
}
