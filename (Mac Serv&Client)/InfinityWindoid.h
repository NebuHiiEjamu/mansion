// *****************************************************************************
//
//	InfinityWindoid.h
//
// —————————————————————————————————————————————————————————————————————————————
//	DESCRIPTION:
//		This file contains headers that can be used by an application using
//		the Infinity Windoid WDEF.
//
//		In order to create a window using the Infinity Windoid WDEF, the
//		constants in this file are useful.
//
//		For instance, if you want a new windoid with the titlebar down the
//		left side of the window and no zoom or grow box:
//
//		  theWindow = NewWindow(nil, bounds, title, visible, 
//								kInfinitySideProc, 
//								behind, goAwayFlag, refCon);
//
//		The goAwayFlag will determine if the windoid has a close box.
// —————————————————————————————————————————————————————————————————————————————
#ifndef Infinity_INFINITYWINDOID
#define Infinity_INFINITYWINDOID


// —————————————————————————————————————————————————————————————————————————————
//
//	Resource ID
//
// —————————————————————————————————————————————————————————————————————————————
// 		If you change the ID of the WDEF that is created, put that ID here.
// —————————————————————————————————————————————————————————————————————————————
enum {
	kInfinityWindoidID = 128
};


// —————————————————————————————————————————————————————————————————————————————
//
//	Infinity Windoid procIDs (for NewWindow, etc.)
//
// —————————————————————————————————————————————————————————————————————————————
//		Note that these procIDs reflect the System 7.5 floating window procID
//		constants that are listed in Windows.h in the new Universal Headers.
//		The only difference in the names is that these start with ‘kInfinity’ 
//		while the 7.5 ones start with ‘float’ (as in ‘floatZoomProc’).
//
//		The defproc id (multiplied out) is listed in the comment alongside
//		each constant so that you can find one easily if you want to use it
//		in a ‘WIND’ resource.  (Note that these numbers are for an Infinity
//		Windoid WDEF with a resource ID of 128.)
// —————————————————————————————————————————————————————————————————————————————
enum {
	kInfinityProc				= kInfinityWindoidID * 16 +  1,		// == 2049
	kInfinityGrowProc			= kInfinityWindoidID * 16 +  3,		// == 2051
	kInfinityZoomProc			= kInfinityWindoidID * 16 +  5,		// == 2053
	kInfinityZoomGrowProc		= kInfinityWindoidID * 16 +  7,		// == 2055
	kInfinitySideProc			= kInfinityWindoidID * 16 +  9,		// == 2057
	kInfinitySideGrowProc		= kInfinityWindoidID * 16 + 11,		// == 2059
	kInfinitySideZoomProc		= kInfinityWindoidID * 16 + 13,		// == 2061
	kInfinitySideZoomGrowProc	= kInfinityWindoidID * 16 + 15		// == 2063
};


// —————————————————————————————————————————————————————————————————————————————
#endif