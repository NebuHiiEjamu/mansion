// BotComm.c
//
// Wheel of Cheese 
//
// How the Bot communicates to the user and the server via the Palace
//
// Essentially a set of "printf" type functions.

#include "WOFBot.h"

// Comm Stuff

void CommString(char *str)
{
	DoCallback(PC_Chat, 0, str);
}

char gCacheBuf[512];

void CommCache(char *str,...)
{
	char 	tbuf[256];
	va_list args;

	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	strcat(gCacheBuf,tbuf);
}

void CommPrintf(char *str,...)
{
	char 	tbuf[256];
	va_list args;

	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	if (gCacheBuf[0]) {
		char	nbuf[256];
		sprintf(nbuf,"%s%s",gCacheBuf,tbuf);
		strcpy(tbuf,nbuf);
		gCacheBuf[0] = 0;
		tbuf[250] = 0;
	}
	tbuf[250] = 0;
	CommString(tbuf);
}

void PrivatePrintf(PersonID whoID, char *str,...)
{
	char 	tbuf[256];
	va_list args;

	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	DoCallback(PC_Chat, whoID, tbuf);
}


void CarrotPrintf(char *str,...)
{
	char 	tbuf[256];
	va_list args;

	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	CommPrintf("%s%s",CarrotPrefix,tbuf);
}

void StatusPrintf(short code, char *str,...)
{
	char 	tbuf[256];
	va_list args;

	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);
	LogMessage("Puzzle: %s\r",tbuf);
}


// !! print with delay
short gExtraDelay=180L;
long	respTimer,respDelay;
Boolean	respQueued;
char	resp[256];
void AddExtraDelay(short delay);

void AddExtraDelay(short delay)
{
	gExtraDelay = delay;
}

void RespPrintf(char *str,...)
{
	char 	tbuf[256];
	va_list args;

	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	respDelay = 8*strlen(resp) + gExtraDelay;
	respTimer = TickCount();
	respQueued = true;
}


void CheckResponses()
{
	if (respQueued && TickCount() - respTimer > respDelay) {
		respQueued = false;
		CommString(resp);
	}
}
