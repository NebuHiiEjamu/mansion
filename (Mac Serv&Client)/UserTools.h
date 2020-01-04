// UserTools.h
#include "ObjectWindow.h"
#include <QDOffscreen.h>

#ifndef ToolWidth
#define ToolWidth	32
#define ToolHeight	32
#endif

typedef struct {
	Rect	toolRect;
	short	toolIcon,toolHiIcon,toolDimIcon;
	Boolean	active,hilited;
} ToolRec,*ToolRecPtr;

typedef struct {
	WindowPtr	theWin;
	short		nbrTools;
	ToolRec		toolList[1];
} ToolList,*ToolListPtr;

void	InitTools(void);
void	DisposeTools(void);
Handle	NewToolList(WindowPtr theWin);
void	RefreshTools(WindowPtr theWin);
void	AddTool(WindowPtr theWin,short toolIcon,short toolHiIcon,short toolDimIcon,short h,short v);
Boolean	PtInToolList(Point p,short *toolNbr);
Boolean	PtInTool(Point p,short toolIdx);
Boolean	ToolClick(Point p,short *toolNbr);
void	HiliteTool(short toolIdx, Boolean flag);
void	ActivateTool(short toolIdx, Boolean flag, Boolean refresh);
void	DrawTool(short toolIdx);
void	ChangeToolIcon(short toolIdx, short iconNbr);
void	MoveTool(WindowPtr theWin, short toolIdx, short h, short v);
void	GetToolRect(short toolIdx, Rect *r);
void	SetToolBG(RGBColor *bg);
void 	InitToolIcon(short iconNbr);
