/* MacCompat.c */
#include "maccompat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

typedef struct {
	Ptr	    ptr;
    LONG	size,allocSize;
} MasterPtr;

OSErr	gLastError = noErr;

void ErrorExit(char *str);

/* SunOS4.1.3 needs this */

#if NOMEMMOVE
void memmove(char *dst, char *src, size_t length)
{
        if (length <= 0)
                return;
        if ((long) src > (long) dst || (long) src+length <= (long) dst) {
                while (length--) {
                        *(dst++) = *(src++);
                }
        }
        else {
                src += length-1;
                dst += length-1;
                while (length--) {
                        *(dst--) = *(src--);
                }
		}
}

#endif

void SetMemErr(Boolean condition);		/* Internal */
void SetMemErr(Boolean condition)
{
	if (condition) {
		gLastError = memFullErr;
		ErrorExit("ERR: SetMemErr");
	}
	else
		gLastError = noErr;
}

Ptr NewPtr(Size size)
{
	Ptr	p;
	p= malloc(size);
	SetMemErr(p == NULL);
	return p;
}

Ptr NewPtrClear(Size size)
{
	Ptr	p;
	p = malloc(size);
	SetMemErr(p == NULL);
	if (p)
		memset(p,0,size);
	return p;
}

Handle NewHandle(Size size)
{
	MasterPtr	*mp;
	Handle		h = NULL;
	mp = (MasterPtr *) NewPtrClear(sizeof(MasterPtr));
	SetMemErr(mp == NULL);
	if (mp == NULL)
		ErrorExit("ERR: NewHandle failed");
	if (mp) {
		mp->size = mp->allocSize = size;
		if (mp->allocSize < 4)
			mp->allocSize = 4;
		mp->ptr = NewPtr(mp->allocSize);
		SetMemErr(mp->ptr == NULL);
		if (mp->ptr)
			h = &mp->ptr;
		else
			DisposePtr((Ptr) mp);
	}
	return h;
}

Handle NewHandleClear(Size size)
{
	Handle	h;
	h = NewHandle(size);
	SetMemErr(h == NULL);
	if (h && size > 0) {
		memset(*h,0,size);
	}
	return h;
}

OSErr  MemError(void)
{
	return gLastError;
}

void   HLock(Handle h)	/* Nada */
{
}

void   HUnlock(Handle h)	/* nada */
{
}

void   SetHandleSize(Handle handle, Size newSize)
{
	MasterPtr	*mp = (MasterPtr *) handle;
	Ptr			np;
	size_t		aSize;
	if (mp == NULL)
		return;
	if (newSize <= mp->allocSize) {
		mp->size = newSize;
	}
	else {
		aSize = newSize;
		if (aSize < 4)
			aSize = 4;
		np = realloc(mp->ptr, aSize);
		/* np = NewPtr(aSize); */
		SetMemErr(np == NULL);
		if (np) {
			/* BlockMove(mp->ptr,np, mp->size); */
			/* DisposePtr(mp->ptr); */
			mp->ptr = np;
			mp->size = newSize;
			mp->allocSize = aSize;
		}
	}
}

void   DisposeHandle(Handle handle)
{
	MasterPtr	*mp = (MasterPtr *) handle;
	if (mp == NULL) {
		ErrorExit("ERR: Null Handle Freed");
		return;
	}
	if (mp->ptr) {
		DisposePtr(mp->ptr);
		mp->ptr = NULL;
	}
	DisposePtr((Ptr) mp);
}

void   DisposePtr(Ptr ptr)
{
	if (ptr == NULL)
		ErrorExit("ERR: Null Ptr Freed");
	free(ptr);
}

char   HGetState(Handle h)
{
	return 0;
}

void   HSetState(Handle h, char tagByte)	/* nada */
{
}

OSErr  HandToHand(Handle *h)
{
	LONG		size;
	Handle		h1,h2;
	h1 = *h;
	size = GetHandleSize(h1);
	h2 = NewHandle(size);
	if (h2) {
		BlockMove(*h1, *h2, size);
	}
	*h = h2;
	if (h2 == NULL)
		return memFullErr;
	else
		return noErr;
}

Size   GetHandleSize(Handle h)
{
	MasterPtr *mp = (MasterPtr *) h;
	return mp->size;
}

/** File Handling **/
#define MaxOpenFiles	64

FILE *gFileHandles[MaxOpenFiles];
char  gFileNames[MaxOpenFiles][256];
int	  gNbrOpenFiles;				/* Note: File Index is array Index+1 */

void ConvertFileName(StringPtr pName, char *cName);	/* Internal */
void ConvertFileName(StringPtr pName, char *cName)
{
	BlockMove(pName,cName,pName[0]+1);
	PtoCstr((StringPtr) cName);
}

OSErr FSpOpenDF(FSSpec *spec, char permission, short *fRefNum)
{
	FILE *f = NULL;
	char	cName[256];
	ConvertFileName(spec->name,cName);
	switch (permission) {
	case fsRdPerm:
		f = fopen(cName,"r");
		break;
	case fsWrPerm:
		f = fopen(cName,"w");
		break;
	case fsRdWrPerm:
	case fsRdWrShPerm:
	case fsCurPerm:
		f = fopen(cName,"r+");
		break;
	}
	if (f) {
		strcpy(gFileNames[gNbrOpenFiles],cName);
		gFileHandles[gNbrOpenFiles++] = f;
		*fRefNum = gNbrOpenFiles;
		return noErr;
	}
	else {
		*fRefNum = 0;
		return fnfErr;
	}
}

OSErr FSOpen(StringPtr inName, short volNum, short *fRefNum)
{
	FILE *f;
	char	cName[256];
	ConvertFileName(inName,cName);
	f = fopen(cName,"r+");
	if (f) {
		strcpy(gFileNames[gNbrOpenFiles],cName);
		gFileHandles[gNbrOpenFiles++] = f;
		*fRefNum = gNbrOpenFiles;
		return noErr;
	}
	else {
		*fRefNum = 0;
		return fnfErr;
	}
}


OSErr FSpCreate(FSSpec *spec, OSType creator, OSType fileType, short scriptTag)
{
	short	refNum;
	FSpOpenDF(spec,fsWrPerm,&refNum);
	FSClose(refNum);
	return noErr;
}

OSErr Create(StringPtr fName, short vRefNum, OSType creator, OSType fileType)
{
	FILE	*f;
	char	cName[256];
	ConvertFileName(fName,cName);
	f = fopen(cName,"w");
	if (f)
		fclose(f);
	else
		return -1;
	return noErr;
}

OSErr FSClose(short fRefNum)
{
	if (fRefNum && fRefNum <= gNbrOpenFiles) {
		fclose(gFileHandles[fRefNum-1]);
		gFileNames[fRefNum-1][0] = 0;
		gFileHandles[fRefNum-1] = NULL;
		while (gNbrOpenFiles && /* fRefNum == gNbrOpenFiles &&  */
				gFileHandles[gNbrOpenFiles-1] == NULL)
			--gNbrOpenFiles;
		return noErr;
	}
	else
		return fnfErr;
}

OSErr FSRead(short fRefNum,LONG *inOutCount,Ptr buffer)
{
	if (fRefNum && fRefNum <= gNbrOpenFiles) {
		*inOutCount = fread(buffer, 1, *inOutCount, gFileHandles[fRefNum-1]);
		return noErr;
	}
	else
		return fnfErr;
}

OSErr FSWrite(short fRefNum,LONG *inOutCount,Ptr buffer)
{
	if (fRefNum && fRefNum <= gNbrOpenFiles) {
		*inOutCount = fwrite(buffer, 1, *inOutCount, gFileHandles[fRefNum-1]);
		return noErr;
	}
	else
		return fnfErr;
}

OSErr FSDelete(StringPtr inName, short volNum)
{
	char	cName[256];
	int		err;
	ConvertFileName(inName,cName);
	err = remove(cName);
	return err;
}

OSErr FSpDelete(FSSpec *spec)
{
	return FSDelete(spec->name, 0);
}

OSErr FSpRename(FSSpec *spec, StringPtr nName)
{
	char	oldName[256],newName[256];
	int		err;
	ConvertFileName(spec->name,oldName);
	ConvertFileName(nName,newName);
	err = rename(oldName,newName);
	return err;
}

OSErr SetFPos(short fRefNum, short posMode, LONG posOffset)
{
	int	err = 0;
	if (fRefNum && fRefNum <= gNbrOpenFiles) {
		switch (posMode) {
		case fsAtMark:
		case fsFromMark:
			err = fseek(gFileHandles[fRefNum-1], posOffset, SEEK_CUR);
			break;
		case fsFromStart:
			err = fseek(gFileHandles[fRefNum-1], posOffset, SEEK_SET);
			break;
		case fsFromLEOF:
			err = fseek(gFileHandles[fRefNum-1], posOffset, SEEK_END);
			break;
		}
	}
	return err;
}

OSErr GetEOF(short fRefNum, LONG *fileSize)
{
	LONG	curPos,endPos;
	curPos = ftell(gFileHandles[fRefNum-1]);
	fseek(gFileHandles[fRefNum-1], 0, SEEK_END);
	endPos = ftell(gFileHandles[fRefNum-1]);
	fseek(gFileHandles[fRefNum-1], curPos, SEEK_SET);
	*fileSize = endPos;
	return noErr;
}

OSErr SetEOF(short fRefNum, LONG fileSize)	/* !! nada - will need to implement later */
{
	FILE	*f;

	if (fRefNum && fRefNum <= gNbrOpenFiles) {
	
		f = gFileHandles[fRefNum-1];
		if (f) {
			fclose(f);
			truncate(gFileNames[fRefNum-1],fileSize);
			f = fopen(gFileNames[fRefNum-1],"r+");
			gFileHandles[fRefNum-1] = f;
		}
		else
			return fnfErr;
	}
	else
		return fnfErr;
	return noErr;
}

OSErr FlushVol(StringPtr vName, short volNum)	/* Flushes all open files */
{
	int i;
	for (i = 0; i < gNbrOpenFiles; ++i) {
		if (gFileHandles[i])
			fflush(gFileHandles[i]);
	}
	return noErr;
}

OSErr FlushFile(short fRefNum) /* Flushes a specific file */
{
	if (fRefNum && fRefNum <= gNbrOpenFiles && gFileHandles[fRefNum-1]) {
		fflush(gFileHandles[fRefNum-1]);
	}
	return noErr;
}

/* Miscellaneous */

void GetDateTime(unsigned LONG *secs)
{
	time_t	s;
	time(&s);
	*secs = s;
}

void DebugStr(StringPtr str)
{
	void LogStringToFile(char *str,...);
	LogStringToFile("%.*s\n",str[0],&str[1]);
}

void SysBeep(short dmy)	/* Nada */
{
	/* fprintf(stderr,"\x07SysBeep\n"); */
}

void PtoCstr(StringPtr str)
{
	short	len;
	len = str[0];
	memmove(&str[0],&str[1],len);
	str[len] = 0;
}

void CtoPstr(char *str)
{
	short	len;
	len = strlen(str);
	memmove(&str[1],&str[0],len);
	str[0] = len;
}
