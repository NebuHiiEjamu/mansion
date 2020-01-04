/* S-Local.h (UNIX!) */

#ifndef S_LOCAL
#define S_LOCAL	1

#define unix	     1

typedef unsigned LONG SOCKET;

/* Additional Security for Unix-based Servers */
#define HIGHSECURITY		1

#define MaxPeoplePerServer	400
#define MaxUserID			10000

/* macintosh data types */
/**
#if !macintosh
typedef short			Boolean;
typedef char			*Ptr;
typedef unsigned char	Str31[32];
typedef struct {
	short	v,h;
} Point;
typedef struct {
	short	top,left,bottom,right;
} Rect;
#endif
**/
void LogMessage(char *str,...);
void TimeLogMessage(char *str,...);
void LogString(char *str);

extern Boolean			gDebugFlag,gNoForkFlag;

#endif
