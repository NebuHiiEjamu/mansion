// Client Registration Dialog and Local Security functions
//
#include "U-USER.H"
#include "AppMenus.h"
#include "DialogUtils.h"
#include <Folders.h>
#include <Gestalt.h>
#include "U-SECURE.H"

Boolean ValidSerialString(StringPtr str);


#define RegistrationDLOG	504
#define RegInfoDLOG			507
#define RegComplete1DLOG	508
#define RegComplete2DLOG	509

#define SecureRecType		'ser#'
#if RAINBOWRED
#define SecureName			"\pPalace Rainbow Reg"
#else
#define SecureName			"\pPalace Registration"
#endif
#define UCreator			'mUsr'
#define UFType				'rsrc'

pascal Boolean MyDialogFilter(DialogPtr dp, EventRecord *ep, short *itemHit);

#define R_OK			1
#define R_Cancel		2
#define R_Name			3
#define R_Org			4
#define R_SerialNumber	5

Boolean ValidSerialString(StringPtr str)
{
	short	i;
	if (str[0] != 15)
		return false;
	for (i = 1; i <= 15; ++i) {
		switch (i) {
		case 5:
		case 10:
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

void RegistrationDialog(void)
{
	GrafPtr		savePort;
	DialogPtr 	dp;
	short		itemHit;
	Str255		tempName,tempOrg,tempSerial,temp;
	Boolean		badSerial = false,okSerial = false;
	Boolean		validFlag = false,newValidFlag;
	ModalFilterUPP	filterProc = NewModalFilterProc(MyDialogFilter);


	GetPort(&savePort);

	if ((dp = GetNewDialog (RegistrationDLOG, NULL, (WindowPtr) -1)) == NULL)
		return;
	
	SetText(dp, R_Name, "\p");
	SetText(dp, R_Org, "\p");
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
	if (itemHit == OK) {
		GetText(dp, R_Name, tempName);
		GetText(dp, R_Org, tempOrg);
		GetText(dp, R_SerialNumber, tempSerial);
		if ( tempName[0] && tempSerial[0] && ValidSerialString(tempSerial))
		{
			char	tbuf[16];
			unsigned char tCode[16];
			unsigned long crc,counter;
			BlockMove(&tempSerial[1],&tbuf[0],4);
			BlockMove(&tempSerial[6],&tbuf[4],4);
			BlockMove(&tempSerial[11],&tbuf[8],5);
			tbuf[13] = 0;
			ConvertStringToCode(tbuf,tCode,64);
			crc = *((unsigned long *) &tCode[0]);
			counter = *((unsigned long *) &tCode[4]);
			if (!MemberSerialNumber(crc,counter)) {
				badSerial = true;
				goto ErrorRet;
			}
			gSecure.crc = crc;
			gSecure.counter = counter;				
			gSecure.guestAccess = NewbieSerialNumber();
			if (tempName[0] > 63)
				tempName[0] = 63;
			if (tempOrg[0] > 63)
				tempOrg[0] = 63;
			BlockMove(tempName,gSecure.ownerName,tempName[0]+1);
			BlockMove(tempOrg,gSecure.ownerOrg,tempOrg[0]+1);
			okSerial = true;
		}
	}
ErrorRet:
	DisposDialog(dp);
	SetPort(savePort);
	if (badSerial)
		ReportMessage(RE_InvalidSerialNumber);
	else if (okSerial) {
		// 9/3/96 Register is kept on menu now...
		// DelMenuItem(gFileMenu, FM_Register);
		// DelMenuItem(gFileMenu, FM_Div3);
		SaveSecureInfo();
		RefreshSplash();

		SetPort((WindowPtr) gRoomWin);
		RoomWindowDraw((WindowPtr) gRoomWin);

		// 9/3/96 New Thank You Message...
		RegComplete1Dialog();
		// ReportMessage(RE_ThankYou);
	}
}

void GetSerialNumber(Ptr buffer)
{
	*((long *) buffer) = 0x5ebe3744;
	*((long *) (buffer+4)) = 0x21e8e72c;
}

void LoadSecureInfo()	// Called from main()
{
	Handle	h;
	short		volNum = 0,fRefNum = 0;
	long		dirID = 0L;
	FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,&volNum,&dirID);
	fRefNum = HOpenResFile(volNum,dirID,SecureName,fsRdPerm);
	if (fRefNum != -1) {
		h = Get1Resource(SecureRecType,128);
		if (h != NULL) {
			if (((SecureRec *) *h)->secureVersion == SecureVersion)
				gSecure = *((SecureRec *) *h);
			ReleaseResource(h);
		}
		CloseResFile(fRefNum);
	}
}

OSErr CreateSecureFile(void);
OSErr CreateSecureFile(void)
{
	OSErr	oe;
	short		volNum = 0,fRefNum = 0;
	long		dirID = 0L;

	FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,&volNum,&dirID);
	oe = HCreate(volNum, dirID, SecureName, UCreator, UFType);
	if (oe == dupFNErr) {
		HDelete(volNum, dirID, SecureName);
		oe = HCreate(volNum, dirID, SecureName, UCreator, UFType);
	}
	if (oe == noErr) {
		HCreateResFile(volNum, dirID, SecureName);
		oe = ResError();
	}
	return oe;
}

void SaveSecureInfo()	// Called from StorePreferences()
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

#define RI_OK		1
#define RI_Cancel	2
#define RI_ShowIt	3
#define RI_Box1		4

pascal void DrawBoxItem(DialogPtr dp, short itemNbr);
pascal void DrawBoxItem(DialogPtr dp, short itemNbr)
{
	short			t;
	Handle			h;
	Rect			r;

	GetDItem(dp, itemNbr, &t, &h, &r);
	FrameRect(&r);
}

void RegInfo(void)
{
	GrafPtr			savePort;
	DialogPtr 		dp;
	short			itemHit;
	short			t;
	Rect			r;
	Handle			h;
	UserItemUPP		drawBoxProc = NewUserItemProc(DrawBoxItem);

	GetPort(&savePort);

	if ((dp = GetNewDialog (RegInfoDLOG, NULL, (WindowPtr) -1)) == NULL)
		return;
	
	GetDItem(dp, RI_Box1, &t, &h, &r);
	SetDItem(dp, RI_Box1, t, (Handle) drawBoxProc, &r);

	SetControl(dp, RI_ShowIt, !(gPrefs.userPrefsFlags & UPF_DontShowRegInfo));

	SetDialogDefaultItem(dp, OK);
	SetDialogCancelItem(dp, Cancel);
	SetDialogTracksCursor(dp,true);

	ShowWindow(dp);
	SetPort(dp);
	SetCursor(&qd.arrow);
	ShowCursor();

	do {
		ModalDialog(NULL, &itemHit);
		switch (itemHit) {
		case RI_ShowIt:
			gPrefs.userPrefsFlags ^= UPF_DontShowRegInfo;
			SetControl(dp, RI_ShowIt, !(gPrefs.userPrefsFlags & UPF_DontShowRegInfo));
			StorePreferences();
			break;
		}
	} while (itemHit != OK && itemHit != Cancel);

	DisposDialog(dp);
	SetPort(savePort);

	if (itemHit == OK) {
		SetPort((WindowPtr) gRoomWin);
		RoomWindowDraw((WindowPtr) gRoomWin);
		// RefreshRoom(&gOffscreenRect);
		RegistrationDialog();
	}
}

#define RC_OK		1
#define RC_Cancel	2

void RegCompleteDialog(short dialogNbr)
{
	GrafPtr			savePort;
	DialogPtr 		dp;
	short			itemHit;

	GetPort(&savePort);

	if ((dp = GetNewDialog (dialogNbr, NULL, (WindowPtr) -1)) == NULL)
		return;
	
	SetDialogDefaultItem(dp, OK);
	SetDialogCancelItem(dp, Cancel);
	SetDialogTracksCursor(dp,true);

	ShowWindow(dp);
	SetPort(dp);
	SetCursor(&qd.arrow);
	ShowCursor();

	do {
		ModalDialog(NULL, &itemHit);
	} while (itemHit != OK && itemHit != Cancel);
	// Save stateof ShowIt

	DisposDialog(dp);
	SetPort(savePort);

	if (itemHit == OK) {
		Str255	urlStr;
		RefreshRoom(&gOffscreenRect);
		// Connect to Website
		GetIndString(urlStr,129,1);
		if (urlStr[0]) {
			PtoCstr(urlStr);
			GotoURL((char *) urlStr);
		}
	}
}

void RegComplete1Dialog(void)
{
	RegCompleteDialog(RegComplete1DLOG);
}

void RegComplete2Dialog(void)
{
	RegCompleteDialog(RegComplete2DLOG);
}
