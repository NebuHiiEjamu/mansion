// RoomGraphics.c

#include "U-USER.H"
#include "U-TIMOUT.H"
#include "PPAMgr.h"

void ShowStatusBar(long total, long current, long remainingTime);
void ShowStatusBar(long total, long current, long remainingTime)
{
	Rect		wr,pr,sr,dr;
	RoomRec		*rp = &gRoomWin->curRoom;
	GrafPtr		savePort;
	CGrafPtr	curPort;
	GDHandle	curDevice;
	char		sbuf[48];		// 7/11/96 Raised to 48 from 32

	if (total < 1 || total < current)
		return;
	GetPort(&savePort);
	SetPort((WindowPtr) gRoomWin);

	GetGWorld(&curPort,&curDevice);
	SetGWorld(gRoomWin->offWorld,NULL);

	wr = gOffscreenRect;
	wr.top = wr.bottom - 16;
	InsetRect(&wr,1,1);
	sr = dr = wr;

	OffsetRect(&dr,gVideoRoomRect.left,gVideoRoomRect.top);
	ColorPaintRect(&wr,&gGray22);
	InsetRect(&wr,1,1);
	pr = wr;
	pr.right = pr.left + ((long) current*(long) (pr.right - pr.left)) / total;
	SectRect(&pr,&wr,&pr);	// 7/11/96 Make sure within bar
	HiliteRect(&pr);
	InsetRect(&pr,1,1);
	ColorPaintRect(&pr,&gGray44);

	if (remainingTime >= 0) {	// 7/11/96 Changed to >=
		if (remainingTime >= 60L) {
			sprintf(sbuf,"under %ld minutes remaining",remainingTime/60L+1L);
		}
		else if (remainingTime >= 30L) {
			sprintf(sbuf,"under a minute remaining");
		}
		else if (remainingTime >= 15L) {
			sprintf(sbuf,"under 30 seconds remaining");
		}
		else
			sprintf(sbuf,"under 15 seconds remaining");
	}
	else
		sbuf[0] = 0;	// 5/10/96

	if (sbuf[0]) {	// 7/11/96 Added condition
		CtoPstr(sbuf);
		TextFont(geneva);
		TextSize(9);
		MoveTo(sr.left + ((sr.right - sr.left)-StringWidth((StringPtr) sbuf))/2,sr.bottom-4);
		DrawString((StringPtr) sbuf);
	}

	SetGWorld(curPort,curDevice);

	CopyBits((BitMap *) *gRoomWin->offPixMap, &((WindowPtr) gRoomWin)->portBits, &sr, &dr, srcCopy, NULL);

	SetPort(savePort);
}

void ShowDownloadStatus(StringPtr name, unsigned long timeStamp, short blockNbr, short maxBlock);
void ShowDownloadStatus(StringPtr name, unsigned long timeStamp, short blockNbr, short maxBlock)
{
	RoomRecPtr	rp = &gRoomWin->curRoom;
	StringPtr	origName;
	origName = (StringPtr) &rp->varBuf[rp->pictNameOfst];

	if (!EqualString(origName, name, false, false))
		return;
	if (maxBlock < 1)
		return;
	if (blockNbr > 2) {
		long	ticksPerBlock,estTicks;
		ticksPerBlock = (GetTicks() - timeStamp) / blockNbr;
		estTicks = (maxBlock - blockNbr) * ticksPerBlock;
		ShowStatusBar(maxBlock,blockNbr,estTicks/60L);
	}
	else
		ShowStatusBar(maxBlock,blockNbr,-1L);
}

short gLightLevel;

void SetLightLevel(short level);
void SetLightLevel(short level)
{
	gLightLevel = level;
	RefreshNewRoom();
	gLightLevel = 0;
}

void DarkenRoom(short level);
void DarkenRoom(short level)
{
	unsigned char *dp,*p,*fp;
	unsigned long dRowBytes;
	short			y;
	short			x;
	if (level == 0 || level >= 100)
		return;

	dp = (unsigned char *) GetPixBaseAddr(gRoomWin->offPictureMap);
	dRowBytes = (*gRoomWin->offPictureMap)->rowBytes & 0x3FFF;
	level = 100-level;
	level = (level * NbrFades)/100;
	if (level > NbrFades-1)
		level = NbrFades-1;
	fp = &gRoomWin->fTrans[((long) level) << 8];
	for (y = 0; y < 384; ++y) {
		p = dp;
		for (x = 0; x < 512; ++x,++p) {
			*p = fp[*p];
		}
		dp += dRowBytes;
	}
}

#if 1
void RoomDrawTest(void);
void RoomDrawTest(void)
{
	PicHandle	picH;
	short		i;
	CGrafPtr	curPort;
	GDHandle	curDevice;
	short		transColor=-1;
	void PrepareSplash(void);

	for (i = 0; i < 4; ++i) {
		picH = GetPictureFromFile((i & 1)? "\pPGATE.GIF" : "\pBAR.GIF");
		if (picH) {
			// 7/6/95 Added hooks to DIB routines
			if (IsDIB((Handle) picH)) {
				DIBRender((Handle) picH, gRoomWin->offPicture, topLeft(gOffscreenRect));
			}
			else if (IsGIF((Handle) picH)) {
				GIFRender((Handle) picH, gRoomWin->offPicture, topLeft(gOffscreenRect), &transColor);
			}
			// 3/26/97 JAB Added JPEG Support
			else if (IsJPEG((Handle) picH)) {
				JPEGRender((Handle) picH, gRoomWin->offPicture, topLeft(gOffscreenRect));
			}
			else {
				GetGWorld(&curPort,&curDevice);			// 7/12/95 MOVED to prevent log msg problems
				SetGWorld(gRoomWin->offPicture,NULL);
				DrawPicture(picH, &gOffscreenRect);
				SetGWorld(curPort,curDevice);
			}
			DisposeHandle((Handle) picH);
		}
		RefreshRoom(&gOffscreenRect);
		ShowRoomStatus();
	}
	PrepareSplash();
	RefreshRoom(&gOffscreenRect);
}
#endif

// esr moved here from UserRoom.c 5/19
void RefreshNewRoom(void)
{
	PicHandle	picH;
	CGrafPtr	curPort;
	GDHandle	curDevice;
	StringPtr	origName;
	RoomRec		*rp = &gRoomWin->curRoom;
	short		transColor=-1;

	KillAuthoring();


	// TestTCPCorruption('A');


	origName = (StringPtr) &rp->varBuf[rp->pictNameOfst];
	if (FileIsBeingDownloaded(origName))
		picH = NULL;
	else
		picH = GetPictureFromFile(origName);
	if (picH) {

		// 7/6/95 Added hooks to DIB routines
		if (IsDIB((Handle) picH)) {
			DIBRender((Handle) picH, gRoomWin->offPicture, topLeft(gOffscreenRect));
		}
		else if (IsGIF((Handle) picH)) {
			GIFRender((Handle) picH, gRoomWin->offPicture, topLeft(gOffscreenRect), &transColor);
		}
		// 3/26/97 JAB Added JPEG Support
		else if (IsJPEG((Handle) picH)) {
			JPEGRender((Handle) picH, gRoomWin->offPicture, topLeft(gOffscreenRect));
		}
		else {
			GetGWorld(&curPort,&curDevice);			// 7/12/95 MOVED to prevent log msg problems
			SetGWorld(gRoomWin->offPicture,NULL);
			DrawPicture(picH, &gOffscreenRect);
			SetGWorld(curPort,curDevice);
		}

		DisposeHandle((Handle) picH);
		gRoomWin->noRoomPicture = false;
	}
	else {
		char	tbuf[128];
		if (!FileIsBeingDownloaded(origName) && (gPrefs.userPrefsFlags & UPF_DownloadGraphics) > 0)
		{
			RequestFile(origName, MT_Picture);
		}

		GetGWorld(&curPort,&curDevice);			// JAB 7/12/95 MOVED to prevent logmsg problems
		SetGWorld(gRoomWin->offPicture,NULL);

		PaintRect(&gOffscreenRect);
		if (origName[0]) {
			sprintf(tbuf,"Picture %.*s missing",origName[0],&origName[1]);
			MoveTo(gCenterP.h - TextWidth(tbuf,0,strlen(tbuf))/2,gCenterP.v);
			TextMode(srcBic);
			DrawText(tbuf,0,strlen(tbuf));
			TextMode(srcOr);
		}

		SetGWorld(curPort,curDevice);

		gRoomWin->noRoomPicture = true;
	}

	// TestTCPCorruption('B');

	if (gLightLevel)
		DarkenRoom(gLightLevel);
	RefreshRoom(&gOffscreenRect);
	ShowRoomStatus();
	gLightLevel = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// 4/4/95 Switched to 8-bit format for faces - simplified code...
//
// Same as DrawPropPixMap, but colorizes
//
/**
static char gFadeMask[16][16] = 
	// 0,   8,   3,   11,    12,  4,   15,  7,     2,   10,  1,   9,     14,  6,   13,  5,
	{
	 0xff,0x00,0x00,0x00,  0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00,
	 0xff,0x00,0x00,0x00,  0x00,0x00,0x00,0x00,  0x00,0x00,0xff,0x00,  0x00,0x00,0x00,0x00,
	 0xff,0x00,0x00,0x00,  0x00,0x00,0x00,0x00,  0xff,0x00,0xff,0x00,  0x00,0x00,0x00,0x00,
	 0xff,0x00,0xff,0x00,  0x00,0x00,0x00,0x00,  0xff,0x00,0xff,0x00,  0x00,0x00,0x00,0x00,

	 0xff,0x00,0xff,0x00,  0x00,0xff,0x00,0x00,  0xff,0x00,0xff,0x00,  0x00,0x00,0x00,0x00,
	 0xff,0x00,0xff,0x00,  0x00,0xff,0x00,0x00,  0xff,0x00,0xff,0x00,  0x00,0x00,0x00,0xff,
	 0xff,0x00,0xff,0x00,  0x00,0xff,0x00,0x00,  0xff,0x00,0xff,0x00,  0x00,0xff,0x00,0xff,
	 0xff,0x00,0xff,0x00,  0x00,0xff,0x00,0xff,  0xff,0x00,0xff,0x00,  0x00,0xff,0x00,0xff,

	 0xff,0xff,0xff,0x00,  0x00,0xff,0x00,0xff,  0xff,0x00,0xff,0x00,  0x00,0xff,0x00,0xff,
	 0xff,0xff,0xff,0x00,  0x00,0xff,0x00,0xff,  0xff,0x00,0xff,0xff,  0x00,0xff,0x00,0xff,
	 0xff,0xff,0xff,0x00,  0x00,0xff,0x00,0xff,  0xff,0xff,0xff,0xff,  0x00,0xff,0x00,0xff,
	 0xff,0xff,0xff,0xff,  0x00,0xff,0x00,0xff,  0xff,0xff,0xff,0xff,  0x00,0xff,0x00,0xff,

	 0xff,0xff,0xff,0xff,  0xff,0xff,0x00,0xff,  0xff,0xff,0xff,0xff,  0x00,0xff,0x00,0xff,
	 0xff,0xff,0xff,0xff,  0xff,0xff,0x00,0xff,  0xff,0xff,0xff,0xff,  0x00,0xff,0xff,0xff,
	 0xff,0xff,0xff,0xff,  0xff,0xff,0x00,0xff,  0xff,0xff,0xff,0xff,  0xff,0xff,0xff,0xff,
	 0xff,0xff,0xff,0xff,  0xff,0xff,0xff,0xff,  0xff,0xff,0xff,0xff,  0xff,0xff,0xff,0xff};

static char fadeOrder[16] = {0,8,3,11,
									 12,4,15,7,
									 2,10,1,9,
									 14,6,13,5};
static unsigned char gFadeArray[] = 
	{0xff,0xff,
	 0x7f,0x7f,
	 0x3F,0x3F,
	 0x1F,0x1F,
	 0x0F,0x0F,
	 0x07,0x07,
	 0x03,0x03,
	 0x01,0x01};

static char gFadeOrder[16] = {0,8,4,12,
							  2,10,6,14,
							  1,9,5,13,
							  3,11,7,15};
**/

void DrawFacePixmap(short faceNbr, short faceColor, 
					short h, short v, 
					short fadePass,
					PixMapHandle pixMap)
{
	long			dRowBytes;
	short			y,x;
	Handle			faceH;
	PropHeaderPtr	ph;
	unsigned		char	cb,mc,pc,*sp,*dp,*cp,*fp;

	faceH = gRoomWin->faceHandles[faceNbr];
	ph = (PropHeaderPtr) *faceH;
	sp = (unsigned char *) *faceH + sizeof(PropHeader);

	dp = (unsigned char *) GetPixBaseAddr(pixMap);
	dRowBytes = (*pixMap)->rowBytes & 0x3FFF;
	dp += dRowBytes * v + h;
	cp = &gRoomWin->cTrans[((unsigned short) faceColor) << 8];
	dRowBytes -= ph->width;

	if (fadePass) {
		fp = &gRoomWin->fTrans[(NbrFades-fadePass) << 8];
		for (y = 0; y < ph->height; ++y) {
			x = ph->width;
			while (x) {
				cb = *((unsigned char *) sp);	++sp;
				mc = cb >> 4;
				pc = cb & 0x0F;
				x -= (mc + pc);
				dp += mc;
				while (pc--)
					*(dp++) = fp[cp[*(sp++)]];
			}
			dp += dRowBytes;
		}
	}
	else {
		for (y = ph->height-1; y >= 0; --y) {
			x = ph->width;
			while (x > 0) {
				cb = *((unsigned char *) sp);	++sp;
				mc = cb >> 4;
				pc = cb & 0x0F;
				x -= (mc + pc);
				dp += mc;
				while (pc--)
					*(dp++) = cp[*(sp++)];
			}
			dp += dRowBytes;
		}
	}
}

void DrawFace(short faceNbr, short faceColor, short h, short v, short fadePass)
{
	DrawFacePixmap(faceNbr,faceColor,h,v,fadePass,gRoomWin->offPixMap);
}


void DrawMansionPropPixMap(Handle	propH, PixMapHandle pMap, short h, short v, short fadePass)
{
	unsigned char	*sp,*dp,*fp,*gp;
	long			dRowBytes;
	short			y,x;
	PropHeaderPtr	ph;
	unsigned		char	cb,mc,pc;
	
	if (propH == NULL)
		return;

	ph = (PropHeaderPtr) *propH;
	sp = (unsigned char *) *propH + sizeof(PropHeader);

	if (ph->height < 1 || ph->height > 64) {
		SwapShort((unsigned short *) &ph->height);
		SwapShort((unsigned short *) &ph->height);
		SwapShort((unsigned short *) &ph->hOffset);
		SwapShort((unsigned short *) &ph->vOffset);
		SwapLong((unsigned long *) &ph->flags);
	}

	// 6/21/95 Don't draw if outside of bounds of pixel map
#if 0
	if (v < 0 || h < 0 || v+ph->height > (*pMap)->bounds.bottom-(*pMap)->bounds.top ||
		h+ph->width > (*pMap)->bounds.right-(*pMap)->bounds.left)
	{
		ErrorExit("Clipping Problem");
		return;
	}
#endif

	dp = (unsigned char *) GetPixBaseAddr(pMap);
	dRowBytes = (*pMap)->rowBytes & 0x3FFF;
	dp += dRowBytes * v + h;
	dRowBytes -= ph->width;

	if (ph->flags & PF_PropGhostFlag) {
		gp = gRoomWin->gTrans;
		if (fadePass) {
			fp = &gRoomWin->fTrans[(NbrFades-fadePass) << 8];
			for (y = 0; y < ph->height; ++y) {
				x = ph->width;
				while (x) {
					cb = *((unsigned char *) sp);	++sp;
					mc = cb >> 4;
					pc = cb & 0x0F;
					x -= (mc + pc);
					if (x < 0)			// 10/23/95 abort early on bad props!
						return;
					dp += mc;
					while (pc--)
						*(dp++) = fp[gp[((unsigned short) *dp << 8) | *(sp++)]];
				}
				dp += dRowBytes;
			}
		}
		else {
			for (y = ph->height-1; y >= 0; --y) {
				x = ph->width;
				while (x > 0) {
					cb = *((unsigned char *) sp);	++sp;
					mc = cb >> 4;
					pc = cb & 0x0F;
					x -= (mc + pc);
					if (x < 0)			// 10/23/95 abort early on bad props!
						return;
					dp += mc;
					while (pc--)
						*(dp++) = gp[((unsigned short) *dp << 8) | *(sp++)];
				}
				dp += dRowBytes;
			}
		}
	}
	else if (fadePass) {
		fp = &gRoomWin->fTrans[(NbrFades-fadePass) << 8];
		for (y = 0; y < ph->height; ++y) {
			x = ph->width;
			while (x) {
				cb = *((unsigned char *) sp);	++sp;
				mc = cb >> 4;
				pc = cb & 0x0F;
				x -= (mc + pc);
				if (x < 0)			// 10/23/95 abort early on bad props!
					return;
				dp += mc;
				while (pc--)
					*(dp++) = fp[*(sp++)];
			}
			dp += dRowBytes;
		}
	}
	else {
		for (y = ph->height-1; y >= 0; --y) {
			x = ph->width;
			while (x > 0) {
				cb = *((unsigned char *) sp);	++sp;
				mc = cb >> 4;
				pc = cb & 0x0F;
				x -= (mc + pc);
				if (x < 0)			// 10/23/95 abort early on bad props!
					return;
				dp += mc;
				while (pc--)
					*(dp++) = *(sp++);
			}
			dp += dRowBytes;
		}
	}
}

void DrawProp(Handle propH, short h, short v, short fadePass)
{
	// 6/21/95 Compute Rectangle and Make sure it's within Room Rect
	// Note: Coordinates are already adjusted for LeftMargin and TopMargin 
	if (h+PropWidth > LeftMargin && v+PropHeight > TopMargin &&
		h<LeftMargin+gOffscreenRect.right && v<TopMargin+gOffscreenRect.bottom)
		DrawMansionPropPixMap(propH, gRoomWin->offPixMap, h, v, fadePass);
}


void DrawPictureRes(GWorldPtr pWorld, Rect *sr, Rect *dr, short transColor)
{
	unsigned char	*sp,*dp;
	long			sRowBytes,dRowBytes;
	short			y,x,w,h;
	PixMapHandle	pMap;

	pMap = GetGWorldPixMap(pWorld);
	w = sr->right - sr->left;
	h = sr->bottom - sr->top;
	sp = (unsigned char *) GetPixBaseAddr(pMap);
	dp = (unsigned char *) GetPixBaseAddr(gRoomWin->offPixMap);
	sRowBytes = (*pMap)->rowBytes & 0x3FFF;
	dRowBytes = (*gRoomWin->offPixMap)->rowBytes & 0x3FFF;
	sp += sRowBytes*sr->top + sr->left;
	dp += dRowBytes * TopMargin + LeftMargin;
	dp += dRowBytes * dr->top + dr->left;
	if (transColor >= 0) {
		for (y = h-1; y >= 0; --y) {
			for (x = w-1; x >= 0; --x) {
				if (*sp != transColor)
					*dp = *sp;
				++sp;
				++dp;
			}
			dp += dRowBytes - w;
			sp += sRowBytes - w;
		}
	}
	else {
		for (y = h-1; y >= 0; --y) {
			for (x = w-1; x >= 0; --x) {
				*(dp++) = *(sp++);
			}
			dp += dRowBytes - w;
			sp += sRowBytes - w;
		}
	}
}


void RefreshSpots(Rect *rr)
{
	short			i;
	HotspotPtr 		hs;
	StateRecPtr		srp;
	PictureRecPtr	ps;
	Rect			sr,dr,tr;
	GWorldPtr		gw;
	short			pictID;
	RoomRec			*rp = &gRoomWin->curRoom;

	hs = (HotspotPtr) &rp->varBuf[rp->hotspotOfst];
	for (i = 0; i < rp->nbrHotspots; ++i,++hs) {
		srp = (StateRecPtr) &rp->varBuf[hs->stateRecOfst];
		pictID = srp[hs->state].pictID;
		if (pictID && !(hs->flags & HS_Invisible)) {
			ps = GetPictureRec(pictID);
			if (ps) {
				gw = (GWorldPtr) ps->refCon;
				sr = gw->portRect;
				dr = sr;
				OffsetRect(&dr,-((dr.right-dr.left)/2),-((dr.bottom-dr.top)/2));
				OffsetRect(&dr,hs->loc.h,hs->loc.v);
				OffsetRect(&dr,srp[hs->state].picLoc.h,srp[hs->state].picLoc.v);
				if (SectRect(&dr,rr,&tr)) {
					// 5/9/95 Align with Refresh Rect instead of gOffscreenRect
					//		(speed improvement!)
					if (dr.left < rr->left) {
						sr.left += rr->left - dr.left;
						dr.left = rr->left;
					}
					if (dr.top < rr->top) {
						sr.top += rr->top - dr.top;
						dr.top = rr->top;
					}
					if (dr.bottom > rr->bottom) {
						sr.bottom -= dr.bottom - rr->bottom;
						dr.bottom = rr->bottom;
					}
					if (dr.right > rr->right) {
						sr.right -= dr.right - rr->right;
						dr.right = rr->right;
					}
					if (!EmptyRect(&sr)) {
						// copy picture based on transparency color
						DrawPictureRes(gw,&sr,&dr,ps->transColor);
					}
				}
			}
		}
		if (hs->flags & HS_Fill) {
			// Consider using Region...
		}
		if (hs->flags & HS_ShowFrame) {
			DrawSpotFrame(hs,patCopy,false);
		}
		if (hs->flags & HS_Shadow) {
			// Consider using Region...
		}
		if ((hs->flags & HS_ShowName) > 0 && hs->nameOfst) {
			StringPtr nameStr;
			// RGBForeColor(&gBlackColor);
			// RGBBackColor(&gWhiteColor);
			TextMode(srcCopy);
			nameStr = (StringPtr) &rp->varBuf[hs->nameOfst];
			MoveTo(hs->loc.h-StringWidth(nameStr)/2,hs->loc.v);
			DrawString(nameStr);
		}
	}
}

// 4/24/96 JAB - added code for Animating Props

void RefreshFaces(Rect *r)
{
	LocalUserRecPtr	up;
	short			i,j,animePropNbr;
	Rect			dr,tr,sr;
	FontInfo		fi;
	RoomRec			*rp = &gRoomWin->curRoom;
	short			userFade;
	up = gRoomWin->userList;

	GetFontInfo(&fi);				// 6/13/95

	for (i = 0; i < rp->nbrPeople; ++i) {
		// 11/8 added fade feedback for private chat
		userFade = up->userFade;
		if (gRoomWin->targetID && up->user.userID != gRoomWin->meID &&
			up->user.userID != gRoomWin->targetID && (userFade == 0 || userFade > 4))
			userFade = 4;
		ComputeUserRRect(up,&dr);
		if (SectRect(&dr,r,&tr)) {
			if (!(up->localFlags & UF_PropFace))  // 6/22/95
				DrawFace(up->user.faceNbr, up->user.colorNbr,
						 up->user.roomPos.h+LeftMargin-FaceWidth/2,
						 up->user.roomPos.v+TopMargin-FaceHeight/2,
						  userFade);
			// 6/8/95 modified to allow variable prop list
			if (up->user.nbrProps) {
				animePropNbr = 0;	// 4/94/96 JAB
				for (j = 0; j < up->user.nbrProps; ++j) {
					if (up->propHandle[j]) {
						// 4/24/96 JAB - skip animating frames that aren't on
						if (((PropHeaderPtr) *up->propHandle[j])->flags & PF_Animate) {
							if (animePropNbr++ != up->propFrameNumber)
								continue;
						}
						ComputePropRect(j,up,&sr);
						DrawProp(up->propHandle[j],sr.left+LeftMargin,sr.top+TopMargin, userFade);
					}
				}
			}
			if (gShowNames) {
				short	h,v,w;
				Rect	r;
				// RGBForeColor(&gWhiteColor);
				// RGBBackColor(&gBlackColor);
	
				// NOTE: This code should be identical to that in ComputeNameRect
				w = StringWidth(up->user.name);
				h = up->user.roomPos.h - w/2;
				v = up->user.roomPos.v + NameGap + fi.ascent;
				if (v+fi.descent > gOffscreenRect.bottom)
					v -= (v+fi.descent)-gOffscreenRect.bottom;
				if (h+w > gOffscreenRect.right)
					h -= (h+w) - gOffscreenRect.right;
				if (v-fi.ascent < 0)
					v += gOffscreenRect.top-(v-fi.ascent);
				if (h < gOffscreenRect.left)
					h = gOffscreenRect.left;
				SetRect(&r,h-1,v-(fi.ascent+1),h+w+1,v+fi.descent+1);
				PaintRect(&r);
				MoveTo(h,v);
				TextMode(srcBic);
				DrawString(up->user.name);
				TextMode(srcOr);
				// RGBBackColor(&gWhiteColor);
				// RGBForeColor(&gBlackColor);
			}
		}
		++up;
	}
}

void RefreshRoom(Rect *rr)
{
	CGrafPtr	curPort;
	GDHandle	curDevice;
	Rect		sr,dr,r;


	// return; // 10/17/95 test

	// Make sure refresh rectangle is within screen boundaries
	SectRect(rr,&gOffscreenRect,&r);

	// Setup to draw offscreen, after we finish drawing, we'll blit
	GetGWorld(&curPort,&curDevice);
	SetGWorld(gRoomWin->offWorld,NULL);

	TextFont(gPrefs.fontID);
	TextSize(gPrefs.fontSize);

	// Copy the room background art
	CopyBits((BitMap *) *gRoomWin->offPictureMap, (BitMap *) *gRoomWin->offPixMap,
			 &r, &r, srcCopy, NULL);


	// Refresh hotspots (doors & such)
	RefreshSpots(&r);			// Hotspots

	// Refresh user drawings, background layer
	RefreshDrawObjects(&r, false);	// Background DrawObjects

#if PPASUPPORT
	PPAMgrRefreshPPAs(&r, false);
#endif

	// Refresh loose props
	RefreshLProps(&r);			// Loose Props (may want to do these behind background draw objects

	// Refresh Faces
	RefreshFaces(&r);

	// Refresh user drawings, top layer
	RefreshDrawObjects(&r,1);	// Foreground DrawObjects

	// Refresh Balloons
	DrawBalloons(&r);

	if (gMode == M_Authoring || gRoomWin->noRoomPicture || gShowFrames)
		RefreshSpotFrames();

#if PPASUPPORT
	PPAMgrRefreshPPAs(&r, true);
#endif

	// 1/16/97 JAB Add timer for demo...
	DemoTimerDisplay(&r);

	SetGWorld((CGrafPtr) gRoomWin, NULL);

	if (!gIconized) {
		sr = r;
		if (g12InchMode && gRoomWin->msgActive && sr.bottom > 384-45)
			sr.bottom = 384-45;
		dr = sr;
		OffsetRect(&dr,gVideoRoomRect.left,gVideoRoomRect.top);
	}
	else {
		if (gRoomWin->mePtr) {
			SetRect(&sr,gRoomWin->mePtr->user.roomPos.h,gRoomWin->mePtr->user.roomPos.v,
						gRoomWin->mePtr->user.roomPos.h,gRoomWin->mePtr->user.roomPos.v);
			InsetRect(&sr,-32,-32);
		}
		else
			SetRect(&sr,0,0,64,64);
		SetRect(&dr,0,0,64,64);
	}	

#if M5SUPPORT
	{
		void M5Update(Rect *rr);
		M5Update(&sr);
	}
#else
	CopyBits((BitMap *) *gRoomWin->offPixMap, &((WindowPtr) gRoomWin)->portBits, &sr, &dr, srcCopy, NULL);
#endif

	SetGWorld(curPort,curDevice);
}



RGBColor	gBlackColor = {0,0,0};
RGBColor	gWhiteColor	= {0xFFFF,0xFFFF,0xFFFF};
RGBColor	gRedColor	= {0xFFFF,0x0000,0x0000};
RGBColor	gCyanColor	= {0x0000,0x8888,0x8888};
RGBColor	gPurpleColor = {0x8888,0x0000,0x8888};
RGBColor	gGrayColor =  {0x8888,0x8888,0x8888};
RGBColor	gGrayAA =  		{0xAAAA,0xAAAA,0xAAAA};
RGBColor	gGray66 =  		{0x6666,0x6666,0x6666};
RGBColor	gGray44 =  		{0x4444,0x4444,0x4444};
RGBColor	gGray22 =  		{0x2222,0x2222,0x2222};

void HiliteRect(Rect *r)
{
	RGBForeColor(&gGray66);
	MoveTo(r->left,r->top);
	LineTo(r->left,r->bottom-1);
	MoveTo(r->left,r->top);
	LineTo(r->right-1,r->top);

	RGBForeColor(&gGray22);
	MoveTo(r->right-1,r->bottom-1);
	LineTo(r->left,r->bottom-1);
	MoveTo(r->right-1,r->bottom-1);
	LineTo(r->right-1,r->top);
	
	RGBForeColor(&gBlackColor);
}

void DkHiliteRect(Rect *r)
{
	RGBForeColor(&gGray44);
	MoveTo(r->left,r->top);
	LineTo(r->left,r->bottom-1);
	MoveTo(r->left,r->top);
	LineTo(r->right-1,r->top);

	RGBForeColor(&gBlackColor);
	MoveTo(r->right-1,r->bottom-1);
	LineTo(r->left,r->bottom-1);
	MoveTo(r->right-1,r->bottom-1);
	LineTo(r->right-1,r->top);
	
	RGBForeColor(&gBlackColor);
}


void RevHiliteRect(Rect *r)
{
	RGBForeColor(&gGray22);
	MoveTo(r->left,r->top);
	LineTo(r->left,r->bottom-1);
	MoveTo(r->left,r->top);
	LineTo(r->right-1,r->top);

	RGBForeColor(&gGray66);
	MoveTo(r->right-1,r->bottom-1);
	LineTo(r->left,r->bottom-1);
	MoveTo(r->right-1,r->bottom-1);
	LineTo(r->right-1,r->top);
	
	RGBForeColor(&gBlackColor);
}

void DkRevHiliteRect(Rect *r)
{
	RGBForeColor(&gBlackColor);
	MoveTo(r->left,r->top);
	LineTo(r->left,r->bottom-1);
	MoveTo(r->left,r->top);
	LineTo(r->right-1,r->top);

	RGBForeColor(&gGray44);
	MoveTo(r->right-1,r->bottom-1);
	LineTo(r->left,r->bottom-1);
	MoveTo(r->right-1,r->bottom-1);
	LineTo(r->right-1,r->top);
	
	RGBForeColor(&gBlackColor);
}


void WhiteRect(Rect *r)
{
	RGBForeColor(&gWhiteColor);
	FrameRect(r);
	RGBForeColor(&gBlackColor);
}

void ColorPaintRect(Rect *r, RGBColor *fc)
{
	RGBForeColor(fc);
	PaintRect(r);
	RGBForeColor(&gBlackColor);
}

#define RES_INIT	0
#define CTRANS_RSRC	'cTra'

void InitRoomColors()
{
#if RES_INIT
	short		i,j;
	RGBColor	fc,srce,dest,trans;
	HSLColor	hc;
	double		dropOff;
	Handle		h;
	Ptr			p;
	SysBeep(1);
	// Initialize Colors
	//
	// 4/4/95 Modified color table to 256 shades (faces are now 8-bits instead of 4-bit)
	//
	for (i = 0; i < NbrColors; ++i) {
		hc.hue = (i*256)/NbrColors;
		hc.hue = (hc.hue << 8) | hc.hue;
		hc.saturation = 0xFFFF;
		for (j = 0; j < NbrShades; ++j) {
			fc = (*gCurCLUT)->ctTable[j].rgb;
			// regular brightness equation, damped by a bit
			hc.lightness = (unsigned short ) fc.red * 0.260 + (unsigned short ) fc.green * 0.391 + (unsigned short ) fc.blue * 0.173;
			HSL2RGB(&hc,&fc);
			gRoomWin->cTrans[i*256+j] = Color2Index(&fc);
		}
	}
	for (i = 0; i < 16; ++i) {
		dropOff = 1.0-(double) i/(15);
		hc.hue = (i*256)/NbrColors;
		hc.hue = (hc.hue << 8) | hc.hue;
		hc.saturation = 0xFFFF;
		for (j = 0; j < NbrShades; ++j) {
			fc = (*gCurCLUT)->ctTable[j].rgb;
			fc.red = (fc.red >> 8) & 0x00FF;
			fc.red = fc.red*dropOff;
			fc.red = (fc.red << 8) | fc.red;
			fc.green = (fc.green >> 8) & 0x00FF;
			fc.green = fc.green*dropOff;
			fc.green = (fc.green << 8) | fc.green;
			fc.blue = (fc.blue >> 8) & 0x00FF;
			fc.blue = fc.blue*dropOff;
			fc.blue = (fc.blue << 8) | fc.blue;
			// regular brightness equation, damped by a bit
			gRoomWin->fTrans[i*256+j] = Color2Index(&fc);
		}
	}
	for (i = 0; i < 256; ++i) {
		srce = (*gCurCLUT)->ctTable[i].rgb;
		for (j = 0; j < 256; ++j) {
			dest = (*gCurCLUT)->ctTable[j].rgb;
			trans.red = ((unsigned long) srce.red + (unsigned long) dest.red) >> 1;
			trans.green = ((unsigned long) srce.green + (unsigned long) dest.green) >> 1;
			trans.blue = ((unsigned long) srce.blue + (unsigned long) dest.blue) >> 1;
			gRoomWin->gTrans[i*256+j] = Color2Index(&trans);
		}
	}
	SetResLoad(false);
	while ((h = GetResource(CTRANS_RSRC,128)) != NULL) {
		RmveResource(h);
		DisposeHandle(h);
	}
	SetResLoad(true);

	/* 7/1/96 JAB Changed to NewHandleClear */
	h = NewHandleClear((long) NbrColors*NbrShades +
				  (long) NbrFades*NbrShades +
				  (long) NbrShades*NbrShades);
	if (h) {
		p = *h;
		BlockMove(gRoomWin->cTrans, p, (long)NbrColors*NbrShades);
		p += (long) NbrColors*NbrShades;
		BlockMove(gRoomWin->fTrans, p, (long)NbrFades*NbrShades);
		p += (long) NbrFades*NbrShades;
		BlockMove(gRoomWin->gTrans, p, (long)NbrShades*NbrShades);
	
		AddResource(h,CTRANS_RSRC,128,"\pColor Table Trans");
		WriteResource(h);
		ReleaseResource(h);
	}
	else
		ErrorExit("Can't allocate colors");
	SysBeep(1);
#else
	Handle	h;
	Ptr		p;
	h = GetResource(CTRANS_RSRC,128);
	if (h) {
		p = *h;
		BlockMove(p, gRoomWin->cTrans, (long) NbrColors*NbrShades);
		p += (long) NbrColors*NbrShades;
		BlockMove(p, gRoomWin->fTrans, (long) NbrFades*NbrShades);
		p += (long) NbrFades*NbrShades;
		BlockMove(p, gRoomWin->gTrans, (long) NbrShades*NbrShades);
		ReleaseResource(h);
	}
#endif
}
