// Preferences.c
#include <Folders.h>
#include <Gestalt.h>

#include "U-USER.H"
#include "m-cmds.H"
#include "AppMenus.h"
#include "U-SECURE.H"
#include "DialogUtils.h"
#include "UserTCP.h"

#define UserPrefsType	'uPrf'
#define MacPrefsType	'mPrf'
#define MacroRecType	'uMcr'
#define UCreator		'mUsr'
#define UFType			'rsrc'

#define PrefsVersion		2L
#define MacPrefsVersion		2L

#define OldMacrosVersion	2L	// 5/9/95
#define MacrosVersion		3L	// 5/9/95

#define DefaultPrefsFlags	(UPF_UseFadeEffects | UPF_DownloadGraphics | \
							 UPF_TintedBalloons | UPF_AutoShowNames | UPF_UsePropAnimations)


UserPrefs 	gPrefs = {PrefsVersion, DefaultPrefsFlags, "\pGuest", 1, geneva, 
					  9,9998,


// DEFAULT SERVER NOW PROVIDED BY 'PRE'
#if RAINBOWRED
/* NOTE: Usermenus also needs to be changed */
					  "palace.rainbowalley.com",
#else
#if PALACE_IN_A_BOX
					  "welcome.thePalace.com",
#else
#if KIDS_CLIENT
					  "palace.4healing.com",
#else
					  "palace.thePalace.com",
#endif
#endif
#endif
					  0,0,5,"\p:Pictures:","\p",
#if KIDS_CLIENT
					   2,
					  {{"palace.4healing.com",0L,9998},
  					  {"palace.kidsnation.com",0L,9998}}
#else


					  7,
					  {
// LIST OF SERVERS NOW PROVIDED BY 'PRE'
#if PALACE_IN_A_BOX
					  {"welcome.thepalace.com",0L,9998},
#else
					  {"palace.thepalace.com",0L,9998},
#endif
					 /*  {"boardwalk.intel.com",0L,9998}, */
					  {"totalny.com",0L,9998},
					  {"palace.hob.com",0L,9998},
					  {"hollywoodandvine.com",0L,9998},
					  {"palace.thepalace.com",0L,9994},
					  {"palace.foxworld.com",0L,9998},
  					  {"palace.thepalace.com",0L,9998}}
#endif
					  };
MacPrefs	gMacPrefs = {MacPrefsVersion};
#if RAINBOWRED
Str31		prefsName = "\pPalace Rainbow Prefs";
#else
Str31		prefsName = "\pPalace Preferences";
#endif
Str31		oldPrefsName = "\pMansion Prefs";
MacroRec	gMacros = {MacrosVersion,MaxMacros};	// 5/9/96

pascal Boolean MyDialogFilter(DialogPtr dp, EventRecord *ep, short *itemHit);


void LoadPreferences(void)
{
	Handle		h;
	short		volNum = 0,fRefNum = 0;
	long		dirID = 0L;
	long		length;

	// look for prefs file in current (palace) folder
	fRefNum = HOpenResFile(0,0L,prefsName,fsRdPerm);
	if (fRefNum == -1) {
		// if that fails look in preferences folder (old location)
		FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,&volNum,&dirID);
		fRefNum = HOpenResFile(volNum,dirID,prefsName,fsRdPerm);
		// if that fails try old style name "Mansion Prefs"
		// removed 10/2/96
		// if (fRefNum == -1)
		//	fRefNum = HOpenResFile(volNum,dirID,oldPrefsName,fsRdPerm);
	}
	if (fRefNum != -1) {
		h = Get1Resource(UserPrefsType,128);
		if (h != NULL) {
			length = GetHandleSize(h) < sizeof(UserPrefs)? GetHandleSize(h) : sizeof(UserPrefs);
			if (((UserPrefs *) *h)->versID == PrefsVersion)
				BlockMove(*h, &gPrefs, length);
			else {
				long	result;
				Gestalt(gestaltNativeCPUtype, &result);
				if (result < gestaltCPU68040) {
					gPrefs.userPrefsFlags &= ~UPF_UseFadeEffects;
					gPrefs.userPrefsFlags &= ~UPF_UsePropAnimations;
				}
			}
			ReleaseResource(h);
		}
		// Load Macintosh Prefs
		h = Get1Resource(MacPrefsType,128);
		if (h != NULL) {
			length = GetHandleSize(h) < sizeof(MacPrefs)? GetHandleSize(h) : sizeof(MacPrefs);
			if (((MacPrefs *) *h)->versID == MacPrefsVersion)
				BlockMove(*h, &gMacPrefs, length);
			ReleaseResource(h);
		}

		// 5/9/96 - switched to new typeof MacroRec, still supporting old one
		h = Get1Resource(MacroRecType,128);
		if (h != NULL) {
			if (((MacroRec *) *h)->versID == OldMacrosVersion) {
				BlockMove(&((OldMacroRec *) *h)->mac[0],&gMacros.mac[0],OldMaxMacros*sizeof(MRec));
			}
			else if (((MacroRec *) *h)->versID == MacrosVersion) {
				long	n;
				n = ((MacroRec *) *h)->nbrMacros < MaxMacros? ((MacroRec *) *h)->nbrMacros : MaxMacros;
				BlockMove(&((MacroRec *) *h)->mac[0],&gMacros.mac[0],n*sizeof(MRec));
				// gMacros = *((MacroRec *) *h);
			}
			ReleaseResource(h);
		}

		CloseResFile(fRefNum);
	}
	else {
		char	tStr[256]="";
		short	portNbr = 9998;
		char	*p,*tmp;
		Boolean	flag;
		Handle	h;
		short	n,i;

		// 10/2/96 - Get Preferences Presets
		h = GetResource('ppre',128);
		if (h == NULL)
			return;
		HLock(h);
		p = *h;

		// Default Server Name

		BlockMove(p,tStr,p[0]+1);
		p += p[0]+1+((p[0] & 1)? 0 : 1);
		if (tStr[0]) {
			PtoCstr((StringPtr) tStr);
			portNbr = 9998;
			if ((tmp = strchr(tStr,':')) != NULL) {
				portNbr = atoi(tmp+1);
				*tmp = 0;
			}
			gPrefs.remotePort = portNbr;
			strcpy(gPrefs.remoteHost,tStr);
		}

		// Origination Code
		BlockMove(p,gPrefs.originationCode,p[0]+1);
		p += p[0]+1+((p[0] & 1)? 0 : 1);

		// Show Originiation Code Flag
		flag = *p;
		p += sizeof(short);
		if (flag)
			gPrefs.userPrefsFlags |= UPF_ShowOrigCode;
		else
			gPrefs.userPrefsFlags &= ~UPF_ShowOrigCode;

		// Show Overlay Flag
		flag = *p;
		p += sizeof(short);
		if (flag)
			gPrefs.userPrefsFlags |= UPF_ShowOverlay;
		else
			gPrefs.userPrefsFlags &= ~UPF_ShowOverlay;

		// Number of Preset Servers
		n = *((short *) p);
		p += sizeof(short);
		if (n > MaxNetaddresses)
			n = MaxNetaddresses;
		gPrefs.nbrNetaddresses = 0;
		for (i = 0; i < n; ++i) {
			BlockMove(p,tStr,p[0]+1);
			p += p[0]+1;
			if (tStr[0]) {
				PtoCstr((StringPtr) tStr);
				portNbr = 9998;
				if ((tmp = strchr(tStr,':')) != NULL) {
					portNbr = atoi(tmp+1);
					*tmp = 0;
				}
				gPrefs.nAddr[i].ip_addr = 0;
				gPrefs.nAddr[i].portNumber = portNbr;
				strcpy(gPrefs.nAddr[i].name,tStr);
				gPrefs.nbrNetaddresses++;
			}
		}
		HUnlock(h);
		ReleaseResource(h);
	}
}

OSErr CreatePrefsFile(void)
{
	OSErr	oe;
	short	fRefNum= 0;
	short	volNum = 0;
	long	dirID = 0L;

	// 5/9/96 JAB not using preferences folder any more
	// 1/2/97 CDM Use prefs folder first, then application folder
	FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,&volNum,&dirID);
	oe = HCreate(volNum, dirID, prefsName, UCreator, UFType);
	//oe = HCreate(0, 0L, prefsName, UCreator, UFType);

	if (oe == dupFNErr) 
	{
		HDelete(volNum, dirID, prefsName);
		oe = HCreate(volNum, dirID, prefsName, UCreator, UFType);
		//HDelete(0,0L, prefsName);
		//oe = HCreate(0, 0L, prefsName, UCreator, UFType);
	}
	
	if (oe == noErr) 
	{
		HCreateResFile(volNum, dirID, prefsName);
		//HCreateResFile(0, 0L, prefsName);
		oe = ResError();
	}
	return oe;
}

void StorePreferences(void)
{
	Handle		h,h2;
	short		fRefNum = 0;
	short		volNum = 0;
	long		dirID = 0L;

	// FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,&volNum,&dirID);
	// fRefNum = HOpenResFile(volNum,dirID,prefsName,fsRdWrPerm);
	fRefNum = HOpenResFile(0,0L,prefsName,fsRdWrPerm);
	if (fRefNum == -1) 
	{
		/* 1/2/97 CDM  Changed to put prefs file in Prefs folder first */
		FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,&volNum,&dirID);
		fRefNum = HOpenResFile(volNum,dirID,prefsName,fsRdWrPerm);
		
		if (fRefNum == -1)
		{
			if (CreatePrefsFile() != noErr)
				return;
				
			fRefNum = HOpenResFile(volNum,dirID,prefsName,fsRdWrPerm);
		}
		// fRefNum = HOpenResFile(volNum,dirID,prefsName,fsRdWrPerm);
		//fRefNum = HOpenResFile(0,0L,prefsName,fsRdWrPerm);
	}
	if (fRefNum == -1)
		return;

	/* 7/1/96 JAB Changed to NewHandleClear */
	h = NewHandleClear(sizeof(UserPrefs));
	if (h) {
		SetResLoad(false);
		while ((h2 = Get1Resource(UserPrefsType, 128)) != NULL) {
			RmveResource(h2);
			DisposHandle(h2);
		}
		SetResLoad(true);
		*((UserPrefs *) *h) = gPrefs;
		AddResource(h,UserPrefsType,128,"\pPreferences");
		WriteResource(h);
		ReleaseResource(h);
	}
	h = NewHandleClear(sizeof(MacPrefs));	/* 7/1/96 JAB Changed to NewHandleClear */
	if (h) {
		SetResLoad(false);
		while ((h2 = Get1Resource(MacPrefsType, 128)) != NULL) {
			RmveResource(h2);
			DisposHandle(h2);
		}
		SetResLoad(true);
		*((MacPrefs *) *h) = gMacPrefs;
		AddResource(h,MacPrefsType,128,"\pMac Preferences");
		WriteResource(h);
		ReleaseResource(h);
	}

	h = NewHandleClear(sizeof(MacroRec));	/* 7/1/96 JAB Changed to NewHandleClear */
	if (h) {
		SetResLoad(false);
		while ((h2 = Get1Resource(MacroRecType, 128)) != NULL) {
			RmveResource(h2);
			DisposHandle(h2);
		}
		SetResLoad(true);
		*((MacroRec *) *h) = gMacros;
		AddResource(h,MacroRecType,128,"\pMacros");
		WriteResource(h);
		ReleaseResource(h);
	}

	CloseResFile(fRefNum);
}

// Computes a macro number based on a seed (0-9) plus the state of 3 shift keys
// allowing for 80 unique macros
//
short ComputeMacroNumber(short seed, short modifiers)
{
	short	base;

	// Use state of 3 shift keys to generate base 0 thru 7
	// the keys are shiftKey (0200) optionKey (0800) and controlKey (1000)
	base = ((modifiers & shiftKey) > 0) +
	       (((modifiers & optionKey) > 0)<<1) +
			(((modifiers & controlKey) > 0)<<2);

	return base*10 + seed;
}


void RecordMacro(short n)
{
	// 5/9/96 Limit Check
	if (gRoomWin->mePtr == NULL || n < 0 || n >= MaxMacros)
		return;
	gMacros.mac[n].colorNbr = gRoomWin->mePtr->user.colorNbr;
	gMacros.mac[n].faceNbr = gRoomWin->mePtr->user.faceNbr;

	// 6/7/95
	gMacros.mac[n].nbrProps = gRoomWin->mePtr->user.nbrProps;
	BlockMove(gRoomWin->mePtr->user.propSpec,gMacros.mac[n].propSpec,gRoomWin->mePtr->user.nbrProps*sizeof(AssetSpec));
	StorePreferences();
}

void PlayMacro(short n)
{
	short	i;
	// 5/9/96 New Check on N
	if (gRoomWin->mePtr == NULL || n < 0 || n >= MaxMacros)
		return;

	if (gSecure.guestAccess) {
		if (MembersOnly(gMacros.mac[n].colorNbr != 3))
			return;
	
		for (i = 0; i < gMacros.mac[n].nbrProps; ++i) {
			if (gMacros.mac[n].propSpec[i].id < MinReservedProp || 
				gMacros.mac[n].propSpec[i].id > MaxReservedProp)
			{
					if (MembersOnly(true))
						return;
					if (!(gRoomWin->userFlags & U_SuperUser) && DeniedByServer(PM_AllowCustomProps))
						return;
			}
		}
	}

	// 1/16/96 JAB Open the damn eyes if the macro is uninitialized...
	if (gMacros.mac[n].faceNbr == 0 && 
		gMacros.mac[n].colorNbr == 0 &&
		gMacros.mac[n].nbrProps == 0)
	{
		gMacros.mac[n].faceNbr = FACE_Normal;
		gMacros.mac[n].colorNbr = MyRandom(16);
	}


	// 6/7/95 New Function for changing user description
	// 5/9/96 - Don't call GenerateUserEvent for high numbered macros
		
	if (n >= MaxScriptMacros || !GenerateUserEvent(PE_Macro0 + n))
		ChangeUserDesc(gMacros.mac[n].faceNbr,gMacros.mac[n].colorNbr,
					gMacros.mac[n].nbrProps,
					gMacros.mac[n].propSpec);
}


#define PrefsDLOG			500
#define P_NameItem			3
#define P_ReadSpeed0		4
#define P_ReadSpeed1		5
#define P_ReadSpeed2		6
#define P_FontItem			7
#define P_FontSizeItem		8
#define P_UseFadeEffects	9
#define P_DownloadGraphics	10
#define P_TintedBalloons	11
#define P_AutoShowNames		12
#define P_UsePropAnimations	13

void PreferencesDialog(void)
{
	GrafPtr		savePort;
	DialogPtr 	dp;
	short		itemHit;
	short		readingSpeed;
	short		n,fontCount;
	Str63		fontName,checkName;
	Str255		tempName;
	long		flags;
	ModalFilterUPP	filterProc = NewModalFilterProc(MyDialogFilter);

	GetPort(&savePort);

	gFontMenu = GetMenu(FontMENU);
	gFontSizeMenu = GetMenu(FontSizeMENU);
	AddResMenu(gFontMenu, 'FONT');
	
	if ((dp = GetNewDialog (PrefsDLOG, NULL, (WindowPtr) -1)) == NULL)
		return;
	
	readingSpeed  = gPrefs.readingSpeed;
	if (gSecure.guestAccess)
		SetText(dp,P_NameItem, "\pGuest");
	else
		SetText(dp,P_NameItem, gPrefs.name);
	SelIText(dp,P_NameItem,0,32767);
	SetControl(dp, P_ReadSpeed0+readingSpeed,1);
	flags = gPrefs.userPrefsFlags;

	SetControl(dp, P_UseFadeEffects, (flags & UPF_UseFadeEffects) > 0);
	SetControl(dp, P_DownloadGraphics, (flags & UPF_DownloadGraphics) > 0);
	SetControl(dp, P_TintedBalloons, (flags & UPF_TintedBalloons) > 0);
	SetControl(dp, P_AutoShowNames, (flags & UPF_AutoShowNames) > 0);
	SetControl(dp, P_UsePropAnimations, (flags & UPF_UsePropAnimations) > 0);

	GetFontName(gPrefs.fontID, fontName);
	fontCount = CountMItems(gFontMenu);
	for (n = 1; n <= fontCount; ++n){
		GetItem(gFontMenu, n, checkName);
		if (EqualString(checkName,fontName,false,false)) {
			CheckItem(gFontMenu, n, true);
			SetControl(dp, P_FontItem, n);
		}
	}

	switch (gPrefs.fontSize) {
	case 9:		SetControl(dp, P_FontSizeItem, 1);	break;
	case 10:	SetControl(dp, P_FontSizeItem, 2);	break;
	case 12:	SetControl(dp, P_FontSizeItem, 3);	break;
	case 14:	SetControl(dp, P_FontSizeItem, 4);	break;
	case 18:	SetControl(dp, P_FontSizeItem, 5);	break;
	case 24:	SetControl(dp, P_FontSizeItem, 6);	break;
	}

	SetDialogDefaultItem(dp, OK);
	SetDialogCancelItem(dp, Cancel);
	SetDialogTracksCursor(dp,true);

	ShowWindow(dp);
	SetPort(dp);
	SetCursor(&qd.arrow);
	ShowCursor();
	do {
		ModalDialog(filterProc, &itemHit);
		switch (itemHit) {
		case P_NameItem:
			if (MembersOnly(true))
				SetText(dp,P_NameItem, "\pGuest");
			break;
		case P_ReadSpeed0:
		case P_ReadSpeed1:
		case P_ReadSpeed2:
			SetControl(dp, P_ReadSpeed0+readingSpeed,0);
			readingSpeed = itemHit - P_ReadSpeed0;
			SetControl(dp, P_ReadSpeed0+readingSpeed,1);
			break;
		case P_UseFadeEffects:
			flags ^= UPF_UseFadeEffects;
			SetControl(dp, P_UseFadeEffects, (flags & UPF_UseFadeEffects) > 0);
			break;
		case P_UsePropAnimations:
			flags ^= UPF_UsePropAnimations;
			SetControl(dp, P_UsePropAnimations, (flags & UPF_UsePropAnimations) > 0);
			break;
		case P_TintedBalloons:
			flags ^= UPF_TintedBalloons;
			SetControl(dp, P_TintedBalloons, (flags & UPF_TintedBalloons) > 0);
			break;
		case P_AutoShowNames:
			flags ^= UPF_AutoShowNames;
			SetControl(dp, P_AutoShowNames, (flags & UPF_AutoShowNames) > 0);
			break;
		case P_DownloadGraphics:
			flags ^= UPF_DownloadGraphics;
			SetControl(dp, P_DownloadGraphics, (flags & UPF_DownloadGraphics) > 0);
			break;
		}
	} while (itemHit != OK && itemHit != Cancel);
	if (itemHit == OK) {
		GetText(dp,P_NameItem, tempName);
		if (tempName[0] > 31)
			tempName[0] = 31;
		if (tempName[0])
			BlockMove(tempName,gPrefs.name,tempName[0]+1);
		gPrefs.readingSpeed = readingSpeed;
		gPrefs.userPrefsFlags = flags;
		n = GetControl(dp, P_FontItem);
		GetItem(gFontMenu, n, fontName);
		GetFNum(fontName,&gPrefs.fontID);

		n = GetControl(dp, P_FontSizeItem);
		switch (n) {
		case 1:	gPrefs.fontSize = 9;	break;
		case 2:	gPrefs.fontSize = 10;	break;
		case 3:	gPrefs.fontSize = 12;	break;
		case 4:	gPrefs.fontSize = 14;	break;
		case 5:	gPrefs.fontSize = 18;	break;
		case 6:	gPrefs.fontSize = 24;	break;
		}
		ChangeBalloonFont(gPrefs.fontID,gPrefs.fontSize);
		if (ConfigureNetscape() == noErr)
			gPrefs.userPrefsFlags |= UPF_NetscapeConfigured;
		else
			gPrefs.userPrefsFlags &= ~UPF_NetscapeConfigured;
		StorePreferences();
		if (!MembersOnly(!EqualPString(gPrefs.name,"\pGuest",true))) {
			if (gLogWin)
				SetWTitle((WindowPtr) gLogWin,gPrefs.name);
			PostServerEvent(MSG_USERNAME,gRoomWin->meID,(Ptr) gPrefs.name, gPrefs.name[0]+1);
		}
	}
	DisposDialog(dp);
	SetPort(savePort);
}

#define WizardDLOG			502
#define W_PasswordItem		3
#define W_DefButtonItem		5

Str31	gPassword;
pascal Boolean WizDialogFilter(DialogPtr dp, EventRecord *ep, short *itemHit);

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
			if (gPassword[0])
				*itemHit = OK;
			else
				*itemHit = Cancel;
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
					DisableButton(dp,OK);
					OutlineButton(dp,OK,&qd.white);
					OutlineButton(dp,Cancel,&qd.black);
					return true;
				}
				if (gPassword[0] == 0) {
					DisableButton(dp,OK);
					OutlineButton(dp,OK,&qd.white);
					OutlineButton(dp,Cancel,&qd.black);
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
					if (gPassword[0] == 1) {
						EnableButton(dp,OK);
						OutlineButton(dp,Cancel,&qd.white);
						OutlineButton(dp,OK,&qd.black);
					}
				}
				else {
					return true;
				}
			}
		}
		break;
	  case kHighLevelEvent:
		if (ep->message == ServerEventID) {
			ProcessAppleTalkEvent(ep);
		}
		break;
	  case nullEvent:
		if (gTCPRecord)
			PalTCPIdle(ep)
				;
	  default:
	  	break;

	}
	return false;
}

pascal void DefButtonItem(DialogPtr dp, short itemNbr);
pascal void DefButtonItem(DialogPtr dp, short itemNbr)
{
	if (itemNbr == 5) {
		if (gPassword[0]) {
			EnableButton(dp,OK);
			OutlineButton(dp,Cancel,&qd.white);
			OutlineButton(dp,OK,&qd.black);
		}
		else {
			DisableButton(dp,OK);
			OutlineButton(dp,OK,&qd.white);
			OutlineButton(dp,Cancel,&qd.black);
		}
	}
}

short PasswordDialog(StringPtr str, StringPtr prompt)
{
	GrafPtr		savePort;
	DialogPtr 	dp;
	short		itemHit;
	short		t;
	Handle		h;
	Rect		r;
	ModalFilterUPP	filterProc = NewModalFilterProc(WizDialogFilter);
	UserItemUPP		defButtonProc = NewUserItemProc(DefButtonItem);

	GetPort(&savePort);

	ParamText(prompt, "\p", "\p", "\p");

	if ((dp = GetNewDialog (WizardDLOG, NULL, (WindowPtr) -1)) == NULL)
		return Cancel;
	
	gPassword[0] = 0;

	GetDItem(dp,W_DefButtonItem,&t,&h,&r);
	SetDItem(dp,W_DefButtonItem,t,(Handle) defButtonProc, &r);

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

/* used to set the thing */
void ChildLockDialog(void)
{
	short	item;
	Str31	password,pass2,tPass;
	item = PasswordDialog(password, "\pEnter Unlock Password, Cancel to clear:");
	if (item == Cancel) {
		if (gPrefs.userPassword[0]) {
			gPrefs.userPassword[0] = 0;
			StorePreferences();
			ErrorMessage("Child Lock Cleared - no password required to access");
		}
	}
	else if (item == OK) {
		if (password[0] > 0) {
			item = PasswordDialog(pass2, "\pEnter again to confirm:");
			if (item != OK)
				return;
			if (!EqualString(password,pass2,false,false)) {
				ErrorMessage("Passwords don't match");
				return;
			}
			EncryptString(password,tPass);
		}
		if (password[0] > 0)
			ErrorMessage("Child Lock Activated - remember your password");
		else
			ErrorMessage("Child Lock Cleared - no password required to access");

		BlockMove(tPass,gPrefs.userPassword,tPass[0]+1);
		StorePreferences();
	}
}

Boolean ChildLock(void)
{
	Str31	password,tPass;
	short	item;
	if (gPrefs.userPassword[0] == 0)
		return false;
	item = PasswordDialog(password, "\pPassword to Unlock:");
	if (item != OK || password[0] == 0)
		return true;
	EncryptString(password,tPass);
	{
		Str31	debugPass="\pdebug",dPass;
		EncryptString(debugPass,dPass);
		if (EqualPString(tPass,dPass,false))
			return false;
	}
	if (EqualPString(tPass,gPrefs.userPassword,false))
		return false;
	else
		return true;
}

void RestoreWindowPos(WindowPtr theWin, Point *p)
{
	extern EventRecord	gTheEvent;
	RgnHandle	grayRgn;
	Rect		winRect;

	grayRgn = GetGrayRgn();
	winRect = theWin->portRect;
	OffsetRect(&winRect,p->h,p->v);
	if ((p->h != 0 || p->v != 0) && !(gTheEvent.modifiers & shiftKey) &&
		RectInRgn(&winRect, grayRgn)) {
		MoveWindow(theWin, p->h, p->v, false);
	}
}

void RestoreWindowSize(WindowPtr theWin, Point *p)
{
	extern EventRecord	gTheEvent;
	RgnHandle	grayRgn;
	Rect		winRect;
	GrafPtr		savePort;

	GetPort(&savePort);
	SetPort(theWin);

	grayRgn = GetGrayRgn();
	winRect = theWin->portRect;
	winRect.right = winRect.left + p->h;
	winRect.bottom = winRect.top + p->v;
	LocalToGlobal(&topLeft(winRect));
	LocalToGlobal(&botRight(winRect));

	if (p->h != 0 || p->v != 0 && !(gTheEvent.modifiers & shiftKey) &&
		RectInRgn(&winRect, grayRgn)) {
		SizeWindow(theWin, p->h, p->v, true);
	}

	SetPort(savePort);
}

void SaveWindowPos(WindowPtr theWin, Point *p)
{
	GrafPtr	savePort;
	GetPort(&savePort);
	SetPort(theWin);
	p->h = theWin->portRect.left;
	p->v = theWin->portRect.top;
	LocalToGlobal(p);
	SetPort(savePort);
}

void SaveWindowSize(WindowPtr theWin, Point *p)
{
	p->h = theWin->portRect.right - theWin->portRect.left;
	p->v = theWin->portRect.bottom - theWin->portRect.top;
}
