/* Unix version of Local.h */
#ifndef __LOCAL__
#define __LOCAL__

#ifndef unix
#define unix	1
#endif

#define LONG  int  /* fix for machine specific */

#include "maccompat.h"
#include <time.h>

#define HugePtr Ptr
#define huge	

/*////////////////////////////////////////////////////////////////// */
/* esr 5-19 */
/* #define Long2Host(x) (x) */
#define	UnionRectMac(r1,r2,r3)  	UnionRect(r1,r2,r3)
#define SetRectMac(r,x1,y1,x2,y2)	SetRect(r,x1,y1,x2,y2)
#define OffsetRectMac(r,x,y)		OffsetRect(r,x,y)
#define SectRectMac(r1,r2,r3)  		SectRect(r1,r2,r3)
#define IsRectEmptyMac(r)  			EmptyRect(r)
#define PtInRgnMac					PtInRgn
#define PtInRectMac					PtInRect
#define InsetRectMac				InsetRect
#define LPSTR						char *
#define BOOL						Boolean
#define TIMER_FASTEST	0L
#define TIMER_FAST 		1L
#define TIMER_SLOW		20L
#define TIMER_SLEEP		-1L
/*////////////////////////////////////////////////////// */

#ifdef CLOCKS_PER_SEC
#define TICK_SECONDS            CLOCKS_PER_SEC
#define GetTicks                        clock
#else
#define TICK_SECONDS    1L
#define GetTicks        clock
LONG clock(void);
#endif

#ifdef NOBCOPY

#define bcopy(src,dst,count)		memmove(dst,src,count)
#define bzero(dst,count)		memset(dst,0,count)

#endif

#ifdef NOGETDTABLESIZE
#define getdtablesize()			FD_SETSIZE	/* _NFILE */
/* 9/11/96 use of _NFILE was reducing capacity to 20 users on solaris2 !! */
#endif

/* Prototypes should be in S-FUNC.H */

#endif /* __LOCAL__ */
