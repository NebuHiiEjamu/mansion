// RoomWindow.c

#include "U-USER.H"
#include "m-cmds.H"
#include "U-SCRIPT.H"
#include "UserTools.h"	// 6/10/95
#include "U-SECURE.H"
#include "U-TIMOUT.H"	// 1/14/97 JAB
#include <strings.h>
#include <Palettes.h>			// 4/4/95 JBUM Custom Palette

#define		RoomWIND	129

RoomWindowPtr	gRoomWin;
Boolean			g12InchMode;
Point			gCenterP;
Rect			gOffscreenRect,gOffWorldRect,gOffPictRect,
				gVideoRoomRect;
Ptr				gScreenMem;
long			gScreenRowBytes;
extern long		gTimeSlice;
Boolean 		gFullScreen = true;
Boolean			gIconized = false;
Boolean			gEnteringRoom = false;
Boolean			gShowNames = false, gShowFrames = false;
IconSetup		*gIconSetup;
CTabHandle		gCurCLUT;			// 4/4/95 JBUM Custom Palette

short			gWToolIcon[NbrWindowTools] = {FaceIcon, SatchelIcon, PPaletteIcon,TrashIcon};
void ActivateMsg(Boolean flag);


void PrepareSplash(void);
void RoomWindowDispose(WindowPtr theWin);

// 4/4/95 JBUM New Function to set window to custom palette
//
void SetWindowToCLUT(WindowPtr theWindow, short clutID)
{
	CTabHandle			ctab;
	PaletteHandle		gPal;
	int					GetPixelDepth();

	ctab = GetCTable(clutID);
	// Removing this (and setting ctSeed) seems to have helped the
	// crashing problem
	// (*ctab)->ctFlags |= 0x4000;		
			// JAB removed 8/10/96 - not sure what
			// it's for - may have contributed to crash
	(*ctab)->ctSeed = GetCTSeed();	// JAB 8/10/96

	// JAB 8/10/96
	// The Palette Manager may have been to cause NewGWorld
	// to Crash if depth > 256
	// if (GetPixelDepth() <= 8) {	
		gPal = NewPalette((*ctab)->ctSize,ctab,pmTolerant+pmExplicit,0);
		SetPalette(theWindow, gPal, true);
		SetPalette((WindowPtr) -1L, gPal, true);	// Default Palette for
	// }												// all windows
													// Inside Mac VI
	gCurCLUT = ctab;
}

// Create a new main window using a 'WIND' template from the resource fork
//
void NewRoomWindow(void)
{
	WindowPtr		theWindow;
	RoomWindowPtr	roomRec;
	// CTabHandle		cTab;		// 4/4/95 JBUM
	OSErr			oe;
	Handle			h;
	CGrafPtr		curPort;
	GDHandle		curDevice;
	short			i;

	// Choose between 512x384 and 640x480 modes
	//

	if (qd.screenBits.bounds.bottom - qd.screenBits.bounds.top < 480 ||
		qd.screenBits.bounds.right - qd.screenBits.bounds.left < 640) 
		g12InchMode = true;
	SetRect(&gOffscreenRect,0,0,RoomWidth,RoomHeight);
	gOffWorldRect = gOffscreenRect;
	InsetRect(&gOffWorldRect,-LeftMargin,-TopMargin);
/*	gVideoRoomRect = gOffscreenRect;*/
/*	OffsetRect(&gVideoRoomRect,(qd.screenBits.bounds.right-RoomWidth)/2,(qd.screenBits.bounds.bottom-RoomHeight)/2);*/

	// Calc the center point
	gCenterP.h = (gOffscreenRect.right - gOffscreenRect.left) / 2;
	gCenterP.v = (gOffscreenRect.bottom - gOffscreenRect.top) / 2;

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 
	roomRec = (RoomWindowPtr) NewPtrClear(sizeof(RoomWindowRec));
	theWindow = InitObjectWindow(RoomWIND, (ObjectWindowPtr) roomRec,false);
	SetWTitle(theWindow,"\pThe Palace");
	gRoomWin = (RoomWindowPtr) theWindow;

	((ObjectWindowPtr) theWindow)->Draw = RoomWindowDraw;
	((ObjectWindowPtr) theWindow)->Activate = RoomWindowActivate;
	((ObjectWindowPtr) theWindow)->Idle = RoomWindowIdle;
	((ObjectWindowPtr) theWindow)->HandleClick = RoomWindowClick;
	((ObjectWindowPtr) theWindow)->AdjustCursor = RoomWindowAdjustCursor;
	((ObjectWindowPtr) theWindow)->ProcessKey = RoomProcessKey;
	((ObjectWindowPtr) theWindow)->Dispose = RoomWindowDispose;
	


	if (gMacPrefs.partialScreen) {
		short	width,height;
		gFullScreen = 0;
		width = RoomWidth;
		height= RoomHeight+(g12InchMode? 0 : 45);
		SizeWindow(theWindow,width,height,false);
		RestoreWindowPos(theWindow, &gMacPrefs.roomPos);
	}
	else {
		gFullScreen = 1;
		MoveWindow(theWindow,qd.screenBits.bounds.left,qd.screenBits.bounds.top,false);
		SizeWindow(theWindow,qd.screenBits.bounds.right, qd.screenBits.bounds.bottom,false);
	}

	// Show the window
	ShowWindow(theWindow);
	
	SetWindowToCLUT(theWindow,999);	// 4/4/95 JBUM

	// Make it the current grafport
	SetPort(theWindow);

	InitTools();

	h = GetResource('icnr',128);
	MoveHHi(h);
	HLock(h);
	HNoPurge(h);
	gIconSetup = (IconSetup *) *h;

	InvalRect(&theWindow->portRect);	// 6/12/95 - window wasn't refreshing properly in 1000s

	TextFont(geneva);
	TextSize(12);	// 9

	ComputeMessageRects();
 	
	RGBBackColor(&gGray44);
	RGBForeColor(&gWhiteColor);
	gRoomWin->msgTEH = TEStylNew(&gRoomWin->msgRect,&gRoomWin->msgRect);
	TEAutoView(true,gRoomWin->msgTEH);
	RGBBackColor(&gWhiteColor);
	RGBForeColor(&gBlackColor);
	{
		TextStyle	ts;
		short		lineHeight,ascent;
		TEGetStyle(0,&ts,&lineHeight,&ascent,gRoomWin->msgTEH);
		ts.tsColor = gWhiteColor;
		TESetStyle(doColor,&ts,false,gRoomWin->msgTEH);
	}

	TextFont(monaco);
	TextSize(9);

	PaintRect(&theWindow->portRect);

	// Allocate an offscreen pixel map
	//
	// cTab = gCurCLUT;

	// 4/4/95 JBUM
	if ((oe = NewGWorld(&gRoomWin->offWorld,8,&gOffWorldRect,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"NewRoomWindow");
		return;
	}
	// 4/4/95 JBUM
	if ((oe = NewGWorld(&gRoomWin->offPicture,8,&gOffscreenRect,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"NewRoomWindow");
		return;
	}


	gRoomWin->offPixMap = GetGWorldPixMap(gRoomWin->offWorld);
	LockPixels(gRoomWin->offPixMap);

	gRoomWin->offPictureMap = GetGWorldPixMap(gRoomWin->offPicture);
	LockPixels(gRoomWin->offPictureMap);

	gScreenRowBytes = (*gRoomWin->offPixMap)->rowBytes & 0x3FFF;
	gScreenMem = GetPixBaseAddr(gRoomWin->offPixMap);

	SetToolBG(&gGray22);
	for (i = 0; i < NbrWindowTools; ++i)
		AddTool(theWindow, gWToolIcon[i], gWToolIcon[i]+4,0,gRoomWin->toolRect.left+PropWidth*i,gRoomWin->toolRect.top);
	ActivateTool(WT_Trash,false,false);

	GetGWorld(&curPort,&curDevice);
	SetGWorld(gRoomWin->offWorld,NULL);
	PaintRect(&gOffWorldRect);
	TextFont(gPrefs.fontID);
	TextSize(gPrefs.fontSize <= 0? 9 : gPrefs.fontSize);
	InitBalloons();
	gRoomWin->cTrans = (unsigned char*) NewPtrClear(256L*16L);
	gRoomWin->fTrans = (unsigned char*) NewPtrClear(256L*16L);
	gRoomWin->gTrans = (unsigned char*) NewPtrClear(256L*256L);
	InitRoomColors();
	SetGWorld(gRoomWin->offPicture,NULL);
	PaintRect(&gOffscreenRect);


	gRoomWin->faceHandles = (Handle *) NewPtrClear(gIconSetup->nbrFaces * sizeof(Handle));
	gRoomWin->lPropHandles = (Handle *) NewPtrClear(sizeof(Handle) * 128);

	for (i = 0; i < gIconSetup->nbrFaces; ++i) {
		gRoomWin->faceHandles[i] = GetAsset(RT_PROP,i+gIconSetup->firstFace);	// 6/7/95
		// 10/9/95
		DetachAsset(gRoomWin->faceHandles[i]);
	}
	LoadPropFavorites();

	SetGWorld(curPort,curDevice);

	PrepareSplash();

	SetConnectionType(C_None, true);
}

void RoomWindowDispose(WindowPtr theWin)
{
	SetPort(theWin);
	PaintRect(&theWin->portRect);
	gMacPrefs.partialScreen = !gFullScreen;
	SaveWindowPos(theWin, &gMacPrefs.roomPos);
	DefaultDispose(theWin);
}

void PrepareTextColors(void)
{
	TextStyle	ts;
	short		lineHeight,ascent;

	SetPort((WindowPtr) gRoomWin);
	TextFont(geneva);
	TextSize(12);
	// RGBForeColor(&gWhiteColor);
	if (gRoomWin->msgActive)
		RGBBackColor(&gGray44);
	else
		RGBBackColor(&gGray22);

	TEGetStyle((*gRoomWin->msgTEH)->selStart,&ts,&lineHeight,&ascent,gRoomWin->msgTEH);
	ts.tsColor = gWhiteColor;
	TESetStyle(doColor,&ts,false,gRoomWin->msgTEH);
}

void RestoreWindowColors(void)
{
	RGBForeColor(&gBlackColor);	
	RGBBackColor(&gWhiteColor);
}

char	gStatusMessage[64];
#define StatusMsgSTR	132

void StdStatusMessage(short strCode)
{
	Str255	theString;
	GetIndString(theString,StatusMsgSTR,strCode);
	if (theString[0]) {
		PtoCstr(theString);
		StatusMessage((char *) theString,0);
	}
}

void ErrStatusMessage(short strCode, short errCode)
{
	Str255	theString;
	GetIndString(theString,StatusMsgSTR,strCode);
	if (theString[0]) {
		PtoCstr(theString);
		StatusMessage((char *) theString,errCode);
	}
}

void StatusMessage(char *str, short errNbr)
{
	if (!gRoomWin->msgActive)
		ActivateMsg(true);

	SetLogColor(&gRedColor);
	if (str && *str) {
		if (errNbr) {
			LogMessage("%s [%d]\r",str,errNbr);
			sprintf(gStatusMessage,"%s [%d]",str,errNbr);
		}
		else {
			LogMessage("%s\r",str);
			strcpy(gStatusMessage,str);
		}
	}
	else if (errNbr) {
		Handle	h;
		if ((h = GetResource('STR ',errNbr)) != NULL) {
			BlockMove(*h+1,gStatusMessage,**h);
			gStatusMessage[**h] = 0;
			LogMessage("%s\r",gStatusMessage);		// 7/29/95
			ReleaseResource(h);
		}
		else
			gStatusMessage[0] = 0;
	}
	SetLogColor(&gBlackColor);
	DrawRoomStatus();
}

void ShowRoomStatus()
{
	gStatusMessage[0] = 0;
	DrawRoomStatus();
}

void DrawRoomStatus(void)
{
	char	tbuf[128];
	short	xo,yo;
	Rect	r;
	LocalUserRecPtr	tUser;

	GrafPtr	savePort;

	if (g12InchMode && !gRoomWin->msgActive)
		return;

	GetPort(&savePort);
	SetPort((WindowPtr) gRoomWin);

	RGBBackColor(&gWhiteColor);
	RGBForeColor(&gBlackColor);

	TextFont(geneva);
	TextSize(10);
	TextMode(srcBic);

	xo = gVideoRoomRect.left+4;
	yo = gVideoRoomRect.bottom+11;
	if (g12InchMode)
		yo -= 45;

	SetRect(&r,xo,yo-9,gVideoRoomRect.right,yo+3);	// Status Line
	ColorPaintRect(&r,&gGray22);

	MoveTo(xo,yo);
	DrawString((StringPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.roomNameOfst]);

	// MoveTo(xo+150,yo);
	if (gStatusMessage[0]) {
		strcpy(tbuf,gStatusMessage);
		// gStatusMessage[0] = 0;	
	}
	else if (gConnectionType == C_None)
		strcpy(tbuf,"no connection");
	else if (gRoomWin->targetID && (tUser = GetUser(gRoomWin->targetID)) != NULL) {
		sprintf(tbuf,"You are talking to Ò%sÓ, click here to cancel",CvtToCString(tUser->user.name));
	}
	else if (gRoomWin->targetID) {
		sprintf(tbuf,"You are talking to Ò%sÓ, click here to cancel",CvtToCString(gRoomWin->targetUserName));
	}
	else if (gConnectionType == C_AppleTalk || gConnectionType == C_PalaceTCP || gConnectionType == C_PalaceTelnet)
		sprintf(tbuf,"People: %d/%d",gRoomWin->curRoom.nbrPeople,gRoomWin->totalPeople);
	else
		sprintf(tbuf,"People: %d",gRoomWin->curRoom.nbrPeople);
	xo = gVideoRoomRect.left + (512 - TextWidth(tbuf,0,strlen(tbuf)))/2;
	MoveTo(xo,yo);
	DrawText(tbuf,0,strlen(tbuf));

/**
	if (gRoomWin->targetID && gConnectionType != C_None) {
		tUser = GetUser(gRoomWin->targetID);
		if (tUser) {
			MoveTo(xo+250,yo);
			DrawString("\pTalking to: ");
			DrawString(tUser->user.name);
		}
	}
**/

	if (gDebugFlag) {
		long	size=0L,size2;
		xo = gVideoRoomRect.left+4;
		MoveTo(xo+428,yo);
		size2 = MaxMem(&size);
		sprintf(tbuf,"Mem: %-7ld",FreeMem());
		DrawText(tbuf,0,strlen(tbuf));
	}
	else if (!gRecordCoords) {
		// 7/25/95 Server Name
		MoveTo(gVideoRoomRect.right-(4 + StringWidth(gRoomWin->serverInfo.serverName)), yo);
		DrawString(gRoomWin->serverInfo.serverName);
	}

	TextMode(srcOr);

	SetPort(savePort);

	RefreshTools((WindowPtr) gRoomWin);	// Tools overlap status line
}

void RecordCoordsIdle()
{
	short	xo,yo;
	char	tbuf[16];
	Point	p;
	Rect	r;
	GrafPtr	savePort;

	GetPort(&savePort);
	SetPort((WindowPtr) gRoomWin);

	TextMode(srcBic);
	xo = gVideoRoomRect.left+4;
	yo = gVideoRoomRect.bottom+11;

	SetRect(&r,xo+436,yo-9,gVideoRoomRect.right,yo+3);	// Status Line
	ColorPaintRect(&r,&gGray22);
	GetMouse(&p);
	p.h -= gVideoRoomRect.left - gOffscreenRect.left;
	p.v -= gVideoRoomRect.top - gOffscreenRect.top;
	sprintf(tbuf,"%d,%d ",p.h,p.v);
	MoveTo(xo+436,yo);
	DrawText(tbuf,0,strlen(tbuf));
	TextMode(srcOr);

	SetPort(savePort);
}

void ClearRoomWindow(void)
{
	extern WindowPtr	gRLWin, gULWin;

	if (gRLWin)
		((ObjectWindowPtr) gRLWin)->Dispose(gRLWin);
	if (gULWin)
		((ObjectWindowPtr) gULWin)->Dispose(gULWin);

	memset(&gRoomWin->curRoom,0,sizeof(RoomRec));
	gRoomWin->curHotspot = 0;
	gRoomWin->totalPeople = 0;
	gRoomWin->meID = 0;
	gRoomWin->targetID = 0;
	gRoomWin->nbrFadeIns = 0;
	gRoomWin->nbrFadeOuts = 0;
	gRoomWin->mePtr = NULL;
	ClearBalloons();
	RefreshSplash();
}

// Respond to an update event - BeginUpdate has already been called.
//
void RoomWindowDraw(WindowPtr theWindow)
{
	// Draw the room
	RgnHandle	clipRgn,tempRgn,newClipRgn;
	Rect		r;

	if (gIconized) {
		Rect	sr,dr;
		if (gRoomWin->mePtr) {
			SetRect(&sr,gRoomWin->mePtr->user.roomPos.h,gRoomWin->mePtr->user.roomPos.v,
						gRoomWin->mePtr->user.roomPos.h,gRoomWin->mePtr->user.roomPos.v);
			InsetRect(&sr,-32,-32);
		}
		else
			SetRect(&sr,0,0,64,64);
		SetRect(&dr,0,0,64,64);
		CopyBits((BitMap *) *gRoomWin->offPixMap, &theWindow->portBits, &sr, &dr, srcCopy, NULL);
	}
	else {
		clipRgn = NewRgn();
		tempRgn = NewRgn();
		newClipRgn = NewRgn();
		GetClip(tempRgn);			// 6/10/95 use current clip region, rather than theWindowPortRect...
		// RectRgn(tempRgn,&theWindow->portRect);
		RectRgn(newClipRgn,&gVideoRoomRect);
		DiffRgn(tempRgn,newClipRgn,newClipRgn);
		GetClip(clipRgn);
		SetClip(newClipRgn);
		ColorPaintRect(&theWindow->portRect,&gGray22);
		SetClip(clipRgn);
	
		RefreshRoom(&gOffscreenRect);		// 7/10/95 Make sure world state is current...

		CopyBits((BitMap *) *gRoomWin->offPixMap, &theWindow->portBits, &gOffscreenRect, &gVideoRoomRect, srcCopy, NULL);
	
		r = gVideoRoomRect;
		InsetRect(&r,-1,-1);

		DkHiliteRect(&r);

		if (!g12InchMode || gRoomWin->msgActive) {
			if (g12InchMode)
				ColorPaintRect(&gRoomWin->controlFrame12,&gGray22);
			DrawRoomStatus();
		}
		if (!g12InchMode || gRoomWin->msgActive) {
			DkRevHiliteRect(&gRoomWin->msgFrame);
	
			r = gRoomWin->msgFrame;
			InsetRect(&r,1,1);
			if (gRoomWin->msgActive) {
				ColorPaintRect(&r,&gGray44);
			}
			else if (g12InchMode)
				ColorPaintRect(&r,&gGray22);
	
			PrepareTextColors();
			TEUpdate(&gRoomWin->msgRect,gRoomWin->msgTEH);
			RestoreWindowColors();
		}
	
		DisposeRgn(clipRgn);
		DisposeRgn(tempRgn);
		DisposeRgn(newClipRgn);
	}
}

Boolean PrivateMsgTarget(Point p)
{
	short	i;
	LocalUserRecPtr	up;
	Rect	r;
	long	oldTarget = gRoomWin->targetID;

	up = gRoomWin->userList;
	for (i = 0; i < gRoomWin->curRoom.nbrPeople; ++i) {
		SetRect(&r,0,0,FaceWidth,FaceHeight);
		OffsetRect(&r,up->user.roomPos.h-FaceWidth/2,
						up->user.roomPos.v-FaceHeight/2);
		if (PtInRect(p,&r)) {
			if (up == gRoomWin->mePtr) {
				gRoomWin->targetID = 0;
				// Didn't click on a person, go to that spot
				MoveUser(p.h - gRoomWin->mePtr->user.roomPos.h,
						 p.v - gRoomWin->mePtr->user.roomPos.v);
			}
			// Toggle private chat
			else if (up->user.userID == gRoomWin->targetID)
				gRoomWin->targetID = 0;
			else {
				BlockMove(up->user.name, gRoomWin->targetUserName, up->user.name[0]+1);
				gRoomWin->targetID = up->user.userID;
			}
			if (gRoomWin->targetID != oldTarget)
				RefreshRoom(&gOffscreenRect);
			ShowRoomStatus();
			return true;
		}
		++up;
	}
	return false;
}

// 7/24/95 Modified to get frontmost hotspot
//
Boolean HotSpotTarget(Point p)
{
	short	i;
	HotspotPtr hsl = (HotspotPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];
	short	clickSpot = -1;
	for (i = 0; i < gRoomWin->curRoom.nbrHotspots; ++i,++hsl) {
		if (PtInHotspot(p,hsl)) {
			clickSpot = i;
		}
	}
	if (clickSpot != -1) {
		UserClickHotspot(p,clickSpot);
		return true;
	}
	else
		return false;
}

Boolean SelfPropClick(Point p, EventRecord *er)	// modified to highlight tools
{
	Rect	r,r2;
	Point	np,op;
	short	i;
	long	cmd[3];
	Boolean	rareFlag;
	Boolean	optionFlag = (er->modifiers & optionKey) > 0;
	short	toolNbr,activeTool = 0;
	for (i = gRoomWin->mePtr->user.nbrProps-1; i >= 0; --i) {	// 6/7/95
		ComputePropRect(i,gRoomWin->mePtr,&r);
		if (PtInRect(p,&r) && PtInProp(p,topLeft(r),&gRoomWin->mePtr->user.propSpec[i])) // 6/7/95
		{
			// Drag the Prop, drop it
			// 10/23 - add support for rare props - can't be copied
			rareFlag = IsRareProp(&gRoomWin->mePtr->user.propSpec[i]);
			if (rareFlag)
				optionFlag = 0;

			OffsetRect(&r,gVideoRoomRect.left,gVideoRoomRect.top);
			SetCursor(*gCursHandles[CloseHandCursor]);
			PenMode(srcXor);
			ActivateTool(WT_Face,false,false);
			ActivateTool(WT_Trash,true,false);
			ActivateTool(WT_PPalette,false,false);
			FrameRect(&r);	// Use Prop to Region, later...
			op = p;
			while (WaitMouseUp()) {
				GetMouse(&np);
				np.h -= gVideoRoomRect.left;
				np.v -= gVideoRoomRect.top;
				if (!EqualPt(np,op)) {
					FrameRect(&r);
					OffsetRect(&r,np.h-op.h,np.v-op.v);
					FrameRect(&r);
					op = np;
					// Hilite the Icon if you're in it (Satchel, Trash, Face)
					np.h += gVideoRoomRect.left;
					np.v += gVideoRoomRect.top;
					if (PtInToolList(np,&toolNbr)) {
						if (toolNbr != activeTool-1) {
							FrameRect(&r);	// off
							if (activeTool) {
								HiliteTool(activeTool-1,false);
							}
							activeTool = toolNbr+1;
							HiliteTool(activeTool-1,true);
							FrameRect(&r);	// on
						}
					}
					else {
						if (activeTool) {
							FrameRect(&r);	// off
							HiliteTool(activeTool-1,false);
							FrameRect(&r);	// on
						}
						activeTool = 0;
					}
				}
				
			}
			FrameRect(&r);
			if (activeTool)
				HiliteTool(activeTool-1,false);
			ActivateTool(WT_Face,true,false);
			ActivateTool(WT_PPalette,true,false);
			ActivateTool(WT_Trash,false,false);
			PenNormal();
			OffsetRect(&r,-gVideoRoomRect.left,-gVideoRoomRect.top);
			// Detach the Prop and Add it
			if (!EqualPt(np,p)) {
				ComputeUserRect(gRoomWin->mePtr,&r2);
				if (!PtInRect(np,&r2)) {
					if (activeTool) {
						switch (activeTool-1) {
						case WT_Satchel:
							AddPropFavorite(&gRoomWin->mePtr->user.propSpec[i]);	// 6/7/95
							if (!optionFlag)
								ToggleProp(&gRoomWin->mePtr->user.propSpec[i]);	// 6/7/95
							break;
						case WT_Trash:
							ToggleProp(&gRoomWin->mePtr->user.propSpec[i]);	// 6/7/95
							break;
						}
					}
					else if (PtInRect(np,&gOffscreenRect)) {
						cmd[0] = gRoomWin->mePtr->user.propSpec[i].id;	// 6/7/95
						cmd[1] = gRoomWin->mePtr->user.propSpec[i].crc;	// 6/7/95
						cmd[2] = *((long *) &topLeft(r));
						if (!optionFlag)
							ToggleProp(&gRoomWin->mePtr->user.propSpec[i]);	// 6/7/95
						if (rareFlag) {
							// 10/23/95 remove from picker
							DeletePropFavorite(&gRoomWin->mePtr->user.propSpec[i]);
						}
						DoPalaceCommand(PC_AddLooseProp, (long) &cmd[0], NULL);
					}
				}
			}
			return true;
		}
	}
	return false;
}

Boolean LoosePropClick(Point p, EventRecord *er)
{
	LPropPtr	lp;
	Rect		r,r2;
	short		propIdx;
	Point		np,op;
	long		cmd[2];
	short		toolNbr,activeTool=0;
	RoomRec		*rp = &gRoomWin->curRoom;
	Boolean		optionFlag = (er->modifiers & optionKey) > 0,rareFlag;

	if (rp->nbrLProps <= 0)
		return false;

	lp = (LPropPtr) &rp->varBuf[rp->firstLProp];
	lp = PtInLooseProp(lp,p,0,&propIdx);
	if (lp == NULL)
		return false;

	// 10/23/95 no copies of rare
	rareFlag = IsRareProp(&lp->propSpec);
	if (rareFlag)
		optionFlag = 0;

	if (lp->propSpec.id < MinReservedProp || lp->propSpec.id > MaxReservedProp) {
		if (MembersOnly(true))
			return true;
		if (DeniedByServer(PM_AllowCustomProps))
			return true;
	}

	SetRect(&r,0,0,PropWidth,PropHeight);
	OffsetRect(&r,lp->loc.h,lp->loc.v);
	OffsetRect(&r,gVideoRoomRect.left,gVideoRoomRect.top);
	SetCursor(*gCursHandles[CloseHandCursor]);
	PenMode(srcXor);
	ActivateTool(WT_PPalette,false,false);	// deactivate palette
	ActivateTool(WT_Trash,true,false);
	FrameRect(&r);	// Use Prop to Region, later...
	np = op = p;
	while (WaitMouseUp()) {
		GetMouse(&np);
		np.h -= gVideoRoomRect.left;
		np.v -= gVideoRoomRect.top;
		if (!EqualPt(np,op)) {
			FrameRect(&r);	// off
			OffsetRect(&r,np.h-op.h,np.v-op.v);
			FrameRect(&r);	// on
			op = np;
			// Hilite the Icon if you're in it (Satchel, Trash, Face)
			np.h += gVideoRoomRect.left;
			np.v += gVideoRoomRect.top;
			if (PtInToolList(np,&toolNbr)) {
				if (toolNbr != activeTool-1) {
					FrameRect(&r);	// off
					if (activeTool) {
						HiliteTool(activeTool-1,false);
					}
					activeTool = toolNbr+1;
					HiliteTool(activeTool-1,true);
					FrameRect(&r);	// on
				}
			}
			else {
				if (activeTool) {
					FrameRect(&r);	// off
					HiliteTool(activeTool-1,false);
					FrameRect(&r);	// on
				}
				activeTool = 0;
			}
		}
	}
	FrameRect(&r);	// off
	if (activeTool)
		HiliteTool(activeTool-1,false);
	ActivateTool(WT_PPalette,true,false);	// deactivate palette
	ActivateTool(WT_Trash,false,false);
	PenNormal();
	OffsetRect(&r,-gVideoRoomRect.left,-gVideoRoomRect.top);
	// Drag the Prop, move it, or pick it up
	if (!EqualPt(np,p)) {
		ComputeUserRect(gRoomWin->mePtr,&r2);
		if (PtInRect(np,&r2)) {
			cmd[0] = propIdx;
			if (!PropInUse(lp->propSpec.id,lp->propSpec.crc)) {
				DonProp(lp->propSpec.id,lp->propSpec.crc);		// 6/7/95
				AddPropFavorite(&lp->propSpec);	// 6/7/95
			}
			if (!optionFlag)
				DoPalaceCommand(PC_DelLooseProp, (long) &cmd[0], NULL);
		}
		else if (activeTool) {
			switch (activeTool-1) {
			case WT_Satchel:
				AddPropFavorite(&lp->propSpec);	// 6/7/95
				cmd[0] = propIdx;
				if (!optionFlag)
					DoPalaceCommand(PC_DelLooseProp, (long) &cmd[0], NULL);
				break;
			case WT_Trash:
				cmd[0] = propIdx;
				if (!optionFlag)
					DoPalaceCommand(PC_DelLooseProp, (long) &cmd[0], NULL);
				break;
			case WT_Face:


				if (!PropInUse(lp->propSpec.id,lp->propSpec.crc)) {
					DonProp(lp->propSpec.id,lp->propSpec.crc);		// 6/7/95
					AddPropFavorite(&lp->propSpec);	// 6/7/95
				}
				AddPropFavorite(&lp->propSpec);
				cmd[0] = propIdx;
				if (!optionFlag)
					DoPalaceCommand(PC_DelLooseProp, (long) &cmd[0], NULL);
				break;
			}
		}
		else if (PtInRect(np,&gOffscreenRect)) {
			if (optionFlag) {
				cmd[0] = lp->propSpec.id;				// 6/7/95
				cmd[1] = lp->propSpec.crc;				// 6/7/95
				cmd[2] = *((long *) &topLeft(r));		// 6/7/95
				DoPalaceCommand(PC_AddLooseProp, (long) &cmd[0], NULL);
			}
			else {
				cmd[0] = propIdx;
				cmd[1] = *((long *) &topLeft(r));
				DoPalaceCommand(PC_MoveLooseProp, (long) &cmd[0], NULL);
			}
		}
	}
	return true;
}

// Respond to a mouse-click - highlight cells until the user releases the button
//
void RoomWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent)
{
	Point		p;
	short		toolNbr;

	Boolean DragDoors(Point p);

	SetPort(theWin);

	if (gIconized) {
		DeIconize();
		return;
	}

	p = where;
	GlobalToLocal(&p);

	// 6/8/95 if DrawInRoom returns false, process click normally (allows movement while drawing)
	if (gRoomWin->navInProgress)
		return;
	else if (gDrawWin && !(theEvent->modifiers & cmdKey) &&
		PtInRect(p,&gVideoRoomRect) && DrawInRoom(p,theEvent))
		return;
	else if (gMode == M_Authoring)
		ClickAuthoring(p,(theEvent->modifiers & shiftKey)? true: false);
	else {
		if (PtInRect(p,&gRoomWin->msgFrame)) {
			if (!gRoomWin->msgActive) {
				ActivateMsg(true);
			}
			else {
				PrepareTextColors();
				TEClick(p,(theEvent->modifiers & shiftKey)? true: false,gRoomWin->msgTEH);
				RestoreWindowColors();
			}
		}
		else if ((!g12InchMode || gRoomWin->msgActive) && ToolClick(p,&toolNbr)) {
			switch (toolNbr) {
			case WT_Face:
				ToggleFacePicker();
				break;
			case WT_PPalette:
				ToggleDrawPalette();
				break;
			case WT_Satchel:
				TogglePropPicker();
				break;
			case WT_Trash:
				break;
			}
		}
		else {
			if (gRoomWin->msgActive) {
				ActivateMsg(false);
			}
	
			// 3/7/95 JBUM added && gRoomWin->mePtr to prevent handling of
			// mouse clicks during room -> room transitions

			if (PtInRect(p,&gVideoRoomRect) && gRoomWin->mePtr) {
				p.h -= gVideoRoomRect.left - gOffscreenRect.left;
				p.v -= gVideoRoomRect.top - gOffscreenRect.top;
	
				if (gDragDoors && DragDoors(p)) {
					return;
				}
				else {
					if (SelfPropClick(p,theEvent))	// Check if Click on Self Prop
						return;
					if (LoosePropClick(p,theEvent))	// Check if Click on Loose Prop
						return;
					if (PrivateMsgTarget(p))		// Target Users for Private Messages
						return;
					if (HotSpotTarget(p))
						return;
					// Didn't click on a person, go to that spot
					MoveUser(p.h - gRoomWin->mePtr->user.roomPos.h,
							 p.v - gRoomWin->mePtr->user.roomPos.v);
				}
			}
			else {

				if (gRoomWin->targetID != 0) {
					gRoomWin->targetID = 0;
					RefreshRoom(&gOffscreenRect);
					ShowRoomStatus();
				}
			}
		}
	}
}

//
// 5/10/95 - Don't process movements and colorchanges till you get a key up.
//


void ActivateMsg(Boolean flag)
{
	GrafPtr	savePort;
	GetPort(&savePort);
	SetPort((WindowPtr) gRoomWin);
	gRoomWin->msgActive = flag;
	if (gRoomWin->msgActive) {
		TEActivate(gRoomWin->msgTEH);
	}
	else{
		TEDeactivate(gRoomWin->msgTEH);
	}
	if (g12InchMode)
		InvalRect(&gRoomWin->controlFrame12);
	else
		InvalRect(&gRoomWin->msgFrame);
	SetPort(savePort);
}

void RoomProcessKey(WindowPtr theWin, EventRecord *theEvent)
{
	char	c,code;
	static char reservedKeys[] = {0x7b,0x7c,0x7d,0x7e,0x52,0x53,
								  0x54,0x55,0x56,0x57,0x58,0x59,0x5b,0x5c,
								  0x4e,0x47,0x51,0x4b,0x45,0x43,0};
	static char arrowKeys[] = {0x7b,0x7c,0x7d,0x7e,0};

	SetPort(theWin);

	c = theEvent->message & charCodeMask;
	code = (theEvent->message & keyCodeMask) >> 8;

	if (gPEWin && PEProcessKey(theEvent))
		return;
	else if (gMode == M_Authoring && KeyAuthoring(code))
		return;
	else if (c == '\033') {
		if (gRoomWin->msgActive) {	// 5/25/95
			PrepareTextColors();
			TESetText("",0L,gRoomWin->msgTEH);
			TESetSelect(0,0,gRoomWin->msgTEH);
			RestoreWindowColors();
			ColorPaintRect(&gRoomWin->msgRect,&gGray44);
		}
	}
	else if (c == '\t') {
		ActivateMsg(!gRoomWin->msgActive);
	}
	else if (code == 0 || strchr(reservedKeys,code) == NULL ||
			(strchr(arrowKeys,code) != NULL && gRoomWin->msgActive)) {
		if (!gRoomWin->msgActive) {
			ActivateMsg(true);
		}
		if (c == '\r') {
			Handle	ht;
			if ((*gRoomWin->msgTEH)->teLength && !gRoomWin->navInProgress) {
				char	hState;
				PushHistoryText(gRoomWin->msgTEH);		// 7/10/05
				ht = TEGetText(gRoomWin->msgTEH);
				hState = HGetState(ht);
				HLock(ht);
				ProcessUserString(*ht,(*gRoomWin->msgTEH)->teLength);
				HSetState(ht,hState);
				PrepareTextColors();	// 7/13/95
				TESetText("",0L,gRoomWin->msgTEH);
				TESetSelect(0,0,gRoomWin->msgTEH);
				RestoreWindowColors();	// 7/13/95
				ColorPaintRect(&gRoomWin->msgRect,&gGray44);
			}
		}
		else {
			// Text Editing...
			switch (code) {
			case 0x7d:		// Down Arrow
				HistoryForward();
				break;
			case 0x7e:		// Up Arrow
				HistoryBackward();
				break;
			case 0x73:		// Home
					PrepareTextColors();
					TESetSelect(0,0,gRoomWin->msgTEH);
					RestoreWindowColors();
					break;
			case 0x77:		// End
					PrepareTextColors();
					TESetSelect(32767,32767,gRoomWin->msgTEH);
					RestoreWindowColors();
					break;
			case 0x75:		// KP Del
				if ((*gRoomWin->msgTEH)->selEnd > (*gRoomWin->msgTEH)->selStart) {
					PrepareTextColors();
					TEKey('\b',gRoomWin->msgTEH);
					RestoreWindowColors();
				}
				else {
					PrepareTextColors();
					TESetSelect((*gRoomWin->msgTEH)->selStart,(*gRoomWin->msgTEH)->selStart+1,gRoomWin->msgTEH);
					if ((*gRoomWin->msgTEH)->selEnd > (*gRoomWin->msgTEH)->selStart)
						TEKey('\b',gRoomWin->msgTEH);
					RestoreWindowColors();
				}
				break;
			default:
				{
					char	c = theEvent->message & charCodeMask;
					// 11/28/95
					if ((*gRoomWin->msgTEH)->teLength >= 248 && c != '\b')
						SysBeep(1);
					else {
						PrepareTextColors();
						TEKey(c,gRoomWin->msgTEH);
						RestoreWindowColors();
					}
				}
			}
		}
	}
	else {
		if (gRoomWin->navInProgress)		// 9/12/95
			return;

		if (theEvent->what == autoKey)		// 11/7/95
			return;

		switch (code) {
		case 0x7b:	// left
			MoveUser(-4,0);
			break;
		case 0x7c:	// right
			MoveUser(4,0);
			break;
		case 0x7d:	// down
			MoveUser(0,4);
			break;
		case 0x7e:	// up
			MoveUser(0,-4);
			break;
		case 0x52:	// kp0
		case 0x06:	// Z
			ChangeFace(0);
			break;
		case 0x53:	// kp1
		case 0x07:	// X
			ChangeFace(1);
			break;
		case 0x54:	// kp2
		case 0x08:	// C
			ChangeFace(2);
			break;
		case 0x55:	// kp3
		case 0x09:	// V
			ChangeFace(3);
			break;
		case 0x56:	// kp4
		case 0x01:	// S
			ChangeFace(4);
			break;
		case 0x57:	// kp5
		case 0x02:	// D
			ChangeFace(5);
			break;
		case 0x58:	// kp6
		case 0x03:	// F
			ChangeFace(6);
			break;
		case 0x59:	// kp7
		case 0x0D:	// W
			ChangeFace(7);
			break;
		case 0x5B:	// kp8
		case 0x0E:	// E
			ChangeFace(8);
			break;
		case 0x5C:	// kp9
		case 0x0F:	// R
			ChangeFace(9);
			break;
		case 0x47:	// kp clear
		case 0x13:	// 2
			ChangeFace(10);
			break;
		case 0x51:	// kp =
		case 0x14:	// 3
			ChangeFace(11);
			break;
		case 0x4b:	// kp /
		case 0x15:	// 4
			ChangeFace(12);
			break;
		case 0x43: // kp *
		case 0x1c: // 8/*
			TogglePropPicker();
			break;
		case 0x41:	// KP .
		case 0x2C:	// .
			ToggleDrawPalette();
			break;
		case 0x4e:	// kp -
		case 0x1B:	// -
			// Switch Colors
			if (!MembersOnly(true))
			{
				short	color;
				color = gRoomWin->mePtr->user.colorNbr - 1;
				if (color < 0)
					color = NbrColors - 1;
				ChangeColor(color);
				// gAutoFlag = true;
			}
			break;
		case 0x45:	// kp +
		case 0x18:	// =
			// Switch Colors
			if (!MembersOnly(true))
			{
				short	color;
				color = gRoomWin->mePtr->user.colorNbr + 1;
				if (color >= NbrColors)
					color = 0;
				ChangeColor(color);
				// gAutoFlag = true;
			}
			break;
		}
	}
}

void RoomWindowIdle(WindowPtr theWindow, EventRecord *theEvent)
{
	short			i;
	LocalUserRecPtr	up;
	void			SplashIdle(void);

	if (gRoomWin->msgActive)
		TEIdle(gRoomWin->msgTEH);

	BalloonIdle();

	if (gRecordCoords)
		RecordCoordsIdle();

	// 4/24/96 JAB Joined Fade In and Fade Out code
	if (gRoomWin->nbrFadeIns || gRoomWin->nbrFadeOuts) {
		up = gRoomWin->userList;
		for (i = 0; i < gRoomWin->curRoom.nbrPeople; ++i,++up) {
			if (up->localFlags & UF_FadeIn) {
#if __MC68K__
				up->userFade += 1;
#else
				up->userFade += 1;
#endif
				if (up->userFade > 15) {	// 4/24/96 changed from 16 (bug fix??)
					up->userFade = 0;
					up->localFlags &= ~UF_FadeIn;
					--gRoomWin->nbrFadeIns;
					if (gRoomWin->nbrFadeIns == 0L && gRoomWin->nbrFadeOuts == 0L &&
						gRoomWin->hasFaceAnimations == false)
						SetIdleTimer(TIMER_SLOW);
				}
				RefreshUser(up);
			}
			if (up->localFlags & UF_FadeOut) {
#if __MC68K__
				up->userFade -= 1;
#else
				up->userFade -= 1;
#endif
				if (up->userFade <= 0) {
					up->userFade = 0;
					up->localFlags &= ~UF_FadeOut;
					--gRoomWin->nbrFadeOuts;
					if (gRoomWin->nbrFadeIns == 0L && gRoomWin->nbrFadeOuts == 0L &&
						gRoomWin->hasFaceAnimations == false)
						SetIdleTimer(TIMER_SLOW);
					ExitPerson(up->user.userID);
					return;	// return, since user list is screwed up now
				}
				RefreshUser(up);
			}
		}
	}

	// 4/24/96 JAB Added code for FaceAnimations
	if (gRoomWin->hasFaceAnimations) {
		Boolean	hasAnimations = false;
		unsigned LONG	t;
		up = gRoomWin->userList;
		t = GetTicks();

		for (i = 0; i < gRoomWin->curRoom.nbrPeople; ++i,++up) {
			if (up->localFlags & UF_IsAnimating) {
				hasAnimations = true;
				if (t - up->lastAnimate > TICK_SECONDS / 3) {
					up->lastAnimate = t;
					if (up->localFlags & UF_IsPalindrome)  {
						if (!up->palindromeDirection) {
							++up->propFrameNumber;
							if (up->propFrameNumber >= up->nbrAnimateProps || up->propFrameNumber < 0) {
								up->propFrameNumber = up->nbrAnimateProps-2;
								if (up->propFrameNumber < 0)
									up->propFrameNumber = 0;
								up->palindromeDirection = true;
							}
						}
						else {
							--up->propFrameNumber;
							if (up->propFrameNumber >= up->nbrAnimateProps || up->propFrameNumber < 0) {
								up->propFrameNumber = 1;
								if (up->propFrameNumber >= up->nbrAnimateProps)
									up->propFrameNumber = up->nbrAnimateProps-1;
								up->palindromeDirection = false;
							}
						}
					}
					else {
						++up->propFrameNumber;
						if (up->propFrameNumber >= up->nbrAnimateProps)
							up->propFrameNumber = 0;
					}
					RefreshUser(up);
				}
			}
		}
		if (!hasAnimations)
			gRoomWin->hasFaceAnimations = false;
	}

	// 1/2/96 - cmd-opt acts as a toggle, reversing the state of AutoShowNames
	if (((gPrefs.userPrefsFlags & UPF_AutoShowNames) > 0) ^
		((theEvent->modifiers & cmdKey) && (theEvent->modifiers & optionKey))) {
		if (!gShowNames) {
			gShowNames = true;
			RefreshRoom(&gOffscreenRect);
		}
	}
	else {
		if (gShowNames) {
			gShowNames = false;
			RefreshRoom(&gOffscreenRect);
		}
	}

	if (!gSuspended && (theEvent->modifiers & controlKey) && gPEWin == NULL) {
		if (!gShowFrames) {
			gShowFrames = true;
			RefreshRoom(&gOffscreenRect);
		}
	}
	else {
		if (gShowFrames) {
			gShowFrames = false;
			RefreshRoom(&gOffscreenRect);
		}
	}
	if (gConnectionType == C_None)
		SplashIdle();

	// 1/14/97 JAB Timeout Idle
	UsageIdle();
}

Boolean PointInDoor(Point p);
Boolean PointInDoor(Point p)
{
	short	i;
	HotspotPtr hsl = (HotspotPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];

	for (i = 0; i < gRoomWin->curRoom.nbrHotspots; ++i,++hsl) {
		if (PtInHotspot(p,hsl)) {
			return hsl->type != HS_Normal;
		}
	}
	return false;
}

Boolean PointInSelfProp(Point p);
Boolean PointInSelfProp(Point p)
{
	short	i;
	Rect	r;
	if (gRoomWin->mePtr == NULL)
		return false;
	for (i = gRoomWin->mePtr->user.nbrProps-1; i >= 0; --i) {	// 6/7/95
		ComputePropRect(i,gRoomWin->mePtr,&r);
		if (PtInRect(p,&r) && PtInProp(p,topLeft(r),&gRoomWin->mePtr->user.propSpec[i])) // 6/7/95
		{
			return true;
		}
	}
	return false;
}

Boolean PointInLooseProp(Point p);
Boolean PointInLooseProp(Point p)
{
	RoomRec		*rp = &gRoomWin->curRoom;
	LPropPtr	lp;
	short		propIdx;

	if (gRoomWin->mePtr == NULL)
		return false;

	if (rp->nbrLProps <= 0)
		return false;
	lp = (LPropPtr) &rp->varBuf[rp->firstLProp];
	lp = PtInLooseProp(lp,p,0,&propIdx);
	if (lp == NULL)
		return false;
	else if (lp->propSpec.id >= MinReservedProp && lp->propSpec.id <= MaxReservedProp)
		return true;
	else if (!(gRoomWin->userFlags & U_Guest))
		return true;
	else
		return false;
}

Boolean PointInPrivateTarget(Point p);
Boolean PointInPrivateTarget(Point p)
{
	short	i;
	LocalUserRecPtr	up;
	Rect	r;
	up = gRoomWin->userList;
	if (up == NULL || gRoomWin->mePtr == NULL)
		return false;
	for (i = 0; i < gRoomWin->curRoom.nbrPeople; ++i,++up) {
		SetRect(&r,0,0,FaceWidth,FaceHeight);
		OffsetRect(&r,up->user.roomPos.h-FaceWidth/2,
						up->user.roomPos.v-FaceHeight/2);
		if (PtInRect(p,&r) && up != gRoomWin->mePtr) {
			return true;
		}
	}
	return false;
}


void GetPalaceMouse(Point *p);
void GetPalaceMouse(Point *p)
{
	GrafPtr	savePort;
	GetPort(&savePort);
	SetPort((WindowPtr) gRoomWin);
	GetMouse(p);
	p->h -= gVideoRoomRect.left - gOffscreenRect.left;
	p->v -= gVideoRoomRect.top - gOffscreenRect.top;
	SetPort(savePort);
}

void RoomWindowAdjustCursor(WindowPtr theWin, Point p, EventRecord *er)
{
	Boolean	canPaint = ((!(gRoomWin->curRoom.roomFlags & RF_NoPainting) &&  \
						(gRoomWin->serverInfo.serverPermissions & PM_AllowPainting) > 0) && \
						!(gRoomWin->userFlags & U_Guest)) || \
						(gRoomWin->userFlags & U_SuperUser);

	if (gRoomWin->navInProgress)
		SpinCursor();
	else if (gDrawWin && PtInRect(p,&gVideoRoomRect)) {
		if (er->modifiers & cmdKey)
			goto StdCursorAdjustment;
		else if (er->modifiers & optionKey)
			SetCursor(*gCursHandles[EyedropperCursor]);
		else if (canPaint)
			SetCursor(*gCursHandles[PencilCursor]);
		else
			goto StdCursorAdjustment;
	}
	else if (gRoomWin->msgActive && PtInRect(p,&gRoomWin->msgRect))
		SetCursor(*gCursHandles[IBeamCursor]);
	else
		goto StdCursorAdjustment;
	return;
StdCursorAdjustment:

	p.h -= gVideoRoomRect.left - gOffscreenRect.left;
	p.v -= gVideoRoomRect.top - gOffscreenRect.top;


	if (PointInSelfProp(p))
		SetCursor(*gCursHandles[OpenHandCursor]);
	else if (PointInLooseProp(p))
		SetCursor(*gCursHandles[OpenHandCursor]);
	else if (PointInPrivateTarget(p))
		SetCursor(*gCursHandles[HandCursor]);
	else if (PointInDoor(p))
		SetCursor(*gCursHandles[HandCursor]);
	else
		SetCursor(&qd.arrow);
}

void FullScreen(void)
{
	WindowPtr	theWindow = (WindowPtr) gRoomWin;
	SetPort(theWindow);
	PaintRect(&theWindow->portRect);	// avoid ugliness
	gFullScreen = true;
	MoveWindow(theWindow,qd.screenBits.bounds.left,qd.screenBits.bounds.top,false);
	SizeWindow(theWindow,qd.screenBits.bounds.right,qd.screenBits.bounds.bottom,false);

	ComputeMessageRects();
	(*gRoomWin->msgTEH)->destRect = gRoomWin->msgRect;
	(*gRoomWin->msgTEH)->viewRect = gRoomWin->msgRect;
	InvalRect(&theWindow->portRect);
#if M5SUPPORT
	{
		void M5MovieResize();
		M5MovieResize();
	}
#endif
}

void PartialScreen(Boolean fromFull)
{
	WindowPtr	theWindow = (WindowPtr) gRoomWin;
	short		width,height;

	SetPort(theWindow);
	PaintRect(&theWindow->portRect);	// avoid ugliness
	gFullScreen = false;
	width = RoomWidth;
	height= RoomHeight+(g12InchMode? 0 : 45);
	SizeWindow(theWindow,width,height,false);
	if (gConnectionType != C_None)
		SetWTitle((WindowPtr) gRoomWin,gRoomWin->serverInfo.serverName);
	else
		SetWTitle(theWindow,"\pThe Palace");
	if (fromFull) {
		MoveWindow(theWindow,(qd.screenBits.bounds.right-width)/2,
							  GetMBarHeight()+20 + (qd.screenBits.bounds.bottom-height-GetMBarHeight()-20)/2,
							  false);
	}
	ComputeMessageRects();

	(*gRoomWin->msgTEH)->destRect = gRoomWin->msgRect;
	(*gRoomWin->msgTEH)->viewRect = gRoomWin->msgRect;
	InvalRect(&theWindow->portRect);
#if M5SUPPORT
	{
		void M5MovieResize();
		M5MovieResize();
	}
#endif
}

void Iconize()
{
	WindowPtr	theWindow = (WindowPtr) gRoomWin;
	short		width,height;
	Point		theTop;
	SetPort(theWindow);
	PaintRect(&theWindow->portRect);

	ChangeFace(0);
	theTop.h = theTop.v = 0;
	LocalToGlobal(&theTop);
	width = 64;
	height= 64;
	SizeWindow(theWindow,width,height,false);
	if (theTop.v > 40)
		MoveWindow(theWindow,theTop.h,theTop.v,false);
	else
		MoveWindow(theWindow,40,40,false);
	TEDeactivate(gRoomWin->msgTEH);
	// InvalRect(&theWindow->portRect);
	SetWTitle(theWindow,"\p");
	gIconized = true;
	if (gLogWin)
		HideWindow((WindowPtr) gLogWin);
	if (gDrawWin)
		HideWindow(gDrawWin);
	if (gPropWin)
		HideWindow(gPropWin);
	if (gPEWin)
		HideWindow(gPEWin);
	if (gULWin)
		HideWindow(gPEWin);
	if (gRLWin)
		HideWindow(gPEWin);
	if (gFPWin)
		HideWindow(gFPWin);
/*	if (gIRCWindow)*/
/*		HideWindow(gIRCWindow);*/
/*	if (gTCPWindow)*/
/*		HideWindow(gTCPWindow);*/
	RefreshRoom(&gOffscreenRect);
}

void DeIconize()
{
	WindowPtr	theWin = (WindowPtr) gRoomWin;
	
	SetPort(theWin);
	PaintRect(&theWin->portRect);

	ChangeFace(5);
	if (gFullScreen)
		FullScreen();
	else
		PartialScreen(false);
	gIconized = false;
	if (gLogWin && ((ObjectWindowPtr) gLogWin)->floating)
		ShowWindow((WindowPtr) gLogWin);
	if (gDrawWin) {
		ShowWindow(gDrawWin);
		SelectFloating(gDrawWin);
	}
	if (gPropWin) {
		ShowWindow(gPropWin);
		SelectFloating(gPropWin);
	}
	if (gPEWin) {
		ShowWindow(gPEWin);
		SelectFloating(gPEWin);
	}
	if (gRLWin) {
		ShowWindow(gRLWin);
		SelectFloating(gRLWin);
	}
	if (gULWin) {
		ShowWindow(gULWin);
		SelectFloating(gULWin);
	}
	if (gFPWin) {
		ShowWindow(gFPWin);
		SelectFloating(gFPWin);
	}
/*	if (gIRCWindow)*/
/*		ShowWindow(gIRCWindow);*/
/*	if (gTCPWindow)*/
/*		ShowWindow(gTCPWindow);*/
}

void RoomWindowActivate(WindowPtr theWindow, Boolean activeFlag)
{
	GrafPtr	savePort;
	GetPort(&savePort);
	SetPort(theWindow);

	if (activeFlag) {
		// SetEventMask(everyEvent);  removed 10/18
		if (gRoomWin->msgActive)
			ActivateMsg(true);
		SetIdleTimer(TIMER_SLOW);
	}
	else {
		if (gSuspended || FrontDocument() != theWindow) {
			ActivateMsg(false);
		}
		SetIdleTimer(TIMER_SLEEP);
	}
	DefaultActivate(theWindow, activeFlag);
	SetPort(savePort);
}

void ComputeMessageRects()
{
	short	i;
	gVideoRoomRect = gOffscreenRect;

	if (gFullScreen)
		OffsetRect(&gVideoRoomRect,(qd.screenBits.bounds.right-RoomWidth)/2,(qd.screenBits.bounds.bottom-RoomHeight)/2-8);

	SetRect(&gRoomWin->msgRect,gVideoRoomRect.left+2,gVideoRoomRect.bottom+17,
				gVideoRoomRect.right-(8+PropWidth*NbrWindowTools),gVideoRoomRect.bottom+17+26);
	SetRect(&gRoomWin->toolRect,gVideoRoomRect.right-(PropWidth*NbrWindowTools),gVideoRoomRect.bottom+16,
			gVideoRoomRect.right,gVideoRoomRect.bottom+16+32);
	OffsetRect(&gRoomWin->toolRect,6,-1);

	if (g12InchMode) {
		OffsetRect(&gRoomWin->msgRect,0,-45);
		OffsetRect(&gRoomWin->toolRect,0,-49);
	}
	else if (!gFullScreen) {
		OffsetRect(&gRoomWin->toolRect,0,-4);	//  6/13/95
	}
	gRoomWin->msgFrame = gRoomWin->msgRect;
	InsetRect(&gRoomWin->msgFrame,-2,-2);

	SetRect(&gRoomWin->controlFrame12,gVideoRoomRect.left,gVideoRoomRect.bottom-45,
									gVideoRoomRect.right,gVideoRoomRect.bottom);

	for (i = 0; i < NbrWindowTools; ++i)
		MoveTool((WindowPtr) gRoomWin, i, gRoomWin->toolRect.left+PropWidth*i,gRoomWin->toolRect.top);
}

