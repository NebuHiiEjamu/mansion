/* m-cmds.h */

#ifndef _H_M_CMDS
#define _H_M_CMDS	1


/* Palace (Hotspot) Events */
/* used by server for event handler parsing */

enum {		PE_Select,		/* 4/6/95 JBUM */
			PE_Lock,
			PE_Unlock,
			PE_Hide,
			PE_Show,
			PE_Startup,
			PE_Alarm,		/* Replaced IDLE  6/28/95 */
			PE_Custom,
			PE_InChat,		/* 6/6/95 JAB - formerly "chat" */
			PE_PropChange,	/* 4/6/95 JBUM */
			PE_Enter,		/* 4/6/95 JBUM */
			PE_Leave,		/* 4/6/95 JBUM */
			PE_OutChat,		/* 6/6/95 JAB - outgoing chat (from client) */
			PE_SignOn,		/* User Event */
			PE_SignOff,		/* User Event (not useful?) */
			PE_Macro0,		/* Macros */
			PE_Macro1,
			PE_Macro2,
			PE_Macro3,
			PE_Macro4,
			PE_Macro5,
			PE_Macro6,
			PE_Macro7,
			PE_Macro8,
			PE_Macro9,
			PE_PPAMessage=32,	/* 6/27/96 PPA Only events (don't get passed to hot spots */
			PE_PPAMacro,
			PE_NbrEvents};


/* PalaceCommands - used in DoPalaceCommand and PPA callback function
 * - shouldn't be used by server
 */

enum {
	PC_Lock,
    PC_Unlock,
    PC_IsLocked,
    PC_GotoRoom,
	PC_GlobalMsg,
    PC_RoomMsg,
    PC_SusrMsg,
    PC_LocalMsg,
	PC_SetPicOffset,
    PC_SetLoc,
    PC_SetSpotState,
	PC_NewRoom,
    PC_SetRoomInfo,
    PC_NewSpot,
    PC_SetSpotInfo,
	PC_DeleteSpot,
    PC_AddLooseProp,
    PC_DelLooseProp,
	PC_MoveLooseProp,
    PC_LaunchApp,
    PC_SuperUser,
	PC_KillUser,
	PC_GotoURL,
	PC_GetSpotState,		/* 6/27/95 */
	PC_SetSpotStateLocal,	/* 6/28/95 */
	PC_Chat,				/* 7/20/95 - regular chat */
	PC_PlaySound,
	PC_ExecuteIptscrae,		/* 6/27/96 */
	PC_LogMessage,			/* 6/27/96 */
	PC_LaunchPPA,
	PC_KillPPA,
	PC_UpdateRect,
	PC_GetMyID,
	PC_GetNumberPeople,
	PC_GetUserByIndex,
	PC_GetUserByID,
	PC_GetUserByName,
	/* All these are still unimplimented */
	PC_FreezeScreen,
	PC_UnfreezeScreen,
	PC_SetSpotName,	
	PC_GetSpotName,	
	PC_TrackGlobal,
	PC_GetGlobal,
	PC_GetNewWindow,
	PC_DisposeWindow,
	/* 10/8/96 new - DAS */
	PC_BlowThrough,
	/* 10/11/96 new - JAB */
	PC_PostServerEvent,
	
	/* 12/18/96 new - CDM */
	PC_GetWindow,
	PC_GetMediaPathName,
	
    PC_NbrCmds };

#endif
