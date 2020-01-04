// HotRegions.c

#include "U-USER.H"
#include "m-cmds.H"
#include "U-SNDS.H"
#include <string.h>

void CollapseHotspot(HotspotPtr hs)
{
	if ((RgnHandle) hs->refCon) {
		DisposeRgn((RgnHandle) hs->refCon);
		hs->refCon = 0L;
	}
}

void CollapsePicture(PictureRecPtr ps)
{
	if ((GWorldPtr) ps->refCon) {
		DisposeGWorld((GWorldPtr) ps->refCon);
		ps->refCon = 0L;
	}
}


void ExpandHotspot(HotspotPtr hs)
{
	CGrafPtr	curPort;
	GDHandle	curDevice;
	short		i;
	Point		*pPtr;
	short		nbrPts = hs->nbrPts;
	if (hs->nbrPts <= 2) {
		Point	pAry[] = {0,0,0,100,100,100,100,0};
		pPtr = &pAry[0];
		hs->loc.h = 50;
		hs->loc.v = 50;
		nbrPts = 4;
		// return;
	}
	else
		pPtr = (Point *) &gRoomWin->curRoom.varBuf[hs->ptsOfst];

	GetGWorld(&curPort,&curDevice);
	SetGWorld(gRoomWin->offWorld,NULL);

	*((RgnHandle *) &hs->refCon) = NewRgn();
	OpenRgn();
	if (hs->loc.h < 0 || hs->loc.h >= 512)
		hs->loc.h = 256;
	if (hs->loc.v < 0 || hs->loc.v >= 384)
		hs->loc.h = 384/2;
	if (pPtr[0].h < -400)
		pPtr[0].h = -400;
	if (pPtr[0].h > 400)
		pPtr[0].h = 400;
	if (pPtr[0].v < -400)
		pPtr[0].v = -400;
	if (pPtr[0].v > 400)
		pPtr[0].v = 400;
	MoveTo(pPtr[0].h+hs->loc.h,pPtr[0].v+hs->loc.v);
	for (i = 1; i < nbrPts; ++i) {
		if (pPtr[i].h < -400)
			pPtr[i].h = -400;
		if (pPtr[i].h > 400)
			pPtr[i].h = 400;
		if (pPtr[i].v < -400)
			pPtr[i].v = -400;
		if (pPtr[i].v > 400)
			pPtr[i].v = 400;
		LineTo(pPtr[i].h+hs->loc.h,pPtr[i].v+hs->loc.v);
	}
	LineTo(pPtr[0].h+hs->loc.h,pPtr[0].v+hs->loc.v);
	CloseRgn((RgnHandle) hs->refCon);

	SetGWorld(curPort,curDevice);
}

void ExpandPicture(PictureRecPtr ps)
{
	CGrafPtr	curPort;
	GDHandle	curDevice;
	GWorldPtr	picWorld;
	PicHandle	picH;	
	StringPtr	origName;
	Rect		picFrame;
	PixMapHandle	pMap;
	OSErr		oe;

	// 8/15/95 Modified to download hotspot overlays if needed
	origName = (StringPtr) &gRoomWin->curRoom.varBuf[ps->picNameOfst];

	if (FileIsBeingDownloaded(origName))
		picH = NULL;
	else
		picH = GetPictureFromFile(origName);

	if (picH == NULL) {
		if (!FileIsBeingDownloaded(origName) && (gPrefs.userPrefsFlags & UPF_DownloadGraphics) > 0)
			RequestFile(origName,MT_Picture);
		else if (!(gPrefs.userPrefsFlags & UPF_DownloadGraphics))
			LogMessage("Can't find picture %.*s\r",origName[0],&origName[1]);
		return;
	}

	// 7/6/95 support both PICT and DIB images.
	GetPictureFrame((Handle) picH,&picFrame);
	// picFrame = (*picH)->picFrame;

	// 4/4/95 JBUM switched to custom palette gCurClut
	if ((oe = NewGWorld(&picWorld,8,&picFrame,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		LogMessage("Can't allocate picture buffer (out of mem?)\r");
		DisposeHandle((Handle) picH);
		return;
	}

	pMap = GetGWorldPixMap(picWorld);
	LockPixels(pMap);


	// 7/6/95 Support DIB images
	if (IsDIB((Handle) picH))
		DIBRender((Handle) picH,picWorld,topLeft(picFrame));
	else if (IsGIF((Handle) picH)) {
		short	transColor = -1;
		GIFRender((Handle) picH,picWorld,topLeft(picFrame),&transColor);
		if (transColor != -1)
			ps->transColor = transColor;
	}
	// 3/26/97 JAB Added JPEG Support
	else if (IsJPEG((Handle) picH)) {
		JPEGRender((Handle) picH,picWorld,topLeft(picFrame));
	}
	else {
		GetGWorld(&curPort,&curDevice);
		SetGWorld(picWorld,NULL);
		DrawPicture(picH,&picFrame);
		SetGWorld(curPort,curDevice);
	}

	DisposeHandle((Handle) picH);

	ps->refCon = (long) picWorld;
}


Boolean DragDoors(Point p)
{
	Point	op,oloc;
	short	i;
	short	lockRec[4];
	char	tbuf[64];
	Rect	r;
	HotspotPtr hsl = (HotspotPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];
	StateRecPtr		srp;
	PictureRecPtr	ps;
	GWorldPtr		gw;
	HotspotPtr	door = NULL;
	short			pictID;

	for (i = 0; i < gRoomWin->curRoom.nbrHotspots; ++i,++hsl) {
		srp = (StateRecPtr) &gRoomWin->curRoom.varBuf[hsl->stateRecOfst];
		pictID = srp[hsl->state].pictID;
		if (pictID) {
			ps = GetPictureRec(pictID);
			gw = (GWorldPtr) ps->refCon;
			r = gw->portRect;
			OffsetRect(&r,-((r.right-r.left)/2),-((r.bottom-r.top)/2));
			OffsetRect(&r,hsl->loc.h,hsl->loc.v);
			OffsetRect(&r,srp[hsl->state].picLoc.h,srp[hsl->state].picLoc.v);
			if (PtInRect(p,&r)) {
				// User wandered into hotspot
				door = hsl;
				break;
			}
		}
	}
	if (door == NULL)
		return false;
	srp = (StateRecPtr) &gRoomWin->curRoom.varBuf[door->stateRecOfst];
	if (srp[door->state].pictID == 0)
		return false;

	op = p;
	oloc = srp[door->state].picLoc;

	while (WaitMouseUp()) {
		GetMouse(&p);
		p.h -= gVideoRoomRect.left - gOffscreenRect.left;
		p.v -= gVideoRoomRect.top - gOffscreenRect.top;
		if (!EqualPt(p,op)) {
			srp[door->state].picLoc.h += p.h - op.h;
			srp[door->state].picLoc.v += p.v - op.v;
			op = p;

			sprintf(tbuf,"PICLOC %d,%d ",srp[door->state].picLoc.h,srp[door->state].picLoc.v);
			PrepareTextColors();
			TESetText(tbuf,strlen(tbuf),gRoomWin->msgTEH);
			TEUpdate(&gRoomWin->msgRect,gRoomWin->msgTEH);
			RestoreWindowColors();
			RefreshRoom(&gOffscreenRect);
		}
	}
	if (!EqualPt(oloc,srp[door->state].picLoc)) {
		lockRec[0] = gRoomWin->curRoom.roomID;
		lockRec[1] = door->id;
		lockRec[2] = srp[door->state].picLoc.v;
		lockRec[3] = srp[door->state].picLoc.h;
		DoPalaceCommand(PC_SetPicOffset,(long) &lockRec[0],NULL);
	}
	return true;
}
