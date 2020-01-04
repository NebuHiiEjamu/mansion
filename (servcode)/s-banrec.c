/* s-banrec.c */

#include "s-server.h"
#include "s-secure.h"
#include <time.h>
#include <string.h>

IPBanRecPtr	gIPBanList;

static char *banRecWhyStr[] = {"Tracked","Logged Off","Comm Err",
							  "Flooder",
							  "Killed",
							  "Server Down",
							  "Unresponsive",
							  "Site Ban",
							  "ServerFull",
							  "Cracker",
							  "Duplicate",
							  "Convict",
							  "Exile",
							  "Banned",
							  "Guest","Unknown"};
							  
void DoBanList(ServerUserPtr cUser, char *str)
{
	IPBanRecPtr	curRec;
	LONG	remaining;
	LONG	recCnt = 0;
	char	ipStr[64];
	Boolean	allFlag = false,trackFlag = false;

	while (*str == '-') {
		++str;
		switch (*str) {
		case 'a':	allFlag = true;	break;
		case 't':	trackFlag = true;	break;
		}
		++str;
		while (*str == ' ')
			++str;
	}


	curRec = gIPBanList;
	for (curRec = gIPBanList; curRec; curRec = curRec->nextRec) {
		if (curRec->whyBanned == K_InvalidSerialNumber && !allFlag)
			continue;
		if (curRec->banFlags & BR_Tracking)
			curRec->whyBanned = K_Unknown;	/* temporary fix */
	 	if (trackFlag && !(curRec->banFlags & BR_Tracking))
        	continue;
		ConvertIPToString(curRec->ipAddress,ipStr);
		if (str[0] == 0 || 
			strstr(curRec->verbalIP, str) != NULL ||
			strstr(curRec->userName,str) != NULL ||
			strstr(ipStr,str) != NULL) {
			++recCnt;
			remaining = curRec->penaltyTime - (time(NULL) - curRec->timeLastPenalized);
			if (remaining < 0)
				remaining = 0;
			else
				remaining /= 60;
			ConvertIPToString(curRec->ipAddress,ipStr);
			if (strcmp(ipStr,curRec->verbalIP)) {
				UserMessage(cUser,"; %-8s %ld min (%ld) (%s [%s]) %s %s by %s",
					banRecWhyStr[curRec->whyBanned],
					(long) curRec->penaltyTime / 60L,(long) remaining,
					curRec->verbalIP,ipStr,
					curRec->userName[0]? curRec->userName : "site",
					(curRec->banFlags & BR_Tracking)? "tracked" : "banned",
					curRec->whoKilled[0]? curRec->whoKilled : "system");
			}
			else {
				UserMessage(cUser,"; %-8s %ld min (%ld) (%s) %s %s by %s",
					banRecWhyStr[curRec->whyBanned],
					(long) curRec->penaltyTime / 60L,(long) remaining,
					curRec->verbalIP,
					curRec->userName[0]? curRec->userName : "site",
					(curRec->banFlags & BR_Tracking)? "tracked" : "banned",
					curRec->whoKilled[0]? curRec->whoKilled : "system");
			}
			if (curRec->comment[0])
				UserMessage(cUser,"; comment by %s", curRec->comment);
		}
	}
	if (recCnt == 0)
		UserMessage(cUser, "; No Records Found");
}

void DoUnBan(ServerUserPtr cUser, char *str, Boolean trackerFlag)
{
	IPBanRecPtr	curRec,lastRec = NULL,nextRec;
	char		*desc;
	char		ipStr[64];
	short		nbrHits=0;

	if (str[0] == 0)
		return;
	desc = (trackerFlag? "untracked" : "unbanned");
	curRec = gIPBanList;
	for (curRec = gIPBanList; curRec; curRec = nextRec) {

		nextRec = curRec->nextRec;

		if ((trackerFlag != 0) != ((curRec->banFlags & BR_Tracking) != 0))
			continue;

		ConvertIPToString(curRec->ipAddress,ipStr);
		if (strstr(curRec->verbalIP, str) != NULL ||
			strstr(curRec->userName,str) != NULL ||
			strstr(ipStr,str) != NULL) {
			if (strcmp(curRec->verbalIP,ipStr))
				WizGlobalMessage("Page from System: (%s [%s]) %s %s by %s",
					curRec->verbalIP,ipStr,curRec->userName, desc, CvtToCString(cUser->user.name));
			else
				WizGlobalMessage("Page from System: (%s) %s %s by %s",
					curRec->verbalIP,curRec->userName, desc, CvtToCString(cUser->user.name));
			if (lastRec)
				lastRec->nextRec = curRec->nextRec;
			else
				gIPBanList = curRec->nextRec;
			DisposePtr((Ptr) curRec);
			++nbrHits;
		}
		else
			lastRec = curRec;
	}
	if (!nbrHits)
		UserMessage(cUser, "Can't find %s", str);
}

void PurgeBanList(ServerUserPtr cUser)
{
	IPBanRecPtr	curRec,lastRec = NULL,nextRec;
	LONG	remaining;
	int		nbrRecs=0;
	curRec = gIPBanList;
	for (curRec = gIPBanList; curRec; curRec = nextRec) {
		nextRec = curRec->nextRec;
		remaining = curRec->penaltyTime - (time(NULL) - curRec->timeLastPenalized);
		if (remaining <= 0) {
			if (lastRec)
				lastRec->nextRec = curRec->nextRec;
			else
				gIPBanList = curRec->nextRec;
			DisposePtr((Ptr) curRec);
			++nbrRecs;
		}
		else
			lastRec = curRec;
	}
	UserMessage(cUser, "%d records purged from banlist",nbrRecs);
}

/* 6/15/95 JAB If ban record is based on user->counter, then don't return ip address
 * (use counter only)
 */
IPBanRecPtr GetIPBanRecByIP(unsigned LONG ipAddress, Boolean trackerFlag)
{
	IPBanRecPtr	curRec;
	LONG		siteMask1,siteMask2;
	LONG		remaining;
	time_t		curTime = time(NULL);

	if (ipAddress == 0)
		return NULL;
	if (LittleEndian()) {
		siteMask1 = 0x00FFFFFF;
		siteMask2 = 0x0000FFFF;
	}
	else {
		siteMask1 = 0xFFFFFF00;
		siteMask2 = 0xFFFF0000;
	}
	for (curRec = gIPBanList; curRec; curRec = curRec->nextRec) {

		if ((trackerFlag != 0) != ((curRec->banFlags & BR_Tracking) != 0))
			continue;

		remaining = curRec->penaltyTime - (curTime - curRec->timeLastPenalized);

		if ((curRec->banFlags & BR_SiteBan1) > 0) {
			if ((ipAddress & siteMask1) == (curRec->ipAddress & siteMask1) &&
				remaining > 0)
				return curRec;
		}
		else if ((curRec->banFlags & BR_SiteBan2) > 0) {
			if ((ipAddress & siteMask2) == (curRec->ipAddress & siteMask2) &&
				remaining > 0)
				return curRec;
		}
		/* 6/15/95 JAB If ban record contains a member user->counter, 
		 * then don't return ip address (use counter only)
		 */
		else if (ipAddress == curRec->ipAddress && remaining > 0 &&
				 (curRec->regKey == 0L || curRec->regKey == (LONG) DefaultCounter))
			return curRec;
	}
	return NULL;
}


IPBanRecPtr GetIPBanRecByCounter(unsigned LONG counter, Boolean trackerFlag)
{
	IPBanRecPtr	curRec;
	time_t		curTime = time(NULL);
	if (counter == 0 || counter == DefaultCounter)
		return NULL;
	for (curRec = gIPBanList; curRec; curRec = curRec->nextRec) {
		if ((trackerFlag != 0) != ((curRec->banFlags & BR_Tracking) != 0))
			continue;
		/* 1/14/97 JAB Don't routine records for guest bans... */
		if (!(curRec->banFlags & BR_PuidKey) && counter == curRec->regKey && curRec->penaltyTime - (curTime - curRec->timeLastPenalized) > 0)
			return curRec;
	}
	return NULL;
}

IPBanRecPtr GetIPBanRecByPuidKey(unsigned LONG counter, Boolean trackerFlag)
{
	IPBanRecPtr	curRec;
	time_t		curTime = time(NULL);
	if (counter == 0 || counter == DefaultCounter)
		return NULL;
	for (curRec = gIPBanList; curRec; curRec = curRec->nextRec) {
		if ((trackerFlag != 0) != ((curRec->banFlags & BR_Tracking) != 0))
			continue;
		/* 1/14/97 JAB Don't routine records for guest bans... */
		if ((curRec->banFlags & BR_PuidKey) > 0 && counter == curRec->regKey && curRec->penaltyTime - (curTime - curRec->timeLastPenalized) > 0)
			return curRec;
	}
	return NULL;
}

/* 6/14/96 New: Gods may track and kill users by KEY
 *
 */

/* 1/14/97 JAB fixed to also work for demo-users */
int BanUserByKey(ServerUserPtr whoKilled, char *key, short why,
				  LONG penaltyMinutes, short siteFlag)
{
	LONG	WizKeytoSeed(char *seedStr);
	LONG	ctr,crc;
	IPBanRec	rec;
	char	userName[32]="",*p;
	Boolean	isDemoUser = false;

	/* Parse out optional username */
	if ((p = strchr(key,' ')) != NULL) {
		*p = 0;
		++p;
		if (strlen(p) > 31)
			p[31] = 0;
		strcpy(userName,p);
	}

	if (toupper(*key) == 'Z') {
		isDemoUser = true;
		ctr = WizKeytoSeed(key+1);
	}
	else {
		ctr = WizKeytoSeed(key);
		if (ctr < 0 || ctr > 300000000L)
			return -1;
	}
	crc = ComputeLicenseCRC(ctr);

	if (isDemoUser)
		ctr = ctr ^ crc;
	else
		ctr = ctr ^ MAGIC_LONG ^ crc;

	rec.banFlags = siteFlag;
	if (isDemoUser)
		rec.banFlags |= BR_PuidKey;

	rec.ipAddress = 0;
	rec.regKey = ctr;
	rec.timeLastPenalized = time(NULL);
	if (penaltyMinutes < 0)
		rec.penaltyTime = 0x7FFFFFFF;
	else
		rec.penaltyTime = penaltyMinutes*60L;
	rec.whyBanned = why;
	sprintf(rec.verbalIP,"key = %s",key);
	sprintf(rec.userName,userName);

	if (whoKilled)
		strcpy(rec.whoKilled, CvtToCString(whoKilled->user.name));
	else
		rec.whoKilled[0] = 0;
	rec.comment[0] = 0;
	InsertBanRec(&rec);
	return 0;
}


void BanUserByIP(ServerUserPtr whoKilled, ServerUserPtr cUser, short why,
				LONG penaltyMinutes, short siteFlag) 
{
	IPBanRec	rec;
	rec.banFlags = siteFlag;
	rec.ipAddress = GetIPAddress(cUser);

	/* 1/14/97 JAB Fixed to track guests using new client by guest GUID # */
	if (cUser->puidCtr)
	{
		rec.regKey = cUser->puidCtr;
		rec.banFlags |= BR_PuidKey;
	}
	else
		rec.regKey = cUser->counter;
	rec.timeLastPenalized = time(NULL);
	if (penaltyMinutes < 0)
		rec.penaltyTime = 0x7FFFFFFF;
	else
		rec.penaltyTime = penaltyMinutes*60L;
	rec.whyBanned = why;
	ConvertNetAddressToString(cUser, rec.verbalIP);
	strcpy(rec.userName, CvtToCString(cUser->user.name));
	if (whoKilled)
		strcpy(rec.whoKilled, CvtToCString(whoKilled->user.name));
	else
		rec.whoKilled[0] = 0;
	rec.comment[0] = 0;
	InsertBanRec(&rec);
}

void BanIP(ServerUserPtr whoKilled, unsigned LONG ipAddress, LONG penaltyMinutes, short siteFlag) 
{
	IPBanRec	rec;
	rec.banFlags = siteFlag;
	rec.ipAddress = ipAddress;
	rec.regKey = 0;
	rec.timeLastPenalized = time(NULL);
	if (penaltyMinutes < 0)
		rec.penaltyTime = 0x7FFFFFFF;
	else
		rec.penaltyTime = penaltyMinutes*60L;
	if (siteFlag & BR_Tracking)
		rec.whyBanned = K_Unknown;
	else
		rec.whyBanned = K_KilledBySysop;
	ConvertIPToString(ipAddress, rec.verbalIP);
	rec.userName[0] = 0;
	if (whoKilled)
		strcpy(rec.whoKilled, CvtToCString(whoKilled->user.name));
	else
		rec.whoKilled[0] = 0;
	rec.comment[0] = 0;
	InsertBanRec(&rec);
}


void InsertBanRec(IPBanRecPtr	recPtr)
{
	IPBanRecPtr	newRec,curRec;
	char		ipStr[64];

	/* if there's an exact match, update it */
	for (curRec = gIPBanList; curRec; curRec = curRec->nextRec) {
		ConvertIPToString(curRec->ipAddress, ipStr);
		if (curRec->banFlags == recPtr->banFlags &&
			curRec->ipAddress == recPtr->ipAddress &&
			curRec->regKey == recPtr->regKey) {
			curRec->timeLastPenalized = recPtr->timeLastPenalized;
			curRec->penaltyTime = recPtr->penaltyTime;
			curRec->whyBanned = recPtr->whyBanned;
			if (recPtr->verbalIP[0] && strcmp(ipStr, curRec->verbalIP) == 0) {
				strcpy(curRec->verbalIP, recPtr->verbalIP);
			}
			if (recPtr->userName[0]) {
				strcpy(curRec->userName, recPtr->userName);
			}
			return;
		}
	}

	newRec = (IPBanRecPtr) NewPtrClear(sizeof(IPBanRec));
	if (newRec) {
		*newRec = *recPtr;
		newRec->nextRec = gIPBanList;
		gIPBanList = newRec;
	}
}

void SaveBanRecs()
{
	IPBanRecPtr	curRec;

	if (gIPBanList == NULL)
		return;
    AddScriptLine(";\r");
    AddScriptLine("; Ban Records\r");
    AddScriptLine(";\r");

	for (curRec = gIPBanList; curRec; curRec = curRec->nextRec) {
		if (time(NULL) - curRec->timeLastPenalized > curRec->penaltyTime)
			continue;
		AddScriptLine("BANREC ");

		/* 1/14/97 JAB Changed first 4 to unsigned */
		AddScriptLine("0x%-8lx 0x%-8lx 0x%-8lx 0x%-8lx %ld %d ",
					(unsigned long) curRec->banFlags,
					(unsigned long) curRec->ipAddress,
					(unsigned long) curRec->regKey,
					(unsigned long) curRec->timeLastPenalized,
					(long) curRec->penaltyTime,
					(int) curRec->whyBanned);
		AddScriptCString(curRec->verbalIP);
		AddScriptLine(" ");
		AddScriptCString(curRec->userName);
		AddScriptLine(" ");
		AddScriptCString(curRec->whoKilled);
		AddScriptLine(" ");
		AddScriptCString(curRec->comment);
		AddScriptLine("\r");
	}
    AddScriptLine(";\r");
}

void ParseBanRec()
{
	IPBanRec	rec;
	extern char *gToken;

	LongParse(&rec.banFlags);
	LongParse(&rec.ipAddress);
	LongParse((long *) &rec.regKey);
	LongParse(&rec.timeLastPenalized);
	LongParse(&rec.penaltyTime);
	LongParse(&rec.whyBanned);
	GetCString(&rec.verbalIP[0]);
	GetCString(&rec.userName[0]);

	GetToken();
	if (gToken[0] == '\"') {
		/* remove quotes */
		strcpy(gToken,gToken+1);
		gToken[strlen(gToken)-1] = 0;
		strcpy(rec.whoKilled,gToken);

		GetCString(&rec.comment[0]);
	}
	else {
		UngetToken();
		rec.whoKilled[0] = 0;
		rec.comment[0] = 0;
	}
	

	InsertBanRec(&rec);
}

void CommentBanRec(ServerUserPtr cUser, char *str)
{
	IPBanRecPtr	curRec;
	char		frag[256]="",comment[256]="",ipStr[64];
	Boolean		gotSome=false;
	sscanf(str,"%s %s",frag,comment);
	if (frag[0] == 0) {
		UserMessage(cUser,"Syntax: comment ipAddr|name string");
		return;
	}
	strcpy(comment,&str[strlen(frag)]);	/* copy in entire comment, including spaces */
	while (comment[0] && isspace(comment[0])) /* remove leading spaces */
		strcpy(comment, &comment[1]);
	if (comment[0] == 0) {
		UserMessage(cUser,"Syntax: comment ipAddr|name string");
		return;
	}
	curRec = gIPBanList;
	for (curRec = gIPBanList; curRec; curRec = curRec->nextRec) {
		ConvertIPToString(curRec->ipAddress, ipStr);
		if (strstr(curRec->verbalIP, frag) != NULL ||
			strstr(curRec->userName,frag) != NULL ||
			strstr(ipStr,frag) != NULL) {
			sprintf(curRec->comment,"%s: %s",CvtToCString(cUser->user.name),comment);
			UserMessage(cUser,"Ban record(s) commented");
			gotSome = true;
		}
	}
	if (!gotSome)
		UserMessage(cUser,"Ban record for %s not found",frag);
}


void ExtendBanRec(ServerUserPtr cUser, char *str)
{
	IPBanRecPtr	curRec;
	char		frag[256]="",minStr[256]="",ipStr[64];
	int			penaltyMinutes;
	Boolean		gotSome=false;

	sscanf(str,"%s %s",frag,minStr);
	if (frag[0] == 0) {
		UserMessage(cUser,"Syntax: extend ipAddr|name minutes");
		return;
	}
	strcpy(minStr,&str[strlen(frag)]);	/* copy in entire minStr, including spaces */
	while (minStr[0] && isspace(minStr[0])) /* remove leading spaces */
		strcpy(minStr, &minStr[1]);
	if (minStr[0] == 0 || !isdigit(minStr[0])) {
		UserMessage(cUser,"Syntax: extend ipAddr|name minutes");
		return;
	}
	penaltyMinutes = atoi(minStr);
	curRec = gIPBanList;
	for (curRec = gIPBanList; curRec; curRec = curRec->nextRec) {
		ConvertIPToString(curRec->ipAddress, ipStr);
		if (strstr(curRec->verbalIP, frag) != NULL ||
			strstr(curRec->userName,frag) != NULL ||
			strstr(ipStr,frag) != NULL) 
		{
			curRec->timeLastPenalized = time(NULL);
			if (penaltyMinutes < 0)
				curRec->penaltyTime = 0x7FFFFFFF;
			else
				curRec->penaltyTime = penaltyMinutes*60L;
			UserMessage(cUser,"Ban record(s) extended");
			gotSome = true;
		}
	}
	if (!gotSome)
		UserMessage(cUser,"Ban record for %s not found",frag);
}

void TrackCheck(ServerUserPtr cUser)
{
	IPBanRecPtr ipRec = NULL;
	char		numBuf[64];
	char		akaStr[64] = "";

	/* 1/14/97 JAB */
	if (cUser->puidCtr)
		ipRec = GetIPBanRecByPuidKey(cUser->puidCtr, 1);
	if (ipRec == NULL && !NewbieUserSerialNumber(cUser->crc, cUser->counter))
		ipRec = GetIPBanRecByCounter(cUser->counter, 1);
	if (ipRec == NULL) {
		ipRec = GetIPBanRecByIP(GetIPAddress(cUser), 1);
		if (ipRec == NULL)
			return;
	}
	ConvertNetAddressToNumericString(cUser,numBuf);
	if (ipRec->userName[0] && strcmp(ipRec->userName,CvtToCString(cUser->user.name)) != 0) {
		sprintf(akaStr," (a.k.a. %s)",ipRec->userName);
	}
	WizGlobalMessage("Page from *Tracker: %s%s [%s] signed on",
		CvtToCString(cUser->user.name),akaStr,numBuf);
}
