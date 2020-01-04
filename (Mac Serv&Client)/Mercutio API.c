/***********************************************************************************
**
**       Developer's Programming Interface for Mercutio Menu Definition Function
**               © 1992-1995 Ramon M. Felciano, All Rights Reserved
**                         Latest C port -- January 17, 1994
**
************************************************************************************/

/*
**	09Aug94 : Tom Emerson
**	Modified by Tom Emerson (tree@bedford.symantec.com) to work correctly with
**	the universal headers, and hence when calling from PowerPC native code.
**	This has been conditionalized so that it will compile with and without the
**  universal interfaces.
**
**	Also cleaned up the code a bit - removed the prototypes from this file and
**	included the header instead. Wrapped the declarations in the header with
**	'extern "C"' when compiling with a C++ compiler.
**
**	
*/

/*
**	19Dec94 : RMF
**	Updated to full Mercutio 1.2 spec by Ramon Felciano.
**	
**	27Dec94 : RMF
**	MDEF_CalcItemSize now correctly returns a result.
**	MDEF_StripCustomData declares Point at top of function (bug?).
*/

#include "Mercutio API.h"

#define 	_Point2Long(pt)		(* (long *) &pt)		// these would have pbs with register vars
#define 	_Long2Point(long)	(* (Point *) &long)

//#ifndef __CONDITIONALMACROS__
//typedef pascal void (*MDEFProc)(short msg, MenuHandle theMenu, Rect* menuRect, Point hitPt, short *itemID);
//#endif

/***********************************************************************************
**
**   MDEF_GetVersion returns the MDEF version in long form. This can be typecast
**	 to a normal version record if needed.
**
************************************************************************************/
pascal	long	MDEF_GetVersion (MenuHandle menu)
{
	SignedByte state;
	Handle	proc;
	Rect	dummyRect;
	short	dummyInt;
	Point 	pt;
	MenuDefUPP	menuProcUPP;
	
	proc = (*menu)->menuProc;	/* same as **menu.menuProc */
	state = HGetState(proc);
	HLock(proc);
	dummyRect.top = dummyRect.left = dummyRect.bottom = dummyRect.right = 0;

	SetPt(&pt,0,0);

	menuProcUPP = (MenuDefUPP)NewRoutineDescriptor((ProcPtr)*proc, uppMenuDefProcInfo, kM68kISA);
	CallMenuDefProc(menuProcUPP, getVersionMsg, menu, &dummyRect, pt, &dummyInt);	
	DisposeRoutineDescriptor(menuProcUPP);
	
	HSetState(proc, state);
	
	/* the result, a long, is returned in dummyRect.topLeft */
	return _Point2Long(topLeft(dummyRect));
}

/***********************************************************************************
**
**   MDEF_GetCopyright returns a stringHandle to the copyright message for the MDEF.
**
**   IMPORTANT: THE CALLER IS RESPONSIBLE FOR DISPOSING OF THIS HANDLE WHEN DONE
**              WITH IT.
**
************************************************************************************/
pascal	StringHandle	MDEF_GetCopyright (MenuHandle menu)
{
	SignedByte state;
	Handle	proc;
	Rect	dummyRect;
	short	dummyInt;
	Point 	pt;
	MenuDefUPP	menuProcUPP;
	
	proc = (*menu)->menuProc;	/* same as **menu.menuProc */
	state = HGetState(proc);
	HLock(proc);
	dummyRect.top = dummyRect.left = dummyRect.bottom = dummyRect.right = 0;

	SetPt(&pt,0,0);

	menuProcUPP = (MenuDefUPP)NewRoutineDescriptor((ProcPtr)*proc, uppMenuDefProcInfo, kM68kISA);
	CallMenuDefProc(menuProcUPP, getCopyrightMsg, menu, &dummyRect, pt, &dummyInt);	
	DisposeRoutineDescriptor(menuProcUPP);

	HSetState(proc, state);
	
	/* the result, a stringHandle, is returned in dummyRect.topLeft */
	return *(StringHandle*)(&dummyRect);
}

/***********************************************************************************
**
**   IsCustomMenu returns true if hMenu is controlled by a custom MDEF. This relies on my}
**   convention of returning the customDefProcSig constant in the rect parameter: this obtuse}
**   convention should be unique enough that only my custom MDEFs behave this way.}
**
************************************************************************************/
pascal	Boolean MDEF_IsCustomMenu (MenuHandle menu)
{
	SignedByte state;
	Handle	proc;
	Rect	dummyRect;
	short	dummyInt;
	Point 	pt;
	MenuDefUPP	menuProcUPP;
	
	proc = (*menu)->menuProc;	/* same as **menu.menuProc */
	state = HGetState(proc);
	HLock(proc);
	dummyRect.top = dummyRect.left = dummyRect.bottom = dummyRect.right = 0;

	SetPt(&pt,0,0);

	menuProcUPP = (MenuDefUPP)NewRoutineDescriptor((ProcPtr)*proc, uppMenuDefProcInfo, kM68kISA);
	CallMenuDefProc(menuProcUPP, areYouCustomMsg, menu, &dummyRect, pt, &dummyInt);	
	DisposeRoutineDescriptor(menuProcUPP);

	HSetState(proc, state);
	
	/* the result, a long, is returned in dummyRect.topLeft */
	return (_Point2Long(topLeft(dummyRect)) == (long) (customDefProcSig));
}


/***********************************************************************************
**
**   PowerMenuKey is a replacement for the standard toolbox call MenuKey for use with the}
**   Mercutio. Given the keypress message and modifiers parameters from a standard event, it }
**   checks to see if the keypress is a key-equivalent for a particular menuitem. If you are currently}
**   using custom menus (i.e. menus using Mercutio), pass the handle to one of these menus in}
**   hMenu. If you are not using custom menus, pass in NIL or another menu, and PowerMenuKey will use the}
**   standard MenuKey function to interpret the keypress.}
**
**   As with MenuKey, PowerMenuKey returns the menu ID in high word of the result, and the menu}
**   item in the low word.}
**
************************************************************************************/

pascal	long MDEF_MenuKey (long theMessage, short theModifiers, MenuHandle menu)
{
	
	if ((menu == NULL) || (!MDEF_IsCustomMenu(menu))) {
		return(MenuKey((char)(theMessage & charCodeMask)));
	} else {
		Handle proc = (*menu)->menuProc;
		char state = HGetState(proc);
		Rect dummyRect;
		Point pt = _Long2Point(theMessage);
		MenuDefUPP	menuProcUPP;
		
		HLock(proc);
		dummyRect.top = dummyRect.left = 0;
		
		menuProcUPP = (MenuDefUPP)NewRoutineDescriptor((ProcPtr)*proc, uppMenuDefProcInfo, kM68kISA);
		CallMenuDefProc(menuProcUPP, mMenuKeyMsg, menu, &dummyRect, pt, &theModifiers);	
		DisposeRoutineDescriptor(menuProcUPP);

		HSetState(proc, state);
		return( _Point2Long(topLeft(dummyRect)));
	}
}















pascal void MDEF_SetCallbackProc (MenuHandle menu, ProcPtr theProc)
{
	Rect	dummyRect;
	short	dummyInt;
	Point 	pt;
	MenuDefUPP	menuProcUPP;

	Handle proc = (*menu)->menuProc;
	char state = HGetState(proc);
	HLock(proc);
	
	pt.h = (short) (0x0000FFFF & (long) theProc);
	pt.v = (short) ((long) theProc >> 16);
	
	menuProcUPP = (MenuDefUPP)NewRoutineDescriptor((ProcPtr)*proc, uppMenuDefProcInfo, kM68kISA);
	CallMenuDefProc(menuProcUPP, setCallbackMsg, menu, &dummyRect, pt, &dummyInt);	
	DisposeRoutineDescriptor(menuProcUPP);

	HSetState(proc, state);
}




pascal void MDEF_SetMenuPrefs (MenuHandle menu, MenuPrefsRec *thePrefs)
{
	Rect	dummyRect;
	short	dummyInt;
	Point 	pt;
	MenuDefUPP	menuProcUPP;

	Handle proc = (*menu)->menuProc;
	char state = HGetState(proc);
	HLock(proc);

	pt.h = (short) (0x0000FFFF & (long) thePrefs);
	pt.v = (short) ((long) thePrefs >> 16);

	menuProcUPP = (MenuDefUPP)NewRoutineDescriptor((ProcPtr)*proc, uppMenuDefProcInfo, kM68kISA);
	CallMenuDefProc(menuProcUPP, setPrefsMsg, menu, &dummyRect, pt, &dummyInt);	
	DisposeRoutineDescriptor(menuProcUPP);

	HSetState(proc, state);
}



pascal void MDEF_StripCustomData (MenuHandle menu)
{
	Rect	dummyRect;
	short	dummyInt;
	MenuDefUPP	menuProcUPP;

	Handle proc;
	char state;
	Point 	pt = {0,0};
	
	proc = (*menu)->menuProc;
	state = HGetState(proc);
	HLock(proc);


	menuProcUPP = (MenuDefUPP)NewRoutineDescriptor((ProcPtr)*proc, uppMenuDefProcInfo, kM68kISA);
	CallMenuDefProc(menuProcUPP, stripCustomDataMsg, menu, &dummyRect, pt, &dummyInt);	
	DisposeRoutineDescriptor(menuProcUPP);

	HSetState(proc, state);
}




pascal void MDEF_DrawItem (MenuHandle menu, short item, Rect destRect)
{
	short	dummyInt;
	Point 	pt = {0,0};
	MenuDefUPP	menuProcUPP;

	Handle proc = (*menu)->menuProc;
	char state = HGetState(proc);
	HLock(proc);

	menuProcUPP = (MenuDefUPP)NewRoutineDescriptor((ProcPtr)*proc, uppMenuDefProcInfo, kM68kISA);
	CallMenuDefProc(menuProcUPP, mDrawItemMsg, menu, &destRect, pt, &dummyInt);	
	DisposeRoutineDescriptor(menuProcUPP);

	HSetState(proc, state);
}


pascal void MDEF_DrawItemState (MenuHandle menu, short item, Rect destRect, Boolean isHilited, Boolean isEnabled)
{
	Point	pt;
	MenuDefUPP	menuProcUPP;

	Handle proc = (*menu)->menuProc;
	char state = HGetState(proc);
	HLock(proc);

	pt.h = (short) isHilited;
	pt.v = (short) isEnabled;

	menuProcUPP = (MenuDefUPP)NewRoutineDescriptor((ProcPtr)*proc, uppMenuDefProcInfo, kM68kISA);
	CallMenuDefProc(menuProcUPP, mDrawItemStateMsg, menu, &destRect, pt, &item);	
	DisposeRoutineDescriptor(menuProcUPP);

	HSetState(proc, state);
}


pascal void MDEF_CalcItemSize (MenuHandle menu, short item, Rect *destRect)
{
	Point 	pt = {0,0};
	MenuDefUPP	menuProcUPP;

	Handle proc = (*menu)->menuProc;
	char state = HGetState(proc);
	HLock(proc);

	menuProcUPP = (MenuDefUPP)NewRoutineDescriptor((ProcPtr)*proc, uppMenuDefProcInfo, kM68kISA);
	CallMenuDefProc(menuProcUPP, mCalcItemMsg, menu, destRect, pt, &item);	
	DisposeRoutineDescriptor(menuProcUPP);

	HSetState(proc, state);
}


