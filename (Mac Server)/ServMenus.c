/************************************************************************************
 * Menus.c
 *
 *	Menus for Palace Server.
 *
 *
 ************************************************************************************/
#include "S-SERVER.H"
#include "AppMenus.h"
#include "ServRegistration.h"

//void NewABWindow(void);

// Menu Handles
MenuHandle	gAppleMenu, gFileMenu, gEditMenu, gFontMenu, gFontSizeMenu, gCreatorMenu;

// Menu Initialization code.

void MySetUpMenus(void)
{
	Handle	myMenuBar;
	myMenuBar = GetNewMBar(MenuBaseID);
	SetMenuBar(myMenuBar);

	gAppleMenu = GetMHandle(AppleMENU);
	gFileMenu = GetMHandle(FileMENU);
	gEditMenu = GetMHandle(EditMENU);

	AddResMenu(gAppleMenu, 'DRVR');

#if !RELEASE
	AppendMenu(gEditMenu, "\pDebug/D");
#endif
	DrawMenuBar();
}


//	Enable or disable the items in the Edit menu if a DA window
//	comes up or goes away. Our application doesn't do anything with
//	the Edit menu.


void AppAdjustMenus(void)
{
	register WindowPeek wp;
	short windowKind;
	Boolean isDA,isObjectWindow;

	wp = (WindowPeek) FrontWindow();
	windowKind = wp ? wp->windowKind : 0;
	isDA = (windowKind < 0);
	isObjectWindow = (wp->refCon == ObjectWindowID);

	if (gLogWin->logFile)
		SetItem(gFileMenu, FM_Log, "\pStop Log");
	else
		SetItem(gFileMenu, FM_Log, "\pStart LogÉ");

	MyEnableMenuItem(gEditMenu, EM_Undo, isDA);
	MyEnableMenuItem(gEditMenu, EM_Cut, isDA);
	MyEnableMenuItem(gEditMenu, EM_Copy, isDA);
	MyEnableMenuItem(gEditMenu, EM_Paste, isDA);
	MyEnableMenuItem(gEditMenu, EM_Clear, isDA);

#if !RELEASE
	CheckItem(gEditMenu, EM_Debug, gDebugFlag);
#endif

	MyEnableMenuItem(gFileMenu, FM_Open, true);
	MyEnableMenuItem(gFileMenu, FM_Close, isDA || isObjectWindow);
}




//	Handle the menu selection. mSelect is what MenuSelect() and
//	MenuKey() return: the high word is the menu ID, the low word
//	is the menu item
//
void AppProcessCommand (short menuID, short menuItem)

{
	Str255		name;
	GrafPtr		savePort;

	switch (menuID) {
	case AppleMENU:

#ifdef NOTYET
		if (menuItem == AM_About)
			NewABWindow();
		else {
			GetPort(&savePort);
			GetItem(gAppleMenu, menuItem, name);
			OpenDeskAcc(name);
			SetPort(savePort);
		}
#endif
		GetPort(&savePort);
		GetItem(gAppleMenu, menuItem, name);
		OpenDeskAcc(name);
		SetPort(savePort);
		break;
	
	case FileMENU:
	{
		switch (menuItem) 
		{
			case FM_Log:
			{
				ToggleLogFile();
				break;
			}
			
			case FM_UserList:
			{
				ToggleUserList();
				break;
			}
			
			case FM_CreateUserLog:
			{
				void GenerateUserDataFile();
				GenerateUserDataFile();
				break;
			}
				
			case FM_Registration:
			{
				if (RegistrationDialog())
				{
					(void) CheckServerSecurity();  // Calling to get the new max occupancy set
					gPrefs.maxOccupancy = gMaxPeoplePerServer;
					LogMessage("Server capacity changed to %d users\r", gMaxPeoplePerServer);
				}
				
				break;
			}
			
	  		case FM_Quit:
	  		{
				gQuitFlag = true;
				break;
			}
		}
		break;
	}
	
	case EditMENU:
		if (!SystemEdit(menuItem-1)) {
			switch (menuItem) {
			case EM_Prefs:
				PreferencesDialog();
				break;
			case EM_YellowPages:
				YPDialog();
				break;
#if !RELEASE
			case EM_Debug:
				gDebugFlag = !gDebugFlag;
				LogMessage("Debug %s\r",gDebugFlag? "on" : "off");
				break;
#endif
			}
		}
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


