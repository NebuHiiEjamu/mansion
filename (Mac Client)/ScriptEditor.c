/************************************************************************************
 * ScriptEditor.c
 *
 * Todo: Cmd-S saves the script (use dirty flag)
 *		Closing window brings up confirmation/cancel dialog
 *
 *		add editing keys
 *
 ************************************************************************************/
#include "U-USER.H"
#include "m-cmds.H"

#if THINK_C
#include <LoMem.h>
#endif

#define		MainWIND	128
#define		MaxLines	200


typedef struct {
	TEWindowRec		win;
	RoomID			roomID;
	short			spotID;
} ScriptEditorRec, *ScriptEditorPtr;
ScriptEditorPtr	gSEWin;	// note: this data structure is now unused by shared code

void SaveSEWin(WindowPtr theWin);

extern ScriptEditorPtr	gSEWin;
extern char *eName[PE_NbrEvents];

void LoadScriptIntoEditor()
{
	HotspotPtr 		hsl;

	TEWindowPtr		teWin = (TEWindowPtr) gSEWin;
	ScrollWindowPtr	sWin = (ScrollWindowPtr) gSEWin;
	RoomRec			*rp = &gRoomWin->curRoom;
	Point			units;
	Ptr				sp,dp;
	if (gRoomWin->curHotspot <= 0)
		return;
	hsl = (HotspotPtr) &rp->varBuf[rp->hotspotOfst];
	hsl += gRoomWin->curHotspot-1;

	gSEWin->roomID = rp->roomID;
	gSEWin->spotID = hsl->id;
	
	// Turn off Line Wrap
	(*teWin->teH)->crOnly = -1;

	// Convert CR/LF to CR
	// Convert LF to CR
	
	for (sp = dp = (Ptr) &rp->varBuf[hsl->scriptTextOfst]; *sp; ) {
		switch (*sp) {
		case 0x0d:
			*(dp++) = *(sp++);
			if (*sp == 0x0d)
				++sp;
			break;
		case 0x0a:
			*(dp++) = 0x0d;
			++sp;
			break;
		default:
			*(dp++) = *(sp++);
			break;
		}
	}

	if (hsl->scriptTextOfst)
		TEInsert(&rp->varBuf[hsl->scriptTextOfst],strlen(&rp->varBuf[hsl->scriptTextOfst]),teWin->teH);

	units.h = 0;
	units.v = (*teWin->teH)->nLines;
	SetScrollUnits((WindowPtr) teWin,units);

	AdjustScrollBars((WindowPtr) teWin, false);
}

void SaveScriptFromEditor()
{
	TEWindowPtr		teWin = (TEWindowPtr) gSEWin;
	HotspotPtr		hs;
	RoomRec			*rp = &gRoomWin->curRoom;

	if (!teWin->dirty)
		return;

	hs = GetHotspot(gSEWin->spotID);
	if (hs == NULL)
		return;

	hs->scriptTextOfst = AddRoomBuffer((Ptr) *(*teWin->teH)->hText, (*teWin->teH)->teLength);
	// Add Null at End
	rp->varBuf[rp->lenVars++] = 0;
	DoPalaceCommand(PC_SetRoomInfo, (long) &gRoomWin->curRoom, NULL);
	teWin->dirty = false;
}

void NewScriptEditor(void)
{
	WindowPtr		theWindow;
	ScriptEditorPtr	seRec;
	char			tbuf[256];
	StringPtr		roomName;

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 


	seRec = (ScriptEditorPtr) NewPtrClear(sizeof(ScriptEditorRec));
	theWindow = NewTEWindow(MainWIND, (TEWindowPtr) seRec, true);
	RestoreWindowPos(theWindow, &gMacPrefs.scriptPos);
	((ObjectWindowPtr) theWindow)->Save = SaveSEWin;

	((ObjectWindowPtr) theWindow)->Dispose = DisposeSEWin;
	roomName =  (StringPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.roomNameOfst];
	sprintf(tbuf,"Script for %.*s",roomName[0],&roomName[1]);
	CtoPstr(tbuf);
	SetWTitle(theWindow,(StringPtr) tbuf);
	gSEWin = (ScriptEditorPtr) theWindow;
	LoadScriptIntoEditor();
	ShowWindow(theWindow);
}

short SaveYourChanges();
#define SaveDLOG	505

short SaveYourChanges()
{
	GrafPtr		savePort;
	DialogPtr 	dp;
	short		itemHit;

	GetPort(&savePort);

	if ((dp = GetNewDialog (SaveDLOG, NULL, (WindowPtr) -1)) == NULL)
		return Cancel;
	
	SetDialogDefaultItem(dp, OK);
	SetDialogCancelItem(dp, Cancel);
	SetDialogTracksCursor(dp,true);

	ShowWindow(dp);
	SetPort(dp);
	SetCursor(&qd.arrow);
	ShowCursor();
	do {
		ModalDialog(NULL, &itemHit);
	} while (itemHit != OK && itemHit != Cancel && itemHit != 3);
	DisposDialog(dp);
	SetPort(savePort);
	return itemHit;
}

void DisposeSEWin(WindowPtr theWin)
{
	TEWindowPtr		teWin = (TEWindowPtr) gSEWin;
	short			resp;
	if (teWin->dirty) {
		resp = SaveYourChanges();
		if (resp == Cancel)
			return;
		else if (resp == OK)
			SaveScriptFromEditor();
	}
	SaveWindowPos(theWin, &gMacPrefs.scriptPos);
	DisposeTEWin(theWin);
	DisposePtr((Ptr) theWin);
	gSEWin = NULL;
}

void SaveSEWin(WindowPtr theWin)
{
	TEWindowPtr		teWin = (TEWindowPtr) theWin;
	if (teWin->dirty) {
		SaveScriptFromEditor();
	}
}
