/* UnixLocalUtils.c */

#include "s-server.h"
/*
#include "mansion.h"
#if macintosh
#include <unix.h>
#endif
*/
int read(int fileNum, char *buffer, int length);
void ErrorMessage(char *str,...);
void ReportError(short code, char *name);
void ErrorExit(char *str,...);
void LogMessage(char *str,...);

char gLogFileName[512] = "pserver.log";

void OpenLog(char *name)
{
	strcpy(gLogFileName, name);
}

void CloseLog(void)
{
}


void err_sys(char *str)
{
	ErrorExit(str);
}

void err_dump(char *str)
{
	ErrorExit(str);
}

void LogStringToFile(char *str,...)
{
	FILE	*lFile;
	char	tbuf[512];

	va_list args;
	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);


	if ((lFile = fopen(gLogFileName,"a")) != NULL) {
		fputs(tbuf,lFile);
		fclose(lFile);
	}
}

void ErrorMessage(char *str,...)
{
	FILE	*lFile;
	char	tbuf[512];
	va_list args;
	
	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);


	if ((lFile = fopen(gLogFileName,"a")) != NULL) {
		fputs(tbuf,lFile);
		fclose(lFile);
	}
}

void ReportError(short code, char *name)
{
	if (name && name[0]) {
		if (code)
			LogStringToFile("Error %d in %s\n",code,name);
		else
			LogStringToFile("%s\n",name);
	}
	else
		LogStringToFile("Error %d\n",code,name);
}

void ErrorExit(char *str,...)
{
	char	tbuf[512];
	va_list args;
	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	LogStringToFile("%s\n",tbuf);

	abort();
}

void LogMessage(char *str,...)
{
	int		len;
	char	tbuf[512];
	va_list args;
	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);
	len = strlen(tbuf);
	if (len && tbuf[len-1] == '\r')
		tbuf[len-1] = '\n';

	LogStringToFile("%s",tbuf);
}

void LogString(char *str)
{
	int		len;
	char	tbuf[512];
	strcpy(tbuf,str);
	len = strlen(tbuf);
	if (len && tbuf[len-1] == '\r')
		tbuf[len-1] = '\n';

	LogStringToFile("%s",tbuf);
}

void TimeLogMessage(char *str,...)
{
	int		len;
	char	*timeStr;
	time_t	tim;
	struct tm *tm;
	char	tbuf[512];

	va_list args;
	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);
	time(&tim);
	tm = localtime(&tim);
	timeStr = asctime(tm);
	len = strlen(tbuf);
	if (len && tbuf[len-1] == '\r')
		tbuf[len-1] = '\n';
	LogStringToFile("%s : %s",timeStr,tbuf);
}

Boolean EqualPt(Point a, Point b)
{
	return a.h == b.h && a.v == b.v;
}

OSErr OpenFileReadOnly(StringPtr fName, short volNum, short *fRefNum)
{
	FSSpec	spec = {0,0,""};
	OSErr	err;
	BlockMove(fName,spec.name,fName[0]+1);
	err = FSpOpenDF(&spec,fsRdPerm,fRefNum);
	if (err != 0) {
		/* convert name to lower */
		int	len;
		len = spec.name[0];
		while (len > 0 && spec.name[len] != '/') {
			if (isupper(spec.name[len]))
				spec.name[len] = tolower(spec.name[len]);
			--len;
		}
		err = FSpOpenDF(&spec,fsRdPerm,fRefNum);
	}
	return err;
}

Ptr SuckTextFile(StringPtr fileName)	/* p-string */
{
	OSErr	oe;
	short	iFile;
	Ptr		fileBuffer;
	LONG	length;

    if ((oe = OpenFileReadOnly(fileName, 0, &iFile)) != noErr) {
          LogMessage("Can't open %.*s for input (%d)\r",fileName[0],&fileName[1],oe);
          return NULL;
    }

    GetEOF(iFile,&length);
    fileBuffer = (HugePtr)NewPtrClear(length+1);
    FSRead(iFile,&length,(char *)fileBuffer);
    FSClose(iFile);
    fileBuffer[length] = 0;
    return fileBuffer;
}

#ifndef CLOCKS_PER_SEC
/* Berkeley Unix doesn't have clock */
LONG clock(void)
{
  time_t secs;
  secs = time(NULL);
  return secs;
}
#endif
