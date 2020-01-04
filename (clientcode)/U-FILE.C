// U-FILES.C
//
/**

	Todo: Do requests for door overlays, and refresh correctly...
	
**/

#include "U-USER.H"       // esr 5/19 Renamed from User.h

#define MaxFileRequests	32
#define MaxRequestTimeLimit	(180L*TICK_SECONDS)

typedef struct {
	long	transID;
	Str63	name;
	unsigned char pName[128];	// 9/20/95 - pascal string - full path name
	unsigned char tName[128];	// 9/20/95  - added temporary name for downloading...
	FileHandle	oFile;
	short	expectedBlock;
	short	mediaType;			// 8/6/96 JAB expected media type
	long	bytesIn;
	unsigned long timeStamp;
} FReqRec, *FReqPtr;

short		gNbrFileRequests = 0;
FReqPtr		gFileRequestPtr = NULL;


enum {FT_Unknown=0, FT_GIF, FT_DIB, FT_PICT, FT_WAVE};

Boolean RegisterFileRequest(StringPtr name, int mediaType);
void RegisterFileReception(FReqPtr fp);
FReqPtr	GetFileReqByID(long transID, short blockNbr);
FReqPtr	GetFileReqByName(StringPtr name);
void RefreshForPicture(StringPtr fName);
FileHandle CreatePalaceFile(StringPtr name, int mediaType);

// !!! Also call when an overlay is not found
// Note: There are calls to this routine in Local code: 
//	called from HotRegions.c ExpandPicture()
//  called from RoomGraphics.c RefreshNewRoom()

// DEBUG
// void TestTCPCorruption(short flag);

void RequestFile(StringPtr name, int mediaType)
{
	if (name == NULL || name[0] <= 0 || name[0] > 32) {
		return;
	}
	// TestTCPCorruption('I');
	if (name[0] && !RegisterFileRequest(name, mediaType))	{ // 7/17/95

#if macintosh
		if (name == NULL || name[0] <= 0 || name[0] > 32) {
			if (gDebugFlag)
				SysBreakStr("\pDEBUG: Bad File Name;hc");
			return;
		}
#endif

		// !! Boom!!
		// TestTCPCorruption('L');
		PostServerEvent(MSG_FILEQUERY,gRoomWin->meID,(Ptr) name,name[0]+1);
		// TestTCPCorruption('M');

#ifdef WIN32
		LogMsg(0,InfoText,"Requested file %.*s\r",name[0],&name[1]);
#else
		LogMessage("Requested file %.*s\r",name[0],&name[1]);
#endif
	}
}

// !!!! Call this when server goes down or user quits
//     called from shared code
//		    U-EVENTS.C      SignOff()
//     called from local code
//			NetworkMain.c   DisconnectUser()
//
// Note: it's ok to call this twice or more when logging off
//
void AbortDownloads()
{
	FReqPtr	ap;
	short	i;
	if (gFileRequestPtr == NULL || gNbrFileRequests == 0)
		return;
	ap = gFileRequestPtr;
	for (i = 0; i < gNbrFileRequests; ++i,++ap) {
		if (ap->oFile) {
			FSClose(ap->oFile);
			FSDelete(ap->tName,0);
		}
	}
	gNbrFileRequests = 0;
}

// !!!! This is called to determine if file is already being downloaded
//  called from local code (same as RequestFile)
//				called from HotRegoins.c ExpandPicture()
//				called from RoomGraphics.c RefreshNewRoom()

Boolean FileIsBeingDownloaded(StringPtr name)
{
	FReqPtr	ap;
	short	i;
	if (name[0] == 0)
		return false;
	if (gFileRequestPtr == NULL || gNbrFileRequests == 0)
		return false;
	ap = gFileRequestPtr;
	for (i = 0; i < gNbrFileRequests; ++i,++ap) {
		if (EqualString(ap->name,name,false,false)) {
			return true;
		}
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Call this function in response to a MSG_FILESEND msg from the server
//
// This is called from U-EVENTS.C (shared code)
//
// Local Utilities...
void CreateTemporaryFileName(StringPtr seedName, StringPtr tempName);
void RenameFile(StringPtr oldName, StringPtr newName);


void ReceiveFile(Ptr fileMsg)	// Server is sending file block
{
	FileBlockPtr	asp;
	FReqPtr			rp;
	StringPtr		sp;
	long			count;

	asp = (FileBlockPtr) fileMsg;

	if (asp->blockNbr == 0) {

		if (asp->blockSize > asp->varBlock.firstBlockRec.size)
			return;
		rp = GetFileReqByName( asp->varBlock.firstBlockRec.name);
		if (rp == NULL)
			return;
		rp->transID = asp->transactionID;
		// Build Server Directory within /picture directory, if necessary
		switch (rp->mediaType) {
		case MT_Picture:
			CreateServerDirectory(PicturesPathList,1);	// 9/22/95
			sp = BuildMediaFolderName(rp->name,PicturesPathList,1);	// 9/22/95
			break;
		case MT_Sound:
			CreateServerDirectory(SoundsPathList,1);	// 9/22/95
			sp = BuildMediaFolderName(rp->name,SoundsPathList,1);	// 9/22/95
			break;
		case MT_Movie:
			CreateServerDirectory(MoviesPathList,1);	// 9/22/95
			sp = BuildMediaFolderName(rp->name,MoviesPathList,1);	// 9/22/95
			break;
		}
		BlockMove(sp, rp->pName,sp[0]+1);
		CreateTemporaryFileName(rp->pName,rp->tName);
		rp->oFile = CreatePalaceFile(rp->tName, rp->mediaType);
		// Do something with initial data...
		if (rp->oFile) {
			count = asp->blockSize;
			FSWrite(rp->oFile,&count,&asp->varBlock.firstBlockRec.data[0]);
			rp->bytesIn += count;
		}
		rp->expectedBlock = 1;
	}
	else {
		rp = GetFileReqByID(asp->transactionID,asp->blockNbr);
		if (rp == NULL)
			return;
		if (rp->expectedBlock != asp->blockNbr)
			return;
		if (rp->oFile) {
			count = asp->blockSize;
			FSWrite(rp->oFile,&count,&asp->varBlock.nextBlockRec.data[0]);
			rp->bytesIn += count;
		}
		++rp->expectedBlock;
	}
	// Local function, displays progress bar on room window if rp->name
	// matches current room name
	// !!!! 
	ShowDownloadStatus(rp->name, rp->timeStamp, asp->blockNbr,(short)(asp->nbrBlocks-1));
	if (asp->blockNbr == asp->nbrBlocks-1) {
		if (rp->oFile) {
			Str63	fName;
			char	sMsg[256];
			int		mediaType;
			BlockMove(rp->name,fName,rp->name[0]+1);
			FSClose(rp->oFile);
			// Rename the file
			RenameFile(rp->tName,rp->pName);
			mediaType = rp->mediaType;
			RegisterFileReception(rp);

			// 8/8/96 Changed this to a status line message for better feedback
			switch (mediaType) {
			case MT_Picture:
				sprintf(sMsg,"Received picture %.*s",fName[0],&fName[1]);
				break;
			case MT_Sound:
				sprintf(sMsg,"Received sound %.*s",fName[0],&fName[1]);
				break;
			default:
				sprintf(sMsg,"Received file %.*s",fName[0],&fName[1]);
				break;
			}
			StatusMessage(sMsg,0);
			if (mediaType == MT_Picture)
				RefreshForPicture(fName);
		}
	}
}

//
// The remainder of these routines are used internally by the top 4 routines
//

// !!!! Mac-specific
FileHandle CreatePalaceFile(StringPtr name, int mediaType)
{
	long	creator=0x3f3f3f3fL;		// '????'
	long	fileType=0x3f3f3f3fL;	// '????'
	OSErr	oe;
	FileHandle	oFile;
	short	type,i;

	// This should be based on file extension
	type = FT_Unknown;
	for (i = name[0]; i >= 1; --i) {
		if (name[i] == '.') {
			if (strincmp(".pic",(char *) &name[i],4) == 0)
				type = FT_PICT;
			else if (strincmp(".dib",(char *) &name[i],4) == 0)
				type = FT_DIB;
			else if (strincmp(".gif",(char *) &name[i],4) == 0)
				type = FT_GIF;
			else if (strincmp(".wav",(char *) &name[i],4) == 0)
				type = FT_WAVE;
			else
				continue;
			break;
		}
	}

	// If no file extension, make assumption based on media type
	// (palace sounds often have no file extensions)
	if (type == FT_Unknown) {
		switch (mediaType) {
		case MT_Picture:
			type = FT_GIF;
			break;
		case MT_Sound:
			type = FT_WAVE;
			break;
		}
	}

	FSDelete(name,0);

#if macintosh
	switch (type) {
	case FT_PICT:
		fileType = 'PICT';
		creator = '8BIM';		// Photoshop
		break;
	case FT_DIB:
		fileType = 'BMP ';
		creator = '8BIM';		// Photoshop
		break;
	case FT_GIF:
		fileType = 'GIFf';
		creator = '8BIM';		// Photoshop
		break;
	case FT_WAVE:
		fileType = 'BINA';
		creator = 'fB\xA5x';	// SoundEdit 16
		break;
	}
#endif

	oe = Create(name,0,creator,fileType);	
	if (oe != noErr)
		return 0;
	oe = FSOpen(name,0, &oFile);
	if (oe != noErr)
		return 0;
	return oFile;
}

void RefreshForPicture(StringPtr fName)
{
	RoomRec		*rp = &gRoomWin->curRoom;
	// If Room Background picture has been downloaded, reload the background
	if (rp->pictNameOfst && EqualString((StringPtr) &rp->varBuf[rp->pictNameOfst], fName, false, false)) {
		RefreshNewRoom();
	}
	// Else if one of the overlays has been downloaded, reload the whole room
	else {
		short			i;
		PictureRecPtr	prl;
		Boolean			gotOne = false;
		prl = (PictureRecPtr) &rp->varBuf[rp->pictureOfst];
		for (i = 0; i < gRoomWin->curRoom.nbrPictures; ++i,++prl) {	// 6/7/95
			if (EqualString((StringPtr) &rp->varBuf[prl->picNameOfst], fName, false, false)) {
				gotOne= true;
				break;
			}
		}
		if (gotOne)
			ModifyRoomInfo(&gRoomWin->curRoom, false);
	}
}


// 7/17/95 Maintain a list of files that have been requested from server
// (to avoid multiple requests for the same File)
//
// Returns true if file has already been requested, otherwise, adds
// to list of pending file requests
Boolean RegisterFileRequest(StringPtr name, int mediaType)
{
	FReqPtr			ap;
	short			i;
	unsigned long	t;

	if (gFileRequestPtr == NULL) {
		gFileRequestPtr = (FReqPtr) NewPtrClear(sizeof(FReqRec) * MaxFileRequests);
		if (gFileRequestPtr == NULL)
			return false;
	}

	// Kill Old Ones that haven't been honored
	ap = gFileRequestPtr;
	t = GetTicks();
	for (i = 0; i < gNbrFileRequests; ++i,++ap) {
		if (ap->bytesIn == 0 && t - ap->timeStamp > MaxRequestTimeLimit) {
			BlockMove((Ptr)&ap[1],(Ptr)ap,sizeof(FReqRec)*((gNbrFileRequests-1)-i));
			--gNbrFileRequests;
			--i;
			--ap;
		}
	}

	// Check if File has already been requested
	ap = gFileRequestPtr;
	for (i = 0; i < gNbrFileRequests; ++i,++ap) {
		if (EqualString(ap->name, name,false,false))
			return true;
	}

	// If List is full, fail
	if (gNbrFileRequests >= MaxFileRequests)	// 7/12 changed to >= from ==
		return true;
	// Add File to list
	ap = &gFileRequestPtr[gNbrFileRequests];
	memset(ap,0,sizeof(FReqRec));
	BlockMove(name, ap->name, name[0]+1);
	ap->timeStamp = GetTicks();
	ap->mediaType = mediaType;
	++gNbrFileRequests;
	return false;
}

FReqPtr	GetFileReqByName(StringPtr name)
{
	FReqPtr	ap;
	short	i;
	if (gFileRequestPtr == NULL || gNbrFileRequests == 0)
		return NULL;
	ap = gFileRequestPtr;
	for (i = 0; i < gNbrFileRequests; ++i,++ap) {
		// JAB 7/11/96 Make sure expected block is zero (in case same file was asked for twice)
		if (ap->expectedBlock == 0 && EqualString(ap->name,name,false,false)) {
			return ap;
		}
	}
	return NULL;
}

FReqPtr	GetFileReqByID(long id, short expectedBlock)
{
	FReqPtr	ap;
	short	i;
	if (gFileRequestPtr == NULL || gNbrFileRequests == 0)
		return NULL;
	ap = gFileRequestPtr;
	for (i = 0; i < gNbrFileRequests; ++i,++ap) {
		// 7/11/96 Make sure expected block matches
		if (ap->expectedBlock == expectedBlock && ap->transID == id) {
			return ap;
		}
	}
	return NULL;
}

// 7/17/95 Once file is received, remove it from the list
//
// 7/11/96 - use filerecptr itself, rather than filename to identify
//
void RegisterFileReception(FReqPtr	fp)
{
	FReqPtr	ap;
	short	i;
	if (gFileRequestPtr == NULL || gNbrFileRequests == 0)
		return;
	ap = gFileRequestPtr;
	for (i = 0; i < gNbrFileRequests; ++i,++ap) {
		if (ap == fp) {
			BlockMove((Ptr)&ap[1],(Ptr)ap,sizeof(FReqRec)*((gNbrFileRequests-1)-i));
			--gNbrFileRequests;
			return;
		}
	}
}

