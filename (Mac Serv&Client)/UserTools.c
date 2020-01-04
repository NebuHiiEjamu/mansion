// ToolStuff.c
//
// Control Manager-like routines 
#include "UserTools.h"

typedef struct IconSetup
{
	short	nbrTools;
	short	firstTool;
	short	nbrFaces;
	short	firstFace;
	short	nbrProps;
	short	firstProp;
	Point	propOffsets[1];
} IconSetup;

GWorldPtr		gOffTools;
PixMapHandle	gOffToolsMap;
Rect			gToolsRect;
RGBColor		gToolBG = {0x4444,0x4444,0x4444};

void ReportError(short code, char *name);
void DisposeTools();

extern IconSetup		*gIconSetup;


void InitTools()
{
	Handle			h;
	OSErr			oe;
	CGrafPtr		curPort;
	GDHandle		curDevice;
	extern CTabHandle	gCurCLUT;

	h = GetResource('icnr',128);
	MoveHHi(h);
	HLock(h);
	HNoPurge(h);
	gIconSetup = (IconSetup *) *h;

	SetRect(&gToolsRect,0,0,gIconSetup->nbrTools * ToolWidth,ToolHeight);

	// 4/4/95 JBUM
	if ((oe = NewGWorld(&gOffTools,8,&gToolsRect,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"NewRoomWindow");
		return;
	}
	gOffToolsMap = GetGWorldPixMap(gOffTools);
	LockPixels(gOffToolsMap);

	GetGWorld(&curPort,&curDevice);
	SetGWorld(gOffTools,NULL);
	PaintRect(&gToolsRect);
	{
		short		i;
		Rect		r;
		CIconHandle	h;
		for (i = 0; i < gIconSetup->nbrTools; ++i) {
			h = GetCIcon(i+gIconSetup->firstTool);
			if (h) {
				SetRect(&r,0,0,ToolWidth,ToolHeight);
				OffsetRect(&r,ToolWidth*i,0);
				PlotCIcon(&r, h);
				DisposCIcon(h);
			}
		}
	}

	SetGWorld(curPort,curDevice);

}

void InitToolIcon(short iconNbr)
{
	CIconHandle		h;
	Rect			r;
	CGrafPtr		curPort;
	GDHandle		curDevice;
	extern RGBColor	gBlackColor;
	GetGWorld(&curPort,&curDevice);
	SetGWorld(gOffTools,NULL);
	h = GetCIcon(iconNbr+gIconSetup->firstTool);
	if (h) {
		SetRect(&r,0,0,ToolWidth,ToolHeight);
		OffsetRect(&r,ToolWidth*iconNbr,0);
		RGBForeColor(&gToolBG);
		PaintRect(&r);
		RGBForeColor(&gBlackColor);
		PlotCIcon(&r, h);
		DisposCIcon(h);
	}
	SetGWorld(curPort,curDevice);
}

void DisposeTools()
{
}

Handle	NewToolList(WindowPtr win)
{
	Handle		h;
	ToolListPtr	tp;
	h = NewHandleClear(sizeof(ToolList));
	if (h == NULL)
		return NULL;
	tp = (ToolListPtr) *h;
	tp->theWin = win;
	tp->nbrTools = 0;
	((ObjectWindowPtr) win)->toolH = h;
	return h;
}

void	RefreshTools(WindowPtr theWin)
{
	short	i,n;
	ToolListPtr	tp;
	Handle	toolH = ((ObjectWindowPtr) theWin)->toolH;
	GrafPtr	savePort;
	if (toolH == NULL)
		return;
	GetPort(&savePort);
	SetPort(theWin);
	if (((WindowPeek) theWin)->refCon != ObjectWindowID) {
		return;
	}
	tp = (ToolListPtr) *toolH;
	n = tp->nbrTools;
	for (i = 0; i < n; ++i)
		DrawTool(i);
	SetPort(savePort);
}

void AddTool(WindowPtr theWin,short toolIcon,short toolHiIcon,short toolDimIcon,
			short h,short v)
{
	ToolListPtr tp;
	ToolRecPtr	tr;
	Handle		toolH = ((ObjectWindowPtr) theWin)->toolH;
	if (toolH == NULL) {
		NewToolList(theWin);
		toolH =  ((ObjectWindowPtr) theWin)->toolH;
		if (toolH == NULL)
			return;
	}
	tp = (ToolListPtr) *toolH;
	SetHandleSize(toolH,sizeof(ToolList)+sizeof(ToolRec)*(tp->nbrTools+1));
	tp = (ToolListPtr) *toolH;
	tr = &tp->toolList[tp->nbrTools];
	SetRect(&tr->toolRect,0,0,ToolWidth,ToolHeight);
	OffsetRect(&tr->toolRect,h,v);
	tr->active = true;
	tr->hilited = false;
	tr->toolIcon = toolIcon;
	tr->toolHiIcon = toolHiIcon;
	tr->toolDimIcon = toolDimIcon;
	++tp->nbrTools;
	InitToolIcon(toolIcon);
	if (toolHiIcon)
		InitToolIcon(toolHiIcon);
	if (toolDimIcon)
		InitToolIcon(toolDimIcon);
}

Boolean	PtInToolList(Point p,short *toolNbr)
{
	short		i,n;
	ToolListPtr	tp;
	ToolRecPtr	tr;
	Handle		toolH;
	WindowPtr	theWin;
	GetPort(&theWin);
	if (((WindowPeek) theWin)->refCon != ObjectWindowID) {
		return false;
	}
	toolH = ((ObjectWindowPtr) theWin)->toolH;
	if (toolH == NULL)
		return false;
	tp = (ToolListPtr) *toolH;
	n = tp->nbrTools;
	for (i = 0; i < n; ++i) {
		if (PtInTool(p,i)) {
			*toolNbr = i;
			tr = &tp->toolList[i];
			return tr->active;
		}
	}
	return false;
}

Boolean	PtInTool(Point p,short toolIdx)
{
	ToolListPtr tp;
	ToolRecPtr	tr;
	WindowPtr	theWin;
	Handle		toolH;
	GetPort(&theWin);
	if (((WindowPeek) theWin)->refCon != ObjectWindowID) {
		return false;
	}
	toolH = ((ObjectWindowPtr) theWin)->toolH;
	if (toolH == NULL)
		return false;
	tp = (ToolListPtr) *toolH;
	tr = &tp->toolList[toolIdx];
	return PtInRect(p,&tr->toolRect);
}

void	HiliteTool(short toolIdx, Boolean flag)
{
	ToolListPtr tp;
	ToolRecPtr	tr;
	WindowPtr	theWin;
	Handle		toolH;
	GetPort(&theWin);
	if (((WindowPeek) theWin)->refCon != ObjectWindowID) {
		return;
	}
	toolH = ((ObjectWindowPtr) theWin)->toolH;
	if (toolH == NULL)
		return;
	tp = (ToolListPtr) *toolH;
	tr = &tp->toolList[toolIdx];
	tr->hilited = flag;
	DrawTool(toolIdx);
}

void	ActivateTool(short toolIdx, Boolean flag, Boolean refresh)
{
	ToolListPtr tp;
	ToolRecPtr	tr;
	WindowPtr	theWin;
	Handle		toolH;
	GetPort(&theWin);
	if (((WindowPeek) theWin)->refCon != ObjectWindowID) {
		return;
	}
	toolH = ((ObjectWindowPtr) theWin)->toolH;
	if (toolH == NULL)
		return;
	tp = (ToolListPtr) *toolH;
	tr = &tp->toolList[toolIdx];
	tr->active = flag;
	if (refresh)
		DrawTool(toolIdx);
}


void	DrawTool(short toolIdx)
{
	Rect	sr,dr;
	short	icon;
	ToolListPtr tp;
	ToolRecPtr	tr;
	WindowPtr	theWin;
	Handle		toolH;
	RGBColor	fc,bc;
	extern RGBColor	gWhiteColor, gBlackColor;
	
	GetPort(&theWin);
	if (((WindowPeek) theWin)->refCon != ObjectWindowID) {
		return;
	}

	toolH = ((ObjectWindowPtr) theWin)->toolH;
	if (toolH == NULL)
		return;
	tp = (ToolListPtr) *toolH;
	tr = &tp->toolList[toolIdx];
	if (!tr->active && tr->toolDimIcon)
		icon = tr->toolDimIcon;
	else if (tr->hilited && tr->toolHiIcon)
		icon =  tr->toolHiIcon;
	else
		icon = tr->toolIcon;
	SetRect(&sr,0,0,ToolWidth,ToolHeight);
	OffsetRect(&sr,ToolWidth*icon,0);
	dr = tr->toolRect;

	GetForeColor(&fc);
	GetBackColor(&bc);
	RGBForeColor(&gBlackColor);
	RGBBackColor(&gWhiteColor);
	CopyBits((BitMap *) *gOffToolsMap,&theWin->portBits,
				&sr,&dr,srcCopy,NULL);
	RGBForeColor(&fc);
	RGBBackColor(&bc);
}

Boolean	ToolClick(Point p, short *whichTool)
{
	ToolListPtr	tp;
	ToolRecPtr	tr;
	short	toolIdx;
	Point	op,np;
	Boolean	inTool;

	WindowPtr	theWin;
	Handle		toolH;
	GetPort(&theWin);
	toolH = ((ObjectWindowPtr) theWin)->toolH;
	if (((WindowPeek) theWin)->refCon != ObjectWindowID) {
		return false;
	}

	if (toolH == NULL)
		return false;

	if (PtInToolList(p,&toolIdx)) {
		*whichTool = toolIdx;
		tp = (ToolListPtr) *toolH;
		tr = &tp->toolList[toolIdx];
		if (!tr->active)
			return false;
		if (tr->toolHiIcon == 0)	// no hilite icon, don't feedback
			return true;
		HiliteTool(toolIdx,true);
		inTool = true;		
		op = np = p;
		while (WaitMouseUp()) {
			GetMouse(&np);
			if (!EqualPt(op,np)) {
				op = np;
				if (inTool != PtInTool(np,toolIdx)) {
					inTool = !inTool;
					HiliteTool(toolIdx,inTool);
				}
			}
		}
		if (inTool)
			HiliteTool(toolIdx,false);
		return inTool;
	}
	return false;
}

void ChangeToolIcon(short toolIdx, short iconNbr)
{
	ToolListPtr tp;
	ToolRecPtr	tr;
	WindowPtr	theWin;
	Handle		toolH;
	GetPort(&theWin);
	if (((WindowPeek) theWin)->refCon != ObjectWindowID) {
		return;
	}
	toolH = ((ObjectWindowPtr) theWin)->toolH;
	if (toolH == NULL)
		return;
	tp = (ToolListPtr) *toolH;
	tr = &tp->toolList[toolIdx];
	tr->toolIcon = iconNbr;
}

void	MoveTool(WindowPtr theWin, short toolIdx, short h, short v)
{
	ToolListPtr tp;
	ToolRecPtr	tr;
	Handle	toolH;
	if (((WindowPeek) theWin)->refCon != ObjectWindowID) {
		return;
	}
	toolH = ((ObjectWindowPtr) theWin)->toolH;
	if (toolH == NULL)
		return;
	tp = (ToolListPtr) *toolH;
	tr = &tp->toolList[toolIdx];
	SetRect(&tr->toolRect,h,v,h+ToolWidth,v+ToolHeight);
}

void	GetToolRect(short toolIdx, Rect *r)
{
	ToolListPtr tp;
	ToolRecPtr	tr;
	Handle	toolH;
	WindowPtr theWin;
	SetRect(r,0,0,0,0);
	GetPort(&theWin);
	if (((WindowPeek) theWin)->refCon != ObjectWindowID) {
		return;
	}
	toolH = ((ObjectWindowPtr) theWin)->toolH;
	if (toolH == NULL)
		return;
	tp = (ToolListPtr) *toolH;
	tr = &tp->toolList[toolIdx];
	*r = tr->toolRect;
}

void SetToolBG(RGBColor *bg)
{
	gToolBG = *bg;
}