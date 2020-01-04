// ServGraphics.h

typedef struct IconSetup
{
	short	nbrTools;
	short	firstTool;
	short	nbrFaces;
	short	firstFace;
	short	nbrProps;
	short	firstProp;
	Point	propOffsets[1];
} IconSetup;


enum {	KillIcon, KillHiIcon, BanIcon, BanHiIcon, NbrToolIcons};



extern CTabHandle	gCurCLUT;
extern RGBColor		gBlackColor;
extern RGBColor		gWhiteColor;
extern RGBColor		gGrayColor;
extern RGBColor		gGrayAA;
extern RGBColor		gGray66;
extern RGBColor		gGray44;
extern RGBColor		gGray22;
extern IconSetup	*gIconSetup;


void HiliteRect(Rect *r);
void RevHiliteRect(Rect *r);
void ColorPaintRect(Rect *r, RGBColor *fc);
