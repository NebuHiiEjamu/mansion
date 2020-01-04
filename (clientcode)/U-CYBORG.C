// U-CYBORG.C - User Scripts (attached to self, not hotspot)
//
// Shared
//
// 7/20/95
//
//
#include "u-user.h"
#include "m-cmds.H"
#include "u-script.h"
#include "u-secure.h"

#if macintosh
char	*uScriptName = (char *) "\pCyborg.ipt";
#elif WIN32
char	*uScriptName = "\21Cyborg Script.ipt";
#else
char	*uScriptName = "\x0a\Cyborg.ipt";
#endif

typedef struct {
	char			*gSP;
	char			gToken[2048];
	Boolean			ungetFlag;
	Ptr				scriptBuf,varBuf;
	long			eventMask;
	short			nbrScripts;
	long			lenVars;
	EventHandlerRec	spotEvents[32];
} ScriptParser, *ScriptParserPtr;


ScriptParserPtr		gSPPtr;
EventHandlerRec		gSpotEventRec[32];

void InitScriptParser(Ptr buffer);
void CleanUpScriptParser();
Boolean GetScriptToken();
short AddScriptBuffer(Ptr p, short len);
void ParseScriptEventHandler();
Boolean GenerateUserEvent(short eventType);

#define LONGALIGN(x)	x += (4 - (x & 3)) & 3


void ClearUserScript()
{
	gRoomWin->userEventMask = 0L;
	if (gRoomWin->userScriptBuffer) {
		DisposePtr(gRoomWin->userScriptBuffer);
		gRoomWin->userScriptBuffer = NULL;
	}
}

OSErr LoadUserScript()	// Warning - this assumes gRoomWin is active!!
{
	Ptr			buff;
	short		i;
	OSErr		oe;
	FileHandle	refNum;
	long		fileSize;
	ClearUserScript();

	// Quietly Return (harder to hack)
	if (!(gRoomWin->serverInfo.serverPermissions & PM_AllowCyborgs) && !(gRoomWin->userFlags & U_SuperUser))
		return noErr;
	if (gSecure.guestAccess)
		return noErr;

	if (!(gRoomWin->userEventMask)) {
		// SuckFile into buff
		oe = OpenFileReadOnly((StringPtr) uScriptName,0,&refNum);
		if (oe)
			return oe;
		GetEOF(refNum,&fileSize);
		/* 7/1/96 JAB Changed to NewPtrClear */
		buff = NewPtrClear(fileSize+1);
		if (buff == NULL) {
			FSClose(refNum);
			return -1;
		}
		FSRead(refNum, &fileSize, buff);
		buff[fileSize] = 0;
		FSClose(refNum);
	
		InitScriptParser(buff);
		while (GetScriptToken()) {
			if (strcmp(gSPPtr->gToken, "ON") == 0) {
				ParseScriptEventHandler();
			}
		}
		DisposePtr(buff);
	
		gRoomWin->userEventMask = gSPPtr->eventMask;
		/* 7/1/96 JAB Changed to NewPtrClear */
		gRoomWin->userScriptBuffer = NewPtrClear(gSPPtr->lenVars);
		BlockMove(gSPPtr->varBuf,gRoomWin->userScriptBuffer,gSPPtr->lenVars);
		for (i = 0; i < gSPPtr->nbrScripts; ++i) {
			gRoomWin->userScriptPtrs[gSPPtr->spotEvents[i].eventType] = &gRoomWin->userScriptBuffer[gSPPtr->spotEvents[i].scriptTextOfst];
		}
	
		CleanUpScriptParser();
	}

	// Additional Check
	if (!(gRoomWin->serverInfo.serverPermissions & PM_AllowCyborgs) && !(gRoomWin->userFlags & U_SuperUser))
		ClearUserScript();
	if (gSecure.guestAccess)
		ClearUserScript();

	return noErr;
}

void InitScriptParser(Ptr buffer)
{
	gSPPtr = (ScriptParserPtr) NewPtrClear(sizeof(ScriptParser));
	gSPPtr->gSP = buffer;
	/* 7/1/96 JAB Changed to NewPtrClear */
	gSPPtr->scriptBuf = NewPtrClear(32767L);
	gSPPtr->varBuf= NewPtrClear(32767L);
}

void CleanUpScriptParser()
{
	DisposePtr(gSPPtr->scriptBuf);
	DisposePtr(gSPPtr->varBuf);
	DisposePtr((Ptr) gSPPtr);
}

Boolean GetScriptToken()
{
	char	*dp;
	if (gSPPtr->ungetFlag) {
		gSPPtr->ungetFlag = false;
		return true;
	}
reget:
	if (*gSPPtr->gSP == 0)
		return false;
	else if (isspace(*gSPPtr->gSP)) {
		++gSPPtr->gSP;
		goto reget;
	}
	else if (*gSPPtr->gSP == '#' || *gSPPtr->gSP == ';') {
		while (*gSPPtr->gSP && *gSPPtr->gSP != '\r' && *gSPPtr->gSP != '\n')
			++gSPPtr->gSP;
		goto reget;
	}
	else if (isalnum(*gSPPtr->gSP) || *gSPPtr->gSP == '_' || *gSPPtr->gSP == '.' || (*gSPPtr->gSP == '-' && isdigit(gSPPtr->gSP[1])))
	{
		dp = gSPPtr->gToken;
		if (*gSPPtr->gSP == '-')
			*(dp++)  = *(gSPPtr->gSP++);
		while (isalnum(*gSPPtr->gSP) || *gSPPtr->gSP == '_' || *gSPPtr->gSP == '.')
			*(dp++)  = *(gSPPtr->gSP++);
		*dp = 0;
	}
	else if (*gSPPtr->gSP == '\"') {
		dp = gSPPtr->gToken;
		*(dp++) = *(gSPPtr->gSP++);
		while (*gSPPtr->gSP && *gSPPtr->gSP != '\"') {
			/* 3/15/95 parse hyphens properly... */
			if (*gSPPtr->gSP == '\\') {
				*(dp++)  = *(gSPPtr->gSP++);
				if (*gSPPtr->gSP)
					*(dp++)  = *(gSPPtr->gSP++);
			}
			else
				*(dp++)  = *(gSPPtr->gSP++);
		}
		if (*gSPPtr->gSP == '\"')
			*(dp++) = *(gSPPtr->gSP++);
		*dp = 0;
	}
	else if (strchr("{}[](),",*gSPPtr->gSP)) {
		gSPPtr->gToken[0] = *(gSPPtr->gSP++);
		gSPPtr->gToken[1] = 0;
	}
	else if (ispunct(*gSPPtr->gSP)) {
		dp = gSPPtr->gToken;
		while (ispunct(*gSPPtr->gSP) || *gSPPtr->gSP == '_')
			*(dp++)  = *(gSPPtr->gSP++);
		*dp = 0;
	}
	else {
#ifdef WIN32
		LogMsg(0,WarningText,"Script Error\n");
#else
		LogMessage("Script Error\r");
#endif
		return false;
	}
	return true;
}

short AddScriptBuffer(Ptr p, short len)
{
	short	retVal;
	if (len < 0)
		return 0;
	retVal = (short)gSPPtr->lenVars;
	BlockMove(p,&gSPPtr->varBuf[gSPPtr->lenVars],len);
	gSPPtr->lenVars += len;
	return retVal;
}

// 7/24/95 - Single bottleneck routine for parsing event handler names
//
short GetEventType(char *token)
{
	short	type = PE_NbrEvents;
	if (strcmp(token,"SELECT") == 0) {			// 4/6/95 JBUM
		type = PE_Select;
	}
	else if (strcmp(token,"LOCK") == 0) {
		type = PE_Lock;
	}
	else if (strcmp(token,"UNLOCK") == 0) {
		type = PE_Unlock;
	}
	else if (strcmp(token,"HIDE") == 0) {
		type = PE_Hide;
	}
	else if (strcmp(token,"SHOW") == 0) {
		type = PE_Show;
	}
	else if (strcmp(token,"STARTUP") == 0) {
		type = PE_Startup;
	}
	else if (strcmp(token,"CUSTOM") == 0) {
		type = PE_Custom;
	}
	else if (strcmp(token,"CHAT") == 0) {		// 4/6/95 JBUM
		type = PE_InChat;
	}
	else if (strcmp(token,"INCHAT") == 0) {		// 6/6/95 JAB
		type = PE_InChat;
	}
	else if (strcmp(token,"PROPCHANGE") == 0) { 	// 4/6/95 JBUM
		type = PE_PropChange;
	}
	else if (strcmp(token,"ENTER") == 0) {		// 4/6/95 JBUM
		type = PE_Enter;
	}
	else if (strcmp(token,"LEAVE") == 0) {		// 4/6/95 JBUM
		type = PE_Leave;
	}
	else if (strcmp(token,"OUTCHAT") == 0) {		// 6/6/95 JAB
		type = PE_OutChat;
	}
	else if (strcmp(token,"ALARM") == 0) {		// 6/28/95 JAB
		type = PE_Alarm;
	}
	else if (strcmp(token,"SIGNON") == 0) {		// 7/20/95 JAB
		type = PE_SignOn;
	}
	else if (strcmp(token,"SIGNOFF") == 0) {	// 7/20/95 JAB
		type = PE_SignOff;
	}
	else if (strcmp(token,"MACRO0") == 0) {		// 7/20/95 JAB
		type = PE_Macro0;
	}
	else if (strcmp(token,"MACRO1") == 0) {		// 7/20/95 JAB
		type = PE_Macro1;
	}
	else if (strcmp(token,"MACRO2") == 0) {		// 7/20/95 JAB
		type = PE_Macro2;
	}
	else if (strcmp(token,"MACRO3") == 0) {		// 7/20/95 JAB
		type = PE_Macro3;
	}
	else if (strcmp(token,"MACRO4") == 0) {		// 7/20/95 JAB
		type = PE_Macro4;
	}
	else if (strcmp(token,"MACRO5") == 0) {		// 7/20/95 JAB
		type = PE_Macro5;
	}
	else if (strcmp(token,"MACRO6") == 0) {		// 7/20/95 JAB
		type = PE_Macro6;
	}
	else if (strcmp(token,"MACRO7") == 0) {		// 7/20/95 JAB
		type = PE_Macro7;
	}
	else if (strcmp(token,"MACRO8") == 0) {		// 7/20/95 JAB
		type = PE_Macro8;
	}
	else if (strcmp(token,"MACRO9") == 0) {		// 7/20/95 JAB
		type = PE_Macro9;
	}
	return type;
}

// Parser for Cyborg Scripts (should be merged with ParseSpotEventHandler...)
void ParseScriptEventHandler()
{
	short			type = 0;
	short			braceCnt;
	char			*dp;
	EventHandlerPtr	ehp;

	if (!GetScriptToken())
		return;
	type = GetEventType(gSPPtr->gToken);
	
	if (type == PE_NbrEvents)
#ifdef WIN32
		LogMsg(0,WarningText,"Invalid Event Handler: ON %s\n",gSPPtr->gToken);
#else
		LogMessage("Invalid Event Handler: ON %s\r",gSPPtr->gToken);
#endif

	GetScriptToken();	// Skip {
	braceCnt = 1;
	ehp = &gSPPtr->spotEvents[gSPPtr->nbrScripts];
	memset((char *) ehp, 0, sizeof(EventHandlerRec));
	ehp->eventType = type;
	dp = gSPPtr->scriptBuf;
	while (GetScriptToken()) {
		switch (gSPPtr->gToken[0]) {
		case '}':
			--braceCnt;
			if (braceCnt == 0)
				goto DoneParse;
			else {
				*(dp++) = '}';
				*(dp++) = ' ';
			}
			break;
		case '{':
			++braceCnt;
			*(dp++) = '{';
			*(dp++) = ' ';
			break;
		case '(':
		case ')':
			break;
		default:
			strcpy(dp,gSPPtr->gToken);
			dp += strlen(dp);
			*(dp++) = ' ';
			break;
		}
		// if (*gSPPtr->gSP == '\r')		// 4/6/95 Attempted to preserve CRs
		//	*(dp++) = '\r';
	}
DoneParse:
	*dp = 0;

	// 4/6/95 Bug Fix; Using AddRoomBuffer instead of AddRoomString
	//  - addRoomString assumed a pascal string as argument
	//
	ehp->scriptTextOfst = AddScriptBuffer(gSPPtr->scriptBuf,(short)(strlen(gSPPtr->scriptBuf)+1));
	++gSPPtr->nbrScripts;
	gSPPtr->eventMask |= (1L << ehp->eventType);
}

// Parser for Hotspot Scripts
void ParseSpotEventHandler(HotspotPtr	hsl)
{
	short			type;
	short			braceCnt;
	char			*dp;
	EventHandlerPtr	ehp;

	if (!GetScriptToken())
		return;
	type = GetEventType(gSPPtr->gToken);
	
	if (type == PE_NbrEvents)
#ifdef WIN32
		LogMsg(0,WarningText,"Invalid Event Handler: ON %s\n",gSPPtr->gToken);
#else
		LogMessage("Invalid Event Handler: ON %s\r",gSPPtr->gToken);
#endif

	GetScriptToken();	// Skip {
	braceCnt = 1;
	ehp = &gSpotEventRec[hsl->nbrScripts];
	memset((char *) ehp, 0, sizeof(EventHandlerRec));
	ehp->eventType = type;
	dp = gSPPtr->scriptBuf;
	while (GetScriptToken()) {
		switch (gSPPtr->gToken[0]) {
		case '}':
			--braceCnt;
			if (braceCnt == 0)
				goto DoneParse;
			else {
				*(dp++) = '}';
				*(dp++) = ' ';
			}
			break;
		case '{':
			++braceCnt;
			*(dp++) = '{';
			*(dp++) = ' ';
			break;
		case '(':
		case ')':
			break;
		default:
			strcpy(dp,gSPPtr->gToken);
			dp += strlen(dp);
			*(dp++) = ' ';
			break;
		}
		// if (*gSEWin->gSP == '\r')		// 4/6/95 Attempted to preserve CRs
		//	*(dp++) = '\r';
	}
DoneParse:
	*dp = 0;

	// 4/6/95 Bug Fix; Using AddRoomBuffer instead of AddRoomString
	//  - addRoomString assumed a pascal string as argument
	//
	ehp->scriptTextOfst = AddRoomBuffer(gSPPtr->scriptBuf,strlen(gSPPtr->scriptBuf)+1);
	++hsl->nbrScripts;
	hsl->scriptEventMask |= (1L << ehp->eventType);
}

Boolean GenerateUserEvent(short eventType)
{
	long	eventMask = (1L << eventType);
	if (gRoomWin->mePtr == NULL)
		return false;

	if (!(gRoomWin->serverInfo.serverPermissions & PM_AllowCyborgs) && !(gRoomWin->userFlags & U_SuperUser))
		return false;

	//  8/1/95 Cyborg Free Zones (to prevent cheating at games...)
	if ((gRoomWin->curRoom.roomFlags & RF_CyborgFreeZone) > 0 && !(gRoomWin->userFlags & U_SuperUser))
		return false;

	if (gRoomWin->userEventMask & eventMask) {
		// if (MembersOnly(true))
		//	return false;
		DoUserScript(gRoomWin->userScriptPtrs[eventType]);
		return true;
	}
	return false;
}

/////////////////////////////////////
void AddEventHandler(HotspotPtr hp, short type, char *str,...)
{
    EventHandlerPtr 		ehp;
    char                    *dp;
	RoomRec					*rp = &gRoomWin->curRoom;
    va_list 				args;

    ehp = &gSpotEventRec[hp->nbrScripts];
    memset(ehp,0,sizeof(EventHandlerRec));
    ehp->eventType = type;
    ehp->scriptTextOfst = rp->lenVars;
    dp = &rp->varBuf[rp->lenVars];

    va_start(args,str);
    vsprintf(dp,str,args);
    va_end(args);

    rp->lenVars += strlen(&rp->varBuf[rp->lenVars])+1;
    ++hp->nbrScripts;
    hp->scriptEventMask |= (1L << ehp->eventType);
}

void AddDefaultEventHandlers(HotspotPtr  hp)
{
    switch (hp->type) {
    case HS_Door:
            if (hp->dest) {
                    if (!(hp->scriptEventMask & (1L << PE_Select))) // 4/6/95 JBUM
                            AddEventHandler(hp,PE_Select,"DEST GOTOROOM");
            }
            break;
    case HS_LockableDoor:
    case HS_ShutableDoor:
            if (hp->dest) {
                    if (!(hp->scriptEventMask & (1L << PE_Select))) // 4/6/95 JBUM
                            AddEventHandler(hp,PE_Select,"{ \"Sorry, the Door is locked\" LOCALMSG } {  DEST GOTOROOM } ME ISLOCKED IFELSE");
            }
            break;
    case HS_Bolt:
            if (hp->dest) {
                    if (!(hp->scriptEventMask & (1L << PE_Select))) // 4/6/95 JBUM
                            AddEventHandler(hp,PE_Select,"DEST { UNLOCK } { LOCK } DEST ISLOCKED IFELSE");
            }
            break;
    }
}

// 7/24/96 This causes a roomRec memory leak when called repeatedly, because old
// scripts aren't discarded.  It shouldn't be used unless a new RoomRec
// has been received from the server
void LoadRoomScripts()
{
	RoomRec		*rp = &gRoomWin->curRoom;
	HotspotPtr	hsl;
	short		i;
	// For Each Hotspot
	hsl = (HotspotPtr) &rp->varBuf[rp->hotspotOfst];
	for (i = 0; i < rp->nbrHotspots; ++i,++hsl) {
		hsl->scriptRecOfst = 0;
		hsl->nbrScripts = 0;
		hsl->scriptEventMask = 0;
		if (hsl->scriptTextOfst) {
			InitScriptParser(&rp->varBuf[hsl->scriptTextOfst]);
			while (GetScriptToken()) {
				if (strcmp(gSPPtr->gToken, "ON") == 0) {
					ParseSpotEventHandler(hsl);
				}
			}
			CleanUpScriptParser();
		}
		AddDefaultEventHandlers(hsl);
		if (hsl->nbrScripts) {
			LONGALIGN(rp->lenVars);
			hsl->scriptRecOfst = AddRoomBuffer((Ptr) &gSpotEventRec[0], hsl->nbrScripts*sizeof(EventHandlerRec));
		}
	}
}
