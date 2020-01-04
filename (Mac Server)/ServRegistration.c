// ServRegistration

#include "s-server.h"
#include "s-secure.h"
#include "s-magicss.h"
#include "s-funcs.h"
#include "DialogUtils.h"
#include "ServRegistration.h"

#define SecureName			"\pPalace Server Registration"
#define USecureName			"\pPalace Registration"
#define SecureRecType		'ser#'

#define SCreator			'mSrv'
#define SFType				'rsrc'

#define RegistrationDLOG	507

#define R_OK			1
#define R_Cancel		2
#define R_Name			3
#define R_Org			4
#define R_SerialNumber	5

typedef struct {
	unsigned long secureVersion;
	unsigned long crc;
	unsigned long counter;
	unsigned long flags;
	Str63	ownerName;
	Str63	ownerOrg;
} SecureRec;

typedef struct {					// Taken from U-SECURE.H
	unsigned long secureVersion;
	unsigned long crc;
	unsigned long counter;
	short	guestAccess;
	Str63	ownerName;
	Str63	ownerOrg;
} USecureRec;

#define RE_InvalidSerialNumber	1

//SecureRec	gSecure = {SecureVersion, DefaultCRC, DefaultCounter, 1, "", ""};
SecureRec	gSecure = {SecureVersion, 0, 0, 0, "", ""};
USecureRec	gUSecure = {SecureVersion, 0, 0, 0, "", ""};

//char	codeAsc[] = {"ABCDEFGHJKLMNPQRSTUVWXYZ23456789"};

		void	ReportMessage(short strCode);
		
		void 	GetServerSerialNumber(char *str);
		void 	ConvertCodeToString(unsigned char *s, char *sa, short nbrBits);
		int 	ConvertCodeToAscii(short code);
pascal 	Boolean MyDialogFilter(DialogPtr dp, EventRecord *ep, short *itemHit);
		Boolean ValidSerialString(StringPtr str);
		void	ReportError(short code, char *name);
		void	SaveSecureInfo();
		OSErr	CreateSecureFile(void);

#ifdef OLDCODE
// input (8 byte (64-bit) number
// output: absd-laks-jakss  (no hyphens)

// Convert 5 bits to an alpha code - won't be needed anymore once
// s-seriel.c is added.
//
int ConvertCodeToAscii(short code)
{
	if (code >= 0 && code < 32)
		return codeAsc[code];
	else
		return -1;
}

// A routine for converting client serial numbers to ascii
// won't be needed once s-serial.c
//
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
#endif

// This is currently producing the ascii version (ABCDE-ABCDE-ABCDE-ABCDE) of the
// Server serial number.
//
// Use SSBinaryToAscii and then use sprintf to add hyphens (%.5s-%.5s-%.5s-%.5s)
//

void GetServerSerialNumber(char *str)
{
	char	temp[32];
	//ConvertCodeToString((unsigned char *) &gSecure.crc, temp, 64);
	
	SSBinaryToAscii((unsigned char *) &gSecure.crc, temp);
	sprintf(str,"%.5s-%.5s-%.5s-%.5s", &temp[0], &temp[5], &temp[10], &temp[15]);
}

// This loads in the server's serial number in "palace server registration"
//

void LoadSecureInfo()
{
	Handle	h;
	short		volNum = 0,fRefNum = 0;
	long		dirID = 0L;
	
	FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, &volNum, &dirID);
	
	fRefNum = HOpenResFile(volNum, dirID, SecureName, fsRdPerm);
	
	if (fRefNum != -1)
	{
		h = Get1Resource(SecureRecType,128);
		if (h != NULL)
		{
			if (((SecureRec *) *h)->secureVersion == SecureVersion)
				gSecure = *((SecureRec *) *h);
			ReleaseResource(h);
		}
		CloseResFile(fRefNum);
	}
	else
	{
		/* See if we can find a client registration */
		FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, &volNum, &dirID);
		
		fRefNum = HOpenResFile(volNum, dirID, USecureName, fsRdPerm);
		
		if (fRefNum != -1)
		{
			h = Get1Resource(SecureRecType,128);
			if (h != NULL)
			{
				if (((USecureRec *) *h)->secureVersion == SecureVersion)
					gUSecure = *((USecureRec *) *h);
				ReleaseResource(h);
			}
			CloseResFile(fRefNum);
		}
	}
}

// Checks the contents of gSecure (which were loaded in by the above routine)
// to make sure the server's serial number is valid.

Boolean CheckServerSecurity()
{
	Boolean			badRegistration = true;
	int				platform, capacity, flags;
	unsigned long	seed;
	
	do	/* Abort Loop */
	{
		
		if (SSIsValidSerialNumber((unsigned char*)&gSecure.crc))
		{
			SSParseSerialNumber((unsigned char*)&gSecure.crc, &platform, &capacity, &flags, &seed);
			
			if (capacity < 3)
				capacity = 3;
				
			// Make sure they don't exceed the license limit
			if (gPrefs.maxOccupancy > capacity)
				gPrefs.maxOccupancy = capacity;
			
			gMaxPeoplePerServer = capacity;
			
			badRegistration = false;
			break;
		}
		
		/* See if we have a valid client registration */
		if (ValidUserSerialNumber(gUSecure.crc, gUSecure.counter))
		{
			BlockMove(gUSecure.ownerName, gSecure.ownerName, gUSecure.ownerName[0] + 1);
			BlockMove(gUSecure.ownerOrg, gSecure.ownerOrg, gUSecure.ownerOrg[0] + 1);
			
			gMaxPeoplePerServer = (gPrefs.maxOccupancy = 3);
			badRegistration = false;
		}
		
	} while (FALSE);
	
	if (!badRegistration)
	{
		BlockMove(gSecure.ownerName, gPrefs.sysop, gSecure.ownerName[0]+1);
		PtoCstr((StringPtr) gPrefs.sysop);
	}
	
	return (badRegistration);
		
#ifdef OLDCODE
	if (!ValidUserSerialNumber(gSecure.crc, gSecure.counter)) {
		return true;
	}
	if (NewbieUserSerialNumber(gSecure.crc, gSecure.counter))
		return true;
	BlockMove(gSecure.ownerName, gPrefs.sysop, gSecure.ownerName[0]+1);
	PtoCstr((StringPtr) gPrefs.sysop);
	return false;
#endif
}

pascal Boolean MyDialogFilter(DialogPtr dp, EventRecord *ep, short *itemHit)
{
	// char tempChar;
	short	part;
	WindowPtr	whichWin;
	switch (ep->what) {
	case mouseDown:
		part = FindWindow(ep->where,&whichWin);
		if (whichWin == dp) {
			switch (part) {
			case inDrag:		// Transvestite code here
				{
					Rect	dragRect;
					dragRect = qd.screenBits.bounds;
					DragWindow(whichWin, ep->where, &dragRect);
					// Update background windows
					//UpdateMyWindows();
				}
				return true;
			}
		}
		break;
		
#ifdef NOTYET
	case kHighLevelEvent:
		if (ep->message == ServerEventID) {
			//ProcessAppleTalkEvent(ep);
		}
		break;
	case nullEvent:
		if (gTCPRecord)
			//PalTCPIdle(ep)
				;
#endif

	default:
		break;

	}
	return CallFilterProc(dp, ep, itemHit);
}

Boolean ValidSerialString(StringPtr str)
{
	short	i;
	if (str[0] != 23)
		return false;
	for (i = 1; i <= 23; ++i) {
		switch (i) {
		case 6:
		case 12:
		case 18:
			if (str[i] != '-')
				return false;
			break;
		default:
			if (!isalnum(str[i]))
				return false;
			break;
		}
	}
	return true;
}

OSErr CreateSecureFile(void)
{
	OSErr	oe;
	short		volNum = 0,fRefNum = 0;
	long		dirID = 0L;

	FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,&volNum,&dirID);
	oe = HCreate(volNum, dirID, SecureName, SCreator, SFType);
	if (oe == dupFNErr) {
		HDelete(volNum, dirID, SecureName);
		oe = HCreate(volNum, dirID, SecureName, SCreator, SFType);
	}
	if (oe == noErr) {
		HCreateResFile(volNum, dirID, SecureName);
		oe = ResError();
	}
	return oe;
}

void SaveSecureInfo()
{
	Handle	h,h2;
	short		volNum = 0,fRefNum = 0;
	long		dirID = 0L;
	FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,&volNum,&dirID);

	fRefNum = HOpenResFile(volNum,dirID,SecureName,fsRdWrPerm);
	if (fRefNum == -1) {
		if (CreateSecureFile() != noErr)
			return;
		fRefNum = HOpenResFile(volNum,dirID,SecureName,fsRdWrPerm);
	}
	if (fRefNum == -1)
		return;

	/* 7/1/96 JAB Changed to NewHandleClear */
	h = NewHandleClear(sizeof(SecureRec));
	if (h) {
		SetResLoad(false);
		while ((h2 = Get1Resource(SecureRecType, 128)) != NULL) {
			RmveResource(h2);
			DisposHandle(h2);
		}
		SetResLoad(true);
		*((SecureRec *) *h) = gSecure;
		AddResource(h,SecureRecType,128,"\p");
		WriteResource(h);
		ReleaseResource(h);
	}
	CloseResFile(fRefNum);
}

Boolean RegistrationDialog(void)
{
	GrafPtr			savePort;
	DialogPtr 		dp;
	short			itemHit;
	Str255			tempName, tempOrg, tempSerial, temp;
	Boolean			goodSerial = false, cancelled = false;
	Boolean			validFlag = false, newValidFlag;
	ModalFilterUPP	filterProc = NewModalFilterProc(MyDialogFilter);


	GetPort(&savePort);

	if ((dp = GetNewDialog (RegistrationDLOG, NULL, (WindowPtr) -1)) == NULL)
		goto ErrorRet;
	
	SetText(dp, R_Name, gSecure.ownerName);
	SetText(dp, R_Org, gSecure.ownerOrg);
	SetText(dp, R_SerialNumber, "\p");
	SelIText(dp, R_Name, 0, 32767);

	SetDialogDefaultItem(dp, Cancel);
	SetDialogCancelItem(dp, Cancel);
	SetDialogTracksCursor(dp,true);

	ShowWindow(dp);
	SetPort(dp);
	SetCursor(&qd.arrow);
	ShowCursor();

	do {
		ModalDialog(filterProc, &itemHit);
		switch (itemHit) {
		case R_Name:
		case R_Org:
		case R_SerialNumber:
			GetText(dp, itemHit, temp);
			if (temp[0] > 63) {
				temp[0] = 63;
				SetText(dp, itemHit, temp);
				SysBeep(1);
			}
			GetText(dp, R_Name, tempName);
			GetText(dp, R_SerialNumber, tempSerial);
			newValidFlag = tempName[0] && tempSerial[0] && ValidSerialString(tempSerial);
			if (newValidFlag != validFlag) {
				validFlag = newValidFlag;
				if (validFlag) {
					OutlineButton(dp,Cancel,&qd.white);
					OutlineButton(dp,OK,&qd.black);
					SetDialogDefaultItem(dp, OK);
				}
				else {
					OutlineButton(dp,Cancel,&qd.black);
					OutlineButton(dp,OK,&qd.white);
					SetDialogDefaultItem(dp, Cancel);
				}
			}
			break;
		}
	} while (itemHit != OK && itemHit != Cancel);
	
	switch (itemHit)
	{
		case OK:
		{
			GetText(dp, R_Name, tempName);
			GetText(dp, R_Org, tempOrg);
			GetText(dp, R_SerialNumber, tempSerial);
			if ( tempName[0] && tempSerial[0] && ValidSerialString(tempSerial))
			{
				char	tbuf[32];
				unsigned char tCode[32];
				unsigned long crc,counter,flags;
				
				BlockMove(&tempSerial[1],&tbuf[0],5);
				BlockMove(&tempSerial[7],&tbuf[5],5);
				BlockMove(&tempSerial[13],&tbuf[10],5);
				BlockMove(&tempSerial[19],&tbuf[15],5);
				
				tbuf[20] = 0;
				
				SSAsciiToBinary(tbuf, tCode);
				//ConvertStringToCode(tbuf,tCode,64);
				
				crc = *((unsigned long *) &tCode[0]);
				counter = *((unsigned long *) &tCode[4]);
				flags = *((unsigned long *) &tCode[8]);
				
				if (!SSIsValidSerialNumber(tCode)) 
				{
					goto ErrorRet;
				}
				gSecure.crc = crc;
				gSecure.counter = counter;
				gSecure.flags = flags;
						
				//gSecure.guestAccess = NewbieSerialNumber();
				if (tempName[0] > 63)
					tempName[0] = 63;
				if (tempOrg[0] > 63)
					tempOrg[0] = 63;
				BlockMove(tempName,gSecure.ownerName,tempName[0]+1);
				BlockMove(tempOrg,gSecure.ownerOrg,tempOrg[0]+1);
				goodSerial = true;
			}
			
			break;
		}
		
		case Cancel:
		{
			cancelled = TRUE;
			break;
		}
	}
	
ErrorRet:
	DisposDialog(dp);
	SetPort(savePort);
	
	if (goodSerial) 
	{
		// 9/3/96 Register is kept on menu now...
		// DelMenuItem(gFileMenu, FM_Register);
		// DelMenuItem(gFileMenu, FM_Div3);
		SaveSecureInfo();
		//RefreshSplash();

		//SetPort((WindowPtr) gRoomWin);
		//RoomWindowDraw((WindowPtr) gRoomWin);

		// 9/3/96 New Thank You Message...
		//RegComplete1Dialog();
		// ReportMessage(RE_ThankYou);
	}
	else if (!cancelled)
		ReportMessage(RE_InvalidSerialNumber);
		
	return (goodSerial);
}

