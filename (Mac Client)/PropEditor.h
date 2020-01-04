// PropEditor.h
#include "PropEditorLocal.h"

#include "M-ASSETS.H"
#include <QDOffscreen.h>

#define		PEWIND			132
#define		PropPixelSize	4
#define		PalWidth		40		// 4/4/95 JBUM
#define		PalHeight		180		// 4/4/95 JBUM

// Prop Tools
enum	{PT_Pencil, PT_Select, PT_Eraser, PT_Linesize, 
		 PT_Cancel, PT_Save, NbrPropTools};

typedef struct {
	ObjectWindowRecord	win;
	Point			propOffset,oldPropOffset;		// Prop Offset (relative to head)
	short			propFlags,oldPropFlags;
	GWorldPtr		offProp,		// Current Prop
					offPropMask,	// It's Mask
					offComposite,	// Composite of Head & Prop (3x3 tiles)
					palWorld,		// Color Palette graphic
					offSel,			// The Current Selection
					offSelMask,
					offUndo,
					offUndoMask;		// It's Mask
	PixMapHandle	propMap,		// (PixelMaps of above)
					propMaskMap,
					compositeMap,
					palMap,
					selMap,
					selMaskMap,
					undoMap,
					undoMaskMap;
	AssetSpec		propSpec;		// Prop ID# and CRC#
	AssetSpec		oldSpec;		// Original ID# and CRC*
	Boolean			cancel;			// Set if Cancel Button Hit
	Rect			palRect;		// Palette Rect in Graph Port
	Rect			propDispRect,	// Rectangle for Editing
					propPreviewRect,	// Rectangle for Preview
					propRect,		// Prop Rectangle
					propFatRect,	// Prop Display Rectangle
					propCompRect,	// Prop Composite Rect
					propScreenRect;	// Prop Onscreen Rect
	Str63			name,oldName;			// Prop Name
	Rect			selectR;		// Selection Rectangle (relative to prop)
	short			curTool;		// Current Editing Tool
	unsigned char	maskWhite,maskBlack;	// Mask Colors
	short			backgroundNbr;
	TEHandle		nameTEH;
	Rect			nameRect,nameFrame;
	Boolean			nameActive;
	Boolean			hasUndo;
} PEWindowRecord, *PEWindowPtr;

extern WindowPtr	gPEWin;
extern PEWindowPtr	gPEPtr;

void InitPropEditor(void);
void IdleSelection(void);
void PEIdle(WindowPtr theWin, EventRecord *theEvent);
void ClearPropSelection(void);
void PropSelectTool(Point p, EventRecord *theEvent);
void RefreshProp(void);
void RectToScreenRect(Rect	*dr);
void RectToPreviewRect(Rect *dr);
void RectToCompositeRect(Rect *dr);
void IncorporatePropSelection(void);
void SelectPropTool(short tool);
void ClipToRect(Rect *r1, Rect *r2, Rect *clip);
void GetPEPalSelect(Rect *r);
void LoadPropIntoEditor(AssetSpec *propSpec);
void SetPropSelection(Rect 	*r, Boolean liftFlag, Boolean clearFlag);
void UpdatePropPaletteGraphics(void);
void BlitWithMask(PixMapHandle src, PixMapHandle mask, PixMapHandle dest,
				  short h, short v);
void CycleMarquee(void);
void DrawMarquee(Rect *rect);
void RefreshPropSelFrame(void);
Boolean CanPaste(long type);
void PastePict(Boolean unscaled);
void SelectPropAll(void);
void PEProcessCommand(WindowPtr theWin, short theMenu, short theItem);
void PEAdjustMenus(WindowPtr theWin);
void CopyPropSelection(void);
void PreparePETextColors(void);
void RestorePEColors(void);
void DeactivatePEName(void);
void PaintBucketTool(Point p, Boolean eraseFlag);
void MakeCompositePicture(Rect *clipRect);

void FlipPixMapHorizontal(PixMapHandle pMap);
void FlipPixMapVertical(PixMapHandle pMap);
void FlipPEVertical(void);
void FlipPEHorizontal(void);

void SavePropEditUndo();
void DoPropEditUndo();

static void Flip_Long(  PixMapHandle theMap, 
                        short rowBytes,
                        short depth,
                        Rect* area);

static void Flip_Word(  PixMapHandle theMap, 
                        short rowBytes,
                        short depth,
                        Rect* area);


#if !PALACE
void PEProcessKey(WindowPtr theWin, EventRecord *theEvent);
#endif
