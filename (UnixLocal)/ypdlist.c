/* YPDList.c */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>

#define kYPDName 	"ypd.dat"

typedef struct YRec {
	struct YRec	*next;
	char	serverTitle[128];	/* c strings all */
	char	sysop[64];
	char	url[128];
	char	hostMachine[128];
	char	description[256];
	char	announcement[256];
	char	flags[32];
	char	serverRegNumber[32];
	int		nbrUsersLastOn;
	int		nbrRooms;
	int		timeLastChecked;	/* provided by server from here down */
	int		ipAddr;
	int		port;
} YRec, *YRecPtr;

YRecPtr	topList;

enum { R_Bad, R_Newbie, R_Member, R_OldStyle, 
		R_HackedSerialNumber, 
		R_BetaFreebie, R_VIPFreebie};


enum { B_OK, B_InsufficientInfo, B_Error, B_Censored, B_BadSerialNumber,
	B_OldListing};

int BadListing(YRecPtr cp)
{
	FILE	*file;
	char	pline[512],temp[512];
	int	len;
	regex_t	regex;
	int	retCode = B_OK;

	if (cp->serverTitle[0] == 0 ||
		cp->url[0] == 0 ||
		cp->sysop[0] == 0)
			return B_InsufficientInfo;

	file = fopen("ypd.dic","r");
	if (file == NULL)
		return B_Error;

	while (fgets(pline,128,file) && !retCode) {
		if (pline[0] == ';' || pline[0] == '#' ||
			pline[0] == ' ' || pline[0] == 0 ||
			pline[0] == '\n' || pline[0] == '\r')
			continue;
		len = strlen(pline);
		if (pline[len-1] == '\r' || pline[len-1] == '\n')
			pline[len-1] = 0;

		if (regcomp(&regex,pline,REG_EXTENDED | REG_ICASE | REG_NOSUB)
			== 0) {
			if (regexec(&regex,cp->serverTitle,0,NULL,0) == 0 ||
				regexec(&regex,cp->sysop,0,NULL,0) == 0 ||
				regexec(&regex,cp->url,0,NULL,0) == 0 ||
				regexec(&regex,cp->hostMachine,0,NULL,0) == 
0 ||
				regexec(&regex,cp->description,0,NULL,0) == 
0 ||
				regexec(&regex,cp->announcement,0,NULL,0) == 
0)
				retCode = B_Censored;
			regfree(&regex);
		}

	}
	fclose(file);
	switch (CheckRegNumber(cp->serverRegNumber)) {
	case R_Member:
	case R_BetaFreebie:
	case R_VIPFreebie:
		break;
	default:
		retCode = B_BadSerialNumber;
	}

	if (retCode == B_OK && time(NULL) - cp->timeLastChecked > 300L*60L)
		retCode = B_OldListing;

	return retCode;
}

void InsertRecIntoList(YRecPtr rec)
{
	YRecPtr	cp,tmp,newRec;

	if (rec->serverRegNumber[0] == 0)	/* 2/29/95 don't add blank ser# entries */
		return;

	for (cp = topList; cp; cp = cp->next) {
		if (rec->serverRegNumber[0] && strcmp(cp->serverRegNumber,rec->serverRegNumber) == 0)
			break;
	}
	if (cp) {
		/* Replace existing entries */
		tmp = cp->next;
		*cp = *rec;
		cp->next = tmp;
	}
	else {
		newRec = (YRecPtr) malloc(sizeof(YRec));
		*newRec = *rec;
		newRec->next = topList;
		topList = newRec;
	}
}

char *ParseString(char *str, char *p)
{
	*str = 0;
	if (*p == '\"') {
		++p;
		while (*p && *p != '\"')
			*(str++) = *(p++);
		*str = 0;
		if (*p == '\"')
			++p;
	}
	else {
		while (*p && *p != ',')
			*(str++) = *(p++);
		*str = 0;
	}
	while (isspace(*p))
		++p;
	if (*p == ',')
		++p;
	while (isspace(*p))
		++p;
	return p;
}

char *ParseNumber(int *num, char *p)
{
	char	*str,tbuf[32];
	str = &tbuf[0];
	*str = 0;
	while (*p && *p != ',')
		*(str++) = *(p++);
	*str = 0;
	while (isspace(*p))
		++p;
	if (*p == ',')
		++p;
	while (isspace(*p))
		++p;
	*num = atoi(tbuf);
	return p;
}

void LoadListing()	/* load the file */
{
	FILE	*ifile;
	YRec	nRec;
	char	tbuf[1024],*p;
	short	n;

	ifile = fopen(kYPDName,"r");
	if (ifile == NULL)
		return;
	while (fgets(tbuf,1024,ifile)) {
		if (tbuf[0] == '#' || tbuf[0] == ';')
			continue;
		p = tbuf;

		nRec.next = NULL;
		p = ParseString(nRec.serverTitle,p);
		p = ParseString(nRec.sysop,p);
		p = ParseString(nRec.url,p);
		p = ParseString(nRec.hostMachine,p);
		p = ParseString(nRec.description,p);
		p = ParseString(nRec.announcement,p);
		p = ParseString(nRec.flags,p);
		p = ParseString(nRec.serverRegNumber,p);
		p = ParseNumber(&nRec.nbrUsersLastOn,p);
		p = ParseNumber(&nRec.nbrRooms,p);
		p = ParseNumber(&nRec.timeLastChecked,p);
		p = ParseNumber(&nRec.ipAddr,p);
		p = ParseNumber(&nRec.port,p);
		if ((n = BadListing(&nRec)) != B_OK) {
			switch (n) {
			case B_InsufficientInfo:
				strcpy(nRec.flags,"XINC");
				break;
			case B_Error:
				strcpy(nRec.flags,"XERR");
				break;
			case B_Censored:
				strcpy(nRec.flags,"XCEN");
				break;
			case B_BadSerialNumber:
				strcpy(nRec.flags,"XSER");
				break;
			case B_OldListing:
				strcpy(nRec.flags,"XOLD");
				break;
			default:
				strcpy(nRec.flags,"X??");
				break;
			}
		}
		InsertRecIntoList(&nRec);
	}
	fclose(ifile);
}

void SaveListing() /* save the file */
{
	FILE	*ofile;
	YRecPtr	cp;
	ofile = fopen(kYPDName,"w");
	if (ofile == NULL)
		return;
	fprintf(ofile,"# This Listing Automatically Generated\n");
	for (cp = topList; cp; cp = cp->next) {
		if (cp->serverTitle[0] == 0 ||
			cp->url[0] == 0 ||
			cp->sysop[0] == 0)
			continue;

		/* 2/7/96 */
		if (cp->flags[0] == 0 && time(NULL) - cp->timeLastChecked > 300L*60L) {
			strcpy(cp->flags,"XOLD");
		}

		fprintf(ofile,"\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,%d,%d,%d,%d\n",
			cp->serverTitle,
			cp->sysop,
			cp->url,
			cp->hostMachine,
			cp->description,
			cp->announcement,
			cp->flags,
			cp->serverRegNumber,
			cp->nbrUsersLastOn,
			cp->nbrRooms,
			cp->timeLastChecked,
			cp->ipAddr,
			cp->port);
	}
	fclose(ofile);
}

/* parse the buffer, add to the list */
void UpdateListing(char *tbuf, int ipAddr)
{
	YRec	nRec;
	char	*p;
	int	n;

	time(&nRec.timeLastChecked);
	nRec.ipAddr = ipAddr;
	nRec.port = 9998;
	nRec.next = NULL;

	p = tbuf;
	p = ParseString(nRec.serverTitle,p);
	p = ParseString(nRec.sysop,p);
	p = ParseString(nRec.url,p);
	p = ParseString(nRec.hostMachine,p);
	p = ParseString(nRec.description,p);
	p = ParseString(nRec.announcement,p);
	p = ParseString(nRec.flags,p);
	p = ParseString(nRec.serverRegNumber,p);
	p = ParseNumber(&nRec.nbrUsersLastOn,p);
	p = ParseNumber(&nRec.nbrRooms,p);

	p= nRec.url;
	if (strchr(p,':')) {
		p = strchr(p,':') + 1;
		if (strrchr(p,':')) {
			p= strrchr(p,':')+1;
			nRec.port = atoi(p);
		}
	}
	if ((n = BadListing(&nRec)) != B_OK) {
		switch (n) {
		case B_InsufficientInfo:
			strcpy(nRec.flags,"XINC");
			break;
		case B_Error:
			strcpy(nRec.flags,"XERR");
			break;
		case B_Censored:
			strcpy(nRec.flags,"XCEN");
			break;
		case B_BadSerialNumber:
			strcpy(nRec.flags,"XSER");
			break;
		case B_OldListing:
			strcpy(nRec.flags,"XOLD");
			break;
		default:
			strcpy(nRec.flags,"X??");
			break;
		}
		LogMessage("Bad Listing: %s\n%s\n", nRec.flags,tbuf);
	}
	else {
		if (nRec.serverRegNumber[0] == 0) {
			LogMessage("Bad Listing: BLANK\n%s\n",tbuf);
		}
	}
	InsertRecIntoList(&nRec);
	SaveListing();
}
