#include "U-USER.H"
#include "U-TIMOUT.H"	// 1/14/97 JAB
#include "UserTools.h"	// 6/10/95
#include "PPAMgr.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DisconnectUser(void)
{
	// 1/14/97 JAB stop timing the session
	EndSessionTimer();

 	gRoomWin->meID = 0;  // - note: need to reset meID when connecting to new user
 	                     // see how TIYID msg is handled...
	gRoomWin->navInProgress = false;  // 9/12/95

#if PPASUPPORT
	PPAMgrKillAllPPAs(PPAKILL_SERVER);
#endif

	KillAuthoring();

	AbortDownloads();

 	ClearRoomWindow();

	ClearBackStack();

    switch (gConnectionType)
    {
  	case C_AppleTalk:
  		break;
  	case C_PalaceTCP:
	case C_PalaceTelnet:
    	  PalTCPAbort();
		  ClosePalTCP();
      break;
	}
	SetConnectionType(C_None, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Boolean ConnectUser(long sysStuff,short type)
{
// This does nothing on the Mac. On my version
// this handle the different possible connection types.
	gRoomWin->navInProgress = true;  // 9/12/95

  	switch (type)
  	{
  		case C_AppleTalk:
  			break;
  		case C_PalaceTCP:
		case C_PalaceTelnet:
			break;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// DEBUG
// void TestTCPCorruption(short flag);

void PostServerEvent(unsigned long eventType, unsigned long refNum,
					Ptr bufferContents, long bufferLength)
{
	// TestTCPCorruption('N');

	if (gDebugFlag) {
		LogMessage("<-- %.4s\r",&eventType);
	}
	// TestTCPCorruption('O');

	switch (gConnectionType) {
	case C_AppleTalk:
		PostUserAppleTalkEvent(eventType,refNum,bufferContents, bufferLength);
		break;
/*	case C_IRCTCP:*/
/*	case C_IRCSerial:*/
/*		PostUserIRCEvent(eventType,refNum,bufferContents, bufferLength);*/
/*		break;*/
	case C_PalaceTCP:
	case C_PalaceTelnet:
		PostUserPalTCPEvent(eventType,refNum,bufferContents, bufferLength);
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetConnectionType(short mode, Boolean verbose)	// 7/7/95 Used to inactivate relevent tools & set status bar
{
	extern WindowPtr	gRLWin, gULWin;


	if (mode == gConnectionType)	// 8/8/95
		return;
	gConnectionType = mode;
	if (gConnectionType == C_None) {
		if (gRLWin)
			((ObjectWindowPtr) gRLWin)->Dispose(gRLWin);
		if (gULWin)
			((ObjectWindowPtr) gULWin)->Dispose(gULWin);
		if (gRoomWin) {
			SetPort((WindowPtr) gRoomWin);
			ActivateTool(WT_Face, false, false);
			ActivateTool(WT_PPalette, false, false);
			if (verbose)
				StdStatusMessage(SM_NoConnection);
			SetWTitle((WindowPtr) gRoomWin,"\pThe Palace");
		}
	}
	else {
		if (gRoomWin) {
			SetPort((WindowPtr) gRoomWin);
			ActivateTool(WT_Face, true, false);
			ActivateTool(WT_PPalette, true, false);
		}
		if (verbose) {
			switch (mode) {
			case C_AppleTalk:
				StdStatusMessage(SM_ConnectAppletalk);
				break;
			case C_PalaceTCP:
				StdStatusMessage(SM_ConnectTCP);
				break;
			}
		}
	}
}

void DumpBufferToLog(char *label, unsigned char *ptr, LONG len, LONG offsetOffset);
void DumpBufferToLog(char *label, unsigned char *ptr, LONG len, LONG offsetOffset)
{
	int		nbrLines,y,x,n;
	char	hexDump[16*3+1],asciiDump[16+1],*hp,*ap;

	nbrLines = len/16 + 1;
	LogMessage("%s:\r",label);
	n = 0;
	for (y = 0; y < nbrLines; ++y) {
		hp = hexDump;
		ap = asciiDump;
		for (x = 0; x < 16; ++x) {
			sprintf(hp,"%02X ",*ptr);
			hp += 3;
			sprintf(ap,"%c",*ptr > ' ' && *ptr < '~'? *ptr : '.');
			ap += 1;
			++ptr;
			++n;
			if (n >= len)
				break;
		}
		*hp = 0;
		*ap = 0;
		LogMessage("%6x: %-48s  %-16s\r",(int) (y*16-offsetOffset),hexDump,asciiDump);
	}		
}
