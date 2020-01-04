// Bottleneck for Palace Control

#include "U-USER.H"
#include "m-cmds.H"
#include "u-script.h"
#include "u-timout.h"
#include "PPAMgr.h"

// Added this function... 5/24/95 JAB
void ToggleDebug(void);

void ShowUserProps();
void ShowUserProps()
{
	RoomRec			*rp = &gRoomWin->curRoom;
	LocalUserRecPtr	up;
	int				i,j;

	up = gRoomWin->userList;

	for (i = 0; i < rp->nbrPeople; ++i,++up) {
		if (up->user.nbrProps) {
			for (j = 0; j < up->user.nbrProps; ++j) {
				LogMessage("%s id=%08lx crc=%08lx hdl=%08lx\r",
					CvtToCString(up->user.name),
					(long) up->user.propSpec[j].id,
					(long) up->user.propSpec[j].crc,
					(long) up->propHandle[j]);
			}
		}
	}
}

void ProcessMacro(char *str)
{
	long		cmd;
#if BOTCODE
	void		ProcessBotMacro(char *str);
	ProcessBotMacro(str);
#endif
#if PPASUPPORT
	PPAMgrPropagatePalaceEvent(PE_PPAMacro,0,str);
#endif
	cmd = *((long *) str);
	str += 4;
	while (*str == ' ')
		++str;

	switch (cmd) {
	case CMD_SUPERUSER:
		if (strlen(str) >= 1 && strlen(str) < 31) {
			CtoPstr(str);
			EncryptString((StringPtr) str,(StringPtr) str);
			DoPalaceCommand(PC_SuperUser,(long) str, NULL);
		}
		break;
	case CMD_KILLUSER:
		if (!gRoomWin->expert && DeniedByServer(PM_PlayersMayKill))
			return;
		if (!(gRoomWin->userFlags & U_God) && DeniedByServer(PM_WizardsMayKill))
			return;
		if (gRoomWin->targetID)
			DoPalaceCommand(PC_KillUser,(long) gRoomWin->targetID, NULL);
		else if (str[0]) {
			LocalUserRecPtr	up;
			if ((up = GetUserByName(str)) != NULL && up->user.roomID == gRoomWin->curRoom.roomID)
				DoPalaceCommand(PC_KillUser, up->user.userID, NULL);
		}
		break;
	case CMD_GMSG:
	case CMD_SMSG:
		// 8/6/96 JAB Added expert check
		if (strlen(str) && gRoomWin->expert)		// 5/8/95 Error Check
			DoPalaceCommand(PC_GlobalMsg,0L,str);
		break;
	case CMD_PAGE:
	case CMD_WMSG:				// 5/8/95 Error Check
		if (strlen(str))
			DoPalaceCommand(PC_SusrMsg,0L,str);
		break;
	case CMD_RMSG:				// 5/8/95 Error Check
		// 8/6/96 - JAB - Only wizards can use rmsg...
		if (gRoomWin->expert) {
			if (strlen(str))
				DoPalaceCommand(PC_RoomMsg,0L,str);
		}
		break;
	case CMD_KILLPROPS: 		// 6/20/95
		if (gRoomWin->expert)
		{
			long	pcmd[1];
			pcmd[0] = -1L;
			DoPalaceCommand(PC_DelLooseProp,(long) &pcmd[0],NULL);
		}
		break;
	case CMD_CLONE:
		if (gRoomWin->targetID && gRoomWin->expert) {
			LocalUserRecPtr	up;
			short			i;
			if ((up = GetUser(gRoomWin->targetID)) != NULL) {
				ChangeUserDesc(up->user.faceNbr, up->user.colorNbr, up->user.nbrProps,
								up->user.propSpec);
				for (i = 0; i < up->user.nbrProps; ++i)
					AddPropFavorite(&up->user.propSpec[i]);	// 6/7/95
			}
		}
		break;
	case 'psho':
		ShowUserProps();
		break;
#ifdef dontcompile
#define CMD_TESTLOG	0x676f6c74	// 'tlog'

	case CMD_TESTLOG:	// DEBUG !!!!
		{
			static char testBuf[] =
				"Test: 0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
			short	i;
			for (i = 0; i < 300; ++i) {
				LogMessage("%s\r",testBuf);
			}
		}
		break;
#endif

#ifdef WIN32
  case CMD_IPADDR:
    {
      char str[128];
      LogMsg(0,SysInfoText,"IP Address: %s\n",(LPSTR)GetIPAddress(str));
    }
    break;
#endif
	case CMD_CLUBMODE:
		gPrefs.userPrefsFlags |= UPF_ClubMode;
		StorePreferences();

		break;
	case CMD_DEBUG:
		ToggleDebug();
		break;
#ifdef dontcompile
	case 'rerr':	// DEBUG !!!!
		{
			short	rnbr;
			rnbr = atoi(str);
			ReportVerboseMessage(rnbr);
		}
		break;
#endif
#if macintosh
	case 'gtst':
		{
			void RoomDrawTest();
			RoomDrawTest();
		}
		break;
	// 1/14/97 JAB Debugging...
	case 'shoe':
		{
			LogMessage("Elapsed: demo: %ld (%ld minutes)  total: %ld (%ld minutes) \r",
				gAuxRegRec.demoElapsed,gAuxRegRec.demoElapsed/60L,
				gAuxRegRec.totalElapsed,gAuxRegRec.totalElapsed/60L);				
		}
		break;
#endif
/**
#if macintosh
	// For testing user dictionary entries
	case 'phon':
		{
			void GetPhonemes(char *str);
			GetPhonemes(str);
		}
		break;
#endif
**/
	}
}



LONG	DoPalaceCommand(LONG cmdType, LONG arg, char *str)
{
	switch (cmdType) 
	{
		case PC_BlowThrough:
			{
				PostServerEvent(MSG_BLOWTHRU, gRoomWin->meID, str, arg);
				break;
			}
		case PC_Lock:
			{
				short	lockRec[2];
				lockRec[0] = gRoomWin->curRoom.roomID;
				lockRec[1] = (short)arg;
				PostServerEvent(MSG_DOORLOCK,gRoomWin->meID,(Ptr) &lockRec[0],sizeof(short)*2);
			}
			break;
		case PC_Unlock:
			{
				short	lockRec[2];
				lockRec[0] = gRoomWin->curRoom.roomID;
				lockRec[1] = (short)arg;
				PostServerEvent(MSG_DOORUNLOCK,gRoomWin->meID,(Ptr) &lockRec[0],sizeof(short)*2);
			}
			break;
		case PC_IsLocked:
			return DoorIsLocked((short) arg);
			break;
		case PC_GotoRoom:
			if (arg != gRoomWin->curRoom.roomID && !gRoomWin->navInProgress)
			{
				short	roomID;
				roomID = (short)arg;
				// 9/27/95 Removed this restriction
				// if (MembersOnly(roomID < MinGuestRoom || roomID > MaxGuestRoom))
				// 	  return 0;

				// This is a little early to generate the PE_Leave event, since the
				// command may be rejected by server, unforunately, if it is accepted,
				// the server moves us, in which case it is too late to run scripts which
				// say "Fare well".
				UpdateAssetFile();	// 2/28/96
				TriggerHotspotEvents(PE_Leave);
				AddRoomToBackStack();
				PostServerEvent(MSG_ROOMGOTO,gRoomWin->meID,(Ptr) &roomID,sizeof(short));
				gRoomWin->navInProgress = true;  /* 9/12/95 */
			}
			break;
		case PC_GlobalMsg:
			if (strlen(str) > 255)	// 11/1/95
				str[255] = 0;
			PostServerEvent(MSG_GMSG,gRoomWin->meID,str,strlen(str)+1);
			break;
		case PC_RoomMsg:
			if (strlen(str) > 255)	// 11/1/95
				str[255] = 0;
			PostServerEvent(MSG_RMSG,gRoomWin->meID,str,strlen(str)+1);
			break;
		case PC_SusrMsg:
			if (strlen(str) > 255)	// 11/1/95
				str[255] = 0;
			PostServerEvent(MSG_SMSG,gRoomWin->meID,str,strlen(str)+1);
			break;
		case PC_LocalMsg:
			if (strlen(str) > 255)	// 11/1/95
				str[255] = 0;
			ProcessMansionEvent(MSG_TALK,0,str,strlen(str)+1);
			break;
		case PC_SetPicOffset:
			PostServerEvent(MSG_PICTMOVE,gRoomWin->meID,(Ptr) arg, sizeof(short)*4);
			break;
		case PC_SetLoc:
			PostServerEvent(MSG_SPOTMOVE,gRoomWin->meID,(Ptr) arg, sizeof(short)*4);
			break;
		case PC_SetSpotState:
			PostServerEvent(MSG_SPOTSTATE,gRoomWin->meID,(Ptr) arg, sizeof(short)*3);
			break;
		case PC_NewRoom:
			PostServerEventShort(MSG_ROOMNEW);
			break;
		case PC_SetRoomInfo:
			PostServerEvent(MSG_ROOMSETDESC,gRoomWin->meID,(Ptr) arg, sizeof(RoomRec) + ((RoomRecPtr) arg)->lenVars);
			break;
		case PC_NewSpot:
			PostServerEventShort(MSG_SPOTNEW);
			break;
		case PC_SetSpotInfo:
			// !!! need to use accurate record size here!
			// PostServerEvent('sRom',gRoomWin->meID,(Ptr) arg, sizeof(Hotspot)*2+512);
			break;
		case PC_SetSpotStateLocal:		// 6/28/95
			SetSpotState((short *) arg);
			break;
		case PC_DeleteSpot:
			{
				short	spotID;
				spotID = (short)arg;
				PostServerEvent(MSG_SPOTDEL,gRoomWin->meID,(Ptr) &spotID, sizeof(short));
			}
			break;

		case PC_MoveLooseProp:
			PostServerEvent(MSG_PROPMOVE,gRoomWin->meID,(Ptr) arg, sizeof(long)*2);
			break;
		case PC_AddLooseProp:
			// 10/23/95
			if (IsRareProp((AssetSpec *) arg))
				DeletePropFavorite((AssetSpec *) arg);
			PostServerEvent(MSG_PROPNEW,gRoomWin->meID,(Ptr) arg, sizeof(long)*3);	// 6/7/95
			break;
		case PC_DelLooseProp:
			PostServerEvent(MSG_PROPDEL,gRoomWin->meID,(Ptr) arg, sizeof(long));
			break;
		case PC_LaunchApp:
			UpdateAssetFile();	// 2/28/96
			LaunchApp(str);
			break;
		case PC_SuperUser:
			{
				StringPtr pword;
				pword = (StringPtr) arg;
				PostServerEvent(MSG_SUPERUSER, gRoomWin->meID, (Ptr) pword, pword[0]+1);
			}
			break;
		case PC_KillUser:
			PostServerEvent(MSG_KILLUSER, gRoomWin->meID, (Ptr) &arg, sizeof(long));
			break;
		case PC_GotoURL:
			UpdateAssetFile();	// 2/28/96
			GotoURL(str);	// System Dependant Function
			break;
		case PC_GetSpotState:
			return GetSpotState((short) arg);
			break;
		case PC_Chat:		// 7/20/95
			if (strlen(str) > 255)	// 11/1/95
				str[255] = 0;

			if (arg) {		// 6/27/96 Support WHISPERS if ARG > 0
				char	tbuf[256],*p;
				int		len;
				
				if ((len = strlen(str)) > 248) {
					str[248] = 0;
					len = 248;
				}
				p = tbuf;
				*((long *) p) = arg;
				p += 4;
				*((short *) p) = len+3;
				p += 2;
				EncryptCString((unsigned char *)str,(unsigned char *) p,len);
				p[len] = 0;
				PostServerEvent(MSG_XWHISPER, gRoomWin->meID, tbuf, len+7);
			}
			else
				PostServerEvent(MSG_TALK, gRoomWin->meID, str, strlen(str)+1);
			break;
		case PC_PlaySound:	// 9/27/95  play Sound file from Sounds folder asyncronously
			// 8/6/96 Added boolean argument to indicate that fileshould be
			// requested if it can't be played...
			PlayWaveFile(str, true);	// str is C-String
			break;
		case PC_ExecuteIptscrae:	// 6/27/96 JAB
			// 8/27/96 return value of script, if any
			return DoUserScript(str);
			break;
		case PC_LogMessage:			// 6/27/96 JAB
			LogMessage("%s",str);
			break;
	#if PPASUPPORT
		case PC_LaunchPPA:
			PPAMgrPPACommand(str);
			break;
		case PC_KillPPA:
			PPAMgrPPAKillByName(str);
			break;
		case PC_UpdateRect:
			RefreshRoom((Rect *) &str[0]);
			break;
	#endif
		case PC_GetMyID:
			return gRoomWin->meID;
			break;
		case PC_GetNumberPeople:
			return gRoomWin->curRoom.nbrPeople;
			break;
		case PC_GetUserByIndex:
			if (arg >= gRoomWin->curRoom.nbrPeople)
				return 0L;
			else
				return (LONG) &gRoomWin->userList[arg].user;
		case PC_GetUserByID:
			{
				LocalUserRecPtr	up;
				up = GetUser(arg);
				if (up)
					return (LONG) &up->user;
				else
					return NULL;
			}
			break;
		case PC_GetUserByName:
			{
				LocalUserRecPtr	up;
				up = GetUserByName(str);
				if (up)
					return (LONG) &up->user;
				else
					return NULL;
			}
			break;
			
		/* 10/11/96 JAB - added ability for plug-ins to post palace events */
	    case PC_PostServerEvent:
	    {
	        if (str) {
	            LONG eventType,refCon,length;

	            // buffer begins with eventtype, and eventlength
	            eventType = *((LONG *) str);
	            length = *((LONG *) &str[4]);

	            // arg is used for refCon, if 0, we use the userid
	            if (arg != 0)
	                   refCon = arg;
	            else
	                   refCon = gRoomWin->meID;


	            // if eventType is unspecified, we use MSG_BLOWTHRU
	            if (eventType == 0)
	                    eventType = MSG_BLOWTHRU;

	            // if the length looks reasonable, we post the event
	            if (length >= 0 && length < 12000L)
	                    PostServerEvent(eventType,refCon,str+8,length);
	            else
	                    return 1;
	        }
	        else
	                return 1;
	            break;
	    }
	    
	    /* 12/18/96 CDM - added ability for plug-ins to get the client window pointer */
	    case PC_GetWindow:
	    {
	    	return ((long)gRoomWin);
	    	break;
	    }
	    
	    /* 12/18/96 CDM - gave plug-ins a way to get media pathnames */
	    case PC_GetMediaPathName:
	    {
	    	StringPtr	fullName, returnName;
	    	short		n, pathlist;
	    	OSErr		error = noErr;
	    	FInfo		fInfo;
	    	
	    	pathlist = PicturesPathList + arg;
	    	
    		if (str[0] == 0)
				return 0;

			n = 0;
			do
			{
				++n;
				fullName = BuildMediaFolderName((unsigned char*)str, pathlist, n);
				if (fullName)
				{
					error = GetFInfo(fullName, 0, &fInfo);
				}
			} while (fullName && error != noErr);
			
			if (fullName == NULL || error != noErr)
				return 0;
			
			returnName = (StringPtr)NewPtr(fullName[0]+1);
			BlockMove(fullName, returnName, fullName[0]+1);
			
			return ((long)returnName);
			
	    	break;
	    }
	}
	return 0;
}

// New Functions to support "Back" menu command / key

#define MaxBackStack	16
RoomID	gBackStack[MaxBackStack];
int		gNbrBackStack;

Boolean BackStackAvailable()	// Used to enable/disable menu
{
	return gNbrBackStack > 0;
}

void ClearBackStack(void)
{
	gNbrBackStack = 0;
}

void AddRoomToBackStack()
{
	if (gNbrBackStack == MaxBackStack) {
		BlockMove((Ptr)&gBackStack[1],(Ptr)&gBackStack[0],sizeof(RoomID)*(MaxBackStack-1));
		--gNbrBackStack;
	}
	gBackStack[gNbrBackStack++] = gRoomWin->curRoom.roomID;
}

void GoBack()
{
	if (gNbrBackStack) {
		RoomID	roomID;
		--gNbrBackStack;
		roomID = gBackStack[gNbrBackStack];
		DoPalaceCommand(PC_GotoRoom,(long) roomID,NULL);
		if (gNbrBackStack && gBackStack[gNbrBackStack-1] == gRoomWin->curRoom.roomID)
			--gNbrBackStack;	// Don't push room entry when using back key
	}
}
