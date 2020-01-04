// U-SECURE.C

#include "U-USER.H"
#include "u-script.h"
#include "U-SECURE.H"
#include "U-TIMOUT.H"	// 1/14/97 JAB
#include <time.h>


#define oldsecuresupported	0


// Note: missing are the letters  IO and the numbers 1 and 0
char	codeAsc[] = {"ABCDEFGHJKLMNPQRSTUVWXYZ23456789"};

// LoadPreferences routine loads in this structure... (See UserRegistration.c)
// SavePreferences routine saves this sturcture...(See UserRegistration.c)


// note: the strings are actually null p-strings

// 1/14/97 JAB added additional "isRegistered" field
SecureRec	gSecure = {SecureVersion, DefaultCRC,DefaultCounter,0,1,"",""};

void GetServerInfo(ServerInfoPtr buf, Boolean newStyle)
{
	BlockMove((Ptr)buf,(Ptr)&gRoomWin->serverInfo, sizeof(long) + buf->serverName[0]+1);
#if macintosh
	// 8/8/95
	SetWTitle((WindowPtr) gRoomWin,gRoomWin->serverInfo.serverName);
#endif

	// 1/27/96 JAB Added support for Kevin's fields...
	if (newStyle) {
		gRoomWin->serverInfo.serverOptions = buf->serverOptions;
		gRoomWin->serverInfo.ulUploadCaps = buf->ulUploadCaps;
		gRoomWin->serverInfo.ulDownloadCaps = buf->ulDownloadCaps;
	}

	ShowRoomStatus();

	// 1/14/97 JAB Don't allow unregistered users on member's only servers
	//
	if ((gSecure.guestAccess || !gSecure.isRegistered) && DeniedByServer(PM_AllowGuests))
    	  SignOff();	// 7/14/95 - be nice about disconnecting
	if (gExpert && !(gRoomWin->serverInfo.serverPermissions & PM_AllowWizards))
		ToggleWizard();
	if (!(gRoomWin->serverInfo.serverPermissions & PM_AllowCyborgs) && !(gRoomWin->userFlags & U_SuperUser))
		ClearUserScript();
	else
		LoadUserScript();
}

Boolean DeniedByServer(long flag)
{
	if (!(gRoomWin->serverInfo.serverPermissions & flag)) {
		StdStatusMessage(SM_DeniedByServer);
		SysBeep(1);
		return true;
	}
	else
		return false;
}

Boolean MembersOnly(Boolean flag)		// 6/26
{
	if (gSecure.guestAccess && flag) {
		StdStatusMessage(SM_MembersOnly);
		// SysBeep(1);
		return true;
	}
	else
		return false;
}

// Convert a 5-bit code to an ascii character
int ConvertCodeToAscii(short code)
{
	if (code >= 0 && code < 32)
		return codeAsc[code];
	else
		return -1;
}

// Convert ascii character to 5-bit code.
int ConvertAsciiToCode(short asc)
{
	short	i;
	for (i = 0; codeAsc[i]; ++i) {
		if (codeAsc[i] == asc || (isalpha(asc) && toupper(asc) == codeAsc[i]))
			return i;
	}
	return -1;		
}

// input (8 byte (64-bit) number
// output: absd-laks-jakss  (no hyphens)

void ConvertCodeToString(unsigned char *s, char *sa, short nbrBits)
{
	short	sn = 0,oCnt=0,mask = 0x0080;
	long	*lp = (long *) s;
	Boolean	LittleEndian();	/* added to M-UTILS.C */
	if (LittleEndian()) {
		SwapLong((unsigned long *) &lp[0]);
		SwapLong((unsigned long *) &lp[1]);
	}

	while (nbrBits--) {
		sn <<= 1;
		if (*s & mask)
			sn |= 0x01;
		if (++oCnt == 5) {
			*(sa++) = ConvertCodeToAscii(sn);
			sn = oCnt = 0;
		}
		mask >>= 1;
		if (mask == 0) {
			mask = 0x80;
			++s;
		}
	}
	if (oCnt) {
		do {
			sn <<= 1;
		} while (++oCnt < 5);
		*(sa++) = ConvertCodeToAscii(sn);
	}
	*sa = 0;
	// Swap Back - Eddie 8/25/95
	if (LittleEndian()) {
		SwapLong((unsigned long *) &lp[0]);
		SwapLong((unsigned long *) &lp[1]);
	}
}

// input: absd-laks-jakss  (13 bytes, no hyphens)
// output: (8 byte (64-bit) number

void ConvertStringToCode(char *sa, unsigned char *s, short nbrBits)
{
	short	sn = 0,oCnt=0,mask=0x0080;
	long	*lp = (long *) s;
	*s = 0;
	while (nbrBits--) {
		if (oCnt == 0) {
			sn = ConvertAsciiToCode(*(sa++));
			oCnt = 5;
		}
		if (sn & 0x10) {
			*s |= mask;
		}
		sn <<= 1;
		--oCnt;
		mask >>= 1;
		if (mask == 0) {
			mask = 0x80;
			++s;
			*s = 0;
		}
	}
	if (LittleEndian()) {
		SwapLong((unsigned long *) &lp[0]);
		SwapLong((unsigned long *) &lp[1]);
	}
}


#include "M-MAGIC.H"


unsigned long ComputeLicenseCRC(Ptr pt, long len)
{
	unsigned long crc = CRC_MAGIC;
	unsigned char *p = (unsigned char *) pt;
	long	*lp = (long *) pt;
	if (LittleEndian()) {
		SwapLong((unsigned long *) &lp[0]);
	}
	while (len--)
		crc = ((crc << 1L) | ((crc & 0x80000000L)? 1 : 0)) ^ gCRCMask[*(p++)];
	return ~(crc ^ ENCODE_OBFUSCATE);
}

Boolean ValidSerialNumber()
{
	unsigned long crc,counter;
	crc = gSecure.crc;
	counter = gSecure.counter;
	counter ^= (MAGIC_LONG ^ crc);
	// 8/7/96 - additional obfuscation to reduce chances
	// of valid counter being used...
	crc ^= OBFUSCATE_LONG;
	return (crc ^ ComputeLicenseCRC((Ptr) &counter, 4)) == DECODE_OBFUSCATE;
}


Boolean NewbieSerialNumber()
{
	unsigned long crc,counter;
	crc = gSecure.crc;
	counter = gSecure.counter;
	counter ^= (MAGIC_LONG ^ crc);
	// 8/7/96 Added additional obfuscation
	crc ^= OBFUSCATE_LONG;
	if ((crc ^ ComputeLicenseCRC((Ptr) &counter, 4)) != DECODE_OBFUSCATE) {
#if oldsecuresupported
		return OldNewbieSerialNumber();
#else
		return true;
#endif
	}
	if (counter < 500)	// Counter^magic_long on newbie serial# is 0
		return true;
	return false;
}

Boolean MemberSerialNumber(unsigned LONG crc, unsigned LONG counter)
{
	counter ^= (MAGIC_LONG ^ crc);
	if (counter < 500)
		return false;
	// 8/7/96 Added additonal obfuscation
	crc ^= OBFUSCATE_LONG;
	return (crc ^ ComputeLicenseCRC((Ptr) &counter, 4)) == DECODE_OBFUSCATE;
}

Boolean CheckSecurity()	// Called once at beginning, after preferences are loaded
{
	if (!ValidSerialNumber()) {

#if oldsecuresupported
		if (ValidOldSerialNumber()) {
			time_t	now;
			struct tm *date;
			now = time(NULL);
			date = localtime(&now);

			if (date->tm_year > 95 || date->tm_mon > 11) {
				return true;
			}
	
			if (OldNewbieSerialNumber()) {
				ErrorMessage("This version will soon expire.  Check our website at http://www.thePalace.com for more info.");
				gSecure.guestAccess = true;
			}
			else {
				ErrorMessage("Your beta registration number will soon expire.  Check our website at http://www.thePalace.com for more info.");
			}
			NewbieMenus();
			return false;
		}
#endif
		return true;
	}
	else {
		// 1/14/97 JAB - modified to set gSecure.isRegistered correctly
		if (NewbieSerialNumber()) {
			gSecure.guestAccess = true;
			gSecure.isRegistered = false;	// 1/14/97 JAB
			NewbieMenus();
		}
		else {
			gSecure.guestAccess = false;
			gSecure.isRegistered = true;	// 1/14/97 JAB
		}
		AuxSecurityInit();		// 1/14/97 JAB Determine free demo status
								//  and assign guest "puid".
	}
/**
	leave this out...
	else {
		unsigned LONG crc,counter;
		crc = gSecure.crc;
		counter = gSecure.counter;
		counter ^= (MAGIC_LONG ^ crc);
		if (counter < 0 || counter > 200000 ||
			(counter > 1500 && counter < 100000)) {
			gSecure.counter = TickCount();
			gSecure.crc = time(NULL);
			// will fail upon sign in
		}
	}
**/
	return false;
}

void GetSerialNumberString(char *str)
{
	if (NewbieSerialNumber()) {
										// 1/14/97 JAB
		gSecure.isRegistered = false;	// Just to be obnoxious
#if RAINBOWRED
		strcpy(str, "UNREGISTERED VERSION");
#else
		str[0] = 0;
#endif
	}
	else {	// 5/15/96 Club mode

		str[0] = 0;	// 7/24/96 Stop providing serial number
	}
}

void GetOwnerString(char *str)
{
	if (NewbieSerialNumber()) {
										// 1/14/97 JAB
		gSecure.isRegistered = false;	// Just to be obnoxious
		str[0] = 0;
	}
	else {
		if (gSecure.ownerOrg[0]) {
			sprintf(str,"%.*s - %.*s",
				gSecure.ownerName[0],
				&gSecure.ownerName[1],
				gSecure.ownerOrg[0],
				&gSecure.ownerOrg[1]);
		}
		else {
			sprintf(str,"%.*s",
				gSecure.ownerName[0],
				&gSecure.ownerName[1]);
		}
	}
}

// THIS FUNCTION IS CALLED WHEN THE SERVER SENDS MSG_USERSTATUS
//
void ThisIsYourStat(short userFlags)
{
	gRoomWin->userFlags = userFlags;
	gRoomWin->expert = (userFlags & U_SuperUser) > 0;
	ToggleExpertMenu(gRoomWin->expert);

	// 1/14/97 - allow server to grant member access...
	//
	if ((userFlags & U_Guest) != 0) {
		gSecure.guestAccess = true;
	}
	else {
		GrantMemberAccess();
	}

/**
	1/14/97 This check is no longer valid...
	if ((userFlags & U_Guest) && !gSecure.guestAccess) {
		// Software has been hacked...
		gSecure.guestAccess = true;
		gQuitFlag = true;
		SignOff();
		ExitToShell();
	}
 **/
}

#if oldsecuresupported

Boolean ValidOldSerialNumber()
{
	unsigned long crc,counter;
	crc = gSecure.crc;
	counter = gSecure.counter;
	counter ^= OLD_MAGIC_LONG;
	return crc == ComputeOldLicenseCRC((Ptr) &counter, 4);
}


Boolean OldNewbieSerialNumber()
{
	unsigned long crc,counter;
	crc = gSecure.crc;
	counter = gSecure.counter;
	counter ^= OLD_MAGIC_LONG;
	if (crc != ComputeOldLicenseCRC((Ptr) &counter, 4))
		return true;
	if (counter == 0)	// Counter^magic_long on newbie serial# is 0
		return true;
	return false;
}

unsigned long ComputeOldLicenseCRC(Ptr pt, long len)
{
	unsigned long crc = OLD_CRC_MAGIC;
	unsigned char *p = (unsigned char *) pt;
	long	*lp = (long *) pt;
	if (LittleEndian()) {
		SwapLong((unsigned long *) &lp[0]);
	}
	while (len--)
		crc = ((crc << 1L) | ((crc & 0x80000000L)? 1 : 0)) ^ gCRCMask[*(p++)];
	return crc;
}


#endif


