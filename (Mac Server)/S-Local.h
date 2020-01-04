// S-Local.h

#ifndef S_LOCAL
#define S_LOCAL	1

#include <time.h>
#include <MacTCP.h>
#include "TCPHi.h"
#include "TCPRoutines.h"

#define CRIPPLEDSERVER		0
#define PALACE_3D			1

#ifdef OLDCODE
	#if CRIPPLEDSERVER
	#define MaxPeoplePerServer	3
	#else
	#define MaxPeoplePerServer	40
	#endif
#endif

#define MaxUserID			10000

typedef struct {
	TEWindowRec			win;
	Boolean				iconized;
	Rect				saveRect;
	CIconHandle			cIcons[2];
	short				iconState;
	short				logFile;
} LogWindowRec, *LogWindowPtr;

void LogMessage(char *str,...);
void TimeLogMessage(char *str,...);
void LogString(char *str);

extern LogWindowPtr		gLogWin;
extern Boolean			gDebugFlag;
extern int				gMaxPeoplePerServer;

// These should be in s-funcs

#endif

