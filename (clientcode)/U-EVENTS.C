///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "U-USER.H"
#include "U-TIMOUT.H"

#if PPASUPPORT
#include "PPAMgr.h"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/****
	Server Commands - sent FROM Server
	
	'vers'		Mansion Version Number
	'down'		Server is going down
	'init'		Server is going up
	'tiyr'		This is your id#
	'room'		Current Room Description (&RoomRec)
	'rprs'		Room Person Description Array of UserRecs
	'endr'		End Room Description
	'nprs'		New Person in Current Room (&UserRec)
	'eprs'		Person is leaving room
	'uLoc'		User has changed position (id,Point)
	'usrC'		Change User Color short colorNbr
	'usrF'		<change user face>  short faceNbr
	'talk'		&id, text

	'usrN'		<change user name> name[]
	'usrP'		<change user prop> prop
	'whis'		msg[]
	'log '		&id user logged on &totalPeople
	'bye '		&id  user logged off (and is leaving room, of course) &totalPeople
	'ping'		&userID		// Used to see if user is there
	'pong'		&userID		// response to ping
	'lock'		&roomID, &doorID	// Lock Door
	'unlo'		&roomID, &doorID	// Unlock Door
	'sSta'		&room,&spot,&state	// update state (and possibly picture) for hotspot
	'pLoc'		&room,&spot,&loc	// update picture offset
	'sLoc'		&room,&stpo,&loc	// update spot location
	'sRom'		roomdesc			// modify current room description


	Commands  - sent TO Server
	'regi'		<name>				// <sign on>  
	'bye '							// <sign off>
	'loc '		&point				// <location change>
	'usrC'		&UserRec			// <change user selectable parameters (face stuff mostly)>
	'usrF'		short faceNbr		// <change user face>
	'usrN'		name[] 				// New user name
	'talk'		&msg  				// talk (if preceded by :) indicates thought
	'whis'		targeID, msg[]		// whisper
	'navR'		&roomID 			// navigation to new room
	'ping'							// Used to see if server is there
	'pong'							// response to ping
	'lock'		&roomID, &doorID	// Lock Door
	'unlo'		&roomID, &doorID	// Unlock Door
	'sSta'		&room,&spot,&state	// update state (and possibly picture) for hotspot
	'pLoc'		&room,&spot,&loc	// update picture offset
	'sLoc'		&room,&stpo,&loc	// update spot location
	'nRom'							// new Room request
	'nSpo'							// new spot request
	'sRom'		roomdesc			// modify current room description
****/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

short	gConnectionType;	// 0 = None, 1 = AppleTalk, 2 = DDE
long  gUserNetworkID;   // esr - Replace this with a union with other network info.
                        //       Can also fold in the Connection type.

void ProcessBlowThru(long cmd, long msgRefCon, char *buffer, long len);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PostServerEventShort(unsigned long eventType)
{
	PostServerEvent(eventType,gRoomWin->meID,NULL,0L);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SignOn(long sysStuff,short type)
{
	gRoomWin->swapPackets = false;
	gRoomWin->meID = 0;
	gRoomWin->navInProgress = true;  // 9/12/95
	ClearBackStack();

	if (ConnectUser(sysStuff,type))
	{
		gUserNetworkID = gRoomWin->meID;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SignOff(void)
{
  if (gConnectionType != C_None)
  {
	PostServerEvent(MSG_LOGOFF,gRoomWin->meID,NULL,0L);
    DisconnectUser();
    LogOff();
	AbortDownloads();
	ClearBackStack();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TerminateConnection(long reason, char *str);

void TerminateConnection(long reason, char *str)
{
	DisconnectUser();
	switch (reason) {
	case K_KilledByPlayer:
		StdStatusMessage(SM_Killed);
		ReportMessage(RE_KilledByPlayer);
		break;
	case K_KilledBySysop:
		StdStatusMessage(SM_Killed);
		ReportMessage(RE_KilledBySysop);
		break;
	case K_InvalidSerialNumber:
		StdStatusMessage(SM_Killed);
		ReportMessage(RE_KilledBySerialNumber);
		ExitToShell();
		break;
	case K_DuplicateUser:
		StdStatusMessage(SM_Terminated);
		ReportMessage(RE_KilledDuplicateUser);
		break;
	case K_DeathPenaltyActive:
		StdStatusMessage(SM_Terminated);
		ReportMessage(RE_DeathPenaltyActive);
		break;
	case K_Banished:
	case K_BanishKill:
		StdStatusMessage(SM_Terminated);
		ReportMessage(RE_Banished);
		break;
	case K_Unresponsive:
		StdStatusMessage(SM_Terminated);
		ReportMessage(RE_Inactivity);
		break;
	case K_Flood:
		StdStatusMessage(SM_Terminated);
		ReportMessage(RE_Flooding);
		break;
	case K_ServerFull:
		StdStatusMessage(SM_ServerFull);
		break;
	case K_NoGuests:
		StdStatusMessage(SM_MembersOnly);
		break;
	case K_CommError:
		StdStatusMessage(SM_ConnectionSevered);
		// 8/6/96 Added new set of "verbose" defines to U-USER.H
		// and new routine for "verbose" error messages.
		ReportVerboseMessage(VE_ConnectionSevered);
		break;
	// JAB 8/2/96 Added this...
	case K_ServerDown:
		StdStatusMessage(SM_ServerShutDown);
		ReportMessage(RE_ServerShutDown);
		break;

	// JAB 1/16/97 Demonstration has expired
	case K_DemoExpired:
		StdStatusMessage(SM_DemoExpired);
		ReportVerboseMessage(VE_MemberTimeOutOnline);
		break;
	case K_Verbose:
		// Event Data will contain a 'C' string containing an explanation...
		StdStatusMessage(SM_ConnectionSevered);
		if (str && *str) {
			LogMessage("%s\r",str);
			ErrorMessage("%s",str);
		}
		else
			ReportVerboseMessage(VE_ConnectionSevered);
		break;
	// JAB 8/2/96 Changed the default to connectionservered...
	default:
		StdStatusMessage(SM_ConnectionSevered);
		// 8/6/96 Added new set of "verbose" defines to U-USER.H
		// and new routine for "verbose" error messages.
		ReportVerboseMessage(VE_ConnectionSevered);
		break;
	}
	gConnectionType = C_None;
	gRoomWin->navInProgress = false;
}


// DEBUG
// void TestTCPCorruption(short flag);

void ProcessMansionEvent(long cmd, long msgRefCon, char *buffer, long len)
{
#if BOTCODE
	if (BotEntryPoint(gRoomWin, cmd, msgRefCon, len, buffer))
		return;
#endif
#if PPASUPPORT
	PPAMgrPropagateNetworkEvent(cmd, msgRefCon, len, buffer);
#endif
  switch (cmd)
  {
	case MSG_VERSION:
		  if (msgRefCon != MansionVersion)
          {
          	  SignOff();	// 7/14/95 - be nice about disconnecting
			  ErrorExit("Please upgrade your Palace software");
			  gQuitFlag = true;
		  }
		  gRoomWin->signOn = true;	// 7/20/95
		  break;
	case MSG_SERVERDOWN:
		// 1/16/97 JAB Added buffer argument to provide string...
		TerminateConnection(msgRefCon,buffer);
		break;
  	case MSG_ROOMDESC:
		// TestTCPCorruption('C');
	  	NewRoom((RoomRecPtr) buffer);
		// TestTCPCorruption('D');
		break;
  	case MSG_USERNEW:
	  	NewPerson((UserRecPtr) buffer);
		break;
  	case MSG_USEREXIT:
	  	FadeOutPerson(msgRefCon);
		break;
  	case MSG_USERLOG:
  		gRoomWin->totalPeople = (short)*((long *) buffer);
  		LogonPerson(msgRefCon);
	  	break;
  	case MSG_LOGOFF:
  		gRoomWin->totalPeople = (short)*((long *) buffer);
		LogoffPerson(msgRefCon);
  		break;
	case MSG_USERLIST:
		// TestTCPCorruption('E');
  		RoomPeople((short) msgRefCon, (UserRecPtr) buffer);
		// TestTCPCorruption('F');
	  	break;
  	case MSG_ROOMDESCEND:
		// TestTCPCorruption('G');
	  	EndRoomDescription();
		// TestTCPCorruption('H');
		break;
	case MSG_DIYIT:
		gRoomWin->swapPackets = true;
		SwapLong((unsigned long *) &msgRefCon);
		/* no break is intentional */
  	case MSG_TIYID:			// 5/31/95
	  	ThisIsYourRec(msgRefCon);
 		break;
  	case MSG_USERSTATUS:	// 5/31/95
	  	ThisIsYourStat(*((short *)buffer));
 		break;
	// 1/16/97 JAB New server message...
	case MSG_ALTLOGONREPLY:
		AltLogonReply((AuxRegistrationRec *) buffer);
		break;
  	case MSG_USERMOVE:
	  	UpdateUserPosition(msgRefCon, (Point *) buffer);
		break;
  	case MSG_USERCOLOR:
	  	UpdateUserColor(msgRefCon, *((short *) buffer));
		  break;
  	case MSG_USERFACE:
  		UpdateUserFace(msgRefCon, *((short *) buffer));
	  	break;
  	case MSG_USERPROP:
	  	UpdateUserProp(msgRefCon,(long *) buffer);      // esr - buffer was was cast to a short (why?)
		break;											// jab - it was a mistake- whoops
  	case MSG_USERDESC:
	  	UpdateUserDesc(msgRefCon,(short *) buffer);     // 6/8/95 - this one should be a ptr to short
		break;
  	case MSG_USERNAME:
	  	UpdateUserName(msgRefCon, (StringPtr) buffer);
		  break;
  	case MSG_TALK:
	  	UserText(msgRefCon, buffer, false);
		  break;
  	case MSG_WHISPER:
	  	UserText(msgRefCon, buffer, true);
		  break;
 	case MSG_XTALK:
		{
			short	len = *((short *) buffer);
			buffer += 2;
			DecryptCString((unsigned char *) buffer,(unsigned char *)buffer,len-3);
			UserText(msgRefCon, buffer, false);
		}
		break;
	case MSG_XWHISPER:
		{
			short	len = *((short *) buffer);
			buffer += 2;
			DecryptCString((unsigned char *)buffer,(unsigned char *)buffer,len-3);
			UserText(msgRefCon,buffer,true);
		}
		break;
 	case MSG_PING:
	  	PostServerEventShort(MSG_PONG);
		break;
  	case MSG_DOORLOCK:
	  	SetDoorLock((short *) buffer,true);
		break;
  	case MSG_DOORUNLOCK:
	  	SetDoorLock((short *) buffer,false);
		break;
  	case MSG_PICTMOVE:
	  	SetPictureOffset((short *) buffer);
		break;
  	case MSG_SPOTMOVE:
	  	SetSpotLoc((short *) buffer);
		break;
  	case MSG_SPOTSTATE:
	  	SetSpotState((short *) buffer);
		break;
  	case MSG_ROOMSETDESC:
	  	ModifyRoomInfo((RoomRecPtr) buffer, true);
		break;
  	case MSG_ASSETSEND:
	  	ReceiveAsset((Ptr) buffer);
		break;
  	case MSG_ASSETQUERY:
	  	ServerAssetQuery((long *) buffer);
		break;
  	// case MSG_ASSETNEW:			6/8/95 not really needed anymore
	//  	ServerAcknowledgeAsset((long *) buffer);
	//	break;
  	case MSG_PROPMOVE:
	  	MoveLooseProp((long *) buffer);
		break;
  	case MSG_PROPNEW:
	  	AddLooseProp((long *) buffer);
		break;
  	case MSG_PROPDEL:
	  	DelLooseProp((long *) buffer);
		break;
	case MSG_LISTOFALLUSERS:		// 6/9/95
		ProcessUserList(msgRefCon, buffer, len);
		break;
	case MSG_LISTOFALLROOMS:		// 6/10/95
		ProcessRoomList(msgRefCon, buffer, len);
		break;
	case MSG_DRAW:
		ProcessDrawCmd((DrawRecPtr) buffer);
		break;
	case MSG_SERVERINFO:
		// 1/27/97 JAB Changed calling conventions
		GetServerInfo((ServerInfoPtr) buffer, len >= 80);
		break;
  	case MSG_FILESEND:
	  	ReceiveFile((Ptr) buffer);
		break;
	case MSG_NAVERROR:
        // 9/12/95 - may want more specific error messages
        // based on refNum which has error code...
		StdStatusMessage(SM_DeniedByServer);
		gRoomWin->navInProgress = false;
		break;
	case MSG_BLOWTHRU:
    	ProcessBlowThru(cmd, msgRefCon, buffer, len);
    	break;


	default:
#ifdef WIN32
		LogMsg(0, WarningText,"Unknown Event: %.4s\r",&cmd);
#else
		LogMessage("Unknown Event: %.4s\r",&cmd);
#endif
		break;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProcessBlowThru(long cmd, long msgRefCon, char *buffer, long len)
{
	
#ifdef PPASUPPORT	
	// Send it to the plugin manager which will filter the call and send it
	// to appropriate plugins.
	PPAMgrPropagateBlowThruEvent(cmd, msgRefCon, len, buffer);
#endif // PPASUPPORT	

	return;

}