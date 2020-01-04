// ServerMenus.h

extern MenuHandle	gAppleMenu, gFileMenu, gEditMenu, gFontMenu, gFontSizeMenu, gCreatorMenu;

#define MenuBaseID	128

// Menu Resource IDs
enum	{AppleMENU = 128, FileMENU, EditMENU, placeholder, FontMENU, FontSizeMENU, CreatorMENU};

// Menu Item Numbers
enum	{FM_Open = 1, FM_Close,	FM_Div1, FM_Log, FM_Div2, FM_Quit};
enum 	{EM_Undo = 1, EM_Div1, EM_Cut, EM_Copy, EM_Paste, EM_Clear, EM_SelectAll,
		 EM_Div2, EM_Prefs, EM_Debug};
