// DIBRender.c

#include "U-USER.H"
#include "DIB.h"

// Note: first two bytes of DIB are 'BM'
//

#define WordSwap(x)		SwapShort((unsigned short *) x)
#define DWordSwap(x)	SwapLong((unsigned long *) x)

void GetPictureFrame(Handle h, Rect *r);

void GetPictureFrame(Handle h, Rect *r)
{
	if (IsDIB(h)) {
		BITMAPFILEHEADER	bmHead;
		BITMAPINFOHEADER	bmInfo;
		Ptr					p;
		long				count;
		count = sizeof(BITMAPFILEHEADER);
		p = *h;
		BlockMove(p,&bmHead,count);
		p += count;
		DWordSwap(&bmHead.bfSize);
		DWordSwap(&bmHead.bfOffBits);
		count = sizeof(long);
		BlockMove(p,&bmInfo,count);
		p += count;
		DWordSwap(&bmInfo.biSize);
		count = bmInfo.biSize - sizeof(long);
		BlockMove(p,&bmInfo.biWidth,count);
		p += count;
		if (bmInfo.biSize == 0x0C) {
			SMALLINFOHEADER	siTmp;
			BlockMove(&bmInfo,&siTmp,sizeof(SMALLINFOHEADER));
			WordSwap(&siTmp.biWidth);
			WordSwap(&siTmp.biHeight);
			bmInfo.biWidth = siTmp.biWidth;
			bmInfo.biHeight = siTmp.biHeight;
		}
		else {
			DWordSwap(&bmInfo.biWidth);
			DWordSwap(&bmInfo.biHeight);
		}
		SetRect(r,0,0,bmInfo.biWidth,bmInfo.biHeight);
	}
	else if (IsGIF(h)) {
		unsigned short	width,height;
		width = *((short *) (*h + 6));
		height = *((short *) (*h + 8));
		SwapShort(&width);
		SwapShort(&height);
		SetRect(r,0,0,width,height);
	}
	else if (IsJPEG(h)) {
		GetJPEGFrame(h,r);
	}
	else
		*r = (*((PicHandle) h))->picFrame;
}

Boolean IsDIB(Handle h)
{
	short	type;
	long	size;
	Ptr		p;
	p = 	*h;
	type = *((short *) p);
	size = *((long *) (p+2));
	DWordSwap(&size);
	return (type == 'BM' && size > 0 && size < 400000L);
}

// This function no longer assumes the GWorld is already correct for the pMap
//
short DIBRender(Handle h, GWorldPtr	pWorld, Point offset)
{
	unsigned char		*p;
	short				err = 0;
	long				count;
	BITMAPFILEHEADER	bmHead;
	BITMAPINFOHEADER	bmInfo;
	unsigned char		clutTrans[256];
	unsigned short		r,g,b;
	RGBColor			rgb;
	long				dRowBytes;
	short				col,row;
	unsigned char		*dp;
	unsigned char		*baseAddr;
	PixMapHandle		pMap = GetGWorldPixMap(pWorld);
	CGrafPtr			curPort;
	GDHandle			curDevice;

	GetGWorld(&curPort,&curDevice);
	SetGWorld(pWorld, NULL);

	count = sizeof(BITMAPFILEHEADER);
	p = (unsigned char *) *h;
	BlockMove(p,&bmHead,count);
	p += count;
	DWordSwap(&bmHead.bfSize);
	DWordSwap(&bmHead.bfOffBits);
	if (bmHead.bfType != 'BM') {
		err = -1;
		goto ErrorExit;
	}
	count = sizeof(long);
	BlockMove(p,&bmInfo,count);
	p += count;
	DWordSwap(&bmInfo.biSize);
	count = bmInfo.biSize - sizeof(long);
	BlockMove(p,&bmInfo.biWidth,count);
	p += count;
	if (bmInfo.biSize == 0x0C) {
		SMALLINFOHEADER	siTmp;
		BlockMove(&bmInfo,&siTmp,sizeof(SMALLINFOHEADER));
		WordSwap(&siTmp.biWidth);
		WordSwap(&siTmp.biHeight);
		WordSwap(&siTmp.biPlanes);
		WordSwap(&siTmp.biBitCount);
		bmInfo.biWidth = siTmp.biWidth;
		bmInfo.biHeight = siTmp.biHeight;
		bmInfo.biPlanes = siTmp.biPlanes;
		bmInfo.biBitCount = siTmp.biBitCount;
		bmInfo.biCompression = 0L;
		bmInfo.biSizeImage = bmHead.bfSize - bmHead.bfOffBits;
		bmInfo.biXPelsPerMeter = 0L;
		bmInfo.biYPelsPerMeter = 0L;
		bmInfo.biClrUsed = (bmHead.bfOffBits - (sizeof(BITMAPFILEHEADER) + bmInfo.biSize)) / 3;
		bmInfo.biClrImportant = 0L;
	}
	else {
		DWordSwap(&bmInfo.biWidth);
		DWordSwap(&bmInfo.biHeight);
		WordSwap(&bmInfo.biPlanes);
		WordSwap(&bmInfo.biBitCount);
		DWordSwap(&bmInfo.biCompression);
		DWordSwap(&bmInfo.biSizeImage);
		DWordSwap(&bmInfo.biClrUsed);
		DWordSwap(&bmInfo.biClrImportant);
		if (bmInfo.biClrUsed == 0)
			bmInfo.biClrUsed = 256;
	}
	count = bmHead.bfOffBits - (sizeof(BITMAPFILEHEADER) + bmInfo.biSize);
	{
		unsigned char *cp;
		short			i;

		cp = (unsigned char *) p;
		
		for (i = 0; i < 256; ++i) {
			if (i < bmInfo.biClrUsed) {
				b = *(cp++);
				g = *(cp++);
				r = *(cp++);
				if (bmInfo.biSize != 0x0C)
					cp++;	// RGB Quad
				rgb.red = (r << 8) | r;
				rgb.green = (g << 8) | g;
				rgb.blue = (b << 8) | b;
				clutTrans[i] = Color2Index(&rgb);
			}
			else
				clutTrans[i] = 0;
		}
	}
	p += count;
	// Read Each Row...
	dRowBytes = (*pMap)->rowBytes & 0x3FFF;
	baseAddr = (unsigned char *) GetPixBaseAddr(pMap);

	if (bmInfo.biCompression == BI_RLE8 && baseAddr) {
		unsigned char	rCode[2],pixel;

		row = bmInfo.biHeight-1;
		col = 0;
		dp = baseAddr + (dRowBytes * (row+offset.v) + offset.h);
		while (row >= 0) {
			*((short *) &rCode[0]) = *((short *) p);
			p += 2;
			if (rCode[0] == 0) {
				switch (rCode[1]) {
				case 0:
					--row;
					if (row < 0)
						goto DoneExit;
					col = 0;
					dp = baseAddr + (dRowBytes * (row+offset.v) + offset.h);
					break;
				case 1:
					goto DoneExit;
					break;
				case 2:
					*((short *) &rCode[0]) = *((short *) p);
					col += rCode[0];
					row -= rCode[1];
					dp = baseAddr + (dRowBytes * (row+offset.v) + offset.h);
					dp += col;
					break;	// 3/9/93 Added break JAB
				default:
					count = rCode[1];
					col += count;
					while (count--) {
						*(dp++) = clutTrans[*(p++)];
					}
					if (rCode[1] & 1)
						++p;
				}
			}
			else {
				count = rCode[0];
				col += count;
				pixel= clutTrans[rCode[1]];
				while (count--)
					*(dp++) = pixel;
			}
		}
	}
	else if (baseAddr) {
		if (bmInfo.biBitCount != 8)
			goto DoneExit;
		for (row =  bmInfo.biHeight-1; row >= 0; row--)
		{
			dp = baseAddr + (dRowBytes * (row+offset.v) + offset.h);
			BlockMove(p,dp,bmInfo.biWidth);
			p += bmInfo.biWidth;
		}
	}
DoneExit:
ErrorExit:
	SetGWorld(curPort, curDevice);
	return err;
}