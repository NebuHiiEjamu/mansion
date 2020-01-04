// RoomListWindow.c
// Note: Bug!!
// Don't overwrite transcolor of old picturs
// When adding record, if picture already exists in room picture list,
// copy transcolro

#include "U-USER.H"
#include "m-cmds.H"
#include "UserTools.h"

// checkbox to show occupied rooms only...

#define		PLWIND				140
#define     WinBackColor		gGrayAA
#define		MaxPictureList		16

#define 	LONGALIGN(x)	x += (4 - (x & 3)) & 3

typedef struct {
	Str63	pictName;	// 0 for nada
	short	transColor;
	Point	picLoc;
} PictureList;

typedef struct {
	ObjectWindowRecord	win;
	HotspotPtr			door;
	Rect				listRect;
	ListHandle			uList;
	PictureList			*pictList;
	short				nbrPictures,originalState;
	short				doorID,roomID;
	Boolean				changeFlag;
} PLWindowRecord, *PLWindowPtr;

WindowPtr	gPLWin;
PLWindowPtr	gPLPtr;

void NewPLWindow(void);

void BuildPictureList();
void PLWindowDraw(WindowPtr theWindow);
void EditPictureInList(short lineNumber, Boolean replaceFlag);
void PLWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent);
void DisposePLWin(WindowPtr theWin);
void SavePictList(void);

// Create a new main window using a 'WIND' template from the resource fork
//
void NewPLWindow()
{
	WindowPtr		theWindow;
	PLWindowPtr		ulRec;
	Rect			rDataBounds;
	Point			cellSize = {0,0};
	Point			toolP;
	RoomRec			*rp = &gRoomWin->curRoom;

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 

	if (gRoomWin->curHotspot <= 0)
		return;

	SetPort((WindowPtr) gRoomWin);
	RoomWindowDraw((WindowPtr) gRoomWin);

	ulRec = (PLWindowPtr) NewPtrClear(sizeof(PLWindowRecord));
	theWindow = InitObjectWindow(PLWIND, (ObjectWindowPtr) ulRec,true);
	RestoreWindowPos(theWindow, &gMacPrefs.pictListPos);

	// Show the window
	ShowWindow(theWindow);
	SelectFloating(theWindow);

	// Make it the current grafport
	SetPort(theWindow);
	TextFont(monaco);
	TextSize(9);
	RGBBackColor(&WinBackColor);

	gPLWin = theWindow;
	gPLPtr = (PLWindowPtr) theWindow;

	((ObjectWindowPtr) theWindow)->Draw = PLWindowDraw;
	((ObjectWindowPtr) theWindow)->Dispose = DisposePLWin;
	((ObjectWindowPtr) theWindow)->HandleClick = PLWindowClick;
	// ((ObjectWindowPtr) theWindow)->AdjustCursor = PLAdjustCursor;
	
	// Create List Manager Record & Buttons
	gPLPtr->door =  (HotspotPtr) &rp->varBuf[rp->hotspotOfst];
	gPLPtr->door += gRoomWin->curHotspot-1;
	gPLPtr->originalState = gPLPtr->door->state;
	gPLPtr->doorID = gPLPtr->door->id;
	gPLPtr->roomID = rp->roomID;
	
	gPLPtr->listRect = theWindow->portRect;
	InsetRect(&gPLPtr->listRect,4,4);
	gPLPtr->listRect.bottom -= 32;

	SetRect(&rDataBounds,0,0,1,0);

	gPLPtr->listRect.right -= SBarSize;
	gPLPtr->uList = LNew(&gPLPtr->listRect, &rDataBounds, cellSize, 0, theWindow, false, false, 
				false, true);
	if (gPLPtr->uList == NULL) {
		DisposePLWin(theWindow);
		return;
	}
	gPLPtr->listRect.right += SBarSize;

	(*gPLPtr->uList)->selFlags |= (lOnlyOne);

	BuildPictureList();

	toolP.h = theWindow->portRect.right - (ToolWidth*6+1);
	toolP.v = theWindow->portRect.bottom - (ToolHeight+1);
	SetToolBG(&gGray44);
	AddTool(theWindow, AddIcon, AddHiIcon, 0, toolP.h, toolP.v);
	toolP.h += ToolWidth + ToolWidth/2;
	AddTool(theWindow, DeleteIcon, DeleteHiIcon, 0, toolP.h, toolP.v);
	toolP.h += ToolWidth + ToolWidth/2;
	AddTool(theWindow, CancelIcon, CancelHiIcon, 0, toolP.h, toolP.v);
	toolP.h += ToolWidth + ToolWidth/2;
	AddTool(theWindow, SaveIcon, SaveHiIcon, 0, toolP.h, toolP.v);

}

void PLWindowDraw(WindowPtr theWindow)
{
	Rect	r;
	RGBForeColor(&gGray44);
	PaintRect(&theWindow->portRect);
	HiliteRect(&theWindow->portRect);

	r = gPLPtr->listRect;
	InsetRect(&r,-1,-1);
	RevHiliteRect(&r);
	InsetRect(&r,-1,-1);
	RevHiliteRect(&r);
	InsetRect(&r,2,2);
	EraseRect(&r);

	if (gPLPtr->uList)
		LUpdate(theWindow->visRgn,gPLPtr->uList);

	RefreshTools(theWindow);
}

void PLWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent)
{
	short	toolNbr;
	Point	cell;

	// Check if clicked using list Manager
	SetPort(theWin);
	GlobalToLocal(&where);
	if (PtInRect(where,&gPLPtr->listRect)) {
		if (theEvent->modifiers & cmdKey) {
			RefreshRoomList();
			return;
		}
		if (LClick(where,theEvent->modifiers, gPLPtr->uList))
		{
			// Process Double Click
			cell = LLastClick(gPLPtr->uList);
			EditPictureInList(cell.v, true);
			gPLPtr->changeFlag = true;
		}
		else {
			cell = LLastClick(gPLPtr->uList);
			if (cell.v >= 0 && cell.v < gPLPtr->door->nbrStates) {
				gPLPtr->door->state = cell.v;
				RefreshRoom(&gOffscreenRect);
			}
		}
	}
	else if (ToolClick(where,&toolNbr)) {
		switch (toolNbr) {
		case 0:
			// Add New Entry (use PICT file Dialog with NONE button)
			if (gPLPtr->nbrPictures < MaxPictureList)
			{
				LAddRow(1,gPLPtr->nbrPictures,gPLPtr->uList);
				++gPLPtr->nbrPictures;
				EditPictureInList(gPLPtr->nbrPictures-1,false);
				gPLPtr->changeFlag = true;
			}
			break;
		case 1:
			// Delete Entry
			cell.h = cell.v = 0;
			if (LGetSelect(TRUE,&cell,gPLPtr->uList)) {
				if (cell.v >= 0 && cell.v < gPLPtr->nbrPictures) {
					LDelRow(1,cell.v,gPLPtr->uList);
					BlockMove(&gPLPtr->pictList[cell.v+1],
							  &gPLPtr->pictList[cell.v],
							  sizeof(PictureList) * (MaxPictureList-(cell.v+1)));
					gPLPtr->nbrPictures--;
					gPLPtr->changeFlag = true;
				}
			}
			break;
		case 2:
			DisposePLWin(theWin);	
			break;
		case 3:
			// Save Picture List
			if (gPLPtr->changeFlag)
				SavePictList();
			DisposePLWin(theWin);
			break;
		}
	}
}

void DisposePLWin(WindowPtr theWin)
{
	if (gPLPtr->uList)
		LDispose(gPLPtr->uList);
	if (gPLPtr->pictList)
		DisposePtr((Ptr) gPLPtr->pictList);
	SaveWindowPos(theWin, &gMacPrefs.pictListPos);
	gPLWin = NULL;
	gPLPtr = NULL;
	DefaultDispose(theWin);
	DisposePtr((Ptr) theWin);
}

void BuildPictureList()
{
	PictureRecPtr	prl,prec;
	StateRecPtr		srp;
	RoomRec			*rp = &gRoomWin->curRoom;
	short			i;
	short			pictID,rowNumber;
	StringPtr		picName;
	Point			theCell;

	if (gPLPtr->pictList) {
		DisposePtr((Ptr) gPLPtr->pictList);
		gPLPtr->pictList = NULL;
	}

	// Allocate PictList
	gPLPtr->pictList = (PictureList *) NewPtrClear(sizeof(PictureList)*MaxPictureList);
	if (gPLPtr->pictList == NULL)
		return;

	prl = (PictureRecPtr) &rp->varBuf[rp->pictureOfst];
	srp = (StateRecPtr) &rp->varBuf[gPLPtr->door->stateRecOfst];

	// Make picture list entries
	LDoDraw(false,gPLPtr->uList);
	LDelRow(0,0,gPLPtr->uList);
	for (i = 0; i < gPLPtr->door->nbrStates; ++i) {
		pictID = srp[i].pictID;
		if (pictID) {
			prec = GetPictureRec(pictID);
			picName = (StringPtr) &rp->varBuf[prec->picNameOfst];
			BlockMove(picName, gPLPtr->pictList[i].pictName,picName[0]+1);
			gPLPtr->pictList[i].transColor = prec->transColor;
			gPLPtr->pictList[i].picLoc = srp[i].picLoc;
		}
		else {
			prec = NULL;
			gPLPtr->pictList[i].pictName[0] = 0;
			gPLPtr->pictList[i].transColor = -1;
			gPLPtr->pictList[i].picLoc.h = 0;
			gPLPtr->pictList[i].picLoc.v = 0;
		}
		// Add to listrecord
		rowNumber = LAddRow(1,i,gPLPtr->uList);
		theCell.h = 0;
		theCell.v = rowNumber;
		if (gPLPtr->pictList[i].pictName[0])
			LSetCell(&gPLPtr->pictList[i].pictName[1], gPLPtr->pictList[i].pictName[0], theCell, gPLPtr->uList);
		else
			LSetCell("<NONE>",6,theCell,gPLPtr->uList);
	}

	LDoDraw(true,gPLPtr->uList);
	gPLPtr->nbrPictures = gPLPtr->door->nbrStates;
	// !! Process the list
	DefaultRefresh(gPLWin);
}

void EditPictureInList(short n, Boolean replaceFlag)
{
	StandardFileReply	sfReply;
	SFTypeList			typeList = {'PICT', 'DIB ', 'BMP ', 'GIFf'};
	StringPtr			pictName;
	Point				theCell;
	PictureRecPtr		prec;
	StandardGetFile(NULL,4,typeList,&sfReply);
	theCell.h = 0;
	theCell.v = n;
	if (sfReply.sfGood) {
		// Add Record with Name
		pictName = sfReply.sfFile.name;
		LSetCell(&pictName[1], pictName[0], theCell, gPLPtr->uList);
		BlockMove(pictName, gPLPtr->pictList[n].pictName,pictName[0]+1);
		prec = GetPictureRecByName(pictName);
		if (prec)
			gPLPtr->pictList[n].transColor = prec->transColor;
		else
			gPLPtr->pictList[n].transColor = -1;
		if (!replaceFlag) {
			gPLPtr->pictList[n].picLoc.h = 0;
			gPLPtr->pictList[n].picLoc.v = 0;
		}
	}
	else {
		LSetCell("<NONE>",6,theCell,gPLPtr->uList);
		gPLPtr->pictList[n].pictName[0] = 0;
		gPLPtr->pictList[n].transColor = -1;
		gPLPtr->pictList[n].picLoc.h = 0;
		gPLPtr->pictList[n].picLoc.v = 0;
	}
}

void SavePictList()
{
	short		i,j;
	Boolean		newPictFlag=false;
	StateRec	sRecs[MaxPictureList],*srp;
	PictureRec	pRecs[MaxPictureList],*prl;
	short		pictID;
	short		nbrRoomPictures;
	RoomRec		*rp = &gRoomWin->curRoom;
	HotspotPtr	hs;
	
	// Make sure we're still valid...
	if (gPLPtr->roomID != rp->roomID)
		return;

	hs= GetHotspot(gPLPtr->doorID);
	if (hs != gPLPtr->door)
		return;

	// Copy in current state recs and picture list
	prl = (PictureRecPtr) &rp->varBuf[rp->pictureOfst];
	srp = (StateRecPtr) &rp->varBuf[hs->stateRecOfst];

	nbrRoomPictures = rp->nbrPictures;
	if (nbrRoomPictures > MaxPictureList)
		nbrRoomPictures = MaxPictureList;
	BlockMove(prl,&pRecs[0],nbrRoomPictures*sizeof(PictureRec));

	for (i = 0; i < gPLPtr->nbrPictures; ++i) {
		if (gPLPtr->pictList[i].pictName[0] == 0)
			pictID = 0;
		else {
			// Check if picture is already in room's pict list, if so, add state record
			// for that pict id...
			Boolean	foundFlag = false;
			short	maxPicID = 0;
			for (j = 0; j < nbrRoomPictures; ++j) {
				if (pRecs[j].picID > maxPicID)
					maxPicID = pRecs[j].picID;
				if (EqualPString(gPLPtr->pictList[i].pictName,(StringPtr) &rp->varBuf[pRecs[j].picNameOfst],false)) {
					pictID = pRecs[j].picID;
					pRecs[j].transColor = gPLPtr->pictList[i].transColor;							
					foundFlag = true;
					break;
				}
			}
			// else add record to room's pict list
			if (!foundFlag) {
				short	n;
				n = nbrRoomPictures;
				pictID = maxPicID+1;
				pRecs[n].picID = pictID;
				pRecs[n].refCon = 0;
				pRecs[n].picNameOfst = AddRoomString(gPLPtr->pictList[i].pictName);
				pRecs[n].transColor = gPLPtr->pictList[i].transColor;
				pRecs[n].reserved = 0;
				newPictFlag = true;
				++nbrRoomPictures;
			}
		}
		sRecs[i].pictID = pictID;
		sRecs[i].reserved = 0;
		sRecs[i].picLoc = gPLPtr->pictList[i].picLoc;
	}
	if (newPictFlag) {
		rp->nbrPictures = nbrRoomPictures;
		// 9/11/96 This was causing a bug on some servers because the pictureOfst wasn't long aligned
		// after the call to AddRoomString, above
		LONGALIGN(gRoomWin->curRoom.lenVars);
		rp->pictureOfst = AddRoomBuffer((Ptr) &pRecs[0],nbrRoomPictures * sizeof(PictureRec));
	}
	hs->nbrStates = gPLPtr->nbrPictures;
	hs->state = gPLPtr->originalState;
	if (hs->nbrStates) {
		// 9/11/96
		LONGALIGN(gRoomWin->curRoom.lenVars);
		hs->stateRecOfst = AddRoomBuffer((Ptr) &sRecs[0],gPLPtr->nbrPictures * sizeof(StateRec));
	}
	else
		hs->stateRecOfst = 0;
	DoPalaceCommand(PC_SetRoomInfo, (long) &gRoomWin->curRoom, NULL);
}

