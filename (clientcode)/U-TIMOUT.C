/* U-TIMOUT.C (shared routines) */

/* Todo:
	Add appropriate hooks.
	
	Create dialog boxes.

	Add default wizard password to preferences (store scrambled).
*/

#include "u-user.h"
#include "u-secure.h"
#include "u-timout.h"
#include "u-script.h"

AuxRegistrationRec	gAuxRegRec;
TimeOutLocalnfo		gMyTimeoutRec;


// This is routine is called from CheckSecurity() 
// when we've deteremine the user is unregistered
// It initializes the timeout stuff

void AuxSecurityInit()
{
	// Look for AUXRECFILE
	if (LoadAuxFile(&gAuxRegRec)) {
		// All is right with the world...
		// Some sanity checks...
		if (gAuxRegRec.userName[0] > 31)
			gAuxRegRec.userName[0] = 31;
		if (gAuxRegRec.wizPassword[0] > 31)
			gAuxRegRec.wizPassword[0] = 31;
		if (gAuxRegRec.demoElapsed < 0)
			gAuxRegRec.demoElapsed = 0;
		if (gAuxRegRec.totalElapsed < 0)
			gAuxRegRec.totalElapsed = 0;
		if (gAuxRegRec.demoLimit > (60L*60L*20L))
			gAuxRegRec.demoLimit = (60L*60L*20L);

		// 1/27/97 JAB Initialize Kevin's Fields...
		gAuxRegRec.ulRequestedProtocolVersion = 0;
		gAuxRegRec.ulUploadCaps = LI_ULCAPS_ASSETS_PALACE;
		gAuxRegRec.ulDownloadCaps = LI_DLCAPS_ASSETS_PALACE | LI_DLCAPS_FILES_PALACE;
		gAuxRegRec.ul2DEngineCaps = LI_2DENGINECAP_PALACE;
		gAuxRegRec.ul2DGraphicsCaps = LI_2DGRAPHCAP_GIF87;
		gAuxRegRec.ul3DEngineCaps = 0;
	}
	else {
		// Initialize AuxRegistrationRec
		gAuxRegRec.auxFlags = 0;
		gAuxRegRec.demoLimit = DemoTimeOut;
		gAuxRegRec.counter = gSecure.counter;
		gAuxRegRec.crc = gSecure.crc;
		BlockMove(gPrefs.name, gAuxRegRec.userName, gPrefs.name[0]+1);
		// !! Add wizard password,if any...
		// BlockMove(gPrefs.wizPassword, gAuxRegRec.wizPassword, gPrefs.wizPassword[0]+1);
		// Compute guest GUID
		MySRand((LONG) time(NULL));
		MySRand(LongRandom() + GetTicks());
		gAuxRegRec.puidCtr = LongRandom();
		gAuxRegRec.puidCtr ^= (time(NULL) + GetTicks());
		Randomize();
		gAuxRegRec.puidCRC = ComputePuidCRC(gAuxRegRec.puidCtr);
		gAuxRegRec.puidCtr ^= gAuxRegRec.puidCRC;
		gAuxRegRec.demoElapsed = 0;
		gAuxRegRec.totalElapsed = 0;
		gAuxRegRec.desiredRoom = 0;

		// 1/27/97 JAB Initialize Kevin's Fields...
		gAuxRegRec.ulRequestedProtocolVersion = 0;
		gAuxRegRec.ulUploadCaps = LI_ULCAPS_ASSETS_PALACE;
		gAuxRegRec.ulDownloadCaps = LI_DLCAPS_ASSETS_PALACE | LI_DLCAPS_FILES_PALACE;
		gAuxRegRec.ul2DEngineCaps = LI_2DENGINECAP_PALACE;
		gAuxRegRec.ul2DGraphicsCaps = LI_2DGRAPHCAP_GIF87;
		gAuxRegRec.ul3DEngineCaps = 0;
		
		// Create the hidden AuxFile
		// Note: this routine contains additional locally
		// implemented security which will increase the elapsed times
		// to large values if any hacking is detected...
		CreateAuxFile(&gAuxRegRec);	
	}
}

// Called from U-ROOMS.C ThisIsYourRec()
void AlternateLogon()
{
	if (gMyTimeoutRec.lastRoomIDRequest)
	{
		gAuxRegRec.desiredRoom = gMyTimeoutRec.lastRoomIDRequest;
		gMyTimeoutRec.lastRoomIDRequest = 0;
	}
	else
		gAuxRegRec.desiredRoom = 0;

	gAuxRegRec.counter = gSecure.counter;
	gAuxRegRec.crc = gSecure.crc;

#if KIDS_CLIENT
	strcpy(gAuxRegRec.reserved,"KIDS");
#endif

	BlockMove(gPrefs.name, gAuxRegRec.userName, gPrefs.name[0]+1);

	PostServerEvent(MSG_LOGON, gRoomWin->meID, (Ptr) &gAuxRegRec, sizeof(AuxRegistrationRec));
}

// Called from U-EVENTS.C - a message from the server which can grant (or take away) 
// additional demo time, and resolve puid collisions.
//
void AltLogonReply(AuxRegistrationRec *reply)
{
	gAuxRegRec.puidCtr = reply->puidCtr;
	gAuxRegRec.puidCRC = reply->puidCRC;
	gAuxRegRec.demoLimit = reply->demoLimit;
	gAuxRegRec.demoElapsed = reply->demoElapsed;
	gAuxRegRec.totalElapsed = reply->totalElapsed;
	
	WriteAuxFile(&gAuxRegRec, true);

}

// This routine is called (from ThisIsYourRec()) when we've 
// successfully signed on
void BeginSessionTimer()
{
	// initialize local timer
	gMyTimeoutRec.lastTimerCheck = time(NULL);
	DemoTimerDisplayIdle();

	// 1/27/97 JAB Force guest mode initially...
	if (!gSecure.isRegistered)
		gSecure.guestAccess = true;
}

// This routine is called (from DisconnectUser()) when we log off, for any reason
void EndSessionTimer()
{
	// Force writing of session Info
	UpdateTimer();
}

// This routine is called from the local idle loop when we're online 
void UsageIdle()
{
	time_t	elapsed;

	if (gConnectionType == C_None)
		return;

	// measure elapsed time since last increment....
	elapsed = time(NULL) - gMyTimeoutRec.lastTimerCheck;

	// If elapsed time is really strange, user is probably messing with the clock
	// add 2 minutes to elapsed time...
	if (elapsed < 0 || elapsed > 60*60*4)
	{
		gMyTimeoutRec.lastTimerCheck = time(NULL) - 120;
		UpdateTimer();
	}

	// The following bit of code can be NOP hacked to always return,
	// thus defeating the internal timer. 
	// However the server will also kill you if you use up more 
	// than X hours of time as an unregistered member on that server...
	//

	// Update the timer every 25 seconds...
	else if (elapsed < 25) 
		return;
	else 
		UpdateTimer();

	// If member demo has elapsed, sign off and show dialog box
	if (!gSecure.isRegistered && !gSecure.guestAccess && 
		gAuxRegRec.demoElapsed > gAuxRegRec.demoLimit) 
	{
		// Sign off
		SignOff();
		// Throw up dialog for elapsed member status
		ReportVerboseMessage(VE_MemberTimeOutOnline);
	}

#if USEGUESTTIMEOUT
	else if (!gSecure.isRegistered && gAuxRegRec.totalElapsed > GuestTimeOut) 
	{
		// Sign off
		SignOff();
		// Throw up dialog for elapsed guest status
		ReportVerboseMessage(VE_GuestTimeOutOnline);
	}
#endif
	// Check if we need to update the onscreen clock
	DemoTimerDisplayIdle();
}

// Internal routine to update timers
void UpdateTimer()
{
	time_t	elapsed,current;
	current = time(NULL);
	elapsed = current - gMyTimeoutRec.lastTimerCheck;
	if (!gSecure.guestAccess && !gSecure.isRegistered)
		gAuxRegRec.demoElapsed += elapsed;
	gAuxRegRec.totalElapsed += elapsed;
	gMyTimeoutRec.lastTimerCheck = current;
	// Write AUXRECFILE - local routine
	WriteAuxFile(&gAuxRegRec, false);
}

// This is called when guest selects "connect" option - if it returns false,
// don't go any further...
Boolean ApproveSignOn()
{
	if (gSecure.isRegistered)
		return true;

#if USEGUESTTIMEOUT
	// If guest elapsed, don't allow it.  Throw up dialog box. Return false.
	if (gAuxRegRec.totalElapsed > GuestTimeOut) {
		// Throw up dialog for elapsed guest status
		ReportVerboseMessage(VE_GuestTimeOutOffline);
		return false;
	}
	else 
#endif
	if (gAuxRegRec.demoElapsed > gAuxRegRec.demoLimit) {
		// Throw up irritating message, but Okay...
		ReportVerboseMessage(VE_MemberTimeOutOffline);
		return true;
	}
	else
		return true;
}

// Internal
unsigned long ComputePuidCRC(unsigned LONG counter)
{
	unsigned long crc = CRC_MAGIC;
	unsigned char *p;
	unsigned LONG ctr;
	long	*lp;
	int		i;
	extern unsigned long gCRCMask[];

	ctr = counter;
	p = (unsigned char *) &ctr;
	lp = (long *) p;

	if (LittleEndian()) {
		SwapLong((unsigned long *) &lp[0]);
	}
	for (i = 0; i < 4; ++i)
		crc = ((crc << 1L) | ((crc & 0x80000000L)? 1 : 0)) ^ gCRCMask[*(p++)];
	return crc;
}

// Used when palace URLs contain a room number...
void SetDesiredRoom(char *roomname)
{
	int	roomID;
	if (roomname && isdigit(roomname[0]))
		roomID = atoi(roomname);
	else
		roomID = 0;
	gMyTimeoutRec.lastRoomIDRequest = roomID;
}

void GrantMemberAccess()
{
	gSecure.guestAccess = false;
	// Confirm it's OKAY...
	if (!gSecure.isRegistered && gAuxRegRec.demoElapsed > DemoTimeOut) {
		SignOff();
		// Throw up dialog for elapsed member status
		ReportVerboseMessage(VE_MemberTimeOutOnline);
	}
	DemoTimerDisplayIdle();
	if (!(gRoomWin->serverInfo.serverPermissions & PM_AllowCyborgs) && !(gRoomWin->userFlags & U_SuperUser))
		ClearUserScript();
	else
		LoadUserScript();
}
