// ServerYellowPages.c
//
// todo: get user name from whereever it comes from...
//		


#include "s-server.h"
#include "dialogutils.h"
#include <time.h>
#include <MacTCP.h>
#include "TCPHi.h"
#include "TCPRoutines.h"

#define YP_AutoRegisterFlag	0x01
#define YPPrefsType		'yPrf'
#define SCreator		'mSrv'
#define SFType			'rsrc'
#define DefYPPort		9991

extern Str31		prefsName;

void RegisterOnYellowPages(Boolean forceFlag, Boolean upFlag);
void YPDialog();
void MyAddrToStr(long myAddr, char *name);
OSErr ConvertStringToAddr(char *name,unsigned long *netNum);
void CleanString(char *str);

void CleanString(char *str) 
{
	// Remove illegal characters from string for sending to yellow pages server
	while (*str) {
		switch (*str) {
		case '\r':
		case '\n':
			*str = ' ';
			break;
		case '\"':
			*str = '\'';
			break;
		}
		++str;
	}
}

unsigned LONG	ypLastCheck;
int				ypLastStatus = 0;

void YellowPagesIdle()
{
#if !CRIPPLEDSERVER
	unsigned LONG	delta;
	if (!gPrefs.autoRegister)
		return;
	delta = time(NULL) - ypLastCheck;
	if ((ypLastStatus && delta > 60*5L) ||
		(!ypLastStatus && delta > 60*20L)) {
		RegisterOnYellowPages(false,true);
		ypLastCheck = time(NULL);
	}
#endif
}

unsigned long gYellowAddr;
short		  gYellowPort;

void RegisterOnYellowPages(Boolean forceFlag, Boolean upFlag)
{
	char			regStr[1024],serStr[32]="",serverName[32],ascFlags[12] = "";
	char			url[128];
	unsigned long	yellowAddr;
	short			yellowPort=DefYPPort;
	OSErr			oe;
	unsigned long	ypStream;
	void GetServerSerialNumber(char *str);

	if (gMaxPeoplePerServer <= 3)
	{
		LogMessage("Palace Directory registration is disabled on this demonstration version\r");
		return;
	}
	
	// Check Prefs, if not requested, and force is off, don't do it
	if (!forceFlag && !gPrefs.autoRegister)
		return;
	if (!gPrefs.allowTCP)
		return;

	GetServerSerialNumber(serStr);

	if (upFlag)
		LogMessage("Registering with Palace Directory service...\r");
	else
		LogMessage("Unregistering with Palace Directory service...\r");
	// Get Current Servername from Prefs
	BlockMove(gPrefs.serverName, serverName,gPrefs.serverName[0]+1);
	PtoCstr((StringPtr) serverName);

	// Get Other useful info
	if (!upFlag) {
		strcpy(ascFlags,"X");
	}
	// Fill in our dynamic IP Address
	if (gPrefs.url[0] == 0)
	{
		unsigned long	myAddr;
		char	name[128] = "";
		GetMyIP(&myAddr);
		MyAddrToStr(myAddr,name);
		sprintf(url,"palace://%s",name);
		if (gPrefs.localPort != 9998) {
			sprintf(&url[strlen(url)],":%d",gPrefs.localPort);
		}
	}
	else
		strcpy(url,gPrefs.url);

	sprintf(regStr,"\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,%d\n",
			serverName,
			gPrefs.sysop,
			url,
			gPrefs.machineType,
			gPrefs.description,
			gPrefs.announcement,
			ascFlags,
			serStr,
			(int) gNbrUsers,
			(int) gNbrRooms);

	
	// Determine IP Address of yp service

	// 9/3/96 Fixed to not lag so bad - saves previous name
	//
	if (gYellowAddr == 0)
	{
		char	tstr[81],*p;
		strcpy(tstr,gPrefs.ypIPAddr);
		if ((p = strrchr(tstr,':')) != NULL) {
			*p = 0;
			++p;
			yellowPort = atoi(p);
		}
		oe = ConvertStringToAddr(tstr,&yellowAddr);	
		if (oe != noErr) {
			LogMessage("Can't locate yellow pages site\r");
			ypLastStatus = 1;
			return;
		}
		gYellowAddr = yellowAddr;
		gYellowPort = yellowPort;
	}
	else {
		yellowAddr = gYellowAddr;
		yellowPort = gYellowPort;
	}
	// Create Socket
	oe = CreateStream(&ypStream,4096L);
	if (oe != noErr) {
		LogMessage("Can't create yp stream\r");
		ypLastStatus = 1;
		return;
	}

	// Establish Connection
	oe = OpenConnection(ypStream,yellowAddr,yellowPort,20);
	if (oe != noErr) {
		LogMessage("Can't establish Palace Directory connection\r");
		LogMessage("The Palace Directory Server may be down\r");
		ReleaseStream(ypStream);
		return;
	}
	// Output String
	oe = SendData(ypStream,regStr,(unsigned short) strlen(regStr),2);
	if (oe != noErr) {
		LogMessage("YP Server not responding\r");
		MyCloseConnection(ypStream);
		ReleaseStream(ypStream);
		return;
	}

	// Terminate Connection
	MyCloseConnection(ypStream);
	ReleaseStream(ypStream);
	LogMessage("Registration complete\r");
	ypLastCheck = time(NULL);
	ypLastStatus = 0;
}


#define YP_DLOG			506
#define YP_AutoReg		3
// #define YP_Register		4
#define YP_IPAddr		4
#define YP_ServName		5
#define YP_Operator		6
#define YP_ServURL		7
#define YP_Machine		8
#define YP_Description	9
#define YP_Announcement	10

void YPDialog()
{
	GrafPtr	savePort,dp;
	short	itemHit;

	GetPort(&savePort);

	if ((dp = GetNewDialog (YP_DLOG, NULL, (WindowPtr) -1)) == NULL)
		return;

	PrintfItem(dp, YP_IPAddr, "%s", gPrefs.ypIPAddr);
	SetText(dp, YP_ServName, gPrefs.serverName);
	PrintfItem(dp, YP_Operator, "%s", gPrefs.sysop);
	if (gPrefs.url[0] == 0)
		PrintfItem(dp, YP_ServURL, "<dynamic>");
	else
		PrintfItem(dp, YP_ServURL, "%s", gPrefs.url);
	PrintfItem(dp, YP_Machine, "%s", gPrefs.machineType);
	PrintfItem(dp, YP_Description, "%s", gPrefs.description);
	PrintfItem(dp, YP_Announcement, "%s", gPrefs.announcement);

	SetDialogDefaultItem(dp, OK);
	SetDialogCancelItem(dp, Cancel);
	SetDialogTracksCursor(dp,true);

	SetControl(dp, YP_AutoReg, gPrefs.autoRegister);
	SelIText(dp,YP_ServName,0,32767);
	ShowWindow(dp);
	SetPort(dp);
	SetCursor(&qd.arrow);
	ShowCursor();
	do {
		ModalDialog(NULL, &itemHit);
		switch (itemHit) {
		case YP_AutoReg:
			SetControl(dp, YP_AutoReg, !GetControl(dp, YP_AutoReg));
			break;
		}
	} while (itemHit != OK && itemHit != Cancel);
	if  (itemHit == OK) {
		// Save Settings
		Str255	temp;
		if (GetControl(dp, YP_AutoReg))
			gPrefs.autoRegister = true;
		else
			gPrefs.autoRegister = false;
		GetText(dp, YP_IPAddr, (StringPtr) gPrefs.ypIPAddr);		PtoCstr( (StringPtr) gPrefs.ypIPAddr);

		GetText(dp, YP_ServName, (StringPtr) temp);
		if (temp[0] > 31)
			temp[0] = 31;
		BlockMove(temp,gPrefs.serverName,temp[0]+1);

		GetText(dp, YP_Operator,  (StringPtr) gPrefs.sysop);		PtoCstr( (StringPtr) gPrefs.sysop);
		GetText(dp, YP_ServURL,  (StringPtr) gPrefs.url);			PtoCstr( (StringPtr) gPrefs.url);
		if (gPrefs.url[0] == '<')
			gPrefs.url[0] = 0;
		GetText(dp, YP_Machine,  (StringPtr) gPrefs.machineType);	PtoCstr( (StringPtr) gPrefs.machineType);
		GetText(dp, YP_Description,  (StringPtr) gPrefs.description);	PtoCstr( (StringPtr) gPrefs.description);
		GetText(dp, YP_Announcement,  (StringPtr) gPrefs.announcement);	PtoCstr( (StringPtr) gPrefs.announcement);
		CleanString(gPrefs.sysop);
		CleanString(gPrefs.url);
		CleanString(gPrefs.machineType);
		CleanString(gPrefs.description);
		CleanString(gPrefs.announcement);
		StorePreferences();
	}
	DisposeDialog(dp);
	SetPort(savePort);
	if (gPrefs.autoRegister)
		RegisterOnYellowPages(true,true);
	else
		RegisterOnYellowPages(true,false);
}