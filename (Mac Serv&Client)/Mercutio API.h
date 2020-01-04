/***********************************************************************************
**
**       Developer's Programming Interface for Mercutio Menu Definition Function
**               © 1992-1994 Ramon M. Felciano, All Rights Reserved
**                         C port -- January 17, 1994
**
************************************************************************************/

#define		customDefProcSig	'CUST'
#define		areYouCustomMsg		(short) ('*' * 256 + '*')
#define		getVersionMsg		(short) ('*' * 256 + 'v')
#define		getCopyrightMsg		(short) ('*' * 256 + '©')
#define		setCallbackMsg		(short) ('*' * 256 + 'c')
#define		stripCustomDataMsg	(short) ('*' * 256 + 'd')
#define		setPrefsMsg			(short) ('*' * 256 + 'p')
#define		mMenuKeyMsg			(short) ('S' * 256 + 'K')
#define		mDrawItemStateMsg	(short) ('S' * 256 + 'D')
#define		mCountItemMsg		(short) ('S' * 256 + 'C')

#define		cbBasicDataOnlyMsg	1
#define		cbIconOnlyMsg		2
#define		cbGetLongestItemMsg 3

typedef struct {
		Style	isDynamicFlag;
		Style	forceNewGroupFlag;
		Style	useCallbackFlag;
		Style	controlKeyFlag;
		Style	optionKeyFlag;
		Style	shiftKeyFlag;
		Style	cmdKeyFlag;
		Style	unusedFlag;
		short	requiredModifiers;
	} MenuPrefsRec, *MenuPrefsPtr;

typedef	struct	{
		short	version;
		MenuPrefsRec	thePrefs;
	} MenuResPrefs, *MenuResPrefsPtr, **MenuResPrefsHandle;
	

typedef	struct	{	// PACKED RECORD
		char	iconID;
		char	keyEq;
		char	mark;
		Style	textStyle;
	} StdItemData, *StdItemDataPtr;



// ItemFlagsRec is a 2-byte sequence of 1-bit flags. It is defined
// as a short here; use these constants to set the flags.

// high byte
#define	kForceNewGroup	0x8000
#define	kIsDynamic		0x4000
#define	kUseCallback	0x2000
#define	kControlKey		0x1000
#define	kOptionKey		0x0800
#define	kUnused10		0x0400
#define kShiftKey		0x0200
#define	kCmdKey			0x0100
// low byte
#define	kIsHier			0x0080
#define	kChangedByCallback	0x0040
#define	kEnabled		0x0020
#define	kHilited		0x0010
#define	kSmallIcon		0x0008
#define	kHasIcon		0x0004
#define	kUnused1		0x0002
#define	kUnused0		0x0001

typedef	struct	{
		char	iconID;
		char	keyEq;
		char	mark;
		Style	textStyle;
		short	itemID;
		Rect	itemRect;
		short	flags;
		ResType	iconType;
		Handle	hIcon;
		StringPtr pString;
		Str255	itemStr;
		short	cbMsg;
	} RichItemData, *RichItemPtr;

	
extern pascal long MDEF_GetVersion(MenuHandle menu);
extern pascal StringHandle MDEF_GetCopyright(MenuHandle menu);
extern pascal Boolean	MDEF_IsCustomMenu(MenuHandle menu);

extern pascal long	MDEF_MenuKey(long theMessage, short theModifiers, MenuHandle hMenu);
					
extern pascal void	MDEF_CalcItemSize(MenuHandle hMenu, short item, Rect *destRect);
extern pascal void	MDEF_DrawItem(MenuHandle hMenu, short item, Rect destRect);
extern pascal void	MDEF_DrawItemState(MenuHandle hMenu, short item, Rect destRect, Boolean isHilited, Boolean isEnabled);

extern pascal void	MDEF_StripCustomData(MenuHandle hMenu);

extern pascal void	MDEF_SetCallbackProc(MenuHandle hMenu, ProcPtr theProc);
extern pascal void	MDEF_SetMenuPrefs(MenuHandle hMenu, MenuPrefsRec *thePrefs);
