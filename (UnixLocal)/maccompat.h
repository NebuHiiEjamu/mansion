/* maccompat.h */
/* macintosh data types */
#include <string.h>

#ifndef LONG
#define LONG	int
#endif

typedef short			Boolean;
typedef char			*Ptr;
typedef Ptr				*Handle;
typedef unsigned char	*StringPtr;
typedef unsigned char	Str31[32];
typedef unsigned char	Str63[64];
typedef unsigned char	Str255[256];
typedef short			OSErr;
typedef LONG			OSType;
typedef LONG			Size;

typedef struct {
	short	v,h;
} Point;
typedef struct {
	short	top,left,bottom,right;
} Rect;
typedef struct {
	short		vRefNum;
	LONG		parID;
	Str255		name;
} FSSpec;

#define false		0
#define true		!false

#ifndef TRUE
#define TRUE		!FALSE
#define FALSE		0
#endif

#define noErr		0
#define fnfErr		-43
#define memFullErr	-108
#define resNotFound	-192

#ifndef NULL
#define NULL		(void *) 0L
#endif

#define fsAtMark	0
#define fsFromStart	1
#define fsFromLEOF	2
#define fsFromMark	3
#define fsCurPerm	0
#define fsRdPerm	1
#define fsWrPerm	2
#define fsRdWrPerm	3
#define fsRdWrShPerm	4

/* Prototypes for mac toolbox functions we need */
/* Memory */
#if NOMEMMOVE
void memmove(char *dst, char *src, size_t length);
#endif

#define BlockMove(src,dest,size)	memmove(dest,src,size)
/* void   BlockMove(void *src,void *dest, Size size); */
Ptr    NewPtr(Size size);
Ptr    NewPtrClear(Size size);
Handle NewHandle(Size size);
Handle NewHandleClear(Size size);
OSErr  MemError(void);
void   HLock(Handle h);
void   HUnlock(Handle h);
void   SetHandleSize(Handle handle, Size newSize);
void   DisposeHandle(Handle handle);
void   DisposePtr(Ptr ptr);
char   HGetState(Handle h);
void   HSetState(Handle h, char tagByte);
OSErr  HandToHand(Handle *h);
Size   GetHandleSize(Handle h);


/* Files */
OSErr FSOpen(StringPtr fName, short volNum, short *fRefNum);
OSErr Create(StringPtr fName, short vRefNum, OSType creator, OSType fileType);
OSErr FSClose(short fRefNum);
OSErr FSRead(short fRefNum,LONG *inOutCount,Ptr buffer);
OSErr FSWrite(short fRefNum,LONG *inOutCount,Ptr buffer);
OSErr FSDelete(StringPtr fName, short volNum);
OSErr FSWrite(short fRefNum, LONG *inOutCount,Ptr buffer);
OSErr SetFPos(short fRefNum, short posMode, LONG posOffset);
OSErr GetEOF(short fRefNum, LONG *fileSize);
OSErr SetEOF(short fRefNum, LONG fileSize);

OSErr FSpOpenDF(FSSpec *spec, char permission, short *refNum);
OSErr FSpDelete(FSSpec *spec);
OSErr FSpRename(FSSpec *spec, StringPtr nName);
OSErr FSpCreate(FSSpec *spec, OSType creator, OSType fileType, short scriptTag);
OSErr FlushVol(StringPtr vName, short volNum);
OSErr FlushFile(short refNum);

/* Graphics */
Boolean EqualPt(Point a, Point b);

/* Misc */
void GetDateTime(unsigned LONG *secs);
void DebugStr(StringPtr str);
LONG TickCount(void);
void SysBeep(short dmy);
void PtoCstr(StringPtr str);
void CtoPstr(char *str);
