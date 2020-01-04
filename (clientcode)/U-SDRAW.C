// Script-Graphics
#include	"U-SFUNCS.H"
#include	"U-USER.H"
#include 	"U-SCRIPT.H"

RGBColor	gPenColor;
short		gPenSize=1;
Point		gPenPos={384/2,512/2};
short		gPenFrontLayer;

void ScriptLineDraw(short x, short y, short x2, short y2);

void ScriptLineDraw(short x, short y, short x2, short y2)
{
	char		*sp;
	DrawRecPtr	dp;

	if (x < 0)		x = 0;
	if (x >= 512)	x = 511;
	if (y < 0)		y = 0;
	if (y >= 384)	y = 383;
	if (x2 < 0)		x2 = 0;
	if (x2 >= 512)	x2 = 511;
	if (y2 < 0)		y2 = 0;
	if (y2 >= 384)	y2 = 383;

	sp = gPV.tBuf;
	dp = (DrawRecPtr) sp;	sp += sizeof(DrawRecord);

	dp->link.nextOfst = 0;
	dp->drawCmd = DC_Path | (gPenFrontLayer? 0x8000 : 0x0000);
	*((short *) sp) = gPenSize;			sp += sizeof(short);
	*((short *) sp) = 1;				sp += sizeof(short);	// nbr pts - 1
	*((RGBColor *) sp) = gPenColor;		sp += sizeof(RGBColor);
	*((short *) sp) = y;				sp += sizeof(short);
	*((short *) sp) = x;				sp += sizeof(short);
	*((short *) sp) = y2-y;				sp += sizeof(short);
	*((short *) sp) = x2-x;				sp += sizeof(short);
	dp->cmdLength = 28;
	dp->cmdLength -= sizeof(DrawRecord);
	dp->dataOfst = 0;
	PostServerEvent(MSG_DRAW, gRoomWin->meID, (Ptr) gPV.tBuf, sizeof(DrawRecord) + dp->cmdLength);
}

void SF_PENFRONT()
{
	gPenFrontLayer = true;
}

void SF_PENBACK()
{
	gPenFrontLayer = false;
}

void SF_PENCOLOR()
{
	short	rType,gType,bType;
	long	rValue,gValue,bValue;
	if ((bType = PopAtom(&bValue)) == V_Error)
		return;
	AtomToValue(&bType,&bValue);
	if ((gType = PopAtom(&gValue)) == V_Error)
		return;
	AtomToValue(&gType,&gValue);
	if ((rType = PopAtom(&rValue)) == V_Error)
		return;
	AtomToValue(&rType,&rValue);
	gPenColor.red   = (short)(rValue | (rValue << 8));
	gPenColor.green = (short)(gValue | (gValue << 8));
	gPenColor.blue  = (short)(bValue | (bValue << 8));
}

void SF_PENSIZE()
{
	short	xType;
	long	xValue;
	if ((xType = PopAtom(&xValue)) == V_Error)
		return;
	AtomToValue(&xType,&xValue);
	gPenSize = (short)xValue;
	if (gPenSize < 1)
		gPenSize = 1;
	if (gPenSize > MaxPenSize)
		gPenSize = MaxPenSize;
}

void SF_PENPOS()
{
	short	xType,yType;
	long	xValue,yValue;
	if ((yType = PopAtom(&yValue)) == V_Error)
		return;
	AtomToValue(&yType,&yValue);
	if ((xType = PopAtom(&xValue)) == V_Error)
		return;
	AtomToValue(&xType,&xValue);
	gPenPos.v = (short)yValue;
	gPenPos.h = (short)xValue;
}

void SF_PENTO()
{
	short	xType,yType;
	long	xValue,yValue;
	if ((yType = PopAtom(&yValue)) == V_Error)
		return;
	AtomToValue(&yType,&yValue);
	if ((xType = PopAtom(&xValue)) == V_Error)
		return;
	AtomToValue(&xType,&xValue);
	gPenPos.v += (short)yValue;
	gPenPos.h += (short)xValue;
}

void SF_LINETO()
{
	short	xType,yType;
	long	xValue,yValue;
	if ((yType = PopAtom(&yValue)) == V_Error)
		return;
	AtomToValue(&yType,&yValue);
	if ((xType = PopAtom(&xValue)) == V_Error)
		return;
	AtomToValue(&xType,&xValue);
	ScriptLineDraw(gPenPos.h,gPenPos.v,gPenPos.h+(short)xValue,gPenPos.v+(short)yValue);
	gPenPos.v += (short)yValue;
	gPenPos.h += (short)xValue;
}

void SF_LINE()
{
	short	xType1,yType1,xType2,yType2;
	long	xValue1,yValue1,xValue2,yValue2;

	if ((yType2 = PopAtom(&yValue2)) == V_Error)
		return;
	AtomToValue(&yType2,&yValue2);

	if ((xType2 = PopAtom(&xValue2)) == V_Error)
		return;
	AtomToValue(&xType2,&xValue2);

	if ((yType1 = PopAtom(&yValue1)) == V_Error)
		return;
	AtomToValue(&yType1,&yValue1);

	if ((xType1 = PopAtom(&xValue1)) == V_Error)
		return;
	AtomToValue(&xType1,&xValue1);

	ScriptLineDraw((short)xValue1,(short)yValue1,(short)xValue2,(short)yValue2);
	gPenPos.v = (short)yValue2;
	gPenPos.h = (short)xValue2;
}

void SF_PAINTUNDO()
{
	DrawRecord	dp;
	dp.link.nextOfst = 0;
	dp.drawCmd = DC_Delete;
	dp.cmdLength = 0;
	dp.dataOfst = 0;
	PostServerEvent(MSG_DRAW, gRoomWin->meID, (Ptr) &dp, sizeof(DrawRecord));
}

void SF_PAINTCLEAR()
{
	DrawRecord	dp;
	dp.link.nextOfst = 0;
	dp.drawCmd = DC_Detonate;
	dp.cmdLength = 0;
	dp.dataOfst = 0;
	PostServerEvent(MSG_DRAW, gRoomWin->meID, (Ptr) &dp, sizeof(DrawRecord));
}

