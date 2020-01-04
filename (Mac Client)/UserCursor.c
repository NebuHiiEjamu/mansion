// UserCursor.c

#include "U-USER.H"

CursHandle	gCursHandles[NbrCursors];

void InitCursors()
{
	short	i;
	for (i = 0; i < NbrCursors; ++i)
		gCursHandles[i] = GetCursor(128+i);
}

void SpinCursor()
{
	static short cCnt;
	static long lastTicks;
	long		t;
	if ((t = TickCount()) - lastTicks  > 60) {
		lastTicks = t;
		cCnt = (cCnt + 1) % NbrWatchCursors;
	}
	SetCursor(*gCursHandles[WatchCursor1 + cCnt]);
}

