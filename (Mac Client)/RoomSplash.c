// RoomSplash.c
#include "U-USER.H"
#include "U-SECURE.H"
#include <strings.h>
// #include "m-cmds.H"
// #include "U-SCRIPT.H"
// #include "UserTools.h"	// 6/10/95
// #include <Palettes.h>			// 4/4/95 JBUM Custom Palette

static char	gSplashCheck=0;

void PrepareSplash(void);

void DrawBuiltInPict(short resID, short offsetH, short offsetV);

void DrawBuiltInPict(short resID, short offsetH, short offsetV)
{
	CGrafPtr	curPort;
	GDHandle	curDevice;
	Handle		picH;
	Rect		pr;

	GetGWorld(&curPort,&curDevice);
	SetGWorld(gRoomWin->offPicture,NULL);

	picH = GetResource('PICT',resID);
	if (picH) {
		pr =(* ((PicHandle) picH) )->picFrame;
		OffsetRect(&pr, offsetH, offsetV);
		// OffsetRect(&pr,gCenterP.h-pr.right/2,gCenterP.v-pr.bottom/2);
		DrawPicture((PicHandle) picH, &pr);
		ReleaseResource(picH);
	}
	SetGWorld(curPort,curDevice);
}

void PrepareSplash(void)
{
	CGrafPtr	curPort;
	GDHandle	curDevice;
	// Rect		pr;
	// Point		p;
	Str255		tStr;
	short		nbrStrings = 0;
	// Handle		picH;
	short		y,x;
	Rect		sr,dr;
	Handle		versHandle;
	StringPtr	labelStr = "\pRegistered to:";

	GetGWorld(&curPort,&curDevice);
	SetGWorld(gRoomWin->offPicture,NULL);

	PaintRect(&gOffscreenRect);

	// goto EarlyExit; // 10/17/95 test

	if (!gQuitFlag) {

		DrawBuiltInPict(130,0,0);
// Display of overlay is now controlled by ppre resource....
// #ifndef RAINBOWRED
		// 1/14/97JAB - changed logic to use "isRegistered" insteaed
		// of "guestAccess" which is not always true for unregistered users
		if (!gSecure.isRegistered && (gPrefs.userPrefsFlags & UPF_ShowOverlay)) {
			DrawBuiltInPict(150,228,17);
			DrawBuiltInPict(151,344,193);
		}
// #endif
/**		picH = GetResource('PICT',130);
		if (picH) {
			// Test Removed 7/4/96 JAB
			pr =(* ((PicHandle) picH) )->picFrame;
			OffsetRect(&pr,gCenterP.h-pr.right/2,gCenterP.v-pr.bottom/2);

			DrawPicture((PicHandle) picH, &pr);

			ReleaseResource(picH);
		}
**/	
		TextFont(geneva);
		TextSize(9);
		TextMode(srcBic);
		y = 44;
		x = 340;
		RGBBackColor(&gGrayAA);

		GetSerialNumberString((char *) tStr);
#if RAINBOWRED
		strcat((char *) tStr, " (RAINBOW ALLEY)");
#endif
		GetOwnerString((char *) tStr);
		// 1/14/97 JAB - changed to use "isRegistered" instead of "guestAccess"
		if (gSecure.isRegistered && tStr[0]) {
			MoveTo(x - StringWidth(labelStr)/2,y - 33);
			DrawString(labelStr);
			CtoPstr((char *) tStr);
			MoveTo(x - StringWidth(tStr)/2,y - 22);
			DrawString(tStr);
		}

		// MoveTo(392,336);
		MoveTo(24,370);
		versHandle = GetResource('vers',1);
		if (versHandle) {
			HLock(versHandle);
			DrawString("\pVersion ");
			DrawString((StringPtr) ((*versHandle) + 6));
			HUnlock(versHandle);
			ReleaseResource(versHandle);
			// 10/2/96 Show Origination Code if there is one
			if ((gPrefs.userPrefsFlags & UPF_ShowOrigCode) && gPrefs.originationCode[0]) {
				DrawString("\p - ");
				DrawString(gPrefs.originationCode);
			}
		}

		RGBBackColor(&gWhiteColor);
		TextMode(srcOr);
	}
	SetGWorld(gRoomWin->offWorld,NULL);
	sr = gOffscreenRect;
	dr = gOffscreenRect;

	// Copy the room background art
	CopyBits((BitMap *) *gRoomWin->offPictureMap, (BitMap *) *gRoomWin->offPixMap,
			 &sr, &dr, srcCopy, NULL);

EarlyExit:
	SetGWorld(curPort,curDevice);
	gSplashCheck = 0;
	gRecordCoords = 0;
}

#define MaxGlint		4
#define NbrStars		4
#define NbrPoints		7
#define FlashSeconds	3L

void DrawStar(Point p, short len);

void SplashIdle();

void SplashIdle()
{
	static char firstTime = 1;
	static short state=0;
	static short sCnt=0,gCnt=0;
	static unsigned long nextCount=0;
	static Point sP[NbrPoints] = {236,86, 
								  40,156, 
								  278, 181, 
								  192, 242, 
								  151, 289, 
								  197, 336, 
								  135, 437};
	static short gLen[] = {1,2,3,4,3,2,1};
	switch (state) {
	case 0:
		if (TickCount() >= nextCount) {
			++state;
			nextCount = TickCount() + 10;
		}
		gCnt = 0;
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		if (TickCount() >= nextCount) {
			DrawStar(sP[sCnt],gLen[gCnt++]);
			++state;
			nextCount = TickCount() + 5;
		}
		break;
	case 8:
		DrawStar(sP[sCnt],0);
		state = 0;
		++sCnt;
		if (MyRandom(2) == 0)
			++sCnt;
		if (sCnt >= NbrPoints) {
			sCnt = MyRandom(2);
			nextCount = GetTicks() + FlashSeconds * TICK_SECONDS;
		}
		else
			nextCount = 0;
		break;
	}
	if (firstTime) {
		firstTime = 0;
#if !RAINBOWRED
		// Dropped - problem - if Palace is passed a URL from netscape,
		// this still triggers, which we don't want. Also a problem
		// for folks doing automatic startups
		// OpenTCPSession(C_PalaceTCP);	// Fix to keep track of last connect type and use

		// Guest?  Automatically show Registration Info...
		// 1/14/97 JAB - changed to use "isRegistered" instead of "guestAccess"
		if (!gSecure.isRegistered && !(gPrefs.userPrefsFlags & UPF_DontShowRegInfo))
		{
			RegInfo();
		}
#endif
	}
}

void DrawStar(Point p, short len)
{
	Rect	r,dr;
	CGrafPtr	curPort;
	GDHandle	curDevice;
	GetGWorld(&curPort,&curDevice);
	SetGWorld(gRoomWin->offWorld,NULL);

	SetRect(&r,p.h,p.v,p.h+1,p.v+1);
	InsetRect(&r,-MaxGlint,-MaxGlint);
	CopyBits((BitMap *) *gRoomWin->offPictureMap, (BitMap *) *gRoomWin->offPixMap,
			 &r, &r, srcCopy, NULL);
	if (len) {
		RGBForeColor(&gWhiteColor);
		if (len & 1) {
			MoveTo(p.h,p.v-len);	LineTo(p.h,p.v+len);
			MoveTo(p.h-len,p.v);	LineTo(p.h+len,p.v);
		}
		else {
			MoveTo(p.h-len,p.v-len);	LineTo(p.h+len,p.v+len);
			MoveTo(p.h-len,p.v+len);	LineTo(p.h+len,p.v-len);
		}
		RGBForeColor(&gBlackColor);
	}
	SetGWorld(curPort,curDevice);
	SetPort((WindowPtr) gRoomWin);
	dr = r;
	OffsetRect(&dr,gVideoRoomRect.left,gVideoRoomRect.top);
	CopyBits((BitMap *) *gRoomWin->offPixMap, &((WindowPtr) gRoomWin)->portBits, 
		&r, &dr, srcCopy, NULL);
}


void RefreshSplash(void)
{

	PrepareSplash();
	RefreshRoom(&gOffscreenRect);
	DrawRoomStatus();
}

