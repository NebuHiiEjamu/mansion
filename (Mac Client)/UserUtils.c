// UserUtils.c
#include "U-USER.H"
#include <Files.h>

void ToggleDebug(void);

#if 0
StringPtr BuildPictureName(StringPtr pictureName)
{
	static char	picName[256];
	char		tempName[64];
	StringPtr	folderName;

	// remove path spec from filename
	BlockMove(pictureName,tempName,pictureName[0]+1);
	PtoCstr((StringPtr) tempName);
	if (strrchr(tempName,':') != NULL) {
		strcpy(tempName,strrchr(tempName,':')+1);
	}
	CtoPstr(tempName);

	folderName = (StringPtr) gPrefs.picFolder;
	BlockMove(folderName,picName,folderName[0]+1);
	BlockMove(&tempName[1],&picName[picName[0]+1],tempName[0]);
	picName[0] += tempName[0];
	return (StringPtr) picName;
}
#endif

/* 8/5/96 JAB - need to switch to new system that
   uses a STR# resource and index to build the
   name, and returns NULL if index is past the
   end...
   typical STR#  \A\S\P\F
	// \S = Server Name:
	// \F = Filename

	Todo: Make directories properly (may need to make more than one - check this)
	Test speed.  Test Failure.  Fix for Sound.

 */
StringPtr	BuildMediaFolderName(StringPtr pictureName, int catalogNumber, int indexNumber)
{
	static char	assetName[256];
	char		pathTemplate[64]="";
	char		*sp,*dp,*sp2,*dp2;

	// Remove path specifiations from filename
	GetIndString((StringPtr) pathTemplate, catalogNumber, indexNumber);
	if (pathTemplate[0] == 0)
		return NULL;
	PtoCstr((StringPtr) pathTemplate);

	dp = assetName;
	sp = pathTemplate;

	while (*sp) {
		if (*sp == '\\') {
			++sp;
			switch (*sp) {
			case 'S':
				BlockMove(gRoomWin->serverInfo.serverName, dp, gRoomWin->serverInfo.serverName[0]+1);
				PtoCstr((StringPtr) dp);
				for (sp2 = dp2 = dp; *sp2; ++sp2) {
					*dp2 = *sp2;
					if (isalnum(*sp2) || *sp2 == ' ' || *sp2 == '_')
						++dp2;
				}
				*dp2 = 0;
				if (strlen(dp) > 28) {
					dp[28] = ':';
					dp[29] = 0;
				}
				else if (dp[0]) {
					*(dp2++) = ':';
					*dp2 = 0;
				}
				dp += strlen(dp);
				break;
			case 'F':
				BlockMove(pictureName,dp,pictureName[0]+1);
				PtoCstr((StringPtr) dp);
				if (strrchr(dp,':') != NULL) {
					strcpy(dp,strrchr(dp,':')+1);
				}
				dp += strlen(dp);
				break;
			}
			++sp;
		}
		else {
			*(dp++) = *(sp++);
		}
	}
	*dp = 0;
	CtoPstr(assetName);

	// Append file name
	return (StringPtr) assetName;
}


void CreateServerDirectory(int catalogNumber, int indexNumber);	// Creates a directory for current server, if one doesn't exist

void CreateServerDirectory(int catalogNumber, int indexNumber)	// Creates a directory for current server, if one doesn't exist
{
	StringPtr	folderName;
	Str255		buildName;
	int			len,i;
	long		createdDirID;
	OSErr		oe;


	folderName = BuildMediaFolderName("\p",catalogNumber, indexNumber);

	if (folderName == NULL)
		return;
	if (folderName[0] == 0)
		return;

	// Build All directories leading up to it...
	BlockMove(folderName,buildName,folderName[0]+1);
	len = buildName[0];
	for (i = 0; i < len; ++i) {
		if (buildName[i+1] == ':' && i > 0) {
			buildName[0] = i;
			oe = DirCreate(0,0L,(StringPtr) buildName,&createdDirID);
		}
	}
}

#if 0
StringPtr	BuildServerFolderName(StringPtr pictureName);
StringPtr	BuildServerFolderName(StringPtr pictureName)
{
	static char	picName[256];
	char		tempName[64],servName[64];
	char		*sp,*dp;
	StringPtr	folderName;

	// Remove path specifiations from filename
	BlockMove(pictureName,tempName,pictureName[0]+1);
	PtoCstr((StringPtr) tempName);
	if (strrchr(tempName,':') != NULL) {
		strcpy(tempName,strrchr(tempName,':')+1);
	}
	CtoPstr(tempName);

	folderName = (StringPtr) gPrefs.picFolder;
	BlockMove(folderName,picName,folderName[0]+1);

	// Convert ServerName to something useful for directory purposes...
	BlockMove(gRoomWin->serverInfo.serverName, servName, gRoomWin->serverInfo.serverName[0]+1);
	PtoCstr((StringPtr) servName);
	
	for (sp = dp = servName; *sp; ++sp) {
		*dp = *sp;
		if (isalnum(*sp) || *sp == ' ' || *sp == '_')
			++dp;
	}
	*dp = 0;
	if (strlen(servName) > 28) {
		servName[28] = ':';
		servName[29] = 0;
	}
	else if (servName[0]) {
		*(dp++) = ':';
		*dp = 0;
	}
	CtoPstr(servName);

	// Append  :servername:
	BlockMove(&servName[1],&picName[picName[0]+1],servName[0]);
	picName[0] += servName[0];

	// Append file name
	BlockMove(&tempName[1],&picName[picName[0]+1],tempName[0]);
	picName[0] += tempName[0];
	return (StringPtr) picName;
}

#endif

PicHandle	GetPictureFromFile(StringPtr origName)
{
	OSErr		oe;
	FileHandle	refNum;
	PicHandle	pic = NULL;
	long		picSize;
	Boolean		isPICT;
	FInfo		fInfo;
	StringPtr	picName;
	int			n;
	// If user hit OK button
	// Open the file

	if (origName[0] == 0)
		return NULL;

	n = 0;
	do {
		++n;
		picName = BuildMediaFolderName(origName, PicturesPathList, n);
		if (picName) {
			oe = GetFInfo(picName, 0, &fInfo);
		}
	} while (picName && oe != noErr);

	if (picName == NULL || oe != noErr)
		return NULL;

	isPICT = (fInfo.fdType == 'PICT');

	oe = OpenFileReadOnly(picName,0,&refNum);
	// If file opened successfully
	if (oe == noErr) {
		// Figure out how big it is
		GetEOF(refNum,&picSize);
		// Subtract 512 - first 512 bytes are ignored
		if (isPICT)
			picSize -= 512;
		// Allocate handle that size
		/* 7/1/96 JAB Changed to NewHandleClear */
		pic = (PicHandle) NewHandleClear(picSize);
		if (pic) {
			// Skip first 512 bytes
			if (isPICT)
				SetFPos(refNum,fsFromStart,512L);
			// Load in rest of PicHandle
			FSRead(refNum,&picSize,*pic);
		}
		FSClose(refNum);
	}
	return pic;
}

Boolean AssetsAreReadOnly(short fRefNum)
{
	OSErr	oe;
	short	vRefNum;
	Boolean	readOnly = false;
	VolumeParam	pb;
	FCBPBRec	pbf;
	oe = GetVRefNum(fRefNum,&vRefNum);
	if (oe == noErr) {

		pbf.ioCompletion = NULL;
		pbf.ioFCBIndx = 0;
		pbf.ioVRefNum= vRefNum;
		pbf.ioRefNum = fRefNum;
		pbf.ioNamePtr = NULL;
		oe = PBGetFCBInfo(&pbf,false);
		if (oe == noErr && !(pbf.ioFCBFlags & 0x100)) {
			ReportMessage(RE_ReadOnlyAssets);
			readOnly = true;
			goto Done;
		}
		pb.ioCompletion = NULL;
		pb.ioVolIndex = 0;
		pb.ioNamePtr = NULL;
		pb.ioVRefNum = vRefNum;
		oe = PBGetVInfo((ParmBlkPtr) &pb,false);
		if (oe == noErr && (pb.ioVAtrb & 0x8080)) {
			ReportMessage(RE_ReadOnlyVolume);
			readOnly = true;
			goto Done;
		}
	}
Done:
	return readOnly;
}

#define ESCBYTE		6		// 0x35
#define ESCBIT		0x20
#define CMDBYTE		6
#define CMDBIT		0x80
#define PERIODBYTE	5
#define PERIODBIT	0x80

Boolean UserIsTryingToInterrupt()
{
	KeyMap	km;
	char	*kmc;
	
	if (gSuspended)
		return false;

	GetKeys(km);
	kmc = (char *) km;
	if ((kmc[ESCBYTE] & ESCBIT) ||
		((kmc[CMDBYTE] & CMDBIT) && (kmc[PERIODBYTE] & PERIODBIT))) {
		FlushEvents(keyDownMask | autoKeyMask | keyUpMask,0);
		return true;
	}
/**
	if (EventAvail(keyDownMask,&theEvent)) {
		switch (theEvent.message & charCodeMask) {
		case '\033':	// ESC
			return true;
		case '.':
		case 'q':
		case 'Q':
			if (theEvent.modifiers & cmdKey)
				return true;
		}		
	}
**/
	return false;
}

void ToggleDebug(void)
{
		gDebugFlag = !gDebugFlag;
		LogMessage("Debug %s\r",gDebugFlag? "on": "off");
		ShowRoomStatus();
}

