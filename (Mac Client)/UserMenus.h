// UserMenus.h

#define MenuBaseID	128

extern MenuHandle	gAppleMenu, gFileMenu, gEditMenu, gOptionsMenu, gFontMenu, 
					gFontSizeMenu, gRecordMacroMenu, gBaudMenu, gPortMenu, gSoundMenu, 
					gExpertMenu, gMacroMenu, gSpotTypeMenu, gPropMenu, gSpotStateMenu,
					gPropEditMenu;

// Menu Resource IDs
enum	{AppleMENU = 128, FileMENU, EditMENU, OptionsMENU, FontMENU, FontSizeMENU,
		 RecordMacroMENU, BaudMENU, PortMENU, SoundMENU, ExpertMENU,
		 MacroMENU, SpotTypeMENU, PropMENU, SpotStateMENU,PropEditMENU};

// Menu Item Numbers
enum	{AM_About=1};

enum	{FM_SignOn=1, FM_SignOnTCP, FM_Close, FM_Div1, 
		 FM_Log, FM_Guest, FM_Div2, FM_Quit};

enum 	{EM_Undo=1,EM_Div1,  EM_Cut, EM_Copy, EM_Paste, EM_Clear, EM_SelectAll, EM_Div2, EM_Prefs};

enum 	{OM_Draw=1, OM_Sound, OM_FullScreen, OM_Iconize, OM_Div1,
		 OM_ShowLog, OM_UserList, OM_RoomList, OM_Div2,
		 OM_Wizard, OM_Div3, OM_Debug};
		
enum	{MM_RecordMacro=1, MM_Div1, MM_Macro1, 
		 MM_Macro2, MM_Macro3, MM_Macro4, MM_Macro5, 
		 MM_Macro6, MM_Macro7, MM_Macro8, MM_Macro9, MM_Macro0};

enum	{RM_Macro1 = 1, RM_Macro2, RM_Macro3, RM_Macro4, RM_Macro5, 
		 RM_Macro6, RM_Macro7, RM_Macro8, RM_Macro9, RM_Macro0};

enum	{EM_NewRoom=1, EM_RoomInfo, ExM_Div1,
		 EM_NewDoor, EM_DoorInfo, ExM_Div2,
		 EM_Authoring, EM_DragDoors, EM_Div3,
		 EM_ShowCoords, EM_Debug};

enum	{PM_NewProp=1,PM_EditProp,PM_DuplicateProp,PM_DeleteProp,PM_Div1,
		 PM_PropLibrary};
		 
enum	{PEM_Head=1, PEM_Ghost, PEM_Div1,
		 PEM_WhiteBG, PEM_GrayBG, PEM_BlackBG, PEM_Div2,
		 PEM_PropInfo,PEM_PropScript};
		 
Boolean CanPaste(long type);

