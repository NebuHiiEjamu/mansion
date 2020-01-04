// DrawTools.c

#include "U-USER.H"
#include <Picker.h>
#include "UserTools.h"	// 6/10/95
#include "U-SECURE.H"

#define		DrawWIND	133
#define		PalWidth	40		// 4/4/95 JBUM added Constants
#define		PalHeight	180		// 4/4/95 JBUM added Constants

enum		{V_Pencil, V_FreePoly};	// Path Variations

short	gToolIcon[NbrDrawTools] = 
			{	PencilIcon,
				LineIcon,
				DetonatorIcon,
				BackgroundIcon
				};

Rect		gDrawPaletteRect;
Boolean		gFillShape;
short		gCurrentDrawTool = DT_Pencil;

RGBColor	gForeColor={0xFFFF,0,0},
			gBackColor={0,0,0xFFFF};

short		gCurPenSize=3;
short		gCurLayer=0;	// 0 = background, 1=foreground
short		gCurPattern=0;
Pattern		gSysPatterns[MaxPatterns];
short		gCurVariation = V_Pencil;

typedef struct {
	ObjectWindowRecord	win;
	GWorldPtr			palWorld;
	PixMapHandle 		palMap;
	Rect				palRect;
} DrawWindowRecord, *DrawWindowPtr;

WindowPtr		gDrawWin;
DrawWindowPtr	gDrawPtr;


// 4/4/95 NEW JBUM
void GetPalPixel(short x, short y, RGBColor *rgb)
{
	unsigned char	*p;
	unsigned char *baseAddr;
	baseAddr = (unsigned char *) GetPixBaseAddr(gDrawPtr->palMap);
	if (baseAddr) {
		p = baseAddr + ((*gDrawPtr->palMap)->rowBytes & 0x3FFF) * y + x;
		*rgb = (*gCurCLUT)->ctTable[*p].rgb;
	}
	else {
		rgb->red = rgb->green = rgb->blue = 0;
	}
}

// 4/4/95 NEW JBUM
void GetPalSelect(Rect *r)
{
	RGBColor	rgb;
	short		x,y;
	unsigned char *p;
	unsigned char *baseAddr;
	long		rowBytes;

	SetRect(r,0,0,0,0);
	baseAddr = (unsigned char *) GetPixBaseAddr(gDrawPtr->palMap);
	rowBytes = 	((*gDrawPtr->palMap)->rowBytes & 0x3FFF);
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
	return;
}

void NewDrawWindow()
{
	WindowPtr		theWindow;
	DrawWindowPtr	drawRec;
	Rect			r;
	short			n;

	// short		i;
	OSErr		oe;
	CGrafPtr	curPort;
	GDHandle	curDevice;
	// RGBColor	rgb;
	Point		origin;

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 

	SetPort((GrafPtr) gRoomWin);
	origin = topLeft(gVideoRoomRect);
	LocalToGlobal(&origin);
	origin.h -= PropWidth+4;
	if (origin.h < 1)
		origin.h = 1;
	if (origin.v < 28)
		origin.v = 28;

	drawRec = (DrawWindowPtr) NewPtrClear(sizeof(DrawWindowRecord));
	theWindow = InitObjectWindow(DrawWIND, (ObjectWindowPtr) drawRec,true);
	MoveWindow(theWindow, origin.h,origin.v,false);
	RestoreWindowPos(theWindow, &gMacPrefs.drawPos);

	gDrawWin = theWindow;
	gDrawPtr = (DrawWindowPtr) theWindow;
	((ObjectWindowPtr) theWindow)->Draw = DrawWindowDraw;
	((ObjectWindowPtr) theWindow)->Dispose = DisposeDrawWin;
	((ObjectWindowPtr) theWindow)->HandleClick = DrawWindowClick;
	((ObjectWindowPtr) theWindow)->AdjustCursor = DrawAdjustCursor;
	
	// Show the window
	ShowWindow(theWindow);
	SelectFloating(theWindow);

	// Make it the current grafport
	SetPort(theWindow);

	// 4/4/95 JAB - New Palette Handling code
	SetRect(&r,0,0,PalWidth,PalHeight);
	if ((oe = NewGWorld(&gDrawPtr->palWorld,8,&r,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"StartDrawPalette");
		return;
	}
	gDrawPtr->palMap = GetGWorldPixMap(gDrawPtr->palWorld);
	LockPixels(gDrawPtr->palMap);

	GetGWorld(&curPort,&curDevice);
	SetGWorld(gDrawPtr->palWorld,NULL);
	PaintRect(&gDrawPtr->palWorld->portRect);

	// 4/4/95 New Palette Rendering Code
	//
	{
		PicHandle	h;
		h = GetPicture(128);
		if (h) {
			DrawPicture(h,&r);
			ReleaseResource((Handle) h);
		}
	}
/*	for (i = 0; i < 256; ++i) {*/
/*		ComputePaletteRect(i,&r);*/
/*		Index2Color(i,&rgb);*/
/*		RGBForeColor(&rgb);*/
/*		PaintRect(&r);*/
/*	}*/

	RGBForeColor(&gBlackColor);
	SetGWorld(curPort,curDevice);

	SetRect(&gDrawPaletteRect,0,0,ToolCellWidth,ToolCellHeight*NbrDrawTools);
	gDrawPtr->palRect = gDrawPtr->palWorld->portRect;
	OffsetRect(&gDrawPtr->palRect,gDrawPaletteRect.left+2,gDrawPaletteRect.bottom+4);

	SetToolBG(&gGray44);
	for (n = 0; n < NbrDrawTools; ++n)
		AddTool(theWindow, gToolIcon[n],0,0,
				(ToolCellWidth-ToolWidth)/2+gDrawPaletteRect.left,
				(ToolCellHeight-ToolHeight)/2+gDrawPaletteRect.top+ToolCellHeight*n);

}


void DisposeDrawWin(WindowPtr theWin)
{
	if (gDrawPtr->palWorld) {
		DisposeGWorld(gDrawPtr->palWorld);
	}
	SaveWindowPos(theWin, &gMacPrefs.drawPos);
	DefaultDispose(theWin);
	DisposePtr((Ptr) theWin);	// 6/10/95
	gDrawWin = NULL;
	gDrawPtr = NULL;
	gMode = M_Normal;
}

void InitDrawTools(void)
{
	short	i;
	for (i = 1; i <= MaxPatterns; ++i)
		GetIndPattern(&gSysPatterns[i-1],0,i);
}

void ToggleDrawPalette(void)
{
	GrafPtr	savePort;		// 6/10/95
	GetPort(&savePort);	
	if (gDrawWin)
		DisposeDrawWin(gDrawWin);
	else
		NewDrawWindow();
	SetPort(savePort);
}

void UpdatePaletteGraphics(void)
{
	CGrafPtr		curPort;
	GDHandle		curDevice;
	Rect			r;
	RGBColor		fc;
	extern GWorldPtr	gOffTools;
	// Indicate correct layer
	ChangeToolIcon(DT_Layer, gCurLayer? ForegroundIcon : BackgroundIcon);
	ChangeToolIcon(DT_Pencil, gCurVariation == V_FreePoly? FreePolyIcon : PencilIcon);

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

void RefreshDrawPalette(void)
{
	short	n;
	Rect	dr;
	GrafPtr	savePort;

	GetPort(&savePort);
	SetPort(gDrawWin);

	UpdatePaletteGraphics();
	RefreshTools(gDrawWin);

	CopyBits((BitMap *) *gDrawPtr->palMap,&gDrawWin->portBits,
				&gDrawPtr->palWorld->portRect,&gDrawPtr->palRect,srcCopy,NULL);

	n = Color2Index(&gForeColor);

	// 4/4/95 JBUM New Palette Highlighting Code
	GetPalSelect(&dr);
	OffsetRect(&dr,gDrawPtr->palRect.left,gDrawPtr->palRect.top);
	FrameRect(&dr);

	SetPort(savePort);
}

void DrawWindowDraw(WindowPtr theWin)
{
	RGBForeColor(&gGray44);
	PaintRect(&theWin->portRect);

	HiliteRect(&theWin->portRect);

	RefreshDrawPalette();

	RGBForeColor(&gBlackColor);	// JAB 5/1/94
}


void StartDrawPalette(void)
{
	if (gDrawWin == NULL)
		NewDrawWindow();
	else
		SelectWindow(gDrawWin);
}

void ClearDrawPalette(void)
{
	DisposeDrawWin(gDrawWin);
}

Boolean EqualDelta(Point p1, Point p2)
{
	if (p1.h == 0)
		return (p2.h == 0);
	else if (p1.v == 0)
		return (p2.v == 0);
	else if (p1.h == p1.v)
		return (p2.h == p2.v);
	else if (p1.h == p2.h && p1.v == p2.v)
		return true;
	else
		return false;
}

static RgnHandle	saveClip,newClip;

void BeginPaintClipping(void)
{
	RgnHandle	tempClip;
	saveClip = NewRgn();
	newClip = NewRgn();
	GetClip(saveClip);
	RectRgn(newClip,&gVideoRoomRect);
	if ((gMode == M_Authoring) && (g12InchMode || !gFullScreen)) {
		tempClip = NewRgn();
		RectRgn(tempClip,&gDrawPaletteRect);
		DiffRgn(newClip,tempClip,newClip);
		DisposeRgn(tempClip);
	}	

	SetClip(newClip);
}

void EndPaintClipping(void)
{
	SetClip(saveClip);
	DisposeRgn(newClip);
	DisposeRgn(saveClip);
}

Boolean PencilTool(Point p, Boolean shiftFlag)
{
	Point		np,lp,anchorPt,delta,lastDelta;
	Point		sAry[MaxPoints];
	char		sBuf[sizeof(DrawRecord)+32+MaxPoints*4];
	Ptr			sp;
	short		nbrPts,i;
	DrawRecPtr	dp;

	// Bug - local room re-organization is causing room-vars to eventually exceed
	// 20000
	if (gRoomWin->curRoom.lenVars > 20000)
		return false;

	SetCursor(*gCursHandles[PencilCursor]);	

	BeginPaintClipping();

	if (gCurVariation == V_Pencil)
		PenSize(gCurPenSize,gCurPenSize);
	RGBForeColor(&gForeColor);
	lp = p;
	MoveTo(lp.h,lp.v);
	anchorPt = lp;
	anchorPt.h -= gVideoRoomRect.left;
	anchorPt.v -= gVideoRoomRect.top;
	nbrPts = 0;
	lastDelta.h = lastDelta.v = 0;
	// This function compacts the point list by
	// deleting intermediary points of identical slope
	//
	while (WaitMouseUp() && nbrPts < 128) {
		GetMouse(&np);
		if (!EqualPt(np,lp)) {
			LineTo(np.h,np.v);
			delta.h = np.h - lp.h;
			delta.v = np.v - lp.v;
			if (nbrPts && EqualDelta(delta,lastDelta)) {
				sAry[nbrPts-1].h += delta.h;
				sAry[nbrPts-1].v += delta.v;
			}
			else {
				sAry[nbrPts++] = delta;
				lastDelta = delta;
			}
			lp = np;
		}
	}
	EndPaintClipping();

	PenNormal();
	RGBForeColor(&gBlackColor);
	
// 11/25/95 JAB - bad idea - confuses the shit out of users...
//	if (nbrPts == 0)			// 6/8/95
//		return false;
	if (nbrPts == 0) {
		nbrPts = 1;
		sAry[0].h = 0;
		sAry[0].v = 0;
	}

	sp = sBuf;
	dp = (DrawRecPtr) sp;	sp += sizeof(DrawRecord);
	dp->link.nextOfst = 0;
	dp->drawCmd = DC_Path | (gCurLayer? 0x8000 : 0x0000);
	dp->drawCmd |= (gCurVariation << 8);
	*((short *) sp) = gCurPenSize;		sp += sizeof(short);
	*((short *) sp) = nbrPts;			sp += sizeof(short);
	*((RGBColor *) sp) = gForeColor;	sp += sizeof(RGBColor);
	*((Point *) sp) = anchorPt;			sp += sizeof(Point);
	for (i = 0; i < nbrPts; ++i) {
		*((Point *) sp) = sAry[i];		sp += sizeof(Point);
	}
	dp->cmdLength = (long) sp - (long) sBuf;
	dp->cmdLength -= sizeof(DrawRecord);
	dp->dataOfst = 0;
	PostServerEvent(MSG_DRAW, gRoomWin->meID, (Ptr) sBuf, sizeof(DrawRecord) + dp->cmdLength);
	return true;
}

void LineTool(Point p, Boolean shiftFlag)
{
	Point		np,lp,anchorPt;
	char		sBuf[sizeof(DrawRecord)+32];
	Ptr			sp;
	DrawRecPtr	dp;
	Rect		bounds;

	if (gRoomWin->curRoom.lenVars > 20000)
		return;

	BeginPaintClipping();

	PenSize(gCurPenSize,gCurPenSize);
	RGBForeColor(&gForeColor);
	lp = p;
	MoveTo(lp.h,lp.v);
	anchorPt = lp;

	// This function compacts the point list by
	// deleting intermediary points of identical slope
	//
	PenMode(patXor);
	Pt2Rect(anchorPt,anchorPt,&bounds);
	MoveTo(anchorPt.h,anchorPt.v);
	LineTo(anchorPt.h,anchorPt.v);
	while (WaitMouseUp()) {
		GetMouse(&np);
		if (!EqualPt(np,lp)) {
			MoveTo(anchorPt.h,anchorPt.v);
			LineTo(lp.h,lp.v);
			Pt2Rect(anchorPt,np,&bounds);
			MoveTo(anchorPt.h,anchorPt.v);
			LineTo(np.h,np.v);
			lp = np;
		}
	}
	EndPaintClipping();

	PenNormal();
	RGBForeColor(&gBlackColor);
	if (!EqualPt(lp,anchorPt)) {
		SubPt(topLeft(gVideoRoomRect),&anchorPt);
		SubPt(topLeft(gVideoRoomRect),&lp);
		SubPt(anchorPt,&lp);
		sp = sBuf;
		dp = (DrawRecPtr) sp;	sp += sizeof(DrawRecord);
		dp->link.nextOfst = 0;
		dp->drawCmd = DC_Path | (gCurLayer? 0x8000 : 0x0000);
		*((short *) sp) = gCurPenSize;		sp += sizeof(short);
		*((short *) sp) = 1;				sp += sizeof(short);
		*((RGBColor *) sp) = gForeColor;	sp += sizeof(RGBColor);
		*((Point *) sp) = anchorPt;			sp += sizeof(Point);
		*((Point *) sp) = lp;				sp += sizeof(Point);
		dp->cmdLength = (long) sp - (long) sBuf;
		dp->cmdLength -= sizeof(DrawRecord);
		dp->dataOfst = 0;
		PostServerEvent(MSG_DRAW, gRoomWin->meID, (Ptr) sBuf, sizeof(DrawRecord) + dp->cmdLength);
	}
}

void ShapeTool(Point p, Boolean shiftFlag, short shapeType)
{
	Point		np,lp,anchorPt;
	char		sBuf[sizeof(DrawRecord)+32];
	Ptr			sp;
	DrawRecPtr	dp;
	Rect		bounds;

	if (gRoomWin->curRoom.lenVars > 20000)
		return;

	BeginPaintClipping();

	PenSize(gCurPenSize,gCurPenSize);
	RGBForeColor(&gForeColor);
	lp = p;
	MoveTo(lp.h,lp.v);
	anchorPt = lp;

	// This function compacts the point list by
	// deleting intermediary points of identical slope
	//
	PenMode(patXor);
	Pt2Rect(anchorPt,anchorPt,&bounds);
	switch (shapeType) {
	case ST_Rect:	FrameRect(&bounds);	break;
	case ST_Oval:	FrameOval(&bounds);	break;
	}
	while (WaitMouseUp()) {
		GetMouse(&np);
		if (!EqualPt(np,lp)) {
			switch (shapeType) {
			case ST_Rect:	FrameRect(&bounds);	break;
			case ST_Oval:	FrameOval(&bounds);	break;
			}
			Pt2Rect(anchorPt,np,&bounds);
			switch (shapeType) {
			case ST_Rect:	FrameRect(&bounds);	break;
			case ST_Oval:	FrameOval(&bounds);	break;
			}
			lp = np;
		}
	}
	EndPaintClipping();

	PenNormal();

	RGBForeColor(&gBlackColor);

	if (!EmptyRect(&bounds)) {
		OffsetRect(&bounds,-gVideoRoomRect.left,-gVideoRoomRect.top);
		
		sp = sBuf;
		dp = (DrawRecPtr) sp;	sp += sizeof(DrawRecord);
		dp->link.nextOfst = 0;
		dp->drawCmd = DC_Shape | (gCurLayer? 0x8000 : 0x0000);
	
		*((short *) sp) = shapeType;		sp += sizeof(short);
		*((short *) sp) = gCurPenSize;		sp += sizeof(short);
		*((RGBColor *) sp) = gForeColor;	sp += sizeof(RGBColor);
		*((RGBColor *) sp) = gBackColor;	sp += sizeof(RGBColor);
		*((char *) sp) = gFillShape;		sp += sizeof(char);
		*((char *) sp) = gCurPattern;		sp += sizeof(char);
		*((Rect *) sp) = bounds;			sp += sizeof(Rect);
	
		dp->cmdLength = (long) sp - (long) sBuf;
		dp->cmdLength -= sizeof(DrawRecord);
		dp->dataOfst = 0;
		PostServerEvent(MSG_DRAW, gRoomWin->meID, (Ptr) sBuf, sizeof(DrawRecord) + dp->cmdLength);
	}
}


void UseDetonateTool(short type)
{
	DrawRecord	dp;
	dp.link.nextOfst = 0;
	dp.drawCmd = type;
	dp.cmdLength = 0;
	dp.dataOfst = 0;
	PostServerEvent(MSG_DRAW, gRoomWin->meID, (Ptr) &dp, sizeof(DrawRecord));
}

void ChooseDrawColor(RGBColor *rgb, short background)
{
	if (background)
		gBackColor = *rgb;
	else
		gForeColor = *rgb;
	if (gPEWin)
		RefreshPropToolPalette();
	else if (gDrawWin)
		RefreshDrawPalette();
}

void DrawWindowClick(WindowPtr theWin, Point p, EventRecord *theEvent)
{
	static long lastSelectTime, lastSelect;
	Boolean shiftFlag = (theEvent->modifiers & shiftKey) > 0;
	short		toolNbr;

	SetPort(theWin);
	
	GlobalToLocal(&p);

	if (PtInToolList(p,&toolNbr)) {
		SetCursor(&qd.arrow);	

		switch (toolNbr) {
		case DT_Pencil:
			gCurVariation = !gCurVariation;
			RefreshDrawPalette();
			break;
		case DT_Linesize:
			p.v %= ToolCellHeight;
			p.h %= ToolCellWidth;
			if (p.h > p.v+2) {
				if (gCurPenSize < MaxPenSize) {
					++gCurPenSize;
					RefreshDrawPalette();
				}
			}
			else if (p.h < p.v-2) {
				if (gCurPenSize > 1) {
					--gCurPenSize;
					RefreshDrawPalette();
				}
			}
			break;
		case DT_Detonator:
			if  (lastSelect == DT_Detonator && TickCount() - lastSelectTime < GetDblTime())
				UseDetonateTool(DC_Detonate);
			else
				UseDetonateTool(DC_Delete);
			break;
		case DT_Layer:
			gCurLayer = !gCurLayer;
			RefreshDrawPalette();
			break;
		}
		lastSelect = toolNbr;
		lastSelectTime = TickCount();
	}
	else if (PtInRect(p,&gDrawPtr->palRect)) {
		// 4/4/95 JBUM New EyeDropper Code
		//
		RGBColor	rgb,curRgb;
		short		layer;

		SetCursor(*gCursHandles[EyedropperCursor]);		// 6/21/95

		layer = (theEvent->modifiers & cmdKey) > 0;
		if (layer)
			curRgb = gBackColor;
		else
			curRgb = gForeColor;
		GetPalPixel(p.h-gDrawPtr->palRect.left,p.v-gDrawPtr->palRect.top,&rgb);
		ChooseDrawColor(&rgb,layer);
		while (WaitMouseUp()) {
			GetMouse(&p);
			if (PtInRect(p,&gDrawPtr->palRect)) {
				GetPalPixel(p.h-gDrawPtr->palRect.left,p.v-gDrawPtr->palRect.top,&rgb);
				ChooseDrawColor(&rgb,layer);
			}
			else {
				ChooseDrawColor(&curRgb,layer);
			}
		}
	}
}

Boolean DrawInRoom(Point p, EventRecord *theEvent)
{
	Boolean	shiftFlag = (theEvent->modifiers & shiftKey) > 0;
	RoomRecPtr	rp = &gRoomWin->curRoom;

	if ((rp->roomFlags & RF_NoPainting) && !(gRoomWin->userFlags & U_SuperUser)) {
		StdStatusMessage(SM_NoPainting);
		return false;
	}

	if (!(gRoomWin->userFlags & U_SuperUser) && DeniedByServer(PM_AllowPainting))
		return false;

	if ((gRoomWin->userFlags & U_Guest)) {
		StdStatusMessage(SM_NoPainting);
		return false;
	}
	if(PtInRect(p,&gVideoRoomRect)) {
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
			return true;
		}
		else {
			// Perform Drawing
			switch (gCurrentDrawTool) {
			case DT_Pencil:
				return PencilTool(p,shiftFlag);
				break;
			}
		}
	}
	return false;
}

void RefreshDrawObjects(Rect *rr, short layer)
{
	DrawRecPtr	dp = (DrawRecPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.firstDrawCmd];
	short		i,penSize,ptCount,shapeType;
	RGBColor	foreColor,backColor;
	Ptr			p;
	Point		pt,anchorPt;
	Rect		bounds;
	short		fillShape,fillPattern,variation;

	for (i = 0; i < gRoomWin->curRoom.nbrDrawCmds; ++i)
	{
		p = (Ptr) &gRoomWin->curRoom.varBuf[dp->dataOfst];
		if ((dp->drawCmd & 0x8000) > 0 != layer)
			goto NextLoop;
		variation = (dp->drawCmd >> 8) & 0x7F;
		switch (dp->drawCmd & 0x00FF) {
		case DC_Ellipse:
            penSize   = *((short *) p);         p += sizeof(short);
            ptCount   = *((short *) p);         p += sizeof(short);
			foreColor = *((RGBColor *) p);		p += sizeof(RGBColor);
            anchorPt  = *((Point *) p);         p += sizeof(Point);
            pt        = *((Point *) p);
            bounds.left   = anchorPt.h;
            bounds.top    = anchorPt.v;
            bounds.right  = bounds.left + pt.h;
            bounds.bottom = bounds.top  + pt.v;

			RGBForeColor(&foreColor);
            if (variation == V_FreePoly)
				PaintOval(&bounds);
            else {
				PenSize(penSize,penSize);
				FrameOval(&bounds);
			}
			PenNormal();
			RGBForeColor(&gBlackColor);
			break;
		case DC_Path:
			penSize = *((short *) p);		p += sizeof(short);
			ptCount = *((short *) p);		p += sizeof(short);
			foreColor = *((RGBColor *) p);	p += sizeof(RGBColor);
			anchorPt = *((Point *) p);		p += sizeof(Point);

			RGBForeColor(&foreColor);

			switch (variation) {
			case V_Pencil:
				PenSize(penSize,penSize);
				MoveTo(anchorPt.h,anchorPt.v);
				while (ptCount--) {
					pt = *((Point *) p);		p += sizeof(Point);
					Line(pt.h,pt.v);
				}
				PenNormal();
				RGBForeColor(&gBlackColor);
				break;
			case V_FreePoly:
				{
					PolyHandle	poly;
					poly = OpenPoly();
					MoveTo(anchorPt.h,anchorPt.v);
					while (ptCount--) {
						pt = *((Point *) p);		p += sizeof(Point);
						Line(pt.h,pt.v);
					}
					ClosePoly();
					PaintPoly(poly);
					KillPoly(poly);
				}
				break;
			}
			RGBForeColor(&gBlackColor);
			break;
		case DC_Shape:
			shapeType = *((short *) p);		p += sizeof(short);
			penSize = *((short *) p);		p += sizeof(short);
			foreColor = *((RGBColor *) p);	p += sizeof(RGBColor);
			backColor = *((RGBColor *) p);	p += sizeof(RGBColor);
			fillShape = *((char *) p);		p += sizeof(char);
			fillPattern = *((char *) p);	p += sizeof(char);
			bounds = *((Rect *) p);			p += sizeof(Rect);

			if (fillShape) {
				RGBForeColor(&backColor);
				PenPat(&gSysPatterns[fillPattern]);
				switch (shapeType) {
				case ST_Rect:	PaintRect(&bounds);	break;
				case ST_Oval:	PaintOval(&bounds);	break;
				}
				PenNormal();
			}
			RGBForeColor(&foreColor);
			PenSize(penSize,penSize);
			switch (shapeType) {
			case ST_Rect:	FrameRect(&bounds);	break;
			case ST_Oval:	FrameOval(&bounds);	break;
			}
			PenNormal();
			RGBForeColor(&gBlackColor);
			break;

		case DC_Text:
			break;
		}
NextLoop:
		if (dp->link.nextOfst)
			dp = (DrawRecPtr) &gRoomWin->curRoom.varBuf[dp->link.nextOfst];
		else
			break;
	}
}

void DrawAdjustCursor(WindowPtr theWin, Point p, EventRecord *er)
{
	if (PtInRect(p,&theWin->portRect)) {
		if (PtInRect(p,&gDrawPtr->palRect))
			SetCursor(*gCursHandles[EyedropperCursor]);	
		else
			SetCursor(&qd.arrow);
	}
}

