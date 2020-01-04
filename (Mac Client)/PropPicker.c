// PropPicker.c

#include "U-USER.H"
#include "AppMenus.h"
#include "UserTools.h"
#include "DialogUtils.h"

#define		PropWIND	134

enum {PT_New, PT_Edit, PT_Copy, PT_Delete, PT_Naked, NbrPickerTools};
short	gPickerIcons[] = {NewIcon,EditIcon,CopyIcon,DeleteIcon,NakedIcon,NbrPickerTools};

AssetSpec	gSelectProp;		// 6/7/95

typedef struct {
	ObjectWindowRecord	win;
	Rect			pickerRect,offPropRect,scrollRect;
	short			firstProp,nbrXProps,nbrYProps,nbrPickerFrames;
	GWorldPtr		offProp;
	PixMapHandle	offPropMap;
	Boolean			isHoriz;
} PropWindowRecord, *PropWindowPtr;

WindowPtr		gPropWin;
PropWindowPtr	gPropPtr;

void RefreshPickerScrollbar();
void DrawPickerProps(short minFrame, short maxFrame);
void ClickPickerScrollBar(Point p, Boolean dblClick);
void PropAdjustMenus(WindowPtr theWin);
void PropProcessCommand(WindowPtr theWin, short theMenu, short theItem);

void NewPropWindow(void)
{
	WindowPtr		theWindow;
	PropWindowPtr	propRec;
	Point			origin;
	OSErr			oe;

	SetPort((GrafPtr) gRoomWin);
	origin.h = gVideoRoomRect.right;
	origin.v = gVideoRoomRect.top;
	LocalToGlobal(&origin);
	origin.h += 4;
	if (origin.h < 1)
		origin.h = 1;
	if (origin.v < 28)
		origin.v = 28;
	if (origin.h < qd.screenBits.bounds.right && origin.h+152 > qd.screenBits.bounds.right)
		origin.h -= origin.h+152 -  qd.screenBits.bounds.right;

	propRec = (PropWindowPtr) NewPtrClear(sizeof(PropWindowRecord));
	theWindow = InitObjectWindow(PropWIND, (ObjectWindowPtr) propRec,true);
	MoveWindow(theWindow, origin.h,origin.v,false);
	RestoreWindowPos(theWindow, &gMacPrefs.propPickerPos);


	gPropWin = theWindow;
	gPropPtr = (PropWindowPtr) theWindow;
	((ObjectWindowPtr) theWindow)->Draw = PropWindowDraw;
	((ObjectWindowPtr) theWindow)->Dispose = DisposePropWin;
	((ObjectWindowPtr) theWindow)->HandleClick = PropWindowClick;
	((ObjectWindowPtr) theWindow)->AdjustMenus = PropAdjustMenus;
	((ObjectWindowPtr) theWindow)->ProcessCommand = PropProcessCommand;

	// Make it the current grafport
	SetPort(theWindow);

	// Show the window
	ShowWindow(theWindow);
	SelectFloating(theWindow);

	// SetRect(&gPropPtr->pPickerRect,0,0,PropWidth,PropHeight*(NbrPickerFrames+2));

	gPropMenu = GetMenu(PropMENU);
	InsertMenu(gPropMenu, HelpMENU);
	DrawMenuBar();

	SetRect(&gPropPtr->offPropRect,0,0,PropWidth,PropHeight);

	// 4/4/95 JBUM Modified to use gCurClut Custom Palette
	//
	if ((oe = NewGWorld(&gPropPtr->offProp,8,&gPropPtr->offPropRect,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"NewRoomWindow");
		return;
	}
	gPropPtr->offPropMap = GetGWorldPixMap(gPropPtr->offProp);
	LockPixels(gPropPtr->offPropMap);

	// 6/10/95
	SetToolBG(&gGray44);
	gPropPtr->isHoriz = false;
	if (gPropPtr->isHoriz) {
		short	i,x,y;
		x= (gPropWin->portRect.right - ToolWidth*NbrPickerTools)/2;
		y = (gPropWin->portRect.bottom - ToolWidth) - 7;
		for (i= 0; i < NbrPickerTools; ++i,x += ToolWidth)
			AddTool(gPropWin, gPickerIcons[i],gPickerIcons[i]+1,0,x,y);
		gPropPtr->nbrXProps = (gPropWin->portRect.right - 4) / PropWidth;
		gPropPtr->nbrYProps = (gPropWin->portRect.bottom - (ToolWidth + 5 + 10)) / PropHeight;
		SetRect(&gPropPtr->pickerRect, 0, 0, gPropPtr->nbrXProps*PropWidth, gPropPtr->nbrYProps*PropHeight);
		OffsetRect(&gPropPtr->pickerRect, 2, 2);
		gPropPtr->scrollRect = gPropPtr->pickerRect;
		InsetRect(&gPropPtr->scrollRect,2,0);
		gPropPtr->scrollRect.top = gPropPtr->scrollRect.bottom+1;
		gPropPtr->scrollRect.bottom = gPropPtr->scrollRect.top+18;
	}
	else {
		short	i,x,y;
		x = (gPropWin->portRect.right - ToolWidth) - 7;
		y = (gPropWin->portRect.bottom - ToolHeight*NbrPickerTools)/2;
		for (i= 0; i < NbrPickerTools; ++i,y += ToolHeight)
			AddTool(gPropWin, gPickerIcons[i],gPickerIcons[i]+1,0,x,y);
		gPropPtr->nbrXProps = (gPropWin->portRect.right - (ToolWidth + 5 + 10)) / PropWidth;
		gPropPtr->nbrYProps = (gPropWin->portRect.bottom - 4) / PropHeight;
		SetRect(&gPropPtr->pickerRect, 0, 0, gPropPtr->nbrXProps*PropWidth, gPropPtr->nbrYProps*PropHeight);
		OffsetRect(&gPropPtr->pickerRect, 2, 2);
		gPropPtr->scrollRect = gPropPtr->pickerRect;
		InsetRect(&gPropPtr->scrollRect,0,2);
		gPropPtr->scrollRect.left = gPropPtr->scrollRect.right+1;
		gPropPtr->scrollRect.right = gPropPtr->scrollRect.left+18;
	}
	// AddTool(gPropWin, ResizeIcon,0,0,gPropWin->portRect.right-32,gPropWin->portRect.bottom-32);
	gPropPtr->nbrPickerFrames = gPropPtr->nbrXProps * gPropPtr->nbrYProps;

/**
	AddTool(theWindow, UpArrowIcon, 0, UpArrowDim,
		(PropWidth-ToolWidth)/2,
		(PropHeight-ToolHeight)/2);
	AddTool(theWindow, DownArrowIcon,0, DownArrowDim,
		(PropWidth-ToolWidth)/2,
		(PropHeight-ToolHeight)/2+PropHeight*(NbrPickerFrames+1));

	// AddTool(theWindow, ClearIcon, 0, 0,(PropWidth-ToolWidth)/2,(PropHeight-ToolHeight)/2+PropHeight*(NbrPickerFrames+1)+32);
**/

}

void DisposePropWin(WindowPtr theWin)
{
	if (gPropPtr->offProp)
		DisposeGWorld(gPropPtr->offProp);

	SaveWindowPos(theWin, &gMacPrefs.propPickerPos);

	DefaultDispose(theWin);
	DisposePtr((Ptr) theWin);	// 6/10/95
	gPropWin = NULL;
	gPropPtr = NULL;
	gMode = M_Normal;
	DeleteMenu(PropMENU);
	DisposeMenu(gPropMenu);
	gPropMenu = NULL;
	DrawMenuBar();
}

void StartPropPicker(void)
{
	if (gPropWin == NULL)
		NewPropWindow();
	else
		SelectWindow(gPropWin);
}

void TogglePropPicker(void)
{
	GrafPtr	savePort;		// 6/10/95
	GetPort(&savePort);	
	if (gPropWin)
		DisposePropWin(gPropWin);
	else
		NewPropWindow();
	SetPort(savePort);
}

// Prop State has Changed - Redraw the Props...
void RefreshPropPicker(void)
{
	GrafPtr	savePort;

	if (gPropWin == NULL)
		return;
	GetPort(&savePort);
	SetPort(gPropWin);
	// Redraw Props and Scrollbar
	DrawPickerProps(0, gPropPtr->nbrPickerFrames-1);
	RefreshPickerScrollbar();
	SetPort(savePort);
}

void DrawPickerProps(short minFrame, short maxFrame)
{
	short		n,propIdx;
	Rect		sr,dr;
	GrafPtr		savePort,theWin = (GrafPtr) gPropWin;
	CGrafPtr	curPort;
	GDHandle	curDevice;
	Handle		h;
	AssetSpec	*aSpec,*fSpec;
	Boolean		gotIt = false;

	if (theWin == NULL)
		return;

	GetPort(&savePort);
	SetPort(theWin);
	
	// for (n = 0; n < NbrPickerTools; ++n)
	// 	DrawTool(n);
	// 6/10/95
//	ActivateTool(0,gPropPtr->firstProp > 0,true);
//	ActivateTool(1,gPropPtr->firstProp+NbrPickerFrames < gRoomWin->nbrFaveProps,true);
//	DrawTool(2);

	HLock(gRoomWin->faveProps);
	fSpec = (AssetSpec *) *gRoomWin->faveProps;

	SetRect(&sr,0,0,PropWidth,PropHeight);
	for (n = minFrame; n <= maxFrame; ++n) {
		propIdx= n+gPropPtr->firstProp;
		if (propIdx < gRoomWin->nbrFaveProps) {
			dr = sr;
			OffsetRect(&dr,PropWidth*(n%gPropPtr->nbrXProps),PropHeight*(n/gPropPtr->nbrXProps));
			OffsetRect(&dr,2,2);
			// OffsetRect(&dr,0,PropHeight*(n+1));

			GetGWorld(&curPort,&curDevice);
			SetGWorld(gPropPtr->offProp,NULL);
			ColorPaintRect(&sr,&gGray44);
			SetGWorld(curPort,curDevice);
			// 6/7/95
			h = GetAssetWithCRC('Prop',fSpec[propIdx].id,
								fSpec[propIdx].crc);
			if (h) {
				// Set the CRC if you can...6/7/95
				if (fSpec[propIdx].crc == 0L)
					fSpec[propIdx].crc = GetAssetCRC(h);
				DrawMansionPropPixMap(h,gPropPtr->offPropMap,0,0,0);
			}
			// InsetRect(&sr,1,0);
			// InsetRect(&dr,1,0);
			CopyBits((BitMap *) *gPropPtr->offPropMap,&theWin->portBits,
					&sr,&dr,srcCopy,NULL);
			aSpec= (AssetSpec *) *gRoomWin->faveProps;
			if (gSelectProp.id == aSpec[propIdx].id &&
				gSelectProp.crc == aSpec[propIdx].crc && gotIt == false) {
				gotIt = true;
				RevHiliteRect(&dr);
				InsetRect(&dr,1,1);
				RevHiliteRect(&dr);
			}
		}
		else {
			dr = sr;
			OffsetRect(&dr,PropWidth*(n%gPropPtr->nbrXProps),PropHeight*(n/gPropPtr->nbrXProps));
			OffsetRect(&dr,2,2);
			ColorPaintRect(&dr,&gGray44);
		}
	}
	HUnlock(gRoomWin->faveProps);
	SetPort(savePort);
}

void PropWindowDraw(WindowPtr theWin)
{
	Rect	r;
	RGBForeColor(&gGray44);
	PaintRect(&theWin->portRect);

	HiliteRect(&theWin->portRect);
	r = gPropPtr->pickerRect;
	InsetRect(&r,-1,-1);
	RevHiliteRect(&r);
	DrawPickerProps(0, gPropPtr->nbrPickerFrames-1);
	RGBForeColor(&gBlackColor);	// 5/1/94 JAB
	RefreshTools(theWin);
	RefreshPickerScrollbar();
}

void ClearPropPicker(void)
{
	DisposePropWin(gPropWin);
}

void PropWindowClick(WindowPtr theWin, Point p, EventRecord *theEvent)
{
	short	prop;
	short	n;
	Boolean shiftFlag = (theEvent->modifiers & shiftKey) > 0;
	short	toolNbr;
	AssetSpec	*spec;
	static long	lastClick;

	SetPort(theWin);
	
	GlobalToLocal(&p);

	if (ToolClick(p,&toolNbr)) {
		switch (toolNbr) {
		case PT_New:
			if (gPEWin == NULL)
				NewPEWindow(NULL);
			break;
		case PT_Edit:
			if (gPEWin == NULL && gSelectProp.id != 0)
				NewPEWindow(&gSelectProp);
			break;
		case PT_Copy:
			if (gSelectProp.id != 0)
				DuplicatePropFavorite(&gSelectProp);	// 6/9/95
			break;
		case PT_Delete:
			if (gSelectProp.id != 0)
				DeletePropFavorite(&gSelectProp);
			break;
		case PT_Naked:
			if (gRoomWin->mePtr->user.nbrProps)	// 6/7/95
				ChangeProp(0,NULL);
			break;
		case NbrPickerTools:
			// Resize it...
			break;
		}
/**
		switch (toolNbr) {
		case 0:	// up
			if (gPropPtr->firstProp) {
				gPropPtr->firstProp -= NbrPickerFrames - 1;
				if (gPropPtr->firstProp < 0)
					gPropPtr->firstProp = 0;
				RefreshPropPicker();
			}
			break;
		case 1:	// down
			if (gPropPtr->firstProp+NbrPickerFrames < gRoomWin->nbrFaveProps) {
				gPropPtr->firstProp += NbrPickerFrames - 1;
				if (gPropPtr->firstProp+NbrPickerFrames > gRoomWin->nbrFaveProps)
					gPropPtr->firstProp = gRoomWin->nbrFaveProps - NbrPickerFrames;
				RefreshPropPicker();
			}
			break;
		case 2:	// clear
			if (gRoomWin->mePtr->user.nbrProps)	// 6/7/95
				ChangeProp(0,NULL);
			break;
		}
 **/
	}
	else if (PtInRect(p,&gPropPtr->pickerRect)) {
		p.v -= gPropPtr->pickerRect.top;
		n = (p.v / PropHeight) * gPropPtr->nbrXProps + p.h / PropWidth;
		if (n >= 0 && n < gPropPtr->nbrPickerFrames) {
			prop = n + gPropPtr->firstProp;
			spec = &((AssetSpec *) *gRoomWin->faveProps)[prop];
			if (theEvent->when - lastClick < GetDblTime() && gSelectProp.id == spec->id &&
				gSelectProp.crc == spec->crc) {	// 6/23/95 Prop Picker
				if (spec->id < MinReservedProp || spec->id > MaxReservedProp) {
					if (MembersOnly(true))
						return;
					if (DeniedByServer(PM_AllowCustomProps))
						return;
				}
				ToggleProp(spec);
			}
			else {
				gSelectProp = *spec;
				RefreshPropPicker();
			}
		}
	}
	else if (PtInRect(p,&gPropPtr->scrollRect)) {
		// 8/22/95 If we just activated the window (updateRgn is non-empty)
		// Refresh...
		if (!EmptyRgn(((WindowPeek)gPropWin)->updateRgn))
			RefreshPropPicker();
		ClickPickerScrollBar(p,theEvent->when - lastClick <= GetDblTime());
	}
	lastClick = theEvent->when;
}

#define PickerThumbHeight	12
#define PickerThumbWidth	16
#define PickerThumbMargin	4
static short gActivePart=0;	// 1 = thumb 2 = upper 3 = lower

void PickerScroll(short delta);
void ComputePickerThumbRect(Rect *r);
void Diff2Delta(short pixDiff, short *delta);

void Diff2Delta(short pixDiff, short *delta)
{
	long	pixRange,scrollRange;
	short	tmp,t;
	// Convert pixel difference to suggested scroll delta...
	pixRange = gPropPtr->scrollRect.bottom - gPropPtr->scrollRect.top;
	pixRange -= PickerThumbHeight;
	pixRange -= PickerThumbMargin*2;
	scrollRange = gRoomWin->nbrFaveProps - gPropPtr->nbrPickerFrames;
	if ((t = scrollRange % gPropPtr->nbrXProps) > 0)
		scrollRange += gPropPtr->nbrXProps - t;
	tmp = (pixDiff*scrollRange) / pixRange;
	if (tmp < 0)
		while (abs(tmp) % gPropPtr->nbrXProps != 0)
			--tmp;
	else
		while (tmp % gPropPtr->nbrXProps != 0)
			++tmp;
	*delta = tmp;
}

void ComputePickerThumbRect(Rect *r)
{
	long	pixRange,scrollRange;
	short	scrollValue,t;
	SetRect(r,0,0,16,PickerThumbHeight);

	pixRange = gPropPtr->scrollRect.bottom - gPropPtr->scrollRect.top;
	pixRange -= PickerThumbHeight;
	pixRange -= PickerThumbMargin*2;
	scrollRange = gRoomWin->nbrFaveProps - gPropPtr->nbrPickerFrames;
	if ((t = scrollRange % gPropPtr->nbrXProps) > 0)
		scrollRange += gPropPtr->nbrXProps - t;

	if (scrollRange <= 0)
		SetRect(r,0,0,0,0);
	if (gPropPtr->firstProp == 0)
		scrollValue = 0;
	else
		scrollValue = (pixRange * gPropPtr->firstProp) / scrollRange;
	OffsetRect(r,gPropPtr->scrollRect.left+1,gPropPtr->scrollRect.top + PickerThumbMargin + scrollValue);
}

// !! Fix to use offscreen render or clip
void RefreshPickerScrollbar()
{
	Rect	r;
	Rect	thumbR;

	RGBForeColor(&gGray44);
	PaintRect(&gPropPtr->scrollRect);
	RGBForeColor(&gBlackColor);

	if (gPropPtr->isHoriz) {
	}
	else {
		r = gPropPtr->scrollRect;
		InsetRect(&r,4,0);
		RevHiliteRect(&r);
		ComputePickerThumbRect(&thumbR);
		if (!EmptyRect(&thumbR)) {
			DkHiliteRect(&thumbR);
			InsetRect(&thumbR,1,1);
			if (gActivePart == 1)
				RGBForeColor(&gGrayAA);
			else
				RGBForeColor(&gGray66);
			PaintRect(&thumbR);
			RGBForeColor(&gBlackColor);
		}
		if (gActivePart == 2) {	// upper
			InsetRect(&r,1,1);
			r.bottom = thumbR.top-2;
			RGBForeColor(&gGray66);
			PaintRect(&r);
			RGBForeColor(&gBlackColor);
		}
		else if (gActivePart == 3) { // lower
			InsetRect(&r,1,1);
			r.top = thumbR.bottom+2;
			RGBForeColor(&gGray66);
			PaintRect(&r);
			RGBForeColor(&gBlackColor);
		}
	}
}

void ScrollPropPicker(short delta);	// Positive scrolls contents upward
void ScrollPropPicker(short delta)	// Positive scrolls contents upward
{
	GrafPtr	savePort;
	Rect	sr,dr;
	GetPort(&savePort);
	SetPort(gPropWin);
	if (delta > 0) {
		// Redraw Props and Scrollbar
		// Scroll 0 - gPropPtr->nbrPickerFrames-delta-1
		SetRect(&sr,0,0,gPropPtr->nbrXProps*PropWidth,((gPropPtr->nbrPickerFrames-delta)/gPropPtr->nbrXProps)*PropHeight);
		dr = sr;
		OffsetRect(&sr,0,(delta / gPropPtr->nbrXProps) * PropHeight);
		OffsetRect(&sr,2,2);
		OffsetRect(&dr,2,2);
		CopyBits(&gPropWin->portBits, &gPropWin->portBits,
					&sr,&dr,srcCopy,NULL);
		DrawPickerProps(gPropPtr->nbrPickerFrames-delta, gPropPtr->nbrPickerFrames-1);
		RefreshPickerScrollbar();
	}
	else {
		delta = -delta;
		// Scroll delta - end
		SetRect(&sr,0,0,gPropPtr->nbrXProps*PropWidth,((gPropPtr->nbrPickerFrames-delta)/gPropPtr->nbrXProps)*PropHeight);
		dr = sr;
		OffsetRect(&dr,0,(delta / gPropPtr->nbrXProps) * PropHeight);
		OffsetRect(&sr,2,2);
		OffsetRect(&dr,2,2);
		CopyBits(&gPropWin->portBits, &gPropWin->portBits,
					&sr,&dr,srcCopy,NULL);
		DrawPickerProps(0, delta-1);
		RefreshPickerScrollbar();
	}
	SetPort(savePort);
}

// Note: ScrollPropPicker doesn't work if window is obscured...

void PickerScroll(short delta)
{
	short	oldPos = gPropPtr->firstProp,limit,t,absDelta;
	Rect	vRect,sr;
	Boolean	obscured;
	if (delta) {
		vRect = (*gPropWin->visRgn)->rgnBBox;
		SectRect(&gPropWin->portRect, &vRect, &sr);
		obscured = !EqualRect(&gPropWin->portRect, &sr);
		gPropPtr->firstProp += delta;
		limit = gRoomWin->nbrFaveProps - gPropPtr->nbrPickerFrames;
		if ((t = limit % gPropPtr->nbrXProps) > 0)
			limit += gPropPtr->nbrXProps - t;
		if (gPropPtr->firstProp > limit)
			gPropPtr->firstProp = limit;
		if (gPropPtr->firstProp < 0)
			gPropPtr->firstProp = 0;
		if (gPropPtr->firstProp != oldPos) {
			absDelta = abs(gPropPtr->firstProp - oldPos);
			if (!obscured && absDelta < gPropPtr->nbrPickerFrames-1 && absDelta % gPropPtr->nbrXProps == 0)
				ScrollPropPicker(gPropPtr->firstProp - oldPos);
			else
				RefreshPropPicker();
		}
	}
}

void ClickPickerScrollBar(Point p, Boolean dblClick)
{
	Rect	r,thumbR;
	Point	np;
	short	delta;
	ComputePickerThumbRect(&thumbR);
	if (PtInRect(p, &thumbR)) {
		gActivePart = 1;
		RefreshPickerScrollbar();
		while (WaitMouseUp()) {
			GetMouse(&np);
			if (!EqualPt(np,p)) {
				ComputePickerThumbRect(&thumbR);
				Diff2Delta(np.v - (thumbR.bottom+thumbR.top)/2,&delta);
				PickerScroll(delta);
				p = np;
			}
		}
	}
	else {
		short	pageAmount;
		long	lastClick;
		r = gPropPtr->scrollRect;
		InsetRect(&r,4,0);
		pageAmount = gPropPtr->nbrXProps*gPropPtr->nbrYProps - gPropPtr->nbrXProps;
		if (PtInRect(p, &r)) {
			if (p.v < thumbR.top) {
				gActivePart = 2;
				PickerScroll(-pageAmount);
				lastClick = TickCount();
				while (WaitMouseUp()) {
					GetMouse(&np);
					if (TickCount()- lastClick > GetDblTime()) {
						ComputePickerThumbRect(&thumbR);
						if (PtInRect(np,&r) && np.v < thumbR.top)
							PickerScroll(-pageAmount);
					}
				}
			}
			else if (p.v >= thumbR.bottom) {
				gActivePart = 3;
				PickerScroll(pageAmount);
				lastClick = TickCount();
				while (WaitMouseUp()) {
					GetMouse(&np);
					if (TickCount()- lastClick > GetDblTime()) {
						ComputePickerThumbRect(&thumbR);
						if (PtInRect(np,&r) && np.v >= thumbR.bottom)
							PickerScroll(pageAmount);
					}
				}
			}
		}
	}
	gActivePart = 0;
	RefreshPickerScrollbar();
}

#define PurgeDLOG		506
#define P_OK			1
#define P_Cancel		2

short PurgeDialog(void);

short PurgeDialog(void)
{
	GrafPtr		savePort;
	DialogPtr	dp;
	short		itemHit;

	GetPort(&savePort);
	if ((dp = GetNewDialog (PurgeDLOG, NULL, (WindowPtr) -1)) == NULL)
		return P_Cancel;

	SetDialogDefaultItem(dp, P_OK);
	SetDialogCancelItem(dp, P_Cancel);
	SetDialogTracksCursor(dp,true);

	ShowWindow(dp);

	do {
		ModalDialog(NULL, &itemHit);
	} while (itemHit != P_OK && itemHit != P_Cancel);
	DisposeDialog(dp);
	SetPort(savePort);
	return itemHit;
}


void ShowStatusBar(long total, long current, long remainingTime);

void PurgeOldProps(void)
{
	extern AssetFileVars	*curAssetFile;		/* current resource file/map */
	long			nbrPurged=0;
	unsigned long	curTime,cutoffTime;
	long			i,n,j;
	char			smsg[32];
	AssetSpec		pSpec;
	AssetFileVars	*ar;
	AssetTypeRec	*at;
	AssetRec		*ap,*apN;
	LONG			nbrTypeAssets;

	if (PurgeDialog() != P_OK)
		return;

	GetDateTime(&curTime);
	cutoffTime = curTime - 60L*60L*24L;

	StatusMessage("Searching for old props...",0);
	SpinCursor();

#define CLIENT	1

	ar = curAssetFile;
	at = (AssetTypeRec *) *ar->typeList;
	ap = (AssetRec huge  *) *ar->assetList;
	apN = ap;	/* New variable */
	n = 0;
	nbrPurged = 0;

	for (j = 0; j < ar->afMap.nbrTypes; ++j,++at) {
#if CLIENT
		SpinCursor();
#endif
		at->firstAsset = n;
		if (ap == apN && at->assetType != RT_PROP) {
			n += at->nbrAssets;
			ap += at->nbrAssets;
			apN += at->nbrAssets;
		}
		else {
			nbrTypeAssets = at->nbrAssets;
			for (i = 0; i < nbrTypeAssets; ++i) {
				*ap = *apN;
				if (at->assetType == RT_PROP) {
#if CLIENT
					SpinCursor();
#endif
					pSpec.id = ap->idNbr;
					pSpec.crc = ap->crc;
					if (!(ap->flags & AssetLoaded) && 
						 ap->lastUseTime < cutoffTime && 
#if CLIENT
						 !IsPropFavorite(&pSpec) &&
#endif
						(ap->idNbr < 0 || ap->idNbr > MaxReservedProp)) 
					{
						/* Note: This should be replaced with an asset swap with last
						 * asset in group (since we're already screwing with ordering
						 * when we sort the assets)
						 */
						/* 7/20/96 removed this */
						/* if (n < ar->afMap.nbrAssets-1)
						 *	BlockMove((Ptr)(ap+1),(Ptr)ap,(ar->afMap.nbrAssets-(n+1))*sizeof(AssetRec));
						 */
						ar->afMap.nbrAssets--;
						at->nbrAssets--;
						ar->fileNeedsUpdate = true;
						++nbrPurged;
						++apN;	/* 7/20/96 */
					}
					else {
						++n;
						++ap;
						++apN;
					}
				}
				else {
					++n;
					++ap;
					++apN;
				}
			}
		}
	}
	SpinCursor();
	StatusMessage("Compacting Prop File...",0);
	WriteAssetFile();
	sprintf(smsg,"%ld Props Purged\r",nbrPurged);
	StatusMessage(smsg,0);
	// RefreshRoom(&gOffscreenRect);
}


void PropAdjustMenus(WindowPtr theWin)
{
	Boolean	hasSelection,hasClip;
	hasSelection = false;
	hasClip = CanPaste('PICT');
	DefaultAdjustMenus(theWin);
	if (hasClip)
		MyEnableMenuItem(gEditMenu, EM_Paste, true);
}

void PropProcessCommand(WindowPtr theWin, short theMenu, short theItem)
{
	switch (theMenu) {
	case EditMENU:
		switch (theItem) {
		case EM_Paste:
			if (gPEWin == NULL && gPropWin && CanPaste('PICT')) {
				PasteLarge();
				return;
			}
		}
	}
	DefaultProcessCommand(theWin,theMenu,theItem);
}
