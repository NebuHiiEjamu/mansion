// Balloon Text

#include "U-USER.H"
#include <math.h>
#include "U-SNDS.H"
#include "U-SCRIPT.H"		// 4/6/95

extern BalloonRec balloons[];
extern short		nbrBalloons;
extern long		   readingSpeedTicks[];

TEHandle	balTEH;
Rect		balRect;
short		textHeight;

RGBColor	gBalloonColors[] = {0xffff,0x7f7f,0x7f7f,
								0xffff,0xafaf,0x7f7f,
								0xffff,0xe0e0,0x7f7f,

								0xefef,0xffff,0x7f7f,
								0xbebe,0xffff,0x7f7f,
								0x8e8e,0xffff,0x7f7f,
								0x7f7f,0xffff,0xa0a0,
								0x7f7f,0xffff,0xd1d1,
								0x7f7f,0xfefe,0xffff,
								0x7f7f,0xcece,0xffff,
								0x7f7f,0x9d9d,0xffff,
								0x9191,0x7f7f,0xffff,
								0xc2c2,0x7f7f,0xffff,
								0xf2f2,0x7f7f,0xffff,
								0xffff,0x7f7f,0xdddd,
								0xffff,0x7f7f,0xacac};


void InitBalloons(void)
{
	// Font & Port has already been set...
	SetRect(&balRect,0,0,16*6,RoomHeight);
	balTEH = TENew(&balRect,&balRect);
}

static	GDHandle	gCurDevice;
static	CGrafPtr	gCurPort;

void OffscreenTextEnable()
{
	GetGWorld(&gCurPort,&gCurDevice);
	SetGWorld(gRoomWin->offWorld,NULL);
}

void OffscreenTextRestore()
{
	SetGWorld(gCurPort,gCurDevice);
}

void GetBalloonRect(Rect *br, Ptr text, long len, Boolean privateFlag)
{
	Rect		r;
	short		tw,th;

	OffscreenTextEnable();

	ChangeBalloonFont(gPrefs.fontID, gPrefs.fontSize);

	if (privateFlag) {
		TextFace(italic);
		(*balTEH)->txFace = italic;
	}
	TESetText(text,len,balTEH);
	if ((*balTEH)->nLines == 1)
		tw = TextWidth(text,0,len)+2;		// 4/6/95 JBUM Fixed bug - balloon was too narrow.
	else
		tw = 16*6;
	if (privateFlag) {
		TextFace(0);
		(*balTEH)->txFace = 0;
	}
	th = (*balTEH)->nLines * (*balTEH)->lineHeight;
	SetRect(&r,0,0,tw+BalloonMargin*2,th+BalloonMargin*2);
	*br = r;

	OffscreenTextRestore();
}

RgnHandle	ExcitedRegion(Rect *r)
{
	short		hp,vp,hw,vw,x,y,px,py;
	RgnHandle	rh;

	hp = (r->right - r->left)/SpikeWidth;
	if (hp < 2)	// 8/14/95 if less than 2 we get a zero divide
		hp = 2;
	vp = (r->bottom - r->top)/SpikeWidth;
	if (vp < 2)	// 8/14/95 if less than 2 we get a zero divide
		vp = 2;
	hw = (r->right - r->left)/hp;
	vw = (r->bottom - r->top)/vp;
	rh = NewRgn();
	OpenRgn();
	MoveTo(r->left,r->top);
	for (x = 0; x < hp; ++x) {
		px = r->left+x*hw+hw/2+(x*SpikeDisplace)/(hp-1)-SpikeDisplace/2;
		py = r->top-SpikeHeight;
		// px += MyRandom(RandomAmplitude)-(RandomAmplitude>>1);
		// py += MyRandom(RandomAmplitude)-(RandomAmplitude>>1);
		LineTo(px,py);
		LineTo(r->left+x*hw+hw,r->top);
	}
	for (y = 0; y < vp; ++y) {
		px = r->right+SpikeHeight;
		py = r->top+y*vw+vw/2+(y*SpikeDisplace)/(vp-1)-SpikeDisplace/2;
		// px += MyRandom(RandomAmplitude)-(RandomAmplitude>>1);
		// py += MyRandom(RandomAmplitude)-(RandomAmplitude>>1);
		LineTo(px,py);
		LineTo(r->right,r->top+y*vw+vw);
	}
	for (x = 0; x < hp; ++x) {
		px = r->right-x*hw-hw/2+((hp-1-x)*SpikeDisplace)/(hp-1)-SpikeDisplace/2;
		py = r->bottom+SpikeHeight;
		// px += MyRandom(RandomAmplitude)-(RandomAmplitude>>1);
		// py += MyRandom(RandomAmplitude)-(RandomAmplitude>>1);
		LineTo(px,py);
		LineTo(r->right-x*hw-hw,r->bottom);
	}
	for (y = 0; y < vp; ++y) {
		px = r->left-SpikeHeight;
		py = r->bottom-y*vw-vw/2+((vp-1-y)*SpikeDisplace)/(vp-1)-SpikeDisplace/2;
		// px += MyRandom(RandomAmplitude)-(RandomAmplitude>>1);
		// py += MyRandom(RandomAmplitude)-(RandomAmplitude>>1);
		LineTo(px,py);
		LineTo(r->left,r->bottom-y*vw-vw);
	}
	LineTo(r->left,r->top);
	CloseRgn(rh);
	return rh;
}

void DrawBalloons(Rect *rr)
{
	Rect		r,tr,sr;
	short		i,ow;
	PolyHandle	p;
	Point		pAnchor,pOffset;
	BalloonRec	*bPtr;
	// return;

	bPtr = balloons;
	for (i = 0; i < nbrBalloons; ++i,++bPtr) {
		if (bPtr->active && SectRect(&bPtr->rr,rr,&tr)) {
			if (gPrefs.userPrefsFlags & UPF_TintedBalloons) {
				if (bPtr->tint >= 0 && bPtr->tint < 16)
					RGBBackColor(&gBalloonColors[bPtr->tint]);
				else
					RGBBackColor(&gWhiteColor);
			}
			r = bPtr->r;
			if (r.left > bPtr->target.h) {
				pOffset.h = 1;
				pOffset.v = 0;
				if (r.top >= bPtr->target.v) {
					pAnchor.h = r.left+3;
					pAnchor.v = r.top+11;
				}
				else {
					// 6/21 - better balloon anchor
					pAnchor.h = r.left+3;
					pAnchor.v = bPtr->target.v+11;
					if (pAnchor.v > r.bottom-11)
						pAnchor.v = r.bottom-11;
					// pAnchor.h = r.left+3;
					// pAnchor.v = r.bottom-11;
				}
			}
			else {
				pOffset.h = -1;
				pOffset.v = 0;
				if (r.top >= bPtr->target.v) {
					pAnchor.h = r.right-3;
					pAnchor.v = r.top+11;
				}
				else {
					// 6/21/95 - better balloon anchor
					pAnchor.h = r.right-3;
					pAnchor.v = bPtr->target.v+11;
					if (pAnchor.v > r.bottom-11)
						pAnchor.v = r.bottom-11;
					// pAnchor.h = r.right-3;
					// pAnchor.v = r.bottom-11;
				}
			}

			if (bPtr->excited) {
				RgnHandle	rh;
				rh = ExcitedRegion(&r);
				OffsetRgn(rh,1,1);
				FrameRgn(rh);
				OffsetRgn(rh,-1,-1);
				FrameRgn(rh);
				InsetRgn(rh,1,1);
				EraseRgn(rh);
				DisposeRgn(rh);
			}
			else if (bPtr->perm) {		// 6/1/95 Perm Balloon is simple rectangle
				r.right += 1;
				r.bottom += 1;
				PenSize(2,2);
				FrameRect(&r);
				PenSize(1,1);
				r.right -= 1;
				r.bottom -= 1;
				InsetRect(&r,1,1);
				EraseRect(&r);
				InsetRect(&r,-1,-1);
			}
			else { 
				if (!bPtr->thought) {	// Create Pointed Anchor
					p = OpenPoly();
					MoveTo(pAnchor.h,pAnchor.v-PointHeight/2);
					LineTo(pAnchor.h,pAnchor.v+PointHeight/2);
					LineTo((pAnchor.h+bPtr->target.h)/2,
						   (pAnchor.v+bPtr->target.v)/2);
					LineTo(pAnchor.h,pAnchor.v-PointHeight/2);
					ClosePoly();
					OffsetPoly(p,1,1);
					FramePoly(p);
					OffsetPoly(p,-1,-1);
					ErasePoly(p);
					FramePoly(p);
				}
				else
					p = NULL;
				ow = 32;	// 6/15/95

				PenSize(2,2);
				r.right += 1;
				r.bottom += 1;
				FrameRoundRect(&r,ow,ow);
				PenSize(1,1);
				r.right -= 1;
				r.bottom -= 1;
				// OffsetRect(&r,1,1);
				// FrameRoundRect(&r,ow,ow);
				// OffsetRect(&r,-1,-1);
				// FrameRoundRect(&r,ow,ow);

				InsetRect(&r,1,1);
				EraseRoundRect(&r,ow,ow);
	
				if (!bPtr->thought) {
					OffsetPoly(p,pOffset.h,pOffset.v);
					ErasePoly(p);
					KillPoly(p);
				}
				else {
					Rect	cr;
					SetRect(&cr,-6,-6,6,6);
					OffsetRect(&cr,pAnchor.h-pOffset.h*2,pAnchor.v-12);

					FrameOval(&cr);
					InsetRect(&cr,1,1);
					EraseOval(&cr);
	
					SetRect(&cr,-4,-4,4,4);
					OffsetRect(&cr,(pAnchor.h+bPtr->target.h)/2+pOffset.h*4,
							       (pAnchor.v+bPtr->target.v)/2-12);
					FrameOval(&cr);
					InsetRect(&cr,1,1);
					EraseOval(&cr);
				}
			}
			InsetRect(&r, BalloonMargin-1,BalloonMargin-1);
			sr = (*balTEH)->destRect;
			(*balTEH)->destRect = r;
			(*balTEH)->viewRect = r;
			if (bPtr->private)
				(*balTEH)->txFace = italic;
			TESetText(bPtr->str,bPtr->len,balTEH);
			TEUpdate(&r,balTEH);
			if (bPtr->private)
				(*balTEH)->txFace = 0;
			(*balTEH)->destRect = sr;
			(*balTEH)->viewRect = sr;
		}
	}
	RGBBackColor(&gWhiteColor);
}


void ChangeBalloonFont(short fontID, short fontSize)
{
	FontInfo	fi;
	CGrafPtr	curPort;
	GDHandle	curDevice;

	GetGWorld(&curPort,&curDevice);
	SetGWorld(gRoomWin->offWorld,NULL);
	(*balTEH)->txFont = fontID;
	(*balTEH)->txSize = fontSize;
	TextFont(fontID);
	TextSize(fontSize);
	GetFontInfo(&fi);
	textHeight = fi.ascent + fi.descent + fi.leading;
	(*balTEH)->lineHeight = textHeight;
	(*balTEH)->fontAscent = fi.ascent;
	SetGWorld(curPort,curDevice);
}

