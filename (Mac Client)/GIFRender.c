// GIFRender.c
#include "U-USER.H"

Boolean IsGIF(Handle h)
{
	long	type;
	Ptr		p;
	p = 	*h;
	type = *((long *) p);
	return (type == 'GIF8');
}

void DrawGIF (Ptr data, long FileSize, long worldFlags, GWorldPtr *world, short *transIndex);

// This function assumes the GWorld is already correct for the pMap
short GIFRender(Handle h, GWorldPtr	pWorld, Point offset, short *transIndex)
{
	long			size;
	GWorldPtr		myWorld=NULL;
	PixMapHandle	myMap,pMap;
	Rect			sr,dr;
	short			transIdx=-1;
	RGBColor		transColor;
	CGrafPtr		saveWorld;
	GDHandle		saveDevice;
	// RGBColor		sf,sb;


	// return noErr;  test test 


	HLock(h);
	size = GetHandleSize(h);

	// 8/1/96 JAB Bug Fix
	// DrawGIF calls NewGWorld, which crashes in > 256 colors if the
	// current GWorld is offscreen. We fixed this routine to not
	// set the GWorld to pWorld to after the GIF is retreived.
	//
	DrawGIF(*h, size, 0, &myWorld,&transIdx);

	if (myWorld == NULL) {
		SysBeep(1);
		return noErr;
	}
	GetGWorld(&saveWorld,&saveDevice);			// 7/12/95 MOVED to prevent log msg problems
	SetGWorld(pWorld, NULL);
	pMap = GetGWorldPixMap(pWorld);

	if (transIdx != -1) {
		myMap = GetGWorldPixMap(myWorld);
		LockPixels(myMap);	// 10/11/95
		transColor = (*(*myMap)->pmTable)->ctTable[transIdx].rgb;
		UnlockPixels(myMap);
// 		10/11/95
//		slotColor = (*gCurCLUT)->ctTable[transIdx].rgb;
//		if (slotColor.red == transColor.red &&
//			slotColor.green == transColor.green &&
//			slotColor.blue == transColor.blue)
//			*transIndex = transIdx;
//		else
			*transIndex = Color2Index(&transColor);
	}

	if (myWorld) {
		myMap = GetGWorldPixMap(myWorld);
		LockPixels(myMap);
		sr = myWorld->portRect;
		dr = (*pMap)->bounds;
		OffsetRect(&dr,offset.h,offset.v);
		CopyBits((BitMap *) *myMap, (BitMap *) *pMap,
					&sr, &dr, srcCopy, NULL);
		DisposeGWorld(myWorld);
	}

	SetGWorld(saveWorld, saveDevice);

	return noErr;
}
