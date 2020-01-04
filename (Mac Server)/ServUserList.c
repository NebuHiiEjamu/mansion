// UserListWindow.c
/***
	Controls for User List:
		Target for Private Message

***/
#include "S-SERVER.H"
#include "S-SECURE.H"
#include "UserTools.h"	// 6/10/95
#include "ServGraphics.h"
#include "AppMenus.h"

#define		ULWIND			129
#define		WinBackColor	gGrayAA

typedef struct {
	ObjectWindowRecord	win;
	ListHandle		uList;
	TEHandle		teH;
	Boolean			msgActive;
	Rect			listRect;
	Rect			msgRect,msgFrame;
} ULWindowRecord, *ULWindowPtr;

WindowPtr	gULWin;
ULWindowPtr	gULPtr;

void NewULWindow();
void ULWindowDraw(WindowPtr theWindow);
void ULWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent);
void DisposeULWin(WindowPtr theWin);
ServerUserPtr GetUserListRec(short n);
void UserDisplayIdle(WindowPtr theWin, EventRecord *theEvent);
void ULAdjustMenus(WindowPtr theWin);
void ULProcessCommand(WindowPtr theWin, short theMenu, short theItem);
void PrepareULTextColors();
void RestoreULColors();
void DeactivateULName();
void ULWindowKey(WindowPtr theWin, EventRecord *theEvent);
void ULResize(WindowPtr theWin, short w, short h);
Boolean CanPaste(long type);

enum {SU_Kill, SU_Ban, SU_NbrTools};

void ToggleUserList()
{
	if (gULWin)
		DisposeULWin(gULWin);
	else
		NewULWindow();
}

// Create a new main window using a 'WIND' template from the resource fork
//
void NewULWindow()
{
	WindowPtr		theWindow;
	ULWindowPtr		ulRec;
	Rect			rDataBounds;
	Point			cellSize = {0,0};
	Point			toolP;

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 
	ulRec = (ULWindowPtr) NewPtrClear(sizeof(ULWindowRecord));
	theWindow = InitObjectWindow(ULWIND, (ObjectWindowPtr) ulRec,true);
	gULWin = theWindow;
	gULPtr = (ULWindowPtr) theWindow;

	// Show the window
	ShowWindow(theWindow);
	SelectFloating(theWindow);

	// Make it the current grafport
	SetPort(theWindow);
	TextFont(monaco);
	TextSize(9);
	RGBBackColor(&WinBackColor);

	((ObjectWindowPtr) theWindow)->Draw = ULWindowDraw;
	((ObjectWindowPtr) theWindow)->Dispose = DisposeULWin;
	((ObjectWindowPtr) theWindow)->HandleClick = ULWindowClick;
	((ObjectWindowPtr) theWindow)->ProcessKey = ULWindowKey;

	((ObjectWindowPtr) theWindow)->Idle = UserDisplayIdle;
	((ObjectWindowPtr) theWindow)->AdjustMenus = ULAdjustMenus;
	((ObjectWindowPtr) theWindow)->ProcessCommand = ULProcessCommand;
	((ObjectWindowPtr) theWindow)->Resize = ULResize;
	
	// Create List Manager Record & Buttons
	gULPtr->listRect = theWindow->portRect;
	InsetRect(&gULPtr->listRect,4,4);
	gULPtr->listRect.bottom -= 34;

	SetRect(&rDataBounds,0,0,1,0);

	gULPtr->listRect.right -= SBarSize;
	gULPtr->uList = LNew(&gULPtr->listRect, &rDataBounds, cellSize, 0, theWindow, false, false, 
				false, true);
	gULPtr->listRect.right += SBarSize;

	(*gULPtr->uList)->selFlags |= (lOnlyOne);

	SetToolBG(&gGray44);

	gULPtr->msgRect = gULPtr->listRect;
	gULPtr->msgRect.top = gULPtr->listRect.bottom + 5;
	gULPtr->msgRect.bottom = gULPtr->listRect.bottom + 32;
	gULPtr->msgRect.right -= (SU_NbrTools+1)*ToolWidth + ToolWidth/2;
	gULPtr->msgFrame = gULPtr->msgRect;
	InsetRect(&gULPtr->msgFrame,-2,-2);
	gULPtr->teH = TENew(&gULPtr->msgRect, &gULPtr->msgRect);

	toolP.h = theWindow->portRect.right - (ToolWidth*(SU_NbrTools+1)+1);
	toolP.v = theWindow->portRect.bottom - (ToolHeight+1);

	AddTool(theWindow, KillIcon, KillHiIcon, 0, toolP.h,toolP.v);
	toolP.h += ToolWidth+ToolWidth/2;
	AddTool(theWindow, BanIcon, BanHiIcon, 0, toolP.h,toolP.v);

	RebuildUserDisplay();
}

void ULWindowDraw(WindowPtr theWindow)
{
	Rect	r;
	RGBForeColor(&gGray44);
	PaintRect(&theWindow->portRect);
	HiliteRect(&theWindow->portRect);

	r = gULPtr->listRect;
	InsetRect(&r,-1,-1);
	RevHiliteRect(&r);
	
	// Render the List & Buttons
	LUpdate(theWindow->visRgn,gULPtr->uList);
	r = gULPtr->msgFrame;
	RevHiliteRect(&r);
	InsetRect(&r,1,1);
	if (gULPtr->msgActive)
		ColorPaintRect(&r,&gGray66);
	else
		ColorPaintRect(&r,&gGray44);

	PrepareULTextColors();
	TEUpdate(&gULPtr->msgRect, gULPtr->teH);
	RestoreULColors();

	RefreshTools(theWindow);
}

ServerUserPtr GetUserListRec(short n)
{
	ServerUserPtr	curUser;
	short		i;
	for (curUser = gUserList,i = 0; curUser; curUser = curUser->nextUser,++i) {
		if (i == n) {
			return curUser;
		}
	}
	return NULL;
}

void ULWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent)
{
	short	toolNbr;
	Boolean shiftFlag = (theEvent->modifiers & shiftKey) > 0;

	// Check if clicked using list Manager
	SetPort(theWin);
	GlobalToLocal(&where);

	if (PtInRect(where,&gULPtr->msgFrame)) {
		if (!gULPtr->msgActive) {
			gULPtr->msgActive = true;
			TEActivate(gULPtr->teH);
			InvalRect(&gULPtr->msgFrame);
		}
		else {
			PrepareULTextColors();
			TEClick(where,shiftFlag,gULPtr->teH);
			RestoreULColors();
		}
	}
	else { 
		DeactivateULName();

		if (PtInRect(where,&gULPtr->listRect)) {
			if (LClick(where,theEvent->modifiers, gULPtr->uList))
			{
				// Process Double Click
			}
		}
		else if (ToolClick(where,&toolNbr)) {
			switch (toolNbr) {
			case SU_Kill:
				// Kill!!!
				{
					ServerUserPtr	cuser;
					Point			cell;
					cell.h = cell.v = 0;
					LGetSelect(true,&cell,gULPtr->uList);
					cuser = GetUserListRec(cell.v);
					if (cuser) {
						ScheduleUserKill(cuser, K_KilledBySysop, gPrefs.deathPenaltyMinutes);
					}
				}
				break;
			case SU_Ban:
				{
					ServerUserPtr	cuser;
					Point			cell;
					cell.h = cell.v = 0;
					LGetSelect(true,&cell,gULPtr->uList);
					cuser = GetUserListRec(cell.v);
					if (cuser) {
						cuser->flags |= U_Banished;
						ScheduleUserKill(cuser, K_BanishKill, 32767);
					}
				}
				break;
			}
		}
	}
}

void ULWindowKey(WindowPtr theWin, EventRecord *theEvent)
{
	char	c;
	char	tbuf[256];
	c = theEvent->message & charCodeMask;

	if (gULPtr->msgActive) {
		if (c == '\r' && (*gULPtr->teH)->teLength > 1) {
			// Send Message to selected users
			BlockMove(*(*gULPtr->teH)->hText, tbuf, (*gULPtr->teH)->teLength);
			tbuf[(*gULPtr->teH)->teLength] = 0;
			PostGlobalEvent(MSG_TALK,0,tbuf,strlen(tbuf)+1);
			PrepareULTextColors();
			TESetText("",0,gULPtr->teH);
			RestoreULColors();
			InvalRect(&gULPtr->msgFrame);
		}
		else {
			PrepareULTextColors();
			TEKey(c,gULPtr->teH);
			RestoreULColors();
		}
	}
}

void DisposeULWin(WindowPtr theWin)
{
	if (gULPtr->uList)
		LDispose(gULPtr->uList);
	if (gULPtr->teH)
		TEDispose(gULPtr->teH);
	gULWin = NULL;
	gULPtr = NULL;
	DefaultDispose(theWin);
	DisposePtr((Ptr) theWin);
}

void SetUserCell(ServerUserPtr curUser, short i);
void SetUserCell(ServerUserPtr curUser, short i)
{
	Point			theCell;
	ServerRoomPtr	rp;
	StringPtr		rName;
	char			tbuf[128],addrBuf[64],userStatusChar;
	theCell.h = 0;
	theCell.v = i;
	rp = GetRoom(curUser->user.roomID);
	if (rp)
		rName = (StringPtr) &rp->room.varBuf[rp->room.roomNameOfst];
	else
		rName = "\p";
	ConvertNetAddressToString(curUser,addrBuf);
	if (curUser->flags & U_Guest)
		userStatusChar = 'n';
	else if (curUser->flags & U_God)
		userStatusChar = 'g';
	else if (curUser->flags & U_SuperUser)
		userStatusChar = 'w';
	else
		userStatusChar = 'm';

	sprintf(tbuf,"%c %-22.*s %15s %.*s",
		userStatusChar,
		curUser->user.name[0],&curUser->user.name[1],
		addrBuf,
		rName[0],&rName[1]);
	LSetCell(tbuf, strlen(tbuf), theCell, gULPtr->uList);
}

void RebuildUserDisplay()
{
	ServerUserPtr	curUser;
	short			i;

	if (gULWin == NULL)
		return;

	LDoDraw(false,gULPtr->uList);
	LDelRow(0,0,gULPtr->uList);
	for (curUser = gUserList,i = 0; curUser; curUser = curUser->nextUser,++i) {
		LAddRow(1,i,gULPtr->uList);
		SetUserCell(curUser, i);
	}
	LDoDraw(true,gULPtr->uList);
	DefaultRefresh(gULWin);
}

void UpdateUserDisplay(long userID)
{
	ServerUserPtr	curUser;
	short			i;

	if (gULWin == NULL)
		return;

	for (curUser = gUserList,i = 0; curUser; curUser = curUser->nextUser,++i) {
		if (curUser->user.userID == userID) {
			SetUserCell(curUser, i);
			return;
		}
	}
	// user not found... (shouldn't happen, but just in case)
	RebuildUserDisplay();
}

void UserDisplayIdle(WindowPtr theWin, EventRecord *theEvent)
{
	if (gULPtr->msgActive)
		TEIdle(gULPtr->teH);
}

void DeactivateULName()
{
	if (gULPtr->msgActive) {
		gULPtr->msgActive = false;
		TEDeactivate(gULPtr->teH);
		InvalRect(&gULPtr->msgFrame);
	}
}

void ULAdjustMenus(WindowPtr theWin)
{
	Boolean	hasSelection,hasClip;
	if (gULPtr->msgActive) {
		hasSelection = ((*gULPtr->teH)->selEnd > (*gULPtr->teH)->selStart);
		hasClip = CanPaste('TEXT');
	}
	else {
		hasSelection = false;
		hasClip = false;
	}
	DefaultAdjustMenus(theWin);
	MyEnableMenuItem(gEditMenu, EM_Copy, hasSelection);
	MyEnableMenuItem(gEditMenu, EM_Cut, hasSelection);
	MyEnableMenuItem(gEditMenu, EM_Clear, hasSelection);
	MyEnableMenuItem(gEditMenu, EM_Paste, hasClip);
	MyEnableMenuItem(gEditMenu, EM_SelectAll, true);
}

void ULProcessCommand(WindowPtr theWin, short theMenu, short theItem)
{
	switch (theMenu) {
	case EditMENU:
		if (gULPtr->msgActive) {
			PrepareULTextColors();
			switch (theItem) {
			case EM_Cut:	
				TECut(gULPtr->teH);
				break;
			case EM_Copy:
				TECopy(gULPtr->teH);
				break;
			case EM_Clear:
				TEDelete(gULPtr->teH);
				break;
			case EM_Paste:
				TEPaste(gULPtr->teH);
				break;
			case EM_SelectAll:
				TESetSelect(0,32767,gULPtr->teH);
				break;
			}
			RestoreULColors();
		}
		break;
	}
	DefaultProcessCommand(theWin,theMenu,theItem);
}

void PrepareULTextColors()
{
	SetPort(gULWin);
	if (gULPtr->msgActive)
		RGBBackColor(&gGray66);
	else
		RGBBackColor(&gGray44);
	RGBForeColor(&gWhiteColor);
}

void RestoreULColors()
{
	RGBForeColor(&gBlackColor);	
	RGBBackColor(&WinBackColor);	
}

void GenerateUserDataFile();
void GenerateUserDataFile()
{
	FILE		*ofile;
	long		nbrUsers,i;
	Str63		lastDate,lastTime,killDate,killTime;
	char		*templateOK = "%10ld  %21.*s  %3d.%3d.%3d.%3d  %8.*s %8.*s  %8.*s %8.*s  %4ld     %4ld    %4ld   %2d\n";
	char		*templateBAN = "%10ld [BAN] %16.*s  %3d.%3d.%3d.%3d  %8.*s %8.*s  %8.*s %8.*s  %4ld     %4ld    %4ld   %2d\n";
	char		*temp;
	Handle		h;
	UserBasePtr	up;
	long		type,assetID;
	Str63		name;

	ofile = fopen("userdata.lis","w");
	fprintf(ofile,"User ID#    Last Nickname          Last IP Address   Last On            Last Killed     #Log-ins #Deaths #Fails    #TCode\n");
	fprintf(ofile,"=====================================================================================================================\n");
/**
User ID#    Last Nickname          Last IP Address   Last On            Last Killed     #Log-ins #Deaths #Fails    #TCode
===================================================================
        59                 Newbie    0.  0.  0.  0   18/12/95  5:38 PM  18/12/95  5:38 PM    28       13      12   12
**/
	nbrUsers = CountAssets(RT_USERBASE);
	for (i = 0; i < nbrUsers; ++i) {
		h = GetIndAsset(RT_USERBASE, i);
		if (h) {
			GetAssetInfo(h, &type, &assetID, name);
			HLock(h);
			up = (UserBasePtr) *h;

			IUDateString(up->timeLastActive,shortDate,lastDate);
			IUDateString(up->timeLastPenalized,shortDate,killDate);
			IUTimeString(up->timeLastActive,0, lastTime);
			IUTimeString(up->timeLastPenalized,0, killTime);
			if (up->lastFlags & U_Banished)
				temp = templateBAN;
			else
				temp = templateOK;
			fprintf(ofile,temp,
				assetID ^ MAGIC_LONG,
				up->lastName[0],&up->lastName[1],
				(int) (up->lastIPAddress >> 24) & 0x00FF,
				(int) (up->lastIPAddress >> 16) & 0x00FF,
				(int) (up->lastIPAddress >> 8) & 0x00FF,
				(int) (up->lastIPAddress & 0x00FF),
				lastDate[0],&lastDate[1],
				lastTime[0],&lastTime[1],
				killDate[0],&killDate[1],
				killTime[0],&killTime[1],
				up->nbrLogins,
				up->nbrDishonorableDeaths,
				up->nbrFailedAttempts,
				(int) up->whyLastKilled);
				
			HUnlock(h);
			ReleaseAsset(h);
		}
	}
	fclose(ofile);
	LogMessage("Created User Database Log in userdata.lis\r");
	LogMessage("Number of users in the log are %d\r", nbrUsers);
}

void ULResize(WindowPtr theWin, short w, short h)
{
	Point	toolP;
	DefaultResize(theWin, w, h);
	SetPort(theWin);
	gULPtr->listRect = theWin->portRect;
	InsetRect(&gULPtr->listRect,4,4);
	gULPtr->listRect.bottom -= 34;
	gULPtr->listRect.right -= SBarSize;
	LSize(	gULPtr->listRect.right - gULPtr->listRect.left,
			gULPtr->listRect.bottom - gULPtr->listRect.top,
			gULPtr->uList);
	gULPtr->listRect.right += SBarSize;
	gULPtr->msgRect = gULPtr->listRect;
	gULPtr->msgRect.top = gULPtr->listRect.bottom + 5;
	gULPtr->msgRect.bottom = gULPtr->listRect.bottom + 32;
	gULPtr->msgRect.right -= (SU_NbrTools+1)*ToolWidth + ToolWidth/2;
	gULPtr->msgFrame = gULPtr->msgRect;
	InsetRect(&gULPtr->msgFrame,-2,-2);
	(*gULPtr->teH)->destRect = gULPtr->msgRect;
	(*gULPtr->teH)->viewRect = gULPtr->msgRect;
	toolP.h = theWin->portRect.right - (ToolWidth*(SU_NbrTools+1)+1);
	toolP.v = theWin->portRect.bottom - (ToolHeight+1);
	MoveTool(theWin, SU_Kill, toolP.h,toolP.v);
	toolP.h += ToolWidth+ToolWidth/2;
	MoveTool(theWin, SU_Ban,  toolP.h,toolP.v);
	InvalRect(&theWin->portRect);
}