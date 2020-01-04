// PropEditor.c - PROPEDIT version
#include "PropEditor.h"

/***
  todo:
	Bug: Numerous mods on Mike R's prop file caused corruption.
		Saving a prop, opening fave and then opening prop list
		caused problem...

	(Note: Fave Handle is 0 bytes - could this be a problem?)

	Bug: Some prop mods cause corruption of prop file. 
		Prop appears to be 	saved, but remainder of file isn't
		 written.  Note: Doesn't happen if we force a save with CMD-S
		Trace through program execution when cmd-q is hit

	Change Cursor to match Eraser Size...
		
	Add Scripted Props.
 ***/
#include "UserTools.h"	// 6/10/95

#if	PALACE
#include "U-USER.H"
#include "AppMenus.h"
#else
#include "PropEdit.h"
#include <Palettes.h>			// 4/4/95 JBUM Custom Palette
#include "PropMenus.h"
#endif

#include <Picker.h>

WindowPtr	gPEWin;
PEWindowPtr	gPEPtr;

static short		gPropToolIcon[NbrPropTools-2] = 
					{PencilIcon,SelectIcon,EraserIcon,LineIcon};
#if !PALACE
extern CTabHandle	gCurCLUT;
extern RGBColor		gGray66,gGray44;
extern short		gFaceNbr;
#endif


// Create a new main window using a 'WIND' template from the resource fork
//
void NewPEWindow(AssetSpec *propSpec)
{
	WindowPtr		theWindow;
	PEWindowPtr		peRec;
	Rect			r;
	OSErr			oe;
	CGrafPtr		curPort;
	GDHandle		curDevice;
	unsigned long	t;
	short			n;
	Rect			propToolRect;
	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 
	peRec = (PEWindowPtr) NewPtrClear(sizeof(PEWindowRecord));
#if PALACE
	theWindow = InitObjectWindow(PEWIND, (ObjectWindowPtr) peRec,true);
	RestoreWindowPos(theWindow, &gMacPrefs.propEditorPos);
#else
	theWindow = InitObjectWindow(PEWIND, (ObjectWindowPtr) peRec,false);
#endif
	

	gPEWin = theWindow;
	gPEPtr = (PEWindowPtr) theWindow;

	((ObjectWindowPtr) theWindow)->Draw = PEWindowDraw;
	((ObjectWindowPtr) theWindow)->Dispose = DisposePEWin;
	((ObjectWindowPtr) theWindow)->HandleClick = PEWindowClick;
	((ObjectWindowPtr) theWindow)->AdjustCursor = PEAdjustCursor;
	((ObjectWindowPtr) theWindow)->Idle = PEIdle;
	((ObjectWindowPtr) theWindow)->AdjustMenus = PEAdjustMenus;
	((ObjectWindowPtr) theWindow)->ProcessCommand = PEProcessCommand;
#if !PALACE
	((ObjectWindowPtr) theWindow)->ProcessKey = PEProcessKey;
#endif
	
	SetRect(&gPEPtr->propRect,0,0,PropWidth,PropHeight);

	SetRect(&gPEPtr->propFatRect,0,0,PropWidth*2,PropHeight*2);
	OffsetRect(&gPEPtr->propFatRect,-PropWidth/2,-PropHeight/2);

	SetRect(&propToolRect,0,0,ToolCellWidth,ToolCellHeight*(NbrPropTools-2));
	SetRect(&gPEPtr->propDispRect,0,0,PropWidth*2*PropPixelSize,PropHeight*2*PropPixelSize-33);
	OffsetRect(&gPEPtr->propDispRect,PropWidth+1,1);
	SetRect(&gPEPtr->propPreviewRect,0,0,PropWidth*2,PropHeight*2);
	OffsetRect(&gPEPtr->propPreviewRect, gPEWin->portRect.right-PropWidth*2-1,1);
	gPEPtr->propScreenRect = gPEPtr->propRect;
	RectToScreenRect(&gPEPtr->propScreenRect);

	gPEPtr->backgroundNbr = 1;


	// Show the window
	ShowWindow(theWindow);

	
#if !PALACE
	SetWindowToCLUT(theWindow, 999);
	SetWindowParent(theWindow,(WindowPtr) gCurAssetWin);
	((ObjectWindowPtr) theWindow)->Activate = ActivateAssetSubWindow;
	SelectDocument(theWindow);
#else
	SelectFloating(theWindow);
	SetWindowParent(theWindow,(WindowPtr) gRoomWin);
#endif

	gPropEditMenu = GetMenu(PropEditMENU);
#if !PALACE
	InsertMenu(gPropEditMenu, 0);
#else
	InsertMenu(gPropEditMenu, HelpMENU);
#endif
	DrawMenuBar();

	InitPropEditor();

	SetToolBG(&gGray44);
	for (n = 0; n < NbrPropTools-2; ++n)
		AddTool(theWindow, gPropToolIcon[n],0,0,
					(ToolCellWidth-ToolWidth)/2+propToolRect.left,
					(ToolCellHeight-ToolHeight)/2+propToolRect.top+ToolCellHeight*n);

	AddTool(theWindow, CancelIcon, CancelHiIcon, 0, 
				theWindow->portRect.right-(ToolWidth*3+1),theWindow->portRect.bottom-(ToolHeight+1));
	AddTool(theWindow, SaveIcon, SaveHiIcon, 0, 
				theWindow->portRect.right-(ToolWidth*3+1)/2,theWindow->portRect.bottom-(ToolHeight+1));

	if (propSpec) {								// 6/8/95
		gPEPtr->propSpec = *propSpec;
		gPEPtr->oldSpec = *propSpec;
	}
	else {
		gPEPtr->propSpec.id = 0;
		gPEPtr->propSpec.crc = 0;
		gPEPtr->oldSpec.id = 0;
		gPEPtr->oldSpec.crc = 0;
	}

	// Make it the current grafport
	SetPort(theWindow);

	SetRect(&r,0,0,PalWidth,PalHeight);
	if ((oe = NewGWorld(&gPEPtr->palWorld,8,&r,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"StartDrawPalette");
		return;
	}
	gPEPtr->palMap = GetGWorldPixMap(gPEPtr->palWorld);
	LockPixels(gPEPtr->palMap);

	SetRect(&r,0,0,PropWidth,PropHeight);
	if ((oe = NewGWorld(&peRec->offProp,8,&r,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"NewPEWindow");
		return;
	}
	peRec->propMap = GetGWorldPixMap(peRec->offProp);
	LockPixels(peRec->propMap);

	if ((oe = NewGWorld(&peRec->offUndo,8,&r,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"NewPEWindow");
		return;
	}
	peRec->undoMap = GetGWorldPixMap(peRec->offUndo);
	LockPixels(peRec->undoMap);


	if ((oe = NewGWorld(&peRec->offPropMask,8,&r,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"NewPEWindow");
		return;
	}
	peRec->propMaskMap = GetGWorldPixMap(peRec->offPropMask);
	LockPixels(peRec->propMaskMap);

	if ((oe = NewGWorld(&peRec->offUndoMask,8,&r,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"NewPEWindow");
		return;
	}
	peRec->undoMaskMap = GetGWorldPixMap(peRec->offUndoMask);
	LockPixels(peRec->undoMaskMap);

	SetRect(&gPEPtr->propCompRect,0,0,PropWidth*3,PropHeight*3);
	if ((oe = NewGWorld(&peRec->offComposite,8,&gPEPtr->propCompRect,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"NewPEWindow");
		return;
	}
	peRec->compositeMap = GetGWorldPixMap(peRec->offComposite);
	LockPixels(peRec->compositeMap);

	GetGWorld(&curPort,&curDevice);

	SetGWorld(peRec->offProp,NULL);
	PaintRect(&peRec->offProp->portRect);

	SetGWorld(peRec->offPropMask,NULL);
	EraseRect(&peRec->offPropMask->portRect);
	gPEPtr->maskWhite = Color2Index(&gWhiteColor);
	gPEPtr->maskBlack = Color2Index(&gBlackColor);

	SetGWorld(peRec->offComposite,NULL);
	PaintRect(&peRec->offComposite->portRect);

	SetGWorld(gPEPtr->palWorld,NULL);

	{
		PicHandle	h;
		Rect		r;
		h = GetPicture(128);
		r = (*h)->picFrame;
		if (h) {
			DrawPicture(h,&r);
			ReleaseResource((Handle) h);
		}
	}
	RGBForeColor(&gBlackColor);

	SetGWorld(curPort,curDevice);

// Initialize Name Editor
	SetRect(&gPEPtr->nameRect,0,0,ToolCellWidth*4,16);
	OffsetRect(&gPEPtr->nameRect,gPEPtr->propDispRect.left+ToolCellWidth,gPEPtr->propDispRect.bottom+8);
	gPEPtr->nameFrame = gPEPtr->nameRect;
	InsetRect(&gPEPtr->nameFrame,-2,-2);
	TextFont(systemFont);
	TextSize(12);
	gPEPtr->nameTEH = TEStyleNew(&gPEPtr->nameRect,&gPEPtr->nameRect);


	if (propSpec && propSpec->id)	// 6/7/95 JAB
		LoadPropIntoEditor(propSpec);	// 6/7/95 JAB

	// Even when editing old prop, use new asset id#
#if PALACE
	GetDateTime(&t);				// 6/7/95 JAB
	gPEPtr->propSpec.id = t;		// 6/7/95 JAB
#else
	if (gPEPtr->propSpec.id == 0) {
		GetDateTime(&t);				// 6/7/95 JAB
		gPEPtr->propSpec.id = t;		// 6/7/95 JAB
	}
#endif

	gPEPtr->palRect = gPEPtr->palWorld->portRect;
	OffsetRect(&gPEPtr->palRect,propToolRect.left+2,
								propToolRect.bottom+1);
	gPEPtr->curTool = PT_Pencil;
}

void PreparePETextColors()
{
	SetPort(gPEWin);
	if (gPEPtr->nameActive)
		RGBBackColor(&gGray66);
	else
		RGBBackColor(&gGray44);
	RGBForeColor(&gWhiteColor);
}

void RestorePEColors()
{
	RGBForeColor(&gBlackColor);	
	RGBBackColor(&gWhiteColor);	
}

// Respond to an update event - BeginUpdate has already been called.
//
void PEWindowDraw(WindowPtr theWindow)
{
	Rect			dr;

	RGBForeColor(&gGray44);
	PaintRect(&theWindow->portRect);

	HiliteRect(&theWindow->portRect);

	RGBForeColor(&gGray66);
	PaintRect(&gPEPtr->propDispRect);

	RGBForeColor(&gBlackColor);

	dr = gPEPtr->nameFrame;
	RevHiliteRect(&dr);
	InsetRect(&dr,1,1);
	if (gPEPtr->nameActive)
		ColorPaintRect(&dr,&gGray66);
	else
		ColorPaintRect(&dr,&gGray44);

	PreparePETextColors();
	TEUpdate(&gPEPtr->nameRect,gPEPtr->nameTEH);
	RestorePEColors();

	MakeCompositePicture(&gPEPtr->propRect);

	RefreshPropRect(&gPEPtr->propFatRect);
	RefreshPropFrame();
	RefreshPropSelFrame();
	dr = gPEPtr->propDispRect;
	InsetRect(&dr,-1,-1);
	RevHiliteRect(&dr);
	RefreshPropToolPalette();
}

void DisposePEWin(WindowPtr theWin)
{
	Str255	name;
	IncorporatePropSelection();

	// Get Prop Name
	GetIText((*gPEPtr->nameTEH)->hText,name);
	if (name[0] > 63)
		name[0] = 63;
	BlockMove(name,gPEPtr->name,name[0]+1);

	if (!gPEPtr->cancel) {
		SaveProp();
#if !PALACE
		gCurAssetWin->modified = true;
#endif
	}
	DisposeGWorld(((PEWindowPtr) theWin)->offProp);
	DisposeGWorld(((PEWindowPtr) theWin)->offPropMask);
	DisposeGWorld(((PEWindowPtr) theWin)->offUndo);
	DisposeGWorld(((PEWindowPtr) theWin)->offUndoMask);
	DisposeGWorld(((PEWindowPtr) theWin)->offComposite);
	DisposeGWorld(((PEWindowPtr) theWin)->palWorld);
#if !PALACE
	DisposeTools();
#endif
#if PALACE
	SaveWindowPos(gPEWin, &gMacPrefs.propEditorPos);
#endif
	gPEWin = NULL;
	gPEPtr = NULL;
	DefaultDispose(theWin);
	DisposePtr((Ptr) theWin);	// 6/10/95
	DeleteMenu(PropEditMENU);
	DisposeMenu(gPropEditMenu);
	gPropEditMenu = NULL;
	DrawMenuBar();
}

void PEIdle(WindowPtr theWin, EventRecord *theEvent)
{
	if (!EmptyRect(&gPEPtr->selectR))
		IdleSelection();
	if (gPEPtr->nameActive)
		TEIdle(gPEPtr->nameTEH);
}

void DeactivatePEName()
{
	if (gPEPtr->nameActive) {
		gPEPtr->nameActive = false;
		TEDeactivate(gPEPtr->nameTEH);
		InvalRect(&gPEPtr->nameFrame);
	}
}

// Respond to a mouse-click - highlight cells until the user releases the button
//
void PEWindowClick(WindowPtr theWin, Point p, EventRecord *theEvent)
{
	GrafPtr		savePort;
	static long lastSelectTime, lastSelect;
	Boolean shiftFlag = (theEvent->modifiers & shiftKey) > 0;
	Boolean cmdFlag = (theEvent->modifiers & cmdKey) > 0;
	Boolean controlFlag = (theEvent->modifiers & controlKey) > 0;
	short		toolNbr;

	GetPort(&savePort);
	SetPort(theWin);

	GlobalToLocal(&p);
	if (PtInRect(p,&gPEPtr->nameFrame)) {
		if (!gPEPtr->nameActive) {
			gPEPtr->nameActive = true;
			TEActivate(gPEPtr->nameTEH);
			InvalRect(&gPEPtr->nameFrame);
		}
		else {
			PreparePETextColors();
			TEClick(p,shiftFlag,gPEPtr->nameTEH);
			RestorePEColors();
		}
	}
	else { 
		DeactivatePEName();
		if (PtInToolList(p,&toolNbr)) {
			switch (toolNbr) {
			case PT_Linesize:
				p.v %= ToolCellHeight;
				p.h %= ToolCellWidth;
				if (p.h > p.v+2) {
					if (gCurPenSize < MaxPenSize) {
						++gCurPenSize;
						RefreshPropToolPalette();
					}
				}
				else if (p.h < p.v-2) {
					if (gCurPenSize > 1) {
						--gCurPenSize;
						RefreshPropToolPalette();
					}
				}
				// IncorporatePropSelection();
				// SelectPropTool(PT_Pencil);
				break;
			case PT_Pencil:	
				IncorporatePropSelection();
				SelectPropTool(PT_Pencil);
				break;
			case PT_Select:
				SelectPropTool(PT_Select);
				break;
			case PT_Eraser:
				IncorporatePropSelection();
				SelectPropTool(PT_Eraser);
				break;
			case PT_Cancel:
				if (ToolClick(p,&toolNbr)) {
					IncorporatePropSelection();
					gPEPtr->cancel = true;
					DisposePEWin(gPEWin);
					
				}
				return;
			case PT_Save:
				if (ToolClick(p,&toolNbr))
					DisposePEWin(gPEWin);
				return;
			}
			lastSelect = toolNbr;
			lastSelectTime = TickCount();
		}
		else if (PtInRect(p,&gPEPtr->palRect)) {
			//
			// 4/4/95 JBUM New Eyedropper Handling Code
			
			RGBColor	rgb,curRgb;
			short		layer;
	
			SetCursor(*gCursHandles[EyedropperCursor]);	
	
			layer = (theEvent->modifiers & cmdKey) > 0;
	
			if (layer)
				curRgb = gBackColor;
			else
				curRgb = gForeColor;
			GetPEPalPixel(p.h-gPEPtr->palRect.left,p.v-gPEPtr->palRect.top,&rgb);
			ChooseDrawColor(&rgb,layer);
			while (WaitMouseUp()) {
				GetMouse(&p);
				if (PtInRect(p,&gPEPtr->palRect)) {
					GetPEPalPixel(p.h-gPEPtr->palRect.left,p.v-gPEPtr->palRect.top,&rgb);
					ChooseDrawColor(&rgb,layer);
				}
				else {
					ChooseDrawColor(&curRgb,layer);
				}
			}
		}
		// 1/2/95 don't draw if point isn't in the right spot
		else if (PtInRect(p, &gPEPtr->propScreenRect)) {
			if (gPEPtr->curTool == PT_Select) {
				// !! If pt is in selection area, drag it, otherwise
				//    create new selection
				PropSelectTool(p,theEvent);
			}
			else {
				if (theEvent->modifiers & optionKey) {
					RGBColor	rgb;
					short		layer;
					SetCursor(*gCursHandles[EyedropperCursor]);	
					layer = (theEvent->modifiers & cmdKey) > 0;
					GetCPixel(p.h,p.v,&rgb);
					ChooseDrawColor(&rgb,layer);
					while (WaitMouseUp()) {
						GetMouse(&p);
						GetCPixel(p.h,p.v,&rgb);
						ChooseDrawColor(&rgb,layer);
					}
				}
				else {
					// Perform Drawing
					SavePropEditUndo();
					switch (gCurrentDrawTool) {
					case PT_Pencil:
						if (controlFlag)
							PaintBucketTool(p,  cmdFlag || gPEPtr->curTool == PT_Eraser);
						else
							PropPencilTool(p,shiftFlag,cmdFlag || gPEPtr->curTool == PT_Eraser);
						break;
					}
				}
			}
		}
	}
	SetPort(savePort);
}

void PEAdjustCursor(WindowPtr theWin, Point p, EventRecord *er)
{
	if (PtInRect(p,&gPEPtr->propScreenRect)) {
		switch (gPEPtr->curTool) {
		case PT_Select:
			{
				Point	tp;
				if (PointToPropPoint(p,&tp) && PtInRect(tp,&gPEPtr->selectR))
					SetCursor(&qd.arrow);
				else
					SetCursor(*gCursHandles[CrosshairsCursor]);
			}
			break;
		case PT_Eraser:
			if (er->modifiers & controlKey)
				SetCursor(*gCursHandles[PaintBucketCursor]);
			else
				SetCursor(*gCursHandles[EraserCursor]);
			break;
		case PT_Pencil:
			if (er->modifiers & optionKey)
				SetCursor(*gCursHandles[EyedropperCursor]);
			else if (er->modifiers & controlKey)
				SetCursor(*gCursHandles[PaintBucketCursor]);
			else if (er->modifiers & cmdKey)
				SetCursor(*gCursHandles[EraserCursor]);
			else
				SetCursor(*gCursHandles[PencilCursor]);
			break;
		}
	}
	else if (PtInRect(p,&gPEPtr->palRect)) {
		SetCursor(*gCursHandles[EyedropperCursor]);
	}
	else {
		SetCursor(&qd.arrow);
	}
}

void PEAdjustMenus(WindowPtr theWin)
{
	Boolean	hasSelection,hasClip;
	if (gPEPtr->nameActive) {
		hasSelection = ((*gPEPtr->nameTEH)->selEnd > (*gPEPtr->nameTEH)->selStart);
		hasClip = CanPaste('TEXT');
	}
	else {
		hasSelection =  !EmptyRect(&gPEPtr->selectR);
		hasClip = CanPaste('PICT');
	}
	DefaultAdjustMenus(theWin);
	MyEnableMenuItem(gEditMenu, EM_Undo, gPEPtr->hasUndo);
	MyEnableMenuItem(gEditMenu, EM_Copy, hasSelection);
	MyEnableMenuItem(gEditMenu, EM_Cut, hasSelection);
	MyEnableMenuItem(gEditMenu, EM_Clear, hasSelection);
	MyEnableMenuItem(gEditMenu, EM_Paste, hasClip);
	MyEnableMenuItem(gEditMenu, EM_SelectAll, true);
//	MyEnableMenuItem(gPropEditMenu, PEM_PropInfo, false);
//	MyEnableMenuItem(gPropEditMenu, PEM_PropScript, false);
	CheckItem(gPropEditMenu, PEM_Head, gPEPtr->propFlags & PF_PropFaceFlag);
	CheckItem(gPropEditMenu, PEM_Ghost, gPEPtr->propFlags & PF_PropGhostFlag);
	CheckItem(gPropEditMenu, PEM_Rare, gPEPtr->propFlags & PF_PropRareFlag);
	CheckItem(gPropEditMenu, PEM_Animate, gPEPtr->propFlags & PF_Animate);
	CheckItem(gPropEditMenu, PEM_Palindrome, gPEPtr->propFlags & PF_Palindrome);
	CheckItem(gPropEditMenu, PEM_WhiteBG, gPEPtr->backgroundNbr == 0);
	CheckItem(gPropEditMenu, PEM_GrayBG, gPEPtr->backgroundNbr == 1);
	CheckItem(gPropEditMenu, PEM_BlackBG, gPEPtr->backgroundNbr == 2);
//	CheckItem(gPropEditMenu, PEM_Rotate, false);	// not working yet
}

void PEProcessCommand(WindowPtr theWin, short theMenu, short theItem)
{
	extern EventRecord	*gLastEvent;
	switch (theMenu) {
	case EditMENU:
		if (gPEPtr->nameActive) {
			PreparePETextColors();
			switch (theItem) {
			case EM_Cut:	
				TECut(gPEPtr->nameTEH);
				break;
			case EM_Copy:
				TECopy(gPEPtr->nameTEH);
				break;
			case EM_Clear:
				TEDelete(gPEPtr->nameTEH);
				break;
			case EM_Paste:
				TEPaste(gPEPtr->nameTEH);
				break;
			case EM_SelectAll:
				TESetSelect(0,32767,gPEPtr->nameTEH);
				break;
			}
			RestorePEColors();
			return;
		}
		else {
			switch (theItem) {
			case EM_Undo:
				if (gPEPtr->hasUndo)
					DoPropEditUndo();
				break;
			case EM_Cut:
				CopyPropSelection();
				ClearPropSelection();
				break;
			case EM_Copy:
				CopyPropSelection();
				break;
			case EM_Clear:
				ClearPropSelection();
				break;
			case EM_Paste:
				
				PastePict((gLastEvent->modifiers & shiftKey) > 0);
				break;
			case EM_SelectAll:
				SelectPropAll();
				break;
			}
			return;
		}
		break;
	case PropEditMENU:
		switch (theItem){
		case PEM_Head:
			gPEPtr->propFlags ^= PF_PropFaceFlag;
			RefreshProp();
			break;
		case PEM_Ghost:
			gPEPtr->propFlags ^= PF_PropGhostFlag;
			RefreshProp();			
			break;
		case PEM_Rare:
			gPEPtr->propFlags ^= PF_PropRareFlag;
			RefreshProp();
			break;
		case PEM_Animate:
			gPEPtr->propFlags ^= PF_Animate;
			RefreshProp();
			break;
		case PEM_Palindrome:
			gPEPtr->propFlags ^= PF_Palindrome;
			RefreshProp();
			break;
		case PEM_WhiteBG:
			gPEPtr->backgroundNbr = 0;
			SetPort(gPEWin);
			InvalRect(&gPEWin->portRect);
			// RefreshProp();
			break;
		case PEM_GrayBG:
			gPEPtr->backgroundNbr = 1;
			SetPort(gPEWin);
			InvalRect(&gPEWin->portRect);
			// RefreshProp();
			break;
		case PEM_BlackBG:
			gPEPtr->backgroundNbr = 2;
			SetPort(gPEWin);
			InvalRect(&gPEWin->portRect);
			// RefreshProp();
			break;
		case PEM_FlipH:
			FlipPEHorizontal();
			break;
		case PEM_FlipV:
			FlipPEVertical();
			break;
//		case PEM_Rotate:	// nada yet
//			break;
//		case PEM_PropInfo:
//			break;
//		case PEM_PropScript:
//			break;
		}
		return;
	}
	DefaultProcessCommand(theWin,theMenu,theItem);
}



void UpdatePropPaletteGraphics(void)
{
	CGrafPtr		curPort;
	GDHandle		curDevice;
	Rect			r;
	RGBColor		fc;
	extern GWorldPtr	gOffTools;

	GetGWorld(&curPort,&curDevice);
	SetGWorld(gOffTools,NULL);

	// Show Line Thickness
	SetRect(&r,0,0,ToolWidth,ToolHeight);
	OffsetRect(&r,ToolWidth*LineIcon,0);
	InsetRect(&r,2,2);
	fc.red = (153 << 8) | 153;
	fc.green = (204 << 8) | 204;
	fc.blue = 0xFFFF;
	RGBForeColor(&fc);
	PaintRect(&r);

	RGBForeColor(&gForeColor);

	InsetRect(&r,2,2);	

	PenSize(gCurPenSize,gCurPenSize);
	MoveTo(r.left,r.top);
	LineTo(r.right-gCurPenSize,r.bottom-gCurPenSize);

	PenSize(gCurPenSize-1,gCurPenSize-1);
	MoveTo(r.left,r.bottom-10);
	LineTo(r.left+10-gCurPenSize,r.bottom-gCurPenSize);

	PenSize(gCurPenSize+1,gCurPenSize+1);
	MoveTo(r.right-10,r.top);
	LineTo(r.right-gCurPenSize,r.top+10-gCurPenSize);

	PenNormal();

	PenNormal();
	RGBForeColor(&gBlackColor);
	SetGWorld(curPort,curDevice);
}

void RefreshPropToolPalette()
{
	short	n;
	Rect	dr;
	Rect	r;
	GrafPtr	savePort,theWin = (GrafPtr) gPEWin;

	GetPort(&savePort);
	SetPort(theWin);

	UpdatePropPaletteGraphics();

	RefreshTools(theWin);

	GetToolRect(PT_Pencil,&r);
	RGBForeColor(gPEPtr->curTool == PT_Pencil? &gWhiteColor : &gGray44);
	InsetRect(&r,-1,-1);
	FrameRect(&r);

	GetToolRect(PT_Select,&r);
	RGBForeColor(gPEPtr->curTool == PT_Select? &gWhiteColor : &gGray44);
	InsetRect(&r,-1,-1);
	FrameRect(&r);

	GetToolRect(PT_Eraser,&r);
	RGBForeColor(gPEPtr->curTool == PT_Eraser? &gWhiteColor : &gGray44);
	InsetRect(&r,-1,-1);
	FrameRect(&r);

	RGBForeColor(&gBlackColor);
	CopyBits((BitMap *) *gPEPtr->palMap,&theWin->portBits,
				&gPEPtr->palWorld->portRect,&gPEPtr->palRect,srcCopy,NULL);

	n = Color2Index(&gForeColor);
	// 4/4/95 JBUM
	GetPEPalSelect(&dr);
	OffsetRect(&dr,gPEPtr->palRect.left,gPEPtr->palRect.top);
	FrameRect(&dr);

	SetPort(savePort);
}

void MakeCompositePicture(Rect *clipRect)	// ClipRect currently unused (doesn't seem to work properly
{											// with copydeepmask
	CGrafPtr		curPort;
	GDHandle		curDevice;
	Rect	sr,dr;

	GetGWorld(&curPort,&curDevice);
	SetGWorld(gPEPtr->offComposite,NULL);

	switch (gPEPtr->backgroundNbr) {
	case 0:	RGBForeColor(&gWhiteColor);	break;
	case 1:	RGBForeColor(&gGray66);		break;
	case 2: RGBForeColor(&gBlackColor);	break;
	}
		

	PaintRect(&gPEPtr->offComposite->portRect);
	RGBForeColor(&gBlackColor);

	if (!(gPEPtr->propFlags & PF_PropFaceFlag)) {
#if PALACE
		if (gRoomWin->mePtr)
			DrawFacePixmap(gRoomWin->mePtr->user.faceNbr,gRoomWin->mePtr->user.colorNbr,
						PropWidth,PropHeight,0,gPEPtr->compositeMap);
		else
			DrawFacePixmap(FACE_Normal,0,
						PropWidth,PropHeight,0,gPEPtr->compositeMap);
#else
		DrawFaceNumber(gFaceNbr,
						PropWidth,PropHeight,gPEPtr->compositeMap);
#endif
	}
	// Draw Prop
	sr = gPEPtr->propRect;
//	if (!EqualRect(&gPEPtr->propRect, clipRect))
//		SectRect(&sr,clipRect,&sr);
	dr = sr;
	if (!EmptyRect(&sr)) {
		RectToCompositeRect(&dr);
		CopyDeepMask((BitMap *) *gPEPtr->propMap, 
					 (BitMap *) *gPEPtr->propMaskMap,
					 (BitMap *) *gPEPtr->compositeMap,
					 &sr,&sr,&dr,srcCopy,NULL);
	}

	// Add Selection
	if (!EmptyRect(&gPEPtr->selectR) && gPEPtr->offSel) {
		// if (gPEPtr->selectR.left < -4)
		//	DebugStr("\pTest");
		dr = gPEPtr->selectR;
		sr = dr;
		OffsetRect(&sr,-sr.left,-sr.top);
		ClipToRect(&dr,&sr,&gPEPtr->propRect);
		if (!EmptyRect(&dr)) {
			RectToCompositeRect(&dr);
			CopyDeepMask((BitMap *) *gPEPtr->selMap, 
						 (BitMap *) *gPEPtr->selMaskMap,
						 (BitMap *) *gPEPtr->compositeMap,
						 &sr,&sr,&dr,srcCopy,NULL);
		}
	}

	SetGWorld(curPort,curDevice);
}

// Receives Prop Display Coordaintes (2x2 tile centered around prop
//
void RefreshPropRect(Rect *r)	// Uses Prop Coords
{
	Rect		sr,dr;
	GrafPtr		savePort;
	RgnHandle	rh,rh2;

	GetPort(&savePort);
	SetPort(gPEWin);

	MakeCompositePicture(r);

	sr = *r;
	dr = *r;
	ClipToRect(&sr,&dr,&gPEPtr->propFatRect);
	RectToCompositeRect(&sr);
	ClipToRect(&sr,&dr,&gPEPtr->propCompRect);
	RectToScreenRect(&dr);

	rh = NewRgn();
	RectRgn(rh,&gPEPtr->propDispRect);
	rh2 = NewRgn();
	RectRgn(rh2,&gPEPtr->propPreviewRect);
	DiffRgn(rh,rh2,rh);
	SetClip(rh);
	DisposeRgn(rh);
	DisposeRgn(rh2);

	// ClipRect(&gPEPtr->propDispRect);
	CopyBits((BitMap *) *gPEPtr->compositeMap, &gPEWin->portBits, 
				&sr, &dr, srcCopy, NULL);
	ClipRect(&gPEWin->portRect);

	sr = *r;
	dr = *r;
	ClipToRect(&sr,&dr,&gPEPtr->propFatRect);
	RectToCompositeRect(&sr);
	ClipToRect(&sr,&dr,&gPEPtr->propCompRect);
	RectToPreviewRect(&dr);

	ClipRect(&gPEPtr->propPreviewRect);
	CopyBits((BitMap *) *gPEPtr->compositeMap, &gPEWin->portBits, 
				&sr, &dr, srcCopy, NULL);
	ClipRect(&gPEWin->portRect);

	SetPort(savePort);
}


static char	  marqueeCtr;

// Cycle to next pattern in barberpoll
//
void CycleMarquee()
{
	marqueeCtr++;
	marqueeCtr &= 0x07;
}

// Frame the rectangle using the current pattern
//
void DrawMarquee(Rect *rect)
{
	PenPat((Pattern *) &marqueePat[marqueeCtr]);
	FrameRect(rect);
	PenPat(&qd.black);
}

void RefreshPropSelFrame()
{
	Rect	dr,cr;
	if (EmptyRect(&gPEPtr->selectR))
		return;
	
	cr = gPEPtr->propScreenRect;
	InsetRect(&cr,-1,-1);
	ClipRect(&cr);

	dr = gPEPtr->selectR;
	RectToScreenRect(&dr);
	InsetRect(&dr,-1,-1);
	DrawMarquee(&dr);

	ClipRect(&gPEWin->portRect);
}

void RefreshPropFrame()
{
	Rect	dr;

	ClipRect(&gPEPtr->propDispRect);
	dr = gPEPtr->propScreenRect;
	InsetRect(&dr,-1,-1);
	if (gPEPtr->backgroundNbr != 0)
		PenMode(srcBic);
	FrameRect(&dr);
	PenNormal();
	ClipRect(&gPEWin->portRect);
}

Boolean PointToPropPoint(Point p, Point *tp)
{
	p.h -= gPEPtr->propDispRect.left;
	p.v -= gPEPtr->propDispRect.top;
	p.h /= PropPixelSize;
	p.v /= PropPixelSize;
	p.h -= PropWidth/2;
	p.v -= PropHeight/2;
	*tp = p;
	if (p.h >= 0 && p.h < PropWidth &&
		p.v >= 0 && p.v < PropHeight)
		return true;
	else
		return false;
}

void ClipToRect(Rect *r1, Rect *r2, Rect *clip)
{
	if (r1->left < clip->left) {
		r2->left += clip->left - r1->left;
		r1->left = clip->left;
	}
	if (r1->top < clip->top) {
		r2->top += clip->top - r1->top;
		r1->top = clip->top;
	}
	if (r1->right > clip->right) {
		r2->right -= r1->right - clip->right;
		r1->right = clip->right;
	}
	if (r1->bottom > clip->bottom) {
		r2->bottom -= r1->bottom - clip->bottom;
		r1->bottom = clip->bottom;
	}
}

void RectToScreenRect(Rect	*dr)
{
	OffsetRect(dr,PropWidth/2,PropHeight/2);
	dr->left *= PropPixelSize;
	dr->top *= PropPixelSize;
	dr->right *= PropPixelSize;
	dr->bottom *= PropPixelSize;
	OffsetRect(dr,gPEPtr->propDispRect.left,gPEPtr->propDispRect.top);
}


void RectToPreviewRect(Rect *dr)	// PropRect to PreviewRect
{
	OffsetRect(dr,PropWidth/2,PropHeight/2);
	// OffsetRect(dr,gPEPtr->propOffset.h,gPEPtr->propOffset.v);
	OffsetRect(dr,gPEPtr->propPreviewRect.left,gPEPtr->propPreviewRect.top);
}

void RectToCompositeRect(Rect *dr)
{
	OffsetRect(dr,PropWidth+gPEPtr->propOffset.h,
					PropHeight+gPEPtr->propOffset.v);
}

void PropPenMoveTo(Point tp,Boolean eraseFlag)
{
	CGrafPtr		curPort;
	GDHandle		curDevice;

	GetGWorld(&curPort,&curDevice);
	SetGWorld(gPEPtr->offProp,NULL);

	if (eraseFlag) {
		RGBForeColor(&gGray66);
		SetCursor(*gCursHandles[EraserCursor]);	
	}
	else {
		RGBForeColor(&gForeColor);
		SetCursor(*gCursHandles[PencilCursor]);	
	}
	PenSize(gCurPenSize,gCurPenSize);
	MoveTo(tp.h,tp.v);

	SetGWorld(gPEPtr->offPropMask,NULL);
	if (eraseFlag)
		RGBForeColor(&gWhiteColor);
	else
		RGBForeColor(&gBlackColor);

	PenSize(gCurPenSize,gCurPenSize);
	MoveTo(tp.h,tp.v);

	SetGWorld(curPort,curDevice);
}

void PropPenLineTo(Point tp,Boolean eraseFlag)
{
	CGrafPtr		curPort;
	GDHandle		curDevice;
	Rect			sr;
	Point			op = gPEPtr->offProp->pnLoc;

	GetGWorld(&curPort,&curDevice);

	SetGWorld(gPEPtr->offPropMask,NULL);
	LineTo(tp.h,tp.v);

	SetGWorld(gPEPtr->offProp,NULL);
	LineTo(tp.h,tp.v);

	SetGWorld(curPort,curDevice);

	Pt2Rect(op,tp,&sr);
	sr.bottom += gCurPenSize;
	sr.right += gCurPenSize;
	SectRect(&sr,&gPEPtr->propRect,&sr);

	// MakeCompositePicture(&sr);	// This is probably adding significantly to the time...

	// Compute update Rectangle
	RefreshPropRect(&sr);
}

void PropPenDone()
{
	CGrafPtr		curPort;
	GDHandle		curDevice;

	GetGWorld(&curPort,&curDevice);

	SetGWorld(gPEPtr->offProp,NULL);
	RGBForeColor(&gBlackColor);
	PenNormal();

	SetGWorld(gPEPtr->offPropMask,NULL);
	RGBForeColor(&gBlackColor);
	PenNormal();

	SetGWorld(curPort,curDevice);
}


void PropPencilTool(Point p, Boolean shiftFlag, Boolean eraseFlag)
{
	Point	lp,np,tp,op;
	short	constrain = 0;
	lp = p;
	if (PointToPropPoint(p,&tp)) {
		PropPenMoveTo(tp,eraseFlag);
		PropPenLineTo(tp,eraseFlag);
		lp = tp;
		op = p;
	}
	else {
		return;
	}
	while (WaitMouseUp()) {
		GetMouse(&np);
		if (!EqualPt(np,op) && PointToPropPoint(np,&tp) && !EqualPt(tp,lp)) {
			op = np;
			if (shiftFlag){
				if (constrain == 0) {
					if (abs(tp.h - lp.h) > abs(tp.v - lp.v))
						constrain = 2;
					else
						constrain = 1;
				}
				if (constrain == 1) {
					tp.h = lp.h;		// Horizontal
				}
				else {
					tp.v = lp.v;		// Vertical
				}
			}
			PropPenLineTo(tp,eraseFlag);
			lp =tp;
		}
	}
	PropPenDone();
}

short GetPropPixel(Point p);
short GetPropPixel(Point p)
{
	CGrafPtr		curPort;
	GDHandle		curDevice;
	RGBColor		rgb;
	Boolean			mask;	
	short			retVal = 0;
	GetGWorld(&curPort,&curDevice);
	SetGWorld(gPEPtr->offPropMask,NULL);
	mask= GetPixel(p.h,p.v);
	SetGWorld(gPEPtr->offProp,NULL);
	GetCPixel(p.h,p.v,&rgb);
	retVal = Color2Index(&rgb);
	if (!mask)
		retVal = -1;
	SetGWorld(curPort,curDevice);
	return retVal;
}

void FloodFill(Point p, short targetColor, Boolean eraseFlag);
void FloodFill(Point p, short targetColor, Boolean eraseFlag)
{
	Point			tp;
	Rect			r;

	if (p.h < 0 || p.h >= PropWidth || p.v < 0 || p.v >= PropHeight)
		return;
	if (GetPropPixel(p) != targetColor)
		return;
	SetRect(&r,p.h,p.v,p.h+1,p.v+1);

	tp = p;
	for (tp.h--; tp.h >= 0; tp.h--) {
		if (GetPropPixel(tp) == targetColor)
			r.left = tp.h;
		else
			break;
	}
	tp = p;
	for (tp.h++; tp.h < PropWidth; tp.h++) {
		if (GetPropPixel(tp) == targetColor)
			r.right = tp.h+1;
		else
			break;
	}

	if (eraseFlag) {
		SetGWorld(gPEPtr->offPropMask,NULL);
		EraseRect(&r);
	}
	else {
		SetGWorld(gPEPtr->offPropMask,NULL);
		PaintRect(&r);

		SetGWorld(gPEPtr->offProp,NULL);
		PaintRect(&r);
	}

	for (tp = topLeft(r); tp.h < r.right; ++tp.h) {
		tp.v--;
		FloodFill(tp, targetColor, eraseFlag);
		tp.v += 2;
		FloodFill(tp, targetColor, eraseFlag);
		tp.v--;
	}
}


void PaintBucketTool(Point p, Boolean eraseFlag)
{
	Point	tp;
	short	targetColor,fillColor;
	CGrafPtr		curPort;
	GDHandle		curDevice;

	PointToPropPoint(p,&tp);
	targetColor = GetPropPixel(tp);
	if (eraseFlag) {
		if (targetColor == -1)
			return;
		GetGWorld(&curPort,&curDevice);
		FloodFill(tp,targetColor,true);
		SetGWorld(curPort,curDevice);
		RefreshPropRect(&gPEPtr->propRect);
	}
	else {
		GetGWorld(&curPort,&curDevice);
		SetGWorld(gPEPtr->offProp,NULL);
		fillColor = Color2Index(&gForeColor);
		if (targetColor != fillColor) {
			RGBForeColor(&gForeColor);
			FloodFill(tp,targetColor,false);
		}
		SetGWorld(gPEPtr->offProp,NULL);
		RGBForeColor(&gBlackColor);
		SetGWorld(curPort,curDevice);
		RefreshPropRect(&gPEPtr->propRect);
	}
}

void GetPEPalPixel(short x, short y, RGBColor *rgb)
{
	unsigned char	*p;
	unsigned char *baseAddr;
	baseAddr = (unsigned char *) GetPixBaseAddr(gPEPtr->palMap);
	if (baseAddr) {
		p = baseAddr + ((*gPEPtr->palMap)->rowBytes & 0x3FFF) * y + x;
		*rgb = (*(*gPEPtr->palMap)->pmTable)->ctTable[*p].rgb;
	}
	else 
		rgb->red = rgb->green = rgb->blue = 0;
}

// 4/4/95 JBUM Added this 

void GetPEPalSelect(Rect *r)
{
	RGBColor	rgb;
	short		x,y;
	unsigned char	*p;
	unsigned char	*baseAddr;
	long			rowBytes;
	
	SetRect(r,0,0,0,0);
	baseAddr = (unsigned char *) GetPixBaseAddr(gPEPtr->palMap);
	rowBytes = ((*gPEPtr->palMap)->rowBytes & 0x3FFF);

	for (y = 0; y < 180; y += 5) {
		for (x = 0; x <= 36; x += 6) {
			p = baseAddr + rowBytes * y + x;
			rgb = (*gCurCLUT)->ctTable[*p].rgb;
			if (rgb.red == gForeColor.red &&
				rgb.green == gForeColor.green &&
				rgb.blue == gForeColor.blue) {
				if (x < 36)
					SetRect(r,(x/6)*6,(y/5)*5,(x/6)*6+6,(y/5)*5+5);
				else
					SetRect(r,36,(y/11)*11,40,(y/11)*11+11);
				return;
			}
		}
	}
}

void DrawMansionPropMask(Handle	propH, PixMapHandle pMap)
{
	Ptr				sp,dp;
	long			dRowBytes;
	short			y,x;
	PropHeaderPtr	ph;
	unsigned char	cb,mc,pc;

	if (propH == NULL)
		return;

	HLock(propH);
	ph = (PropHeaderPtr) *propH;
	sp = *propH + sizeof(PropHeader);

	dp = GetPixBaseAddr(pMap);
	dRowBytes = (*pMap)->rowBytes & 0x3FFF;

	for (y = ph->height-1; y >= 0; --y) {
		x = ph->width;
		while (x > 0) {
			cb = *((unsigned char *) sp);	++sp;
			mc = cb >> 4;
			pc = cb & 0x0F;
			x -= mc;
			x -= pc;
			dp += mc;
			while (pc--) {
				*(dp++) = gPEPtr->maskBlack;
				++sp;
			}
		}
		dp += dRowBytes - ph->width;
	}
	HUnlock(propH);
}

void LoadPropIntoEditor(AssetSpec *propSpec)
{
	Handle			propH;
	PropHeaderPtr	pp;
	short			oldFlags;
	Str255			name;
	long			type,id;
	propH = GetAssetWithCRC('Prop',propSpec->id,propSpec->crc);
	if (propH) {
		GetAssetInfo(propH,&type,&id,name);
		TESetText(&name[1],name[0],gPEPtr->nameTEH);

		pp = (PropHeaderPtr) *propH;
		oldFlags = pp->flags;
		pp->flags &= ~PF_PropGhostFlag;
		DrawMansionPropPixMap(propH,gPEPtr->propMap,0,0,0);
		DrawMansionPropMask(propH,gPEPtr->propMaskMap);

		pp = (PropHeaderPtr) *propH;
		pp->flags = oldFlags;
		gPEPtr->propOffset.h = pp->hOffset;
		gPEPtr->propOffset.v = pp->vOffset;
		gPEPtr->oldPropOffset = gPEPtr->propOffset;
		gPEPtr->propFlags = pp->flags;
		gPEPtr->oldPropFlags = gPEPtr->propFlags;
	}
}

void MovePropFrame(short hd, short vd)
{
	gPEPtr->propOffset.h += hd;
	gPEPtr->propOffset.v += vd;

	if (gPEPtr->propOffset.h < -PropWidth)
		gPEPtr->propOffset.h = -PropWidth;
	if (gPEPtr->propOffset.h > PropWidth)
		gPEPtr->propOffset.h = PropWidth;
	if (gPEPtr->propOffset.v < -PropHeight)
		gPEPtr->propOffset.v = -PropHeight;
	if (gPEPtr->propOffset.v > PropHeight)
		gPEPtr->propOffset.v = PropHeight;
	
	RefreshPropRect(&gPEPtr->propFatRect);
	SetPort(gPEWin);
	RefreshPropFrame();
	RefreshPropSelFrame();
}




Handle ConvertPixelsToProp(PixMapHandle pMap, PixMapHandle mMap)
{
	Handle		dh;
	short		rp;
	unsigned char *mp,*pp,*cp,*dp;
	short		n,y;
	long		count;
	short		width=PropWidth,height=PropHeight;
	Boolean		hasPixels=false;
	unsigned char *mBaseAddr,*pBaseAddr;
	dh = NewHandleClear(32767L);	/* 7/1/96 JAB Changed to NewHandleClear */
	HLock(dh);
	dp = (unsigned char *) *dh;

	((PropHeaderPtr) dp)->width = width;
	((PropHeaderPtr) dp)->height = height;
	((PropHeaderPtr) dp)->hOffset = gPEPtr->propOffset.h;
	((PropHeaderPtr) dp)->vOffset = gPEPtr->propOffset.v;
	((PropHeaderPtr) dp)->scriptOffset = 0L;
	((PropHeaderPtr) dp)->flags = gPEPtr->propFlags;

	dp += sizeof(PropHeader);
	mBaseAddr = (unsigned char *) GetPixBaseAddr(mMap);
	pBaseAddr = (unsigned char *) GetPixBaseAddr(pMap);

	for (y = 0; y < height; ++y) {
		mp = mBaseAddr + ((*mMap)->rowBytes&0x3FFF) * y;
		pp = pBaseAddr + ((*pMap)->rowBytes&0x3FFF) * y;
		rp = width;

		while (rp) {
			cp = dp;		// Store Count Bit
			++dp;
			*cp = 0;
			if (*mp == 0) {
				// compress run of mask (up to 16)
				n = 0;
				while (n < 15 && rp && *mp == 0) {
					++pp;
					++mp;
					++n;
					--rp;
				}
				*cp |= (n << 4);
			}
			if (rp && *mp) {
				hasPixels = true;	// 6/11/95 - check for existence of pixels
				// store run of pixels (up to 16)
				n = 0;
				while (n < 15 && rp && *mp) {
					*(dp++) = *(pp++);
					++mp;
					++n;
					--rp;
				}
				*cp |= n;
			}
		}
	}
	count = (long) dp - (long) *dh;
	HUnlock(dh);
	if (hasPixels)
		SetHandleSize(dh,count);
	else {				// 6/11/95	- if prop doesn't have pixels, don't save it
		DisposeHandle(dh);
		dh = NULL;
	}
	return dh;
}

// Replace old prop, if there is one...

void SaveProp()
{
	Handle		h,h2;
	unsigned long crc;
	long		length;

	// Compress Prop to Handle
	h = ConvertPixelsToProp(gPEPtr->propMap, gPEPtr->propMaskMap);
	if (h == NULL)
		return;

	length = GetHandleSize(h) - sizeof(PropHeader);		// 6/9/95
	if (length >= 0)
		crc = ComputeCRC(*h+sizeof(PropHeader),length);		// 6/9/95
	else
		crc = 0;

	// 6/8/95 - check if prop is different by comparing CRC
	if (gPEPtr->propSpec.crc && crc == gPEPtr->propSpec.crc &&
		EqualPt(gPEPtr->propOffset,gPEPtr->oldPropOffset) &&
		gPEPtr->propFlags == gPEPtr->oldPropFlags &&
		EqualString(gPEPtr->oldName,gPEPtr->name,true,true)) {
		DisposeHandle(h);
		return;
	}

#if PALACE
	if (MembersOnly(true)) {
		DisposeHandle(h);
		return;
	}
#endif

	if ((h2 = GetAssetWithCRC('Prop',gPEPtr->propSpec.id,gPEPtr->propSpec.crc)) != NULL) {
		long	count;
		long	type,id;
		Str255	oName;
		count = GetHandleSize(h);
		SetHandleSize(h2,count);
		BlockMove(*h,*h2,count);
		ChangedAsset(h2);
		SetAssetCRC(h2,crc);
		// Change Name !!!
		GetAssetInfo(h2,&type,&id,oName);
		SetAssetInfo(h2,id,gPEPtr->name);
	}
	else {
		// Add to Resource file
		AddAsset(h,'Prop',gPEPtr->propSpec.id,gPEPtr->name);
		SetAssetCRC(h,crc);
		gPEPtr->propSpec.crc = crc;							// 6/9/95
#if PALACE
		// Register Resource
		if (!DeniedByServer(PM_AllowCustomProps))
			RegisterAsset('Prop',&gPEPtr->propSpec);
#endif
	}
#if PALACE
	// Add to faves list
	if (gPEPtr->oldSpec.id == 0L)										// 6/9/95
		AddPropFavorite(&gPEPtr->propSpec);
	else
		ReplacePropFavorite(&gPEPtr->oldSpec,&gPEPtr->propSpec);		// 6/9/95

	if (gPropWin == NULL)
		NewPropWindow();
#else
	gCurAssetWin->modified = true;
#endif
	UpdateAssetFile();
}

// Add a selection tool, Select All command.
// CMD-A Select All
// CNTRL-Drag - Make Selection
//
// Drag - if Selection is non-empty, then drag selection.
//
// Paste Pict automatically makes active selection.
//
// Idle routine refreshes selection.
//
// Fix Refresh to stop flashes...
//
void PastePict(Boolean unScaled)
{
	Handle			picH;
	long			scrapOffset;
	Rect			pictR,destR;
	GWorldPtr		offPict;
	PixMapHandle	pictMap;
	CGrafPtr		curPort;
	GDHandle		curDevice;
	OSErr			oe;

	if (!CanPaste('PICT'))
		return;
	picH = NewHandleClear(1L);	/* 7/1/96 JAB Changed to NewHandleClear */
	if (GetScrap(picH,'PICT',&scrapOffset) == 0) {
		ReportError(memFullErr,"PastePict");
		return;
	}
	pictR = (*((PicHandle) picH))->picFrame;
	OffsetRect(&pictR,-pictR.left,-pictR.top);

	// Allocate RGB PixMap at Pict Size and Render PICT Into it
	if ((oe = NewGWorld(&offPict,32,&pictR,NULL,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		DisposeHandle(picH);
		ReportError(memFullErr,"PastePict");
		return;
	}
	pictMap = GetGWorldPixMap(offPict);
	LockPixels(pictMap);  // was gPEPtr->palMap ???
	GetGWorld(&curPort,&curDevice);
	SetGWorld(offPict,NULL);
	EraseRect(&pictR);
	DrawPicture((PicHandle) picH, &pictR);
	SetGWorld(curPort,curDevice);
	DisposeHandle(picH);

	// Copy scaled & dithered into Prop, set Mask accordingly.
	if (pictR.right <= PropWidth && pictR.bottom <= PropHeight) {
		destR = pictR;
		OffsetRect(&destR,(PropWidth-destR.right)/2,
						  (PropHeight-destR.bottom)/2);
	}
	else {
		if (unScaled) {
			// if (pictR.right > PropWidth)
			// 	pictR.right = PropWidth;
			// if (pictR.bottom > PropHeight)
			//	pictR.bottom = PropHeight;
			destR = pictR;
		}
		else {
			if (pictR.right > pictR.bottom) {
				SetRect(&destR,0,0,PropWidth,(PropHeight*pictR.bottom)/pictR.right);
			}
			else {
				SetRect(&destR,0,0,(PropWidth*pictR.right)/pictR.bottom,PropHeight);
			}
		}
	}

	SetPropSelection(&destR, false, false);
	SetGWorld(gPEPtr->offSel,NULL);
	OffsetRect(&destR,-destR.left,-destR.top);
	CopyBits((BitMap *) *pictMap, (BitMap *) *gPEPtr->selMap, &pictR, &destR,
			ditherCopy, NULL);

	SetGWorld(gPEPtr->offSelMask,NULL);
	PaintRect(&destR);
	SetGWorld(curPort,curDevice);

	// Dispose of RGB PixMap
	DisposeGWorld(offPict);

	// Generate a Refresh
	RefreshProp();
}

void RefreshProp()
{
	GrafPtr	savePort;
	GetPort(&savePort);
	SetPort(gPEWin);
	RefreshPropRect(&gPEPtr->propRect);
	RefreshPropFrame();
	RefreshPropSelFrame();
	SetPort(savePort);
}

void IdleSelection()
{
	static long lastTicks;
	long	t;
	if ((t = TickCount()) - lastTicks > 20) {
		lastTicks = t;
		SetPort(gPEWin);
		RefreshPropSelFrame();
		CycleMarquee();
	}
}

void ClearPropSelection()
{
	if (EmptyRect(&gPEPtr->selectR) || gPEPtr->offSel == NULL)
		return;
	SetRect(&gPEPtr->selectR,0,0,0,0);
	if (gPEPtr->offSel) {
		DisposeGWorld(gPEPtr->offSel);
		gPEPtr->offSel = NULL;
	}
	if (gPEPtr->offSelMask) {
		DisposeGWorld(gPEPtr->offSelMask);
		gPEPtr->offSelMask = NULL;
	}
	RefreshProp();
}

void IncorporatePropSelection()
{
	CGrafPtr	curPort;
	GDHandle	curDevice;
	Rect		dr,sr;
	if (EmptyRect(&gPEPtr->selectR) || gPEPtr->offSel == NULL)
		return;

	dr = gPEPtr->selectR;
	sr = dr;
	OffsetRect(&sr,-sr.left,-sr.top);
	ClipToRect(&dr,&sr,&gPEPtr->propRect);
	GetGWorld(&curPort,&curDevice);
	SetGWorld(gPEPtr->offPropMask,NULL);
	PaintRect(&dr);
	SetGWorld(gPEPtr->offProp,NULL);
	CopyBits((BitMap *) *gPEPtr->selMap, (BitMap *) *gPEPtr->propMap, 
			&sr, &dr, srcCopy, NULL);
	SetGWorld(gPEPtr->offPropMask,NULL);
	CopyBits((BitMap *) *gPEPtr->selMaskMap, (BitMap *) *gPEPtr->propMaskMap, 
			&sr, &dr, srcCopy, NULL);
	SetGWorld(curPort,curDevice);

	ClearPropSelection();
}

void SetPropSelection(Rect 	*r, Boolean liftFlag, Boolean clearFlag)
{
	Rect		wr;
	CGrafPtr	curPort;
	GDHandle	curDevice;
	OSErr		oe;

	IncorporatePropSelection();
	gPEPtr->selectR = *r;
	wr = *r;
	OffsetRect(&wr,-wr.left,-wr.top);
	if ((oe = NewGWorld(&gPEPtr->offSel,8,&wr,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"SetPropSel");
		return;
	}
	gPEPtr->selMap = GetGWorldPixMap(gPEPtr->offSel);
	LockPixels(gPEPtr->selMap);

	if ((oe = NewGWorld(&gPEPtr->offSelMask,1,&wr,NULL,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"SetPropSel");
		return;
	}
	gPEPtr->selMaskMap = GetGWorldPixMap(gPEPtr->offSelMask);
	LockPixels(gPEPtr->selMaskMap);

	if (liftFlag) {
		ClipToRect(r,&wr,&gPEPtr->propRect);

		GetGWorld(&curPort,&curDevice);
		SetGWorld(gPEPtr->offSel,NULL);
		CopyBits((BitMap *) *gPEPtr->propMap, (BitMap *) *gPEPtr->selMap, r, &wr,
				srcCopy, NULL);
		SetGWorld(gPEPtr->offSelMask,NULL);
		CopyBits((BitMap *) *gPEPtr->propMaskMap, (BitMap *) *gPEPtr->selMaskMap, r, &wr,
				srcCopy, NULL);
		if (clearFlag) {
			SetGWorld(gPEPtr->offPropMask,NULL);
			EraseRect(r);
		}
		SetGWorld(curPort,curDevice);
		RefreshProp();
	}
	SelectPropTool(PT_Select);
}

void SelectPropAll()
{
	SetPropSelection(&gPEPtr->propRect, true, true);
}

void PropSelectTool(Point p, EventRecord *theEvent)
{
	Point			tp,op,np;
	Rect			oldSelect;
	if (!PointToPropPoint(p,&tp))
		return;
	if (!EmptyRect(&gPEPtr->selectR) && PtInRect(tp,&gPEPtr->selectR)) {
		SavePropEditUndo();
		SetCursor(&qd.arrow);	
		if (theEvent->modifiers & optionKey) {	// Leave a copy behind
			Rect	r;
			r = gPEPtr->selectR;
			IncorporatePropSelection();
			SetPropSelection(&r,true,false);
		}
		oldSelect = gPEPtr->selectR;
		op = tp;
		while (WaitMouseUp()) {
			IdleSelection();
			GetMouse(&np);
			PointToPropPoint(np,&np);
			if (!EqualPt(op,np)) {
				// Drag Selection Rectangle
				OffsetRect(&gPEPtr->selectR,np.h-op.h,np.v-op.v);
				RefreshProp();
				op = np;
			}
		}
	}
	else {
		SetCursor(*gCursHandles[CrosshairsCursor]);	
		IncorporatePropSelection();
		op = tp;
		while (WaitMouseUp()) {
			IdleSelection();
			GetMouse(&np);
			PointToPropPoint(np,&np);
			if (!EqualPt(op,np)) {
				Pt2Rect(tp,np,&gPEPtr->selectR);
				SectRect(&gPEPtr->selectR,&gPEPtr->propRect,&gPEPtr->selectR);
				RefreshProp();
				op = np;
			}
		}
		if (!EmptyRect(&gPEPtr->selectR))
			SetPropSelection(&gPEPtr->selectR, true, true);
	}
}

void SelectPropTool(short tool)
{
	gPEPtr->curTool = tool;
	RefreshPropToolPalette();
}

void CopyPropSelection()	// Copy Selection into Clipboard as a 'PICT'
{
	CGrafPtr	curPort;
	GDHandle	curDevice;
	Rect		r;
	PicHandle	thePic;
	if (EmptyRect(&gPEPtr->selectR) || gPEPtr->offSel == NULL)
		return;

	GetGWorld(&curPort,&curDevice);
	SetGWorld(gPEPtr->offSel,NULL);

	r = gPEPtr->offSel->portRect;
	ClipRect(&r);
	thePic = OpenPicture(&r);
	CopyBits((BitMap *) *gPEPtr->selMap, (BitMap *) *gPEPtr->selMap, &r, &r, srcCopy, NULL);
	ClosePicture();

	SetGWorld(curPort,curDevice);
	
	ZeroScrap();
	HLock((Handle) thePic);
	PutScrap(GetHandleSize((Handle) thePic),'PICT',*((Handle) thePic));
	HUnlock((Handle) thePic);
	KillPicture(thePic);
}

void FlipPixMapHorizontal(PixMapHandle pMap)	// 0= horizontal, 1 = vertical
{
	// Flip 8-bit PixMap Horizontally
	long			rowBytes = (*pMap)->rowBytes & 0x3FFF;
	Boolean			longAligned = (rowBytes & 3) == 0;
	short			depth = (*pMap)->pixelSize;
	Rect			bounds = (*pMap)->bounds;
	unsigned char	*sb,*db,tmp;
	long			y,x,w,h,w2;
	unsigned char	*baseAddr;

	baseAddr = (unsigned char *) GetPixBaseAddr(pMap);

	switch (depth) {
	case 1:
		if (longAligned)
			Flip_Long(pMap, rowBytes, depth, &bounds);
		else
			Flip_Word(pMap, rowBytes, depth, &bounds);
		return;
	case 2:
	case 4:
		return;
	}

	h = (*pMap)->bounds.bottom - (*pMap)->bounds.top;
	w = (*pMap)->bounds.right - (*pMap)->bounds.left;
	w2 = w/2;

	if ((*pMap)->pixelSize == 8) {
		for (y = 0; y < h; ++y) {
			sb = baseAddr + rowBytes*y;
			db = sb + (w - 1);
			x = w2;
			while (x--) {
				tmp = *sb;
				*sb = *db;
				*db = tmp;
				++sb;
				--db;
			}
		}
	}
}

void FlipPixMapVertical(PixMapHandle pMap)
{
	// Flip 1-bit or 8-bit PixMap Vertically
	long	rowBytes;
	unsigned char	*sb,*db,tmp;
	long	y,x,w,h,h2;
	unsigned char *baseAddr = (unsigned char *) GetPixBaseAddr(pMap);

	rowBytes = (*pMap)->rowBytes & 0x3FFF;
	h = (*pMap)->bounds.bottom - (*pMap)->bounds.top;
	w = (*pMap)->bounds.right - (*pMap)->bounds.left;
	h2 = h/2;
	for (y = 0; y < h2; ++y) {
		sb = baseAddr + rowBytes*y;
		db = baseAddr + ((h-y) - 1)*rowBytes;
		x = rowBytes;
		while (x--) {
			tmp = *sb;
			*sb = *db;
			*db = tmp;
			++sb;
			++db;
		}
	}
}

void FlipPEVertical()
{
	if (EmptyRect(&gPEPtr->selectR) || gPEPtr->offSel == NULL)
	{
		FlipPixMapVertical(gPEPtr->propMap);
		FlipPixMapVertical(gPEPtr->propMaskMap);
	}
	else {
		FlipPixMapVertical(gPEPtr->selMap);
		FlipPixMapVertical(gPEPtr->selMaskMap);
	}
	RefreshProp();
}

void FlipPEHorizontal()
{
	if (EmptyRect(&gPEPtr->selectR) || gPEPtr->offSel == NULL)
	{
		FlipPixMapHorizontal(gPEPtr->propMap);
		FlipPixMapHorizontal(gPEPtr->propMaskMap);
	}
	else {
		FlipPixMapHorizontal(gPEPtr->selMap);
		FlipPixMapHorizontal(gPEPtr->selMaskMap);
	}
	RefreshProp();
}

// The following code borrowed from Programmer's Challenge
// Winner Troy Anderson  MacTech Magazine July 1994
// Special cases for 2 and 4 bits were removed...
//

typedef unsigned char UCHAR;

// This is the 1-bit per pixel table
static char byteFlips1[] ={ 
  0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 
  0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
  0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 
  0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
  0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 
  0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4, 
  0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 
  0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc, 
  0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 
  0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2, 
  0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 
  0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa, 
  0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 
  0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6, 
  0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 
  0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe, 
  0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 
  0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1, 
  0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 
  0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9, 
  0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 
  0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5, 
  0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 
  0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
  0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 
  0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
  0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 
  0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb, 
  0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 
  0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7, 
  0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 
  0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff  };

static void Flip_Long(  PixMapHandle theMap, 
                        short rowBytes,
                        short depth,
                        Rect* area)
{
#undef T
#define T long

  register UCHAR  temp;
  short           rowCells = rowBytes / sizeof(T);
  long            bitsPerRow = (area->right - area->left) *
                          (long)depth - 1;
  short           numCells = (bitsPerRow + sizeof(T)*8) /
                          (sizeof(T)*8);
  T*              aRow;
  T*              firstRow = (T*)GetPixBaseAddr( theMap);
  T*              lastRow = firstRow + rowCells * 
                      (long)(area->bottom - area->top);
  
  register T*     cellPtr1, *cellPtr2;

  short           numBitsToShift = ((sizeof(T)*8) -
                      (bitsPerRow % (sizeof(T)*8) + 1));
  T               shiftMask;
  T*              shiftCellPtr;
  char*           flipTable;
  
  

  switch(depth)
  {
    case 1:
      flipTable = byteFlips1;
      break;
    case 2:
    case 4:
	  return;
  }
            

  if (numBitsToShift)
  {
    shiftMask = (1L << numBitsToShift) - 1;

    for ( aRow = firstRow; 
        aRow < lastRow;
        aRow += rowCells)
    {
      // With each pair of cells in the row 
      // (one on the left, the other on the right),
      // flip the pixels in the individual cells 
      // and swap the cells with oneanother.
      for ( cellPtr1 = aRow + numCells - 1, cellPtr2 = aRow;
          cellPtr1 > cellPtr2;
          cellPtr1--, cellPtr2++)
      {
        temp = ((UCHAR*)cellPtr1)[0];
        ((UCHAR*)cellPtr1)[0] = 
            flipTable[((UCHAR*)cellPtr2)[3]];
        ((UCHAR*)cellPtr2)[3] = flipTable[temp];
        
        temp = ((UCHAR*)cellPtr1)[1];
        ((UCHAR*)cellPtr1)[1] = 
            flipTable[((UCHAR*)cellPtr2)[2]];
        ((UCHAR*)cellPtr2)[2] = flipTable[temp];
        
        temp = ((UCHAR*)cellPtr1)[2];
        ((UCHAR*)cellPtr1)[2] = 
            flipTable[((UCHAR*)cellPtr2)[1]];
        ((UCHAR*)cellPtr2)[1] = flipTable[temp];
        
        temp = ((UCHAR*)cellPtr1)[3];
        ((UCHAR*)cellPtr1)[3] = 
            flipTable[((UCHAR*)cellPtr2)[0]];
        ((UCHAR*)cellPtr2)[0] = flipTable[temp];
      }
      
      // If there's an odd number of cells in the row, 
      // there is one cell we haven't touched. 
      // It needs to be flipped.
      if (cellPtr1 == cellPtr2)
      {
        temp = ((UCHAR*)cellPtr1)[0];
        ((UCHAR*)cellPtr1)[0] = 
            flipTable[((UCHAR*)cellPtr1)[3]];
        ((UCHAR*)cellPtr1)[3] = flipTable[temp];
        
        temp = ((UCHAR*)cellPtr1)[1];
        ((UCHAR*)cellPtr1)[1] = 
            flipTable[((UCHAR*)cellPtr1)[2]];
        ((UCHAR*)cellPtr1)[2] = flipTable[temp];
      }

      // Slide the pixels to the left
      for ( shiftCellPtr = aRow;
          shiftCellPtr < aRow + rowCells;
          shiftCellPtr++)
      {
        // shift the bits over
        *shiftCellPtr <<= numBitsToShift;
          
        // bring in the bits from the next cell - 
        // garbage will be brought in during last 
        // iteration, but its put into the last
        // cell, outside the bounds of the image
        // (but still in the data area)
        *shiftCellPtr |= shiftMask & 
                        (*(shiftCellPtr+1) >> 
                          (sizeof(T)*8 - numBitsToShift));
      }
    }
  }
  else  // no need to shift pixels, otherwise, 
  {     // just the same as previous loop
    for ( aRow = firstRow; aRow < lastRow; aRow += rowCells)
    {
      // With each pair of cells in the row (one on the 
      // left, the other on the right), flip the pixels
      // in the individual cells and swap the cells with
      // one another.
      for ( cellPtr1 = aRow + numCells - 1, cellPtr2 = aRow;
            cellPtr1 > cellPtr2;
            cellPtr1--, cellPtr2++)
      {
        temp = ((UCHAR*)cellPtr1)[0];
        ((UCHAR*)cellPtr1)[0] =
            flipTable[((UCHAR*)cellPtr2)[3]];
        ((UCHAR*)cellPtr2)[3] = flipTable[temp];
        
        temp = ((UCHAR*)cellPtr1)[1];
        ((UCHAR*)cellPtr1)[1] = 
            flipTable[((UCHAR*)cellPtr2)[2]];
        ((UCHAR*)cellPtr2)[2] = flipTable[temp];
        
        temp = ((UCHAR*)cellPtr1)[2];
        ((UCHAR*)cellPtr1)[2] = 
            flipTable[((UCHAR*)cellPtr2)[1]];
        ((UCHAR*)cellPtr2)[1] = flipTable[temp];
        
        temp = ((UCHAR*)cellPtr1)[3];
        ((UCHAR*)cellPtr1)[3] = 
            flipTable[((UCHAR*)cellPtr2)[0]];
        ((UCHAR*)cellPtr2)[0] = flipTable[temp];
      }
      
      // If there are an odd number of cells in the row,
      // there is one cell we haven't touched.
      // It needs to be flipped.
      if (cellPtr1 == cellPtr2)
      {
        temp = ((UCHAR*)cellPtr1)[0];
        ((UCHAR*)cellPtr1)[0] = 
            flipTable[((UCHAR*)cellPtr1)[3]];
        ((UCHAR*)cellPtr1)[3] = flipTable[temp];
        
        temp = ((UCHAR*)cellPtr1)[1];
        ((UCHAR*)cellPtr1)[1] = 
            flipTable[((UCHAR*)cellPtr1)[2]];
        ((UCHAR*)cellPtr1)[2] = flipTable[temp];
      }
    }
  }
}





static void Flip_Word(  PixMapHandle theMap, 
                        short rowBytes,
                        short depth,
                        Rect* area)
{
#undef T
#define T short

  register UCHAR  temp;
  short           rowCells = rowBytes / sizeof(T);
  long            bitsPerRow = (area->right - area->left) *
                          (long)depth - 1;
  short           numCells = (bitsPerRow + sizeof(T)*8) /
                          (sizeof(T)*8);
  T*              aRow;
  T*              firstRow = (T*)GetPixBaseAddr( theMap);
  T*              lastRow = firstRow + rowCells * 
                      (long)(area->bottom - area->top);
  
  register T*     cellPtr1, *cellPtr2;

  short           numBitsToShift = ((sizeof(T)*8) -
                      (bitsPerRow % (sizeof(T)*8) + 1));
  T               shiftMask;
  T*              shiftCellPtr;
  char*           flipTable;
  
  

  switch(depth)
  {
    case 1:
      flipTable = byteFlips1;
      break;
    case 2:
	case 4:
	  return;
  }
            

  if (numBitsToShift)
  {
    shiftMask = (1L << numBitsToShift) - 1;

    for ( aRow = firstRow; aRow < lastRow; aRow += rowCells)
    {
      // With each pair of cells in the row 
      // (one on the left, the other on the right),
      // flip the pixels in the individual cells 
      // and swap the cells with oneanother.
      for ( cellPtr1 = aRow + numCells - 1, cellPtr2 = aRow;
          cellPtr1 > cellPtr2;
          cellPtr1--, cellPtr2++)
      {
        temp = ((UCHAR*)cellPtr1)[0];
        ((UCHAR*)cellPtr1)[0] = 
            flipTable[((UCHAR*)cellPtr2)[1]];
        ((UCHAR*)cellPtr2)[1] = flipTable[temp];
        
        temp = ((UCHAR*)cellPtr1)[1];
        ((UCHAR*)cellPtr1)[1] =
            flipTable[((UCHAR*)cellPtr2)[0]];
        ((UCHAR*)cellPtr2)[0] = flipTable[temp];
      }
      
      // If there's an odd number of cells in the row, 
      // there is one cell we haven't touched. 
      // It needs to be flipped.
      if (cellPtr1 == cellPtr2)
      {
        temp = ((UCHAR*)cellPtr1)[0];
        ((UCHAR*)cellPtr1)[0] = 
            flipTable[((UCHAR*)cellPtr1)[1]];
        ((UCHAR*)cellPtr1)[1] = 
            flipTable[temp];
      }

      // Slide the pixels to the left
      for ( shiftCellPtr = aRow;
          shiftCellPtr < aRow + rowCells;
          shiftCellPtr++)
      {
      // shift the bits over
        *shiftCellPtr <<= numBitsToShift;
          
      // bring in the bits from the next cell - 
      // garbage will be brought in during last 
      // iteration, but its put into the last
      // cell, outside the bounds of the image
      // (but still in the data area)
        *shiftCellPtr |= shiftMask & 
                        (*(shiftCellPtr+1) >> 
                          (sizeof(T)*8 - numBitsToShift));
      }
    }
  }
  else  // no need to shift pixels, otherwise, 
  {     // just the same as previous loop
    for ( aRow = firstRow; aRow < lastRow; aRow += rowCells)
    {
      // With each pair of cells in the row (one on the 
      // left, the other on the right), flip the pixels
      // in the individual cells and swap the cells with
      // one another.
      for ( cellPtr1 = aRow + numCells - 1, cellPtr2 = aRow;
            cellPtr1 > cellPtr2;
            cellPtr1--, cellPtr2++)
      {
        temp = ((UCHAR*)cellPtr1)[0];
        ((UCHAR*)cellPtr1)[0] = 
            flipTable[((UCHAR*)cellPtr2)[1]];
        ((UCHAR*)cellPtr2)[1] = flipTable[temp];
        
        temp = ((UCHAR*)cellPtr1)[1];
        ((UCHAR*)cellPtr1)[1] = 
            flipTable[((UCHAR*)cellPtr2)[0]];
        ((UCHAR*)cellPtr2)[0] = flipTable[temp];
      }
      
      // If there are an odd number of cells in the row,
      // there is one cell we haven't touched.
      // It needs to be flipped.
      if (cellPtr1 == cellPtr2)
      {
        temp = ((UCHAR*)cellPtr1)[0];
        ((UCHAR*)cellPtr1)[0] = 
            flipTable[((UCHAR*)cellPtr1)[1]];
        ((UCHAR*)cellPtr1)[1] = flipTable[temp];
      }
    }
  }
}

void SavePropEditUndo()
{
	// Save Current Prop/Mask to Undo Map
	unsigned short	*sp,*dp;
	long	len;
	len = ((*gPEPtr->propMap)->rowBytes & 0x3FFF)*22;
	sp = (unsigned short *) GetPixBaseAddr(gPEPtr->propMap);
	dp = (unsigned short *) GetPixBaseAddr(gPEPtr->undoMap);
	while (len--) {
		*(dp++) = *(sp++);
	}
	len = ((*gPEPtr->propMaskMap)->rowBytes & 0x3FFF)*22;
	sp = (unsigned short *) GetPixBaseAddr(gPEPtr->propMaskMap);
	dp = (unsigned short *) GetPixBaseAddr(gPEPtr->undoMaskMap);
	while (len--) {
		*(dp++) = *(sp++);
	}
	gPEPtr->hasUndo = true;
}


void DoPropEditUndo()
{
	unsigned short	*sp,*dp,tmp;
	long	len;
	len = ((*gPEPtr->propMap)->rowBytes & 0x3FFF)*22;
	sp = (unsigned short *) GetPixBaseAddr(gPEPtr->propMap);
	dp = (unsigned short *) GetPixBaseAddr(gPEPtr->undoMap);
	while (len--) {
		tmp = *sp;
		*(sp++) = *dp;
		*(dp++) = tmp;
	}
	len = ((*gPEPtr->propMaskMap)->rowBytes & 0x3FFF)*22;
	sp = (unsigned short *) GetPixBaseAddr(gPEPtr->propMaskMap);
	dp = (unsigned short *) GetPixBaseAddr(gPEPtr->undoMaskMap);
	while (len--) {
		tmp = *sp;
		*(sp++) = *dp;
		*(dp++) = tmp;
	}
	gPEPtr->hasUndo = true;
	RefreshProp();
}
