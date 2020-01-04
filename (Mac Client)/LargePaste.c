/* LargePaste.c
 *
 * Routine to support pasting of large props on the Mac
 *
 */

#include "U-USER.H"
#include "AppMenus.h"

void PasteLarge(void);
Handle ConvertPixmapToProp(PixMapHandle pMap, short xOffset, short yOffset, 
							short maxwidth, short maxheight);

void PasteLarge(void)
{
	Handle			picH;
	long			scrapOffset;
	Rect			pictR,bigpropR,srcR,dstR,propR;
	GWorldPtr		offPict,offProp,offWork;
	PixMapHandle	pictMap,propMap,workMap;
	CGrafPtr		curPort;
	GDHandle		curDevice;
	Handle			propH;
	OSErr			oe;
	short			x,y,xOffset,yOffset,xo,yo;
	unsigned long	crc,t;
	long			length,propID;
	AssetSpec		propSpecs[MaxUserProps];
	int				propNbr;

	if (!CanPaste('PICT'))
		return;

	if (MembersOnly(true))
		return;

	// allocate buffers - preflight
	picH = NewHandleClear(1L);
	if (GetScrap(picH,'PICT',&scrapOffset) == 0) {
		ReportError(memFullErr,"LargePaste");
		return;
	}
	pictR = (*((PicHandle) picH))->picFrame;
	OffsetRect(&pictR,-pictR.left,-pictR.top);

	// Allocate RGB PixMap at Pict Size and Render PICT Into it
	if ((oe = NewGWorld(&offPict,32,&pictR,NULL,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		DisposeHandle(picH);
		ReportError(memFullErr,"LargePaste");
		return;
	}
	pictMap = GetGWorldPixMap(offPict);
	LockPixels(pictMap);
	GetGWorld(&curPort,&curDevice);
	SetGWorld(offPict,NULL);
	EraseRect(&pictR);
	DrawPicture((PicHandle) picH, &pictR);
	SetGWorld(curPort,curDevice);
	DisposeHandle(picH);
	SetGWorld(curPort,curDevice);

	// Allocate 8-bit PixMap at Prop Size and Copy PICT into it
	bigpropR = pictR;
	if (pictR.right > PropWidth*3 || pictR.bottom > PropHeight*3) {
		if (pictR.right > pictR.bottom) {
			SetRect(&bigpropR,0,0,PropWidth*3,(PropHeight*3*pictR.bottom)/pictR.right);
		}
		else {
			SetRect(&bigpropR,0,0,(PropWidth*3*pictR.right)/pictR.bottom,PropHeight*3);
		}
	}
	if ((oe = NewGWorld(&offProp,8,&bigpropR,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		DisposeGWorld(offPict);
		ReportError(memFullErr,"LargePaste");
		return;
	}
	propMap = GetGWorldPixMap(offProp);
	LockPixels(propMap);
	SetGWorld(offProp,NULL);
	CopyBits((BitMap *) *pictMap, (BitMap *) *propMap, &pictR, &bigpropR, ditherCopy, NULL);
	SetGWorld(curPort,curDevice);
	DisposeGWorld(offPict);

	// Compute base offsets for top left prop
	// X offset is simple - center it
	xOffset = (PropWidth - bigpropR.right) / 2;

	// Y offset is more complex:
	if (bigpropR.bottom >= PropHeight*2) {		// Grow large 3x props from top of frame
		yOffset = -PropHeight;
	}
	else if (bigpropR.bottom < PropHeight) {	// Grow 1x props from within face frame
		yOffset = 0;
	}
	else {									// Grow 2x props from baseline
		yOffset = PropHeight - bigpropR.bottom;
	}

	// Generate a working space for creating prop...
	SetRect(&propR,0,0,PropWidth,PropHeight);
	if ((oe = NewGWorld(&offWork,8,&propR,gCurCLUT,NULL,(GetMMUMode() == false32b ? keepLocal : 0))) != noErr) {
		ReportError(memFullErr,"LargePaste");
		return;
	}
	workMap = GetGWorldPixMap(offWork);
	LockPixels(workMap);


	GetDateTime(&t);	// 6/7/95 JAB
	propID = t;			// 6/7/95 JAB
	propNbr = 0;
	// Tile up into Props, insert into favorites
	for (y = 0; y < bigpropR.bottom; y += PropHeight) {
		for (x = 0; x < bigpropR.right; x += PropWidth) {

			SetRect(&srcR,x,y,x+PropWidth,y+PropHeight);
			yo = yOffset + y;
			xo = xOffset + x;
			dstR = propR;
			// Construct Prop using propMap,srcR,yo,xo
			if (srcR.right > bigpropR.right) {
				dstR.right -= (srcR.right - bigpropR.right);
				srcR.right -= (srcR.right - bigpropR.right);
			}
			if (srcR.bottom > bigpropR.bottom) {
				dstR.bottom -= (srcR.bottom - bigpropR.bottom);
				srcR.bottom -= (srcR.bottom - bigpropR.bottom);
			}
			SetGWorld(offWork,NULL);
			PaintRect(&propR);
			CopyBits((BitMap *) *propMap, (BitMap *) *workMap, &srcR, &dstR, srcCopy, NULL);
			SetGWorld(curPort,curDevice);
			propH = ConvertPixmapToProp(workMap,xo,yo,dstR.right-dstR.left,dstR.bottom-dstR.top);
			if (propH == NULL)
				continue;
			length = GetHandleSize(propH) - sizeof(PropHeader);		// 6/9/95
			if (length >= 0)
				crc = ComputeCRC(*propH+sizeof(PropHeader),length);
			else
				crc = 0;
			AddAsset(propH,'Prop',propID+propNbr,"\p");
			SetAssetCRC(propH, crc);
			propSpecs[propNbr].id = propID+propNbr;
			propSpecs[propNbr].crc = crc;
			++propNbr;
		}
	}

	// Add backwards to fave file (so they are listed from top to bottom)
	for (x = propNbr-1; x >= 0; --x) {
		AddPropFavorite(&propSpecs[x]);
	}

	UpdateAssetFile();
	DisposeGWorld(offWork);
	DisposeGWorld(offProp);
}


Handle ConvertPixmapToProp(PixMapHandle pMap, short xOffset, short yOffset,
							short maxwidth, short maxheight)
{
	Handle		dh;
	short		rp;
	unsigned char *pp,*cp,*dp;
	short		n,y,x;
	long		count;
	short		width=PropWidth,height=PropHeight;

	unsigned char *pBaseAddr;
	dh = NewHandleClear(32767L);	/* 7/1/96 JAB Changed to NewHandleClear */
	HLock(dh);
	dp = (unsigned char *) *dh;

	((PropHeaderPtr) dp)->width = width;
	((PropHeaderPtr) dp)->height = height;
	((PropHeaderPtr) dp)->hOffset = xOffset;
	((PropHeaderPtr) dp)->vOffset = yOffset;
	((PropHeaderPtr) dp)->scriptOffset = 0L;
	((PropHeaderPtr) dp)->flags = 0;

	dp += sizeof(PropHeader);
	pBaseAddr = (unsigned char *) GetPixBaseAddr(pMap);

	for (y = 0; y < height; ++y) {
		pp = pBaseAddr + ((*pMap)->rowBytes&0x3FFF) * y;
		rp = width;
		x = 0;

		while (rp) {
			cp = dp;		// Store Count Bit
			++dp;
			*cp = 0;
			// store run of pixels (up to 16)
			if (y >= maxheight || x >= maxwidth) {
				n = 0;
				while (n < 15 && rp && (y >= maxheight || x >= maxwidth)) {
					++pp;
					++n;
					--rp;
					++x;
				}
				*cp |= (n << 4);
			}
			else {
				n = 0;
				while (n < 15 && rp && x < maxwidth) {
					*(dp++) = *(pp++);
					++n;
					--rp;
					++x;
				}
				*cp |= n;
			}
		}
	}
	count = (long) dp - (long) *dh;
	HUnlock(dh);
	SetHandleSize(dh,count);
	return dh;
}
