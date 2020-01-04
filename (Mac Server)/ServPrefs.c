// ServPrefs.c

#include "S-SERVER.H"
#include "AppMenus.h"
#include <Folders.h>
#include "DialogUtils.h"
#include <Dialogs.h>

extern ServerPrefs		gPrefs;

#define ServerPrefsType	'sPrf'
#define SCreator		'mSrv'
#define SFType			'rsrc'
#define LockIcon		200

#define ServerPrefsVersion	0x00010013L

#ifndef THINK_C
#define OK		1
#define Cancel	2
#endif

#if !RELEASE
#define DefaultPermissions	(PM_AllowGuests | PM_AllowCyborgs | PM_AllowPainting |	\
						     PM_AllowCustomProps | PM_AllowWizards |				\
						     PM_WizardsMayKill | PM_WizardsMayAuthor |				\
						     PM_PurgeInactiveProps | PM_KillFlooders)
#else
#define DefaultPermissions	(PM_AllowGuests | PM_AllowCyborgs | PM_AllowPainting |	\
						     PM_AllowCustomProps | PM_AllowWizards |				\
						     PM_PurgeInactiveProps | PM_KillFlooders)
#endif
#define DefaultPalaceName		"\pThe Palace"
#define DefaultDeathPenalty		0
#define DefaultPurgeInactive	21
#define DefaultFloodEvents		100
#define DefaultTCPPort			9998
#define DefaultTextFont			geneva
#define DefaultTextSize			9
#define DefaultTextCreator		'ttxt'
#define DebugWizardPassword		"\x0b\xfd\x5a\x0b\x1b\x7b\x73\x20\x93\x5c\x81\x56"
#define DebugGodPassword		"\x05\x30\x94\x4a\x9c\x4d"
#define DefaultMaxOccupancy		3
#define DefaultRoomOccupancy	24
#define DefaultPicturesFolder	"\p:Pictures:"
#define DefaultRecycleLimit		10000

ServerPrefs	gPrefs = {ServerPrefsVersion, DefaultPalaceName, 
					  DefaultTextFont, DefaultTextSize, DefaultTextCreator, 
// Wizard Password (Superuser Status)
#if !RELEASE || unix	// abracadabra password
						DebugWizardPassword,
					// spitz password
						DebugGodPassword,
#else
						"\p",
						"\p",
#endif
// Owner Password (to unlock prefs)
						"\p",
						false,true,DefaultTCPPort,
						DefaultPermissions,DefaultDeathPenalty,
						DefaultPurgeInactive, DefaultFloodEvents,
						DefaultMaxOccupancy, DefaultRoomOccupancy,
						DefaultPicturesFolder,		// Old One
						"Joe Sysop",
						"",
						"Macintosh",
						"Generic Description",
						"",		// Announcement
						"mansion.thePalace.com",	// Yellow Pages Server
						true,						// auto register
						DefaultPicturesFolder,
						DefaultRecycleLimit,		// 4/9/95
						0,0,"",
						};

Str31		prefsName = "\pPServer Prefs";

#define PrefsDLOG			500
#define P_LockItem			3
#define P_NameItem			4
#define P_MaxOccupancy		5
#define P_RoomOccupancy		6
#define P_CreatorItem		7
#define P_CreatorMenu		8
#define P_FontItem			9
#define P_FontSizeItem		10
#define P_WPassItem			11
#define P_GPassItem			12
#define P_NetworkItem		13
#define P_UserItem			14
#define P_Permissions		15
#define P_PicFolder			16

#define W_UserItem			5
#define N_UserItem			8

void NetworkDialog();
void PermissionsDialog();

pascal void DefaultOKButton(DialogPtr dp, short itemNbr);

void LoadPreferences()
{
	Handle		h;
	short		volNum = 0,fRefNum = 0;
	long		dirID = 0L;
	long		length;

	FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,&volNum,&dirID);
	fRefNum = HOpenResFile(volNum,dirID,prefsName,fsRdPerm);
	if (fRefNum != -1) {
		h = Get1Resource(ServerPrefsType,128);
		if (h != NULL) {
			length = GetHandleSize(h) < sizeof(ServerPrefs)? GetHandleSize(h) : sizeof(ServerPrefs);
			if (((ServerPrefs *) *h)->versID == ServerPrefsVersion)
				BlockMove(*h, &gPrefs, length);
			ReleaseResource(h);
		}
		CloseResFile(fRefNum);
	}
}

OSErr CreatePrefsFile()
{
	OSErr	oe;
	short		volNum = 0,fRefNum = 0;
	long		dirID = 0L;

	FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,&volNum,&dirID);
	oe = HCreate(volNum, dirID, prefsName, SCreator, SFType);
	if (oe == dupFNErr) {
		HDelete(volNum, dirID, prefsName);
		oe = HCreate(volNum, dirID, prefsName, SCreator, SFType);
	}
	if (oe == noErr) {
		HCreateResFile(volNum, dirID, prefsName);
		oe = ResError();
	}
	return oe;
}

void StorePreferences()
{
	Handle		h,h2;
	short		volNum = 0,fRefNum = 0;
	long		dirID = 0L;

	FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,&volNum,&dirID);

	fRefNum = HOpenResFile(volNum,dirID,prefsName,fsRdWrPerm);
	if (fRefNum == -1) {
		if (CreatePrefsFile() != noErr)
			return;
		fRefNum = HOpenResFile(volNum,dirID,prefsName,fsRdWrPerm);
	}
	if (fRefNum == -1)
		return;

	h = NewHandle(sizeof(ServerPrefs));
	if (h) {
		SetResLoad(false);
		while ((h2 = Get1Resource(ServerPrefsType, 128)) != NULL) {
			RmveResource(h2);
			DisposHandle(h2);
		}
		SetResLoad(true);
		*((ServerPrefs *) *h) = gPrefs;
		AddResource(h,ServerPrefsType,128,"\pPreferences");
		WriteResource(h);
		ReleaseResource(h);
	}
	CloseResFile(fRefNum);
}

pascal void DrawLockItem(DialogPtr dp, short itemNbr)
{
	CIconHandle	cIcon;
	short		iconNbr,t;
	Rect		r;
	GrafPtr		savePort;
	Handle		h;
	GetPort(&savePort);
	SetPort(dp);

	GetDItem(dp,itemNbr,&t,&h,&r);
	EraseRect(&r);

	iconNbr = LockIcon + (gPrefs.ownerPassword[0]? 1 : 0);
	cIcon = GetCIcon(iconNbr);
	if (cIcon) {
		PlotCIcon(&r,cIcon);
		DisposeCIcon(cIcon);
	}
	SetPort(savePort);
}

pascal void PrefsButtonState(DialogPtr dp, short itemNbr)
{
  	if (gPrefs.ownerPassword[0]) {
		OutlineButton(dp,OK,&qd.white);
		OutlineButton(dp,Cancel,&qd.black);
	}
	else {
		OutlineButton(dp,Cancel,&qd.white);
		OutlineButton(dp,OK,&qd.black);
	}
}


pascal Boolean PrefsFilter(DialogPtr dp, EventRecord *ep, short *itemHit)
{
	char tempChar;
	switch (ep->what) {
	  case keyDown:
	  case autoKey:
		tempChar = ep->message & charCodeMask;
		if(tempChar==10 || tempChar==13 || tempChar==3) {
			if (gPrefs.ownerPassword[0])
				*itemHit = Cancel;
			else
				*itemHit = OK;
			SimulateButtonPress(dp,*itemHit);
			return true;
		}
		else if (tempChar == 27) {
			*itemHit = Cancel;
			SimulateButtonPress(dp,*itemHit);
			return true;
		}
		break;
	  default:
	  	break;

	}
	return false;
}

void PreferencesDialog()
{
	GrafPtr		savePort;
	DialogPtr 	dp;
	short		t,itemHit;
	Handle		h;
	Rect		r;
	short		n,fontCount;
	Str63		fontName,checkName;
	Str31		tmp;
	Boolean		isLocked;
	ModalFilterUPP	filterProc = NewModalFilterProc(PrefsFilter);
	UserItemUPP		prefsButtonProc = NewUserItemProc(PrefsButtonState);
	UserItemUPP		drawLockProc = NewUserItemProc(DrawLockItem);

	if (gLogWin->iconized)
		DeIconize();

	GetPort(&savePort);

	gFontMenu = GetMenu(FontMENU);
	gFontSizeMenu = GetMenu(FontSizeMENU);
	gCreatorMenu = GetMenu(CreatorMENU);
	
	AddResMenu(gFontMenu, 'FONT');
	
	isLocked = gPrefs.ownerPassword[0] > 0;

	if ((dp = GetNewDialog (PrefsDLOG, NULL, (WindowPtr) -1)) == NULL)
		return;
	
	GetDItem(dp,P_UserItem,&t,&h,&r);
	SetDItem(dp,P_UserItem,t,(Handle) prefsButtonProc, &r);

	SetText(dp,P_NameItem, gPrefs.serverName);
	SelIText(dp,P_NameItem,0,32767);

	SetText(dp, P_PicFolder, gPrefs.picFolder);

	GetDItem(dp,P_LockItem,&t,&h,&r);
	SetDItem(dp,P_LockItem,t,(Handle) drawLockProc, &r);

	GetFontName(gPrefs.fontID, fontName);
	fontCount = CountMItems(gFontMenu);
	for (n = 1; n <= fontCount; ++n){
		GetItem(gFontMenu, n, checkName);
		if (EqualString(checkName,fontName,false,false)) {
			CheckItem(gFontMenu, n, true);
			SetControl(dp, P_FontItem, n);
		}
	}
	
	sprintf((char *) &tmp[1],"%.4s",&gPrefs.textFileCreator);
	tmp[0] = 4;
	SetText(dp,P_CreatorItem,tmp);

	switch (gPrefs.textFileCreator) {
	case 'ttxt':	SetControl(dp, P_CreatorMenu, 1);	break;
	case 'R*ch':	SetControl(dp, P_CreatorMenu, 2);	break;
	case 'MSWD':	SetControl(dp, P_CreatorMenu, 3);	break;
	case 'KAHL':	SetControl(dp, P_CreatorMenu, 4);	break;
	default:		SetControl(dp, P_CreatorMenu, 6);	break;
	}

	switch (gPrefs.fontSize) {
	case 9:		SetControl(dp, P_FontSizeItem, 1);	break;
	case 10:	SetControl(dp, P_FontSizeItem, 2);	break;
	case 12:	SetControl(dp, P_FontSizeItem, 3);	break;
	case 14:	SetControl(dp, P_FontSizeItem, 4);	break;
	case 18:	SetControl(dp, P_FontSizeItem, 5);	break;
	case 24:	SetControl(dp, P_FontSizeItem, 6);	break;
	}

	PutIntItem(dp, P_MaxOccupancy, gPrefs.maxOccupancy);
	PutIntItem(dp, P_RoomOccupancy, gPrefs.roomOccupancy);

	ShowWindow(dp);
	SetPort(dp);
	SetCursor(&qd.arrow);
	ShowCursor();

  	if (gPrefs.ownerPassword[0]) {
		DisableButton(dp, P_WPassItem);
		DisableButton(dp, P_GPassItem);
		DisableButton(dp, P_Permissions);
		DisableButton(dp, P_NetworkItem);
		DisableButton(dp,OK);
	}
	else {
		EnableButton(dp, P_WPassItem);
		EnableButton(dp, P_GPassItem);
		EnableButton(dp, P_Permissions);
		EnableButton(dp, P_NetworkItem);
		EnableButton(dp,OK);
	}

	do {
		ModalDialog(filterProc, &itemHit);
		switch (itemHit) {
		case P_LockItem:
			if (PasswordDialog(tmp,"\pLock Password:") == OK) {
				EncryptString(tmp,tmp);
				if (gPrefs.ownerPassword[0]) {
					if (EqualPString(gPrefs.ownerPassword,tmp,false)) {
						gPrefs.ownerPassword[0] = 0;
						isLocked = false;
						StorePreferences();
					}
				}
				else {
					BlockMove(tmp,gPrefs.ownerPassword,tmp[0]+1);
					StorePreferences();
				}
				DrawLockItem(dp,P_LockItem);
			}
		  	if (gPrefs.ownerPassword[0]) {
				DisableButton(dp, P_WPassItem);
				DisableButton(dp, P_GPassItem);
				DisableButton(dp, P_Permissions);
				DisableButton(dp, P_NetworkItem);
				DisableButton(dp,OK);
			}
			else {
				EnableButton(dp, P_WPassItem);
				EnableButton(dp, P_GPassItem);
				EnableButton(dp, P_Permissions);
				EnableButton(dp, P_NetworkItem);
				EnableButton(dp,OK);
			}
			break;
		case P_WPassItem:
			if (!isLocked) {
				if (PasswordDialog(tmp,"\pWizard Password:") == OK) 
				{
					EncryptString(tmp,tmp);
					BlockMove(tmp,gPrefs.wizardPassword,tmp[0]+1);
					StorePreferences();
				}
			}
			break;
		case P_GPassItem:
			if (!isLocked) {
				if (PasswordDialog(tmp,"\pGod Password:") == OK) 
				{
					EncryptString(tmp,tmp);
					BlockMove(tmp,gPrefs.godPassword,tmp[0]+1);
					StorePreferences();
				}
			}
			break;
		case P_CreatorMenu:
			switch (GetControl(dp,P_CreatorMenu)) {
			case 1:	SetText(dp, P_CreatorItem, "\pttxt");	break;
			case 2:	SetText(dp, P_CreatorItem, "\pR*ch");	break;
			case 3:	SetText(dp, P_CreatorItem, "\pMSWD");	break;
			case 4:	SetText(dp, P_CreatorItem, "\pKAHL");	break;
			case 6:	SetText(dp, P_CreatorItem, "\p????");	break;
			}
			break;
		case P_NetworkItem:
			NetworkDialog();
			break;
		case P_Permissions:
			PermissionsDialog();
			break;
		}
	} while (itemHit != OK && itemHit != Cancel);

	if (itemHit == OK) {
		if (isLocked)
			ErrorMessage("Sorry, the server is locked");
		else {
			GetText(dp,P_NameItem, gPrefs.serverName);
			GetText(dp, P_PicFolder, gPrefs.picFolder);

			n = GetControl(dp, P_FontItem);
			GetItem(gFontMenu, n, fontName);
			GetFNum(fontName,&gPrefs.fontID);
	
			GetText(dp, P_CreatorItem, tmp);
			while (tmp[0] < 4) {
				tmp[tmp[0]+1] = ' ';
				tmp[0]++;
			}
			PtoCstr(tmp);
			gPrefs.textFileCreator = *((long *) tmp);
	
			n = GetControl(dp, P_FontSizeItem);
			switch (n) {
			case 1:	gPrefs.fontSize = 9;	break;
			case 2:	gPrefs.fontSize = 10;	break;
			case 3:	gPrefs.fontSize = 12;	break;
			case 4:	gPrefs.fontSize = 14;	break;
			case 5:	gPrefs.fontSize = 18;	break;
			case 6:	gPrefs.fontSize = 24;	break;
			}
			//GetText(dp, P_MaxOccupancy, tmp);
			gPrefs.maxOccupancy = GetIntItem(dp, P_MaxOccupancy);
				
			gPrefs.roomOccupancy = GetIntItem(dp, P_RoomOccupancy);
			if (gPrefs.maxOccupancy < 1)
				gPrefs.maxOccupancy = 1;
			if (gPrefs.maxOccupancy > gMaxPeoplePerServer)
				gPrefs.maxOccupancy = gMaxPeoplePerServer;
			if (gPrefs.roomOccupancy < 1)
				gPrefs.roomOccupancy = 1;
			if (gPrefs.roomOccupancy > MaxPeoplePerRoom)
				gPrefs.roomOccupancy = MaxPeoplePerRoom;
			StorePreferences();
			SendServerInfoToAll();
			RegisterOnYellowPages(false,true);
			SetWTitle((WindowPtr) gLogWin,gPrefs.serverName);
		}
	}
	DisposDialog(dp);
	SetPort(savePort);
}

#define WizardDLOG			502
#define W_PasswordItem		3

Str31	gPassword;

pascal Boolean WizDialogFilter(DialogPtr dp, EventRecord *ep, short *itemHit)
{
	char tempChar;
	switch (ep->what) {
	  case keyDown:
	  case autoKey:
		tempChar = ep->message & charCodeMask;
		switch (tempChar) {
		case 10:
		case 13:
		case 3:
			*itemHit = OK;
			SimulateButtonPress(dp,*itemHit);
			return true;
		case 27:
			*itemHit = Cancel;
			SimulateButtonPress(dp,*itemHit);
			return true;
		case 8:	// backspace
			{
				TEHandle	teH = ((DialogPeek) dp)->textH;
				if ((*teH)->selStart == (*teH)->selEnd == gPassword[0]) {
					if (gPassword[0])
						gPassword[0]--;
				}
				else {
					gPassword[0] = 0;
					SetText(dp,W_PasswordItem, "\p");
					return true;
				}
			}
			break;
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			return true;
			break;
		default:
			if (tempChar >= ' ' && tempChar <= '~') {
				if (gPassword[0] < 31) {
					TEHandle	teH = ((DialogPeek) dp)->textH;
					if ((*teH)->selStart != (*teH)->selEnd ||
						(*teH)->selStart != gPassword[0]) 
					{
						gPassword[0] = 0;
						SetText(dp,W_PasswordItem, "\p");
					}
					gPassword[gPassword[0]+1] = tempChar;
					++gPassword[0];
					ep->message &= 0xFFFFFF00;
					ep->message |= 19;	// diamond
				}
				else {
					return true;
				}
			}
		}
		break;
	  default:
	  	break;

	}
	return false;
}

pascal void DefaultOKButton(DialogPtr dp, short itemNbr);
pascal void DefaultOKButton(DialogPtr dp, short itemNbr)
{
	// EnableButton(dp,OK);
	OutlineButton(dp,Cancel,&qd.white);
	OutlineButton(dp,OK,&qd.black);
/**
			DisableButton(dp,OK);
			OutlineButton(dp,OK,&qd.white);
			OutlineButton(dp,Cancel,&qd.black);
**/
}

short PasswordDialog(StringPtr str, StringPtr prompt)
{
	GrafPtr		savePort;
	DialogPtr 	dp;
	short		t,itemHit;
	Handle		h;
	Rect		r;
	ModalFilterUPP	filterProc = NewModalFilterProc(WizDialogFilter);
	UserItemUPP		userProc = NewUserItemProc(DefaultOKButton);
	GetPort(&savePort);

	ParamText(prompt,"\p","\p","\p");
	if ((dp = GetNewDialog (WizardDLOG, NULL, (WindowPtr) -1)) == NULL)
		return Cancel;

	GetDItem(dp,W_UserItem,&t,&h,&r);
	SetDItem(dp,W_UserItem,t,(Handle) userProc, &r);
	
	gPassword[0] = 0;
	SetText(dp,W_PasswordItem, "\p");
	SelIText(dp,W_PasswordItem,0,32767);

	ShowWindow(dp);
	SetPort(dp);
	SetCursor(&qd.arrow);
	ShowCursor();
	do {
		ModalDialog(filterProc, &itemHit);
	} while (itemHit != OK && itemHit != Cancel);
	DisposDialog(dp);
	SetPort(savePort);
	BlockMove(gPassword,str,gPassword[0]+1);
	return itemHit;
}


#define NetworkDLOG			504
#define P_AllowTCP			3
#define P_AllowAppletalk	4
#define P_LocalPort			5

void NetworkDialog()
{
	GrafPtr		savePort;
	DialogPtr 	dp;
	short		itemHit;
	Str63		portName;
	
	GetPort(&savePort);

	if ((dp = GetNewDialog (NetworkDLOG, NULL, (WindowPtr) -1)) == NULL)
		return;

	// GetDItem(dp,N_UserItem,&t,&h,&r);
	// SetDItem(dp,N_UserItem,t,(Handle) userProc, &r);
	
	SetControl(dp,P_AllowTCP,gPrefs.allowTCP);
	SetControl(dp,P_AllowAppletalk,gPrefs.allowAppletalk);

	sprintf((char *) portName, "%d",gPrefs.localPort);
	CtoPstr((char *) portName);
	SetText(dp,P_LocalPort, portName);

	SelIText(dp,P_LocalPort,0,32767);

	SetDialogDefaultItem(dp, OK);
	SetDialogCancelItem(dp, Cancel);
	SetDialogTracksCursor(dp,true);

	ShowWindow(dp);
	SetPort(dp);
	SetCursor(&qd.arrow);
	ShowCursor();
	do {
		ModalDialog(NULL, &itemHit);
		switch (itemHit) {
		case P_AllowTCP:
			if (gPrefs.allowAppletalk || !gPrefs.allowTCP)
				gPrefs.allowTCP = !gPrefs.allowTCP;
			SetControl(dp,P_AllowTCP,gPrefs.allowTCP);
			break;
		case P_AllowAppletalk:
			if (gPrefs.allowTCP || !gPrefs.allowAppletalk)
				gPrefs.allowAppletalk = !gPrefs.allowAppletalk;
			SetControl(dp,P_AllowAppletalk,gPrefs.allowAppletalk);
			break;
		}
	} while (itemHit != OK && itemHit != Cancel);
	if (itemHit == OK) {
		GetText(dp,P_LocalPort, portName);
		PtoCstr(portName);
		gPrefs.localPort = atoi((char *) portName);
		StorePreferences();
	}
	DisposDialog(dp);
	SetPort(savePort);
}

#define PermissionsDLOG	505
#define AllowGuestsItem			3
#define AllowCyborgsItem		4
#define AllowPaintingItem		5
#define AllowCustomPropsItem	6
#define AllowWizardsItem		7
#define WizardsMayKillItem		8
#define WizardsMayAuthorItem	9
#define PlayersMayKillItem		10
#define CyborgsMayKillItem		11

#define DeathPenaltyItem		12
#define PurgeInactivePropsItem	13
#define KillFloodersItem		14
#define DisallowSpoofingItem	15
#define AllowMemberCreatedRoomsItem	16

#define DeathPenaltyN			17
#define PurgeInactiveN			18
#define KillFloodersN			19



#define NbrPermFlags	14
long	CorrFlag[] = {PM_AllowGuests, PM_AllowCyborgs, PM_AllowPainting,
					  PM_AllowCustomProps, PM_AllowWizards, PM_WizardsMayKill,
					  PM_WizardsMayAuthor, PM_PlayersMayKill, PM_CyborgsMayKill,
					  PM_DeathPenalty, PM_PurgeInactiveProps, PM_KillFlooders,
					  PM_NoSpoofing, PM_MemberCreatedRooms
						};

void PermissionsDialog()
{
	GrafPtr		savePort;
	DialogPtr	dp;
	short		itemHit;
	long		permissions;
	short		deathPenaltyMinutes;
	short		purgePropDays;
	short		minFloodEvents;
	short		i;
	GetPort(&savePort);

	if ((dp = GetNewDialog (PermissionsDLOG, NULL, (WindowPtr) -1)) == NULL)
		return;

	SetDialogDefaultItem(dp, OK);
	SetDialogCancelItem(dp, Cancel);
	SetDialogTracksCursor(dp,true);

	permissions = gPrefs.permissions;
	deathPenaltyMinutes = gPrefs.deathPenaltyMinutes;
	purgePropDays = gPrefs.purgePropDays;
	minFloodEvents = gPrefs.minFloodEvents;
	for (i = 0; i < NbrPermFlags; ++i)
		SetControl(dp, i + AllowGuestsItem, (permissions & CorrFlag[i]) > 0);
	PutIntItem(dp,DeathPenaltyN,deathPenaltyMinutes);
	PutIntItem(dp,PurgeInactiveN,purgePropDays);
	PutIntItem(dp,KillFloodersN,minFloodEvents);
	SelIText(dp,DeathPenaltyN,0,32767);
	ShowWindow(dp);
	SetPort(dp);
	SetCursor(&qd.arrow);
	ShowCursor();
	do {
		ModalDialog(NULL, &itemHit);
		switch (itemHit) {
		case AllowGuestsItem:
		case AllowCyborgsItem:
		case AllowPaintingItem:
		case AllowCustomPropsItem:
		case AllowWizardsItem:
		case WizardsMayKillItem:
		case WizardsMayAuthorItem:
		case PlayersMayKillItem:
		case CyborgsMayKillItem:
		case DeathPenaltyItem:
		case PurgeInactivePropsItem:
		case KillFloodersItem:
		case DisallowSpoofingItem:
		case AllowMemberCreatedRoomsItem:
			permissions ^= CorrFlag[itemHit-AllowGuestsItem];
			SetControl(dp, itemHit, (permissions & CorrFlag[itemHit-AllowGuestsItem]) > 0);
			break;
		case DeathPenaltyN:
			deathPenaltyMinutes = GetIntItem(dp, itemHit);
			break;
		case PurgeInactiveN:
			purgePropDays = GetIntItem(dp, itemHit);;
			break;
		case KillFloodersN:
			minFloodEvents = GetIntItem(dp, itemHit);;
			break;
		}
	} while (itemHit != OK && itemHit != Cancel);
	if (itemHit == OK) {
		gPrefs.permissions = permissions;
		gPrefs.deathPenaltyMinutes = deathPenaltyMinutes;
		gPrefs.minFloodEvents = minFloodEvents;
		gPrefs.purgePropDays = purgePropDays;
		StorePreferences();
	}
	DisposeDialog(dp);
	SetPort(savePort);
}
