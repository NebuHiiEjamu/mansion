// ServGraphics.c
#include "S-SERVER.H"
#include "UserTools.h"	// 6/10/95
#include "ServGraphics.h"

CTabHandle	gCurCLUT;
RGBColor	gBlackColor = 	{0,0,0};
RGBColor	gWhiteColor	= 	{0xFFFF,0xFFFF,0xFFFF};
RGBColor	gGrayColor =  	{0x8888,0x8888,0x8888};
RGBColor	gGrayAA =  		{0xAAAA,0xAAAA,0xAAAA};
RGBColor	gGray66 =  		{0x6666,0x6666,0x6666};
RGBColor	gGray44 =  		{0x4444,0x4444,0x4444};
RGBColor	gGray22 =  		{0x2222,0x2222,0x2222};
IconSetup	*gIconSetup;

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

void ColorPaintRect(Rect *r, RGBColor *fc)
{
	RGBForeColor(fc);
	PaintRect(r);
	RGBForeColor(&gBlackColor);
}

