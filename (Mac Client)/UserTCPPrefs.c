// UserTCPPrefs.c
#include "U-USER.H"
#include "UserTCP.h"
#include "AppMenus.h"
#include "DialogUtils.h"

#define HostnameDLOG		501
// #define ServerMENU			144

#define P_NickItem			3
#define P_HostItem			4
#define P_PortItem			5
#define P_ServMenuItem		6
//#define P_SerPortItem		10
//#define P_SerBaudItem		11

void LoadPortName(DialogPtr dp);
void LoadPortName(DialogPtr dp)
{
	Str63		portName;
	Str255		hostName;
	if (gPrefs.remotePort == 9998 || gPrefs.remotePort == 0)
		sprintf((char *) portName, "<default>");
	else
		sprintf((char *) portName, "%d",gPrefs.remotePort);
	CtoPstr((char *) portName);
	SetText(dp,P_PortItem, portName);
	strcpy((char *) hostName,gPrefs.remoteHost);
	// BlockMove(gPrefs.remoteHost,hostName,strlen(gPrefs.remoteHost[0]+1);
	CtoPstr((char *) hostName);
	SetText(dp,P_HostItem, hostName);
}

short HostnameDialog(short connectType)
{
	GrafPtr		savePort;
	DialogPtr 	dp;
	short		itemHit;
	Str63		portName;
	Str255		hostName;

	GetPort(&savePort);

	// if (connectType == C_PalaceTelnet) {
	//	gPortMenu = GetMenu(PortMENU);
	//	gBaudMenu = GetMenu(BaudMENU);
	// }

	if ((dp = GetNewDialog (HostnameDLOG, NULL, (WindowPtr) -1)) == NULL)
		return Cancel;
	
/**
	if (connectType == C_PalaceTelnet) {
		switch (gPrefs.serPort) {
		case 0:		SetControl(dp, P_SerPortItem, 1);	break;
		case 1:		SetControl(dp, P_SerPortItem, 2);	break;
		}
		switch (gPrefs.serBaud) {
		case baud1200:		SetControl(dp, P_SerBaudItem, 1);	break;
		case baud2400:		SetControl(dp, P_SerBaudItem, 2);	break;
		case baud9600:		SetControl(dp, P_SerBaudItem, 3);	break;
		case baud19200:		SetControl(dp, P_SerBaudItem, 4);	break;
		case baud57600:		SetControl(dp, P_SerBaudItem, 5);	break;
		}
	}
	else {
		HideDItem(dp, P_SerPortItem);
		HideDItem(dp, P_SerBaudItem);
	}
**/
	LoadPortName(dp);

	// 1/14/97 JAB Changed to use "your name" if it's set to "Guest"
	if (EqualString(gPrefs.name,"\pGuest",false,false))
		SetText(dp,P_NickItem, "\pYour Name");
	else
		SetText(dp,P_NickItem, gPrefs.name);

	SelIText(dp,P_NickItem,0,32767);

	SetDialogDefaultItem(dp, OK);
	SetDialogCancelItem(dp, Cancel);
	SetDialogTracksCursor(dp,true);

	ShowWindow(dp);
	SetPort(dp);
	SetCursor(&qd.arrow);
	ShowCursor();
	do {
		ModalDialog(NULL, &itemHit);
		if (itemHit == P_ServMenuItem && gPrefs.nbrNetaddresses) {
			MenuHandle	mh;
			Str63		nAddress;
			Rect		r;
			short		t,i,n;
			Handle		h;
			long		menuSelect;
			// if (gPrefs.nbrNetaddresses == 0)
			//	return NULL;
			mh = NewMenu(ServerMENU,"\p");
			for (i = 0; i < gPrefs.nbrNetaddresses; ++i) {
				strcpy((char *) nAddress,gPrefs.nAddr[i].name);
				CtoPstr((char *) nAddress);
				AppendMenu(mh,nAddress);
			}
			InsertMenu(mh,hierMenu);
			GetDItem(dp,P_ServMenuItem,&t,&h,&r);
			LocalToGlobal(&topLeft(r));
			menuSelect = PopUpMenuSelect(mh,r.top+7,r.left+22,1);
			n = LoWord(menuSelect) - 1;
			if (n >= 0 && n < gPrefs.nbrNetaddresses) {
				char	*p;
				strcpy(gPrefs.remoteHost, gPrefs.nAddr[n].name);
				if ((p = strrchr((char *) gPrefs.remoteHost,':')) != NULL)
					*p = 0;
				gPrefs.remotePort = gPrefs.nAddr[n].portNumber;
				LoadPortName(dp);
			}
			DeleteMenu(ServerMENU);
			DisposeMenu(mh);
		}
	} while (itemHit != OK && itemHit != Cancel);
	if (itemHit == OK) {
		GetText(dp,P_NickItem, gPrefs.name);
		GetText(dp,P_HostItem, hostName);
		GetText(dp,P_PortItem, portName);
		PtoCstr(hostName);
		PtoCstr(portName);
		strcpy(gPrefs.remoteHost,(char *) hostName);
		gPrefs.remotePort = atoi((char *) portName);
		if (gPrefs.remotePort == 0)
			gPrefs.remotePort = 9998;

/**
		if (connectType == C_PalaceTelnet) {
			short	n;
			n = GetControl(dp, P_SerPortItem);
			gPrefs.serPort = n-1;
			n = GetControl(dp, P_SerBaudItem);
			switch (n) {
			case 1:	gPrefs.serBaud = baud1200;	break;
			case 2:	gPrefs.serBaud = baud2400;	break;
			case 3:	gPrefs.serBaud = baud9600;	break;
			case 4:	gPrefs.serBaud = baud19200;	break;
			case 5:	gPrefs.serBaud = baud57600;	break;
			}
		}
**/
		StorePreferences();
	}
	DisposDialog(dp);
	SetPort(savePort);
	return itemHit;
}

void StoreNetAddress(char *name, long ip_addr, short port);

void StoreNetAddress(char *name, long ip_addr, short port)
{
	// Find if it's in there, by ip_addr
	short		i;
	NetAddress	*np,tmp;
	np = &gPrefs.nAddr[0];
	for (i = 0; i < gPrefs.nbrNetaddresses; ++i,++np) {
		if (np->ip_addr == ip_addr && np->portNumber == port) {
			tmp = *np;
			strcpy(tmp.name,name);
			tmp.portNumber = port;
			// Sort it to the top (this'll keep last used one at bottom)
			while (i > 0) {
				gPrefs.nAddr[i] = gPrefs.nAddr[i-1];
				--i;
			}
			gPrefs.nAddr[0] = tmp;
			StorePreferences();
			return;
		}
	}
	for (i = gPrefs.nbrNetaddresses-1; i >= 0; --i) {
		if (i < MaxNetaddresses-1)
			gPrefs.nAddr[i+1] = gPrefs.nAddr[i];
	}
	np = &gPrefs.nAddr[0];
	strcpy(np->name,name);
	if (port != 9998) {
		sprintf(&np->name[strlen(np->name)],":%d",port);
	}
	np->ip_addr = ip_addr;
	np->portNumber = port;
	if (gPrefs.nbrNetaddresses < MaxNetaddresses)
		++gPrefs.nbrNetaddresses;
	StorePreferences();
}

