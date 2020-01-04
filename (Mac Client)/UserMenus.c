/************************************************************************************
 * EvtMenus.c									for Class 3
 *
 *	Routines for EvtLab demo menus.
 *
 *
 ************************************************************************************/
#include <Menus.h>
#include "U-USER.H"
#include "m-cmds.H"
#include "AppMenus.h"
#include "U-SECURE.H"
#include "U-TIMOUT.H"
#include "PPAMgr.h"

#include <ConnectionTools.h>

// Menu Handles
MenuHandle	gAppleMenu, gFileMenu, gEditMenu, gOptionsMenu, gFontMenu, 
			gFontSizeMenu, gRecordMacroMenu, gPortMenu, gBaudMenu, gSoundMenu,
			gExpertMenu, gMacroMenu, gSpotTypeMenu, gPropMenu, gSpotStateMenu,
			gPropEditMenu, gServerMenu, gLayerMenu, gSpotOptionsMenu, gVoicesMenu,
			gRoomOptionsMenu, gHelpMenu, gPlugInsMenu;

// Menu Initialization code.
void AppAdjustMenus(void);
void AppProcessCommand (short menuID, short menuItem);

void MySetUpMenus(void)
{
	Handle	myMenuBar;
	myMenuBar = GetNewMBar(MenuBaseID);
	SetMenuBar(myMenuBar);

	gAppleMenu = GetMHandle(AppleMENU);
	gFileMenu = GetMHandle(FileMENU);
	gEditMenu = GetMHandle(EditMENU);
	gOptionsMenu = GetMHandle(OptionsMENU);
	gMacroMenu = GetMHandle(MacroMENU);
	gHelpMenu = GetMHandle(HelpMENU);

	gRecordMacroMenu = GetMenu(RecordMacroMENU);
	InsertMenu(gRecordMacroMenu, hierMenu);

	gSoundMenu = GetMenu(SoundMENU);
	InsertMenu(gSoundMenu, hierMenu);

	gVoicesMenu = GetMenu(VoicesMENU);
	InsertMenu(gVoicesMenu, hierMenu);

	gPlugInsMenu = GetMenu(PlugInsMENU);
	InsertMenu(gPlugInsMenu, hierMenu);

	AddResMenu(gAppleMenu, 'DRVR');

#if !RELEASE
	AppendMenu(gOptionsMenu, "\p-;Debug/D");
#endif

	DrawMenuBar();
}

void NewbieMenus()
{
// Register is now permanent
//	if (gFileMenu)
//		AppendMenu(gFileMenu, "\p-;RegisterÉ");
}


//	Enable or disable the items in the Edit menu if a DA window
//	comes up or goes away. Our application doesn't do anything with
//	the Edit menu.

void AppAdjustMenus(void)
{
	register WindowPeek wp;
	short windowKind;
	Boolean isDA,isObjectWindow,isEditing,unlockedRoom,roomActive;
	Boolean	frontFloating;	
	Boolean	hasSelection,hasClip,mayAuthor;
	extern Boolean gSpeechAvailable;
	
	wp = (WindowPeek) FrontWindow();
	windowKind = wp ? wp->windowKind : 0;
	isDA = (windowKind < 0);
	isObjectWindow = (wp && wp->refCon == ObjectWindowID);
	roomActive = (WindowPtr) gRoomWin == FrontDocument();
	isEditing = isDA || (roomActive && gRoomWin->msgActive);
	if (isEditing) {
		hasSelection = ((*gRoomWin->msgTEH)->selEnd > (*gRoomWin->msgTEH)->selStart);
		hasClip = CanPaste('TEXT');
	}
	frontFloating = isObjectWindow && ((ObjectWindowPtr) wp)->floating;
	if (gLogWin->logFile)
		SetItem(gFileMenu, FM_Log, "\pStop Log");
	else
		SetItem(gFileMenu, FM_Log, "\pLog to FileÉ");

	MyEnableMenuItem(gFileMenu, FM_Save, false);

	if (gPrefs.userPrefsFlags & UPF_ClubMode)
		MyEnableMenuItem(gFileMenu, FM_ChildLock, false);

	MyEnableMenuItem(gAppleMenu, AM_About, gABWin == NULL);
	MyEnableMenuItem(gEditMenu, EM_Undo, isDA || gDrawWin != NULL);
	MyEnableMenuItem(gEditMenu, EM_Cut, isEditing && hasSelection);
	MyEnableMenuItem(gEditMenu, EM_Copy, isEditing && hasSelection);
	MyEnableMenuItem(gEditMenu, EM_Paste, isEditing && hasClip);
	MyEnableMenuItem(gEditMenu, EM_PasteMultiple, CanPaste('PICT'));
	MyEnableMenuItem(gEditMenu, EM_Clear, isEditing && hasSelection);
	MyEnableMenuItem(gEditMenu, EM_SelectAll, isEditing);
	MyEnableMenuItem(gFileMenu, FM_Close, frontFloating);
	// CheckItem(gFileMenu, FM_Guest, gRoomWin->guestAccess);
	CheckItem(gFileMenu, FM_ChildLock, gPrefs.userPassword[0] > 0);

	MyEnableMenuItem(gOptionsMenu, OM_UserList, gConnectionType != C_None);	// 6/9/95
	MyEnableMenuItem(gOptionsMenu, OM_RoomList, gConnectionType != C_None);	// 6/9/95
	MyEnableMenuItem(gOptionsMenu, OM_Wizard, gConnectionType != C_None);
	MyEnableMenuItem(gOptionsMenu, OM_Draw, gConnectionType != C_None);
	MyEnableMenuItem(gOptionsMenu, OM_GoBack, BackStackAvailable());

	MyEnableMenuItem(gOptionsMenu, OM_TextToSpeech, gSpeechAvailable);

	CheckItem(gOptionsMenu, OM_ShowNames, gPrefs.userPrefsFlags & UPF_AutoShowNames);
	CheckItem(gOptionsMenu, OM_FullScreen, gFullScreen);
	CheckItem(gOptionsMenu, OM_Iconize, gIconized);
	CheckItem(gOptionsMenu, OM_Wizard, gExpert);
	CheckItem(gOptionsMenu, OM_Draw, gDrawWin != NULL);
#if !RELEASE
	CheckItem(gOptionsMenu, OM_Debug, gDebugFlag);
#endif
	if (gExpertMenu && gExpert) {
		mayAuthor = (gRoomWin->serverInfo.serverPermissions & PM_WizardsMayAuthor) > 0;

		MyEnableMenuItem(gExpertMenu, EM_Authoring, mayAuthor);
		MyEnableMenuItem(gExpertMenu, EM_DragDoors, mayAuthor);
		MyEnableMenuItem(gExpertMenu, EM_NewRoom, mayAuthor);
		MyEnableMenuItem(gExpertMenu, EM_RoomInfo, mayAuthor);

		CheckItem(gExpertMenu, EM_Authoring, gMode == M_Authoring);
		CheckItem(gExpertMenu, EM_DragDoors, gDragDoors);
		unlockedRoom = !(gRoomWin->curRoom.roomFlags & RF_AuthorLocked);
		// 2/28/95 JAB
		MyEnableMenuItem(gExpertMenu, EM_NewDoor, mayAuthor && unlockedRoom);
		MyEnableMenuItem(gExpertMenu, EM_DoorInfo, gMode == M_Authoring && gRoomWin->curHotspot != 0 && unlockedRoom && mayAuthor);
		CheckItem(gExpertMenu, EM_ShowCoords, gRecordCoords);
		MyEnableMenuItem(gExpertMenu, EM_Layers, mayAuthor && gMode == M_Authoring && gRoomWin->curHotspot != 0 && unlockedRoom);
	}
	if (gPropWin && gPropMenu) {
		MyEnableMenuItem(gPropMenu, PM_NewProp, gPEWin == NULL);
		MyEnableMenuItem(gPropMenu, PM_EditProp, gPEWin == NULL && gSelectProp.id != 0);	// 6/7/95
		MyEnableMenuItem(gPropMenu, PM_DuplicateProp, gSelectProp.id != 0);	// 6/7/95
		MyEnableMenuItem(gPropMenu, PM_DeleteProp, gSelectProp.id != 0);	// 6/7/95
	}
	CheckItem(gSoundMenu, gPrefs.soundLevel+1, true);
	// 9/3/96 Register Menu is now permanent
	// if (gSecure.guestAccess)
		MyEnableMenuItem(gFileMenu, FM_Register, gConnectionType == C_None);
	MyEnableMenuItem(gFileMenu, FM_SignOff, gConnectionType != C_None);
	PPAMgrPreparePlugInsMenu();
}

void ToggleExpertMenu(Boolean isExpert)
{
	// 1/14/97 JAB Added registration check
	if (isExpert && (!gExpert || gExpertMenu == NULL) && gSecure.isRegistered && !gSecure.guestAccess) {
		gExpert = true;
		if (gExpertMenu == NULL) {
			gExpertMenu = GetMenu(ExpertMENU);
			gLayerMenu = GetMenu(LayerMENU);
			InsertMenu(gExpertMenu, HelpMENU);
			InsertMenu(gLayerMenu, hierMenu);
		}
	}
	else if (!isExpert) {
		gExpert = false;
		gRecordCoords = false;
		if (gMode != M_Normal)
			SwitchModes(M_Normal);
		if (gExpertMenu) {
			DeleteMenu(ExpertMENU);
			DeleteMenu(LayerMENU);
			DisposeMenu(gExpertMenu);
			DisposeMenu(gLayerMenu);
			gExpertMenu = NULL;
			gLayerMenu = NULL;
		}
	}
	DrawMenuBar();
}

void ToggleWizard()
{
	Str63	str;
	// 1/14/97 JAB added registration check
	if (MembersOnly(true) || !gSecure.isRegistered) {
		ReportMessage(RE_NoGuestAccess);	// 8/2/95 New System
		return;
	}
	if (DeniedByServer(PM_AllowWizards))
		return;

	if (!gExpert) {
		if (PasswordDialog(str,"\pWizard Password:") == OK) {
			EncryptString(str,str);
			PostServerEvent(MSG_SUPERUSER,gRoomWin->meID,(Ptr) str,str[0]+1);
		}
		return;
	}
	else {
		ToggleExpertMenu(false);
	}
}

//	Handle the menu selection. mSelect is what MenuSelect() and
//	MenuKey() return: the high word is the menu ID, the low word
//	is the menu item
//
void AppProcessCommand (short menuID, short menuItem)

{
	Str255		name;
	GrafPtr		savePort;
	void		PartialScreen(Boolean fromFull);

	switch (menuID) {
	case AppleMENU:
		if (menuItem == AM_About)
			NewABWindow();
		else {
			GetPort(&savePort);
			GetItem(gAppleMenu, menuItem, name);
			OpenDeskAcc(name);
			SetPort(savePort);
		}
		break;
	
	case FileMENU:
		switch (menuItem) {
		case FM_SignOnTCP:
			// SignOff();		// 7/10/95
#if RAINBOWRED
			BeginTCPConnection("palace.rainbowalley.com",9998);
#else
			if (ApproveSignOn())	// 1/14/97 JAB
				OpenTCPSession(C_PalaceTCP);
#endif
			break;
		case FM_SignOnAppleTalk:
#if !RAINBOWRED
			SignOff();		// 7/10/95
			if (ApproveSignOn())	// 1/14/97 JAB
				SignOnAppleTalk();
#endif
			break;
		case FM_SignOff:
			SignOff();
			break;
		case FM_Close:
			{
				WindowPtr theWin;
				theWin = FrontFloating();
				if (theWin)
					((ObjectWindowPtr) theWin)->Close(theWin);
			}
			break;
		case FM_Log:
			ToggleLogFile();
			break;
		case FM_ChildLock:
			if (!(gPrefs.userPrefsFlags & UPF_ClubMode))
				ChildLockDialog();
			break;
		case FM_ReloadScript:
			if (LoadUserScript() == noErr)
				StdStatusMessage(SM_ScriptLoaded);
			else
				StdStatusMessage(SM_ScriptError);
			break;
  		case FM_Quit:
			gQuitFlag = true;
			break;
		case FM_Register:
			// 1/14/97 JAB Changed to use "isRegistered" instead of "guestAccess" to control this
			if (!gSecure.isRegistered)
				 RegInfo();
			else
				RegComplete2Dialog();
			break;
		}
		break;
	case EditMENU:
		if (menuItem == EM_Prefs)
			PreferencesDialog();
		else {
			if (!SystemEdit(menuItem-1)) {
				switch (menuItem) {
				case EM_Undo:
					if (gDrawWin)
						UseDetonateTool(DC_Delete);
					break;
				case EM_Cut:	
					PrepareTextColors();	
					TECut(gRoomWin->msgTEH);	
					RestoreWindowColors();
					break;
				case EM_Paste:
					PrepareTextColors();	
					TEPaste(gRoomWin->msgTEH);	
					RestoreWindowColors();
					break;
				case EM_PasteMultiple:
					PasteLarge();
					break;
				case EM_Copy:	
					PrepareTextColors();	
					TECopy(gRoomWin->msgTEH);	
					RestoreWindowColors();
					break;
				case EM_Clear:	
					PrepareTextColors();	
					TEDelete(gRoomWin->msgTEH);	
					RestoreWindowColors();
					break;
				case EM_SelectAll:
					PrepareTextColors();
					TESetSelect(0,32767,gRoomWin->msgTEH);
					RestoreWindowColors();
					break;
				}
			}
		}
		break;
	case OptionsMENU:
		switch (menuItem) {
		case OM_ShowNames:
			gPrefs.userPrefsFlags ^= UPF_AutoShowNames;
			// StorePreferences();
			break;
		case OM_FullScreen:
			if (gIconized)
				DeIconize();
			if (gFullScreen)
				PartialScreen(true);
			else
				FullScreen();
			break;
		case OM_Iconize:
			if (gIconized)
				DeIconize();
			else
				Iconize();
			break;
		case OM_GoBack:
			GoBack();
			break;
		case OM_ShowLog:
			if (gLogWin) {
				((ObjectWindowPtr) gLogWin)->floating = true;
				ShowWindow((WindowPtr) gLogWin);
				SelectFloating((WindowPtr) gLogWin);
				if (gTheEvent.modifiers & shiftKey) {
					Point	dPos = {40,40};
					RestoreWindowPos((WindowPtr) gLogWin, &dPos);
				}
			}
			else
				NewLogWindow();
			break;
		case OM_UserList:
			if (gULWin)
				RefreshUserList();
			else
				NewULWindow();
			break;
		case OM_RoomList:
			if (gRLWin)
				RefreshRoomList();
			else
				NewRLWindow();
			break;
		case OM_Wizard:
			ToggleWizard();
			break;
		case OM_Draw:
			ToggleDrawPalette();
			break;
#if !RELEASE
		case OM_Debug:		// 5/25/95 JAB
			// TestCM();
			gDebugFlag = !gDebugFlag;
			LogMessage("Debug %s\r",gDebugFlag? "on": "off");
			ShowRoomStatus();
			break;
#endif
		}
		break;

	case MacroMENU:
		switch (menuItem) {
		case MM_Macro1:
		case MM_Macro2:
		case MM_Macro3:
		case MM_Macro4:
		case MM_Macro5:
		case MM_Macro6:
		case MM_Macro7:
		case MM_Macro8:
		case MM_Macro9:
			{
				short	n;
				n = ComputeMacroNumber(1 + (menuItem - MM_Macro1),gTheEvent.modifiers);
				PlayMacro( n );
			}
			break;
		case MM_Macro0:
			{
				short	n;
				n = ComputeMacroNumber(0,gTheEvent.modifiers);
				PlayMacro( n );
			}
			break;
		}
		break;
	case RecordMacroMENU:
		switch (menuItem) {
		case RM_Macro1:
		case RM_Macro2:
		case RM_Macro3:
		case RM_Macro4:
		case RM_Macro5:
		case RM_Macro6:
		case RM_Macro7:
		case RM_Macro8:
		case RM_Macro9:
			{
				short	n;
				n = ComputeMacroNumber(1 + (menuItem - RM_Macro1),gTheEvent.modifiers);
				RecordMacro(n);
			}
			break;
		case RM_Macro0:
			{
				short	n;
				n = ComputeMacroNumber(0,gTheEvent.modifiers);
				RecordMacro(n);
			}
			break;
		}
		break;
	case SoundMENU:
		CheckItem(gSoundMenu, gPrefs.soundLevel+1, false);
		AdjustUserVolume(menuItem-1);
		CheckItem(gSoundMenu, gPrefs.soundLevel+1, true);
		break;
	case ExpertMENU:
		switch (menuItem) {
		case EM_NewRoom:
			DoPalaceCommand(PC_NewRoom,0L,NULL);
			break;
		case EM_RoomInfo:
			RoomInfoDialog();
			break;
		case EM_NewDoor:
			SwitchModes(M_Authoring);
			DoPalaceCommand(PC_NewSpot,0L,NULL);
			break;
		case EM_DoorInfo:
			DoorInfoDialog();
			break;
		case EM_Authoring:
			if (gMode == M_Authoring)
				SwitchModes(M_Normal);
			else
				SwitchModes(M_Authoring);
			break;
		case EM_DragDoors:
			gDragDoors = !gDragDoors;
			break;
		case EM_ShowCoords:
			gRecordCoords = !gRecordCoords;
			if (!gRecordCoords)
				DrawRoomStatus();
			break;
		}
		break;
	case LayerMENU:
		switch (menuItem) {
		case LM_MoveToBottom:
		case LM_MoveBackward:
		case LM_MoveForward:
		case LM_MoveToTop:
			LayerSpot(menuItem);
			break;
		}
		break;
	case PropMENU:
		switch (menuItem) {
		case PM_NewProp:
			NewPEWindow(NULL);		// 6/8/95
			break;
		case PM_EditProp:
			NewPEWindow(&gSelectProp);
			break;
		case PM_DuplicateProp:						// 6/9/95
			DuplicatePropFavorite(&gSelectProp);	// 6/9/95
			break;
		case PM_DeleteProp:
			DeletePropFavorite(&gSelectProp);
			break;
		case PM_Purge:
			PurgeOldProps();
			break;
		}
		break;
	case VoicesMENU:
		if (menuItem == 1) {
			if (gPrefs.voiceNumber)
				CheckItem(gVoicesMenu, gPrefs.voiceNumber+2,false);
			gPrefs.userPrefsFlags &= ~UPF_TextToSpeech;
			gPrefs.voiceNumber = 0;
			CheckItem(gVoicesMenu, 1, true);
			StorePreferences();
		}
		else {
			if (gPrefs.voiceNumber)
				CheckItem(gVoicesMenu, gPrefs.voiceNumber+2,false);
			else
				CheckItem(gVoicesMenu, 1, false);
			gPrefs.userPrefsFlags |= UPF_TextToSpeech;
			SelectVoice(menuItem-2);
			gPrefs.voiceNumber = menuItem - 2;
			StorePreferences();
			CheckItem(gVoicesMenu, gPrefs.voiceNumber+2,true);
			SpeakChat(0,"Speech On");
		}
		break;
	case HelpMENU:
		{
			Str255	urlStr,fileStr,fileUrlStr;
			GetIndString(urlStr,160,menuItem);
			GetIndString(fileStr,161,menuItem);
			GetIndString(fileUrlStr,162,menuItem);
			if (fileStr[0]) {
				// If File Exists, make a file URL
				OSErr	oe;
				FInfo	fInfo;
				oe = GetFInfo(fileStr, 0, &fInfo);
				if (oe == noErr) {
					OSErr LaunchDocument(FSSpecPtr	theDoc);
					FSSpec	fileSpec;
					FSMakeFSSpec(0,0L,fileStr,&fileSpec);
					oe = LaunchDocument(&fileSpec);
					if (oe)
						StatusMessage("Launch Error",oe);
					HiliteMenu(0);
					return;
					// BlockMove(fileUrlStr,urlStr,fileUrlStr[0]+1);
				}
			}
			if (urlStr[0]) {
				PtoCstr(urlStr);
				GotoURL((char *) urlStr);
			}
		}
		break;
	case PlugInsMENU:
		PPAMgrSelectPlugInsMenuItem(menuItem);
		break;
	}
	HiliteMenu(0);
}


void MyAdjustMenus()
{
	register WindowPeek wp;
	Boolean	isObjectWindow;
	ObjectWindowPtr		op;
	wp = (WindowPeek) FrontWindow();
	isObjectWindow = (wp && wp->refCon == ObjectWindowID);
	op = (ObjectWindowPtr) wp;
	if (isObjectWindow)
		op->AdjustMenus((WindowPtr) wp);
	else
		AppAdjustMenus();
}

// Code to simplify enabling/disabling menu items.
//
void MyEnableMenuItem(MenuHandle menu, short item, short ok)
{
	if (ok)
		EnableItem(menu, item);
	else
		DisableItem(menu, item);
	if (item == 0)
		DrawMenuBar();
}


void MyHandleMenu(long mSelect)
{
	int			menuID = HiWord(mSelect);
	int			menuItem = LoWord(mSelect);
	register 	WindowPeek wp;
	Boolean		isObjectWindow;
	ObjectWindowPtr		op;

	wp = (WindowPeek) FrontWindow();
	isObjectWindow = (wp && wp->refCon == ObjectWindowID);
	op = (ObjectWindowPtr) wp;
	if (isObjectWindow)
		op->ProcessCommand((WindowPtr) op, menuID, menuItem);
	else
		AppProcessCommand(menuID, menuItem);
	HiliteMenu(0);
}

