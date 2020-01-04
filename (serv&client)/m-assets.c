/* Assets.c		MOSTLY SHARED EXCEPT FILE I/O
 *
 *				Equivalent of Macintosh Resource Manager used for resources
 *				which will be identical across Mac and Windows.
 *
 *				Currently the only resources types in use are 'Prop' and 'Fave'
 *
 * These functions duplicate much of the functionality of the Apple Resource Manager
 * but allows for a system where all resources have a longword ID number, which will
 * needed on large palace systems with large numbers of props.
 *
 * once the file handling and memory management is made more portable, this
 * should also allow for a somewhat more portable resource management system
 */
/******************************************************************************************/
/******************************************************************************************/

#include "m-assets.h"

Boolean EqualPString(StringPtr inStr, StringPtr outStr, Boolean caseSens);

#if macintosh
void SpinCursor();
#endif

/******************************************************************************************/

short			gAssetError;      /* Last assets error */
AssetFileVars	*firstAssetFile;	/* first open resource file/map in linked list */
AssetFileVars	*curAssetFile;		/* current resource file/map */

/******************************************************************************************/

ErrNbr AssetError(void)
{
	return gAssetError;
}

/******************************************************************************************/

ErrNbr NewAssetFileRec(FileHandle refNum)
{
	AssetFileVars	*ar;

	gAssetError = 0;

	ar = (AssetFileVars *)NewPtrClear(sizeof(AssetFileVars));
	curAssetFile                = ar;
	ar->nextAssetFile           = firstAssetFile;
	firstAssetFile              = ar;
	ar->aRefNum                 = refNum;
	ar->afHeader.dataOffset     = sizeof(AssetFileHeader);
	ar->afHeader.dataSize       = 0L;
	ar->afHeader.assetMapOffset = sizeof(AssetFileHeader);
	ar->afHeader.assetMapSize   = sizeof(AssetMapHeader);
	ar->typeList                = NewHandleClear(0L);
	ar->assetList               = NewHandleClear(0L);
	ar->nameList                = NewHandleClear(0L);
#if unix
	ar->handleList				= NewHandleClear(64L*sizeof(void *));
	ar->nbrAllocHandles = 64;
#endif
	ar->gLastAssetType = -1;
	ar->gLastAssetNbr = -1;
	return noErr;
}

/******************************************************************************************/

ErrNbr LoadAssetMap(FileHandle refNum)
{
	LONG	          count;
	Handle	        afMap;
	AssetFileVars	 *ar = curAssetFile;
	AssetMapHeader *amh;
	AssetRec		huge *ap;
	LONG			i;
	
	gAssetError = 0;
	count = sizeof(AssetFileHeader);
	if ((gAssetError = FSRead(refNum,&count,(Ptr)&ar->afHeader)) != noErr)
		return gAssetError;

	afMap = NewHandleClear(ar->afHeader.assetMapSize);
	if (afMap == NULL)			{
		gAssetError = MemError();	/* 8/2/95 */
		if (gAssetError == noErr)
			gAssetError = -1;
		return gAssetError;
	}

	if ((gAssetError = SetFPos(refNum,fsFromStart,ar->afHeader.assetMapOffset)) != noErr)
		return gAssetError;
	count = ar->afHeader.assetMapSize;
	HLock(afMap);
	if ((gAssetError = FSRead(refNum,&count,*afMap)) != noErr)
		return gAssetError;
	amh = (AssetMapHeader *) *afMap;

	if (amh->nbrTypes < 0 ||
		amh->nbrAssets < 0 ||
		amh->lenNames < 0)
		return gAssetError;

  SetHandleSize(ar->typeList ,sizeof(AssetTypeRec)*amh->nbrTypes);
  if (ar->typeList == NULL || (gAssetError = MemError()) != noErr)	/* 8/2/95 */
	return gAssetError;
  SetHandleSize(ar->assetList,sizeof(AssetRec)*amh->nbrAssets);
  if (ar->assetList == NULL || (gAssetError = MemError()) != noErr)	/* 8/2/95 */
	return gAssetError;
  SetHandleSize(ar->nameList ,amh->lenNames);
  if (ar->nameList == NULL || (gAssetError = MemError()) != noErr)	/* 8/2/95 */
	return gAssetError;

  BlockMove((Ptr)*afMap,(Ptr)&ar->afMap,sizeof(AssetMapHeader));
	/* boom on the following line */

  BlockMove((*afMap + amh->typesOffset),*ar->typeList,sizeof(AssetTypeRec)*amh->nbrTypes);
  BlockMove((*afMap + amh->recsOffset) ,*ar->assetList,sizeof(AssetRec)*amh->nbrAssets);
  BlockMove((*afMap + amh->namesOffset),*ar->nameList,amh->lenNames);

  /* 12/11/95 clear asset flags */
  ap = (AssetRec huge *) *ar->assetList;
  for (i = 0; i < ar->afMap.nbrAssets; ++i,++ap)
		ap->flags &= ~(AssetLoaded | AssetModified | AssetInTempFile);


	HUnlock(afMap);
	DisposeHandle(afMap);

	return noErr;
}

/******************************************************************************************/

ErrNbr OpenAssetFile(FSSpec *fsFile,FileHandle *refNum)
{
	if ((gAssetError = FSpOpenDF(fsFile,fsRdWrPerm,refNum)) != noErr)
		return gAssetError;
	if ((gAssetError = NewAssetFileRec(*refNum)) != noErr)
		return gAssetError;
	curAssetFile->fsSpec = *fsFile;
	if ((gAssetError = LoadAssetMap(*refNum)) != noErr)
		return gAssetError;

#if unix
	SortAssetsByUsage();  /* 3/4/96 optimization */
#endif

	return noErr;
}

/******************************************************************************************/

ErrNbr NewAssetFile(FSSpec *fsFile, FileHandle *refNum)
{
	gAssetError = 0;
	FSpDelete(fsFile);

	if ((gAssetError = FSpCreate(fsFile,ASSET_CREATOR,ASSET_FILETYPE,0L)) != noErr)
		return gAssetError;
	if ((gAssetError = FSpOpenDF(fsFile,fsRdWrPerm,refNum)) != noErr)
		return gAssetError;
	if ((gAssetError = NewAssetFileRec(*refNum)) != noErr)
		return gAssetError;
	curAssetFile->fsSpec = *fsFile;
	curAssetFile->fileNeedsUpdate = true;
	if ((gAssetError = UpdateAssetFile()) != noErr)
		return gAssetError;
	return noErr;
}

ErrNbr UseTempAssetFile(void)	/* 7/2/96 Request to use temporary file for output */
{
	AssetFileVars	*ar = curAssetFile;
	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}
	if (ar->tempRefNum != 0) {
		/* Already Open */
		gAssetError = -1;
		return gAssetError;
	}
	ar->tempDataSize = 0;
					/* 7/2/96 Locally implemented function */
					/* creates a temp file for r/w access using
					  fsSpec for seed name */
					/* be sure not to overwrite existing file */
	ar->tempFSSpec = ar->fsSpec;
	ar->tempRefNum = CreateAndOpenTempFile(&ar->tempFSSpec);
	if (ar->tempRefNum == 0) {
		gAssetError = -1;
		return gAssetError;
	}
	return noErr;
}

#if macintosh

/* Keep tweaking filename until it's UNIQUE */
void InsureNameIsUnique(FSSpec *tSpec);
void InsureNameIsUnique(FSSpec *tSpec)
{
	FInfo			fInfo;
	short			MyRandom(short max);
	while (HGetFInfo(tSpec->vRefNum,
					 tSpec->parID,
					 tSpec->name, &fInfo) == noErr) {
		tSpec->name[1+MyRandom(tSpec->name[0])] = 'a'+MyRandom(26);
	}
}


FileHandle CreateAndOpenTempFile(FSSpec *tempSpec)
{
	short	tempRefNum;

	if (tempSpec->name[0] == 0) {
		strcpy((char *) tempSpec->name,"TEMPFILE");
		CtoPstr((char *) tempSpec->name);
	}
	/* create a unique filename spec based on the seed name */

	/* locate it in the temporary folder on the mac */
	/* so that it gets automatically deleted if we crash before
	   getting the chance */
	FindFolder(tempSpec->vRefNum,kTemporaryFolderType,kCreateFolder,
					&tempSpec->vRefNum,&tempSpec->parID);
	/* make sure the filename is unique */
	InsureNameIsUnique(tempSpec);

	/* create the file */
	if ((gAssetError = FSpCreate(tempSpec,ASSET_CREATOR,ASSET_FILETYPE,0L)) != noErr)
		return 0;
	/* open it */
	if ((gAssetError = FSpOpenDF(tempSpec,fsRdWrPerm,&tempRefNum)) != noErr)
		return 0;

	return tempRefNum;
}

#else

FileHandle CreateAndOpenTempFile(FSSpec *tempSpec)
{
	FileHandle	tempRefNum = 0;
	strcpy((char *) tempSpec->name,"TEMP2.PRP");
	CtoPstr((char *) tempSpec->name);
	FSpDelete(tempSpec);
	if ((gAssetError = FSpCreate(tempSpec,ASSET_CREATOR,ASSET_FILETYPE,0L)) != noErr)
		return gAssetError;
	if ((gAssetError = FSpOpenDF(tempSpec,fsRdWrPerm,&tempRefNum)) != noErr)
		return gAssetError;
	return tempRefNum;
}

#endif

/******************************************************************************************/

ErrNbr WriteAsset(AssetRec *ap)
{
	AssetFileVars	*ar = curAssetFile;
	LONG	count;
	char	hState;
	FileHandle	oRefNum = curAssetFile->aRefNum;
	Handle	rHandle;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}
	ap->flags &= ~(AssetModified | AssetInTempFile);
	count = ap->dataSize;
	rHandle = GetAssetHandle(ap->rHandle); 
	if (rHandle == NULL)
	{
		gAssetError = -1;
		return gAssetError;
	}
	hState = HGetState(rHandle);
	HLock(rHandle);
	if (ar->tempRefNum && !ar->isClosing) {
		/* 7/2/96 write into temp file, if one is open */
		ap->dataOffset = ar->tempDataSize;
		if ((gAssetError = SetFPos(ar->tempRefNum,fsFromStart,ar->tempDataSize)) != noErr)
			return gAssetError;
		if ((gAssetError = FSWrite(ar->tempRefNum,&count,*rHandle)) != noErr)
			return gAssetError;
		ar->tempDataSize += ap->dataSize;
		ap->flags |= AssetInTempFile;
	}
	else {
		ap->dataOffset = ar->afHeader.dataSize;
		if ((gAssetError = SetFPos(oRefNum,fsFromStart,ap->dataOffset + ar->afHeader.dataOffset)) != noErr)
			return gAssetError;
		if ((gAssetError = FSWrite(oRefNum,&count,*rHandle)) != noErr)
			return gAssetError;
		ar->afHeader.dataSize += ap->dataSize;
	}
	HSetState(rHandle,hState);
	return noErr;
}

/******************************************************************************************/

ErrNbr	WriteAssetMap(FileHandle oRefNum)
{
	AssetFileVars		*ar = curAssetFile;
	LONG			count;
	/* AssetRec	huge	*ap; */
	/* Handle			tempAssetList;
	 * LONG			i;
	 */
	 
	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}

	CompressAssetNames();	/* Compress out discarded names... */

	/* Write Asset Map */
	ar->afMap.typesOffset = sizeof(AssetMapHeader);
	ar->afMap.recsOffset = ar->afMap.typesOffset + ar->afMap.nbrTypes*sizeof(AssetTypeRec);
	ar->afMap.namesOffset = ar->afMap.recsOffset + ar->afMap.nbrAssets*sizeof(AssetRec);

	ar->afHeader.assetMapOffset = ar->afHeader.dataOffset + ar->afHeader.dataSize;
	ar->afHeader.assetMapSize = ar->afMap.namesOffset + ar->afMap.lenNames;

	if ((gAssetError = SetFPos(oRefNum,fsFromStart,ar->afHeader.assetMapOffset)) != noErr)
		return gAssetError;

	count = sizeof(AssetMapHeader);
	gAssetError = FSWrite(oRefNum,&count,(Ptr)&ar->afMap);
    if (gAssetError != noErr)
		return gAssetError;
	HLock(ar->typeList);
	count = ar->afMap.nbrTypes*sizeof(AssetTypeRec);
	if ((gAssetError = FSWrite(oRefNum,&count,*ar->typeList)) != noErr)
		return gAssetError;
	HUnlock(ar->typeList);

	/* Make copy of AssetList, so we can save with correct flags */
	/* 12/11/95 Save with incorrect flags to avoid huge memory allocation */


	/* 12/11/95 Removed */
	/* Write Data for Each Asset, (clear modified flag first) */
	/* tempAssetList = ar->assetList;
	 * HandToHand(&tempAssetList);
	 * ap = (AssetRec huge*) *tempAssetList;
	 *
	 * for (i = 0; i < ar->afMap.nbrAssets; ++i,++ap)
	 * 	ap->flags &= ~AssetLoaded;
	 */
	/* HLock(tempAssetList);
	 * count = ar->afMap.nbrAssets*sizeof(AssetRec);
	 * if ((gAssetError = FSWrite(oRefNum,&count,*tempAssetList)) != noErr)
	 * 	return gAssetError;
	 * HUnlock(tempAssetList);
	 * DisposeHandle(tempAssetList);
	 */
	HLock(ar->assetList);
	count = ar->afMap.nbrAssets*sizeof(AssetRec);
	if ((gAssetError = FSWrite(oRefNum,&count,*ar->assetList)) != noErr)
		return gAssetError;
	HUnlock(ar->assetList);

	HLock(ar->nameList);
	count = ar->afMap.lenNames;
	if ((gAssetError = FSWrite(oRefNum,&count,*ar->nameList)) != noErr)
		return gAssetError;
	HUnlock(ar->nameList);

	/* Update Header */
	if ((gAssetError = SetFPos(oRefNum,fsFromStart,0L)) != noErr)
		return gAssetError;
	count = sizeof(AssetFileHeader);
	if ((gAssetError = FSWrite(oRefNum,&count,(Ptr)&ar->afHeader)) != noErr)
		return gAssetError;
#if macintosh			/* 6/24/95 Flush the disc cache to reduce chances of corruption */
	FlushVol(NULL, ar->fsSpec.vRefNum);
#endif
#if unix
	FlushFile(oRefNum);
#endif
	return noErr;
}

/******************************************************************************************/


/* Write out entire Asset File from Scratch (this compresses out old assets)
 * 6/24/95 Modified to write file in place, so that file refNum isn't changed
 *
 * 7/10/95 Fixed bug which caused file corruption if any assets were currently loaded
 */
ErrNbr	WriteAssetFile(void)
{
	AssetFileVars		*ar = curAssetFile;
	AssetRec	huge	*ap;
	FileHandle			astRefNum = curAssetFile->aRefNum,tempRefNum;
	LONG				count;
	LONG				i;
	Handle				th;
	char				hState=0;
#ifdef macintosh
	FSSpec				tempSpec = {0,0L,"\pTEMP.AST"};
#else
#ifdef unix
	FSSpec				tempSpec = {0,0L,"\x08TEMP.AST"};
#else
	FSSpec				tempSpec = {0,0L,"TEMP.AST"};
#endif
#endif

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}

	/* Open a temporary file for writing */
#ifdef macintosh
	/* !!! This realy should be done on both platforms!   */
	InsureNameIsUnique(&tempSpec);
#else
	FSpDelete(&tempSpec);
#endif
	if ((gAssetError = FSpCreate(&tempSpec,ASSET_CREATOR,ASSET_FILETYPE,0L)) != noErr)
		return gAssetError;
	if ((gAssetError = FSpOpenDF(&tempSpec,fsRdWrPerm,&tempRefNum)) != noErr)
		return gAssetError;

	/* Write Header */
	count = sizeof(AssetFileHeader);
	if ((gAssetError = FSWrite(tempRefNum,&count,(Ptr)&ar->afHeader)) != noErr)
		return gAssetError;
	ar->afHeader.dataSize = 0L;
	ar->afHeader.dataOffset = sizeof(AssetFileHeader);

	HLock(ar->assetList);
	ap = (AssetRec huge *) *ar->assetList;

	/* Write Data for Each Asset, (clear modified flag first) */
	for (i = 0; i < ar->afMap.nbrAssets; ++i,++ap) {
#if macintosh
		SpinCursor();
#endif
		count = ap->dataSize;

		if (ap->flags & AssetLoaded) {
			/* no no no - don't write into old file!  7/10/95
			 *
			 * if ((gAssetError = WriteAsset(ap)) != noErr)
			 *	return gAssetError;
			 */
			th = GetAssetHandle(ap->rHandle);
			hState = HGetState(th);
			HLock(th);
		}
		else {
			short	fileRefNum;
			LONG	fileOffset;

			/*  7/2/96 Read from Temp File if necessary */
			if (ap->flags & AssetInTempFile) {
				fileRefNum = ar->tempRefNum;
				fileOffset = 0L;
			}
			else {
				fileRefNum = astRefNum;
				fileOffset = ar->afHeader.dataOffset;
			}
			SetFPos(fileRefNum,fsFromStart,ap->dataOffset+fileOffset);
			th = NewHandleClear(ap->dataSize); /* 7/1/96 JAB Changed to NewHandleClear */
			if (th == NULL) {
				if ((gAssetError = MemError()) != noErr)	/* 8/2/95 */
					return gAssetError;
				else
					return -1;
			}
			HLock(th);
			if ((gAssetError = FSRead(fileRefNum,&count,*th)) != noErr)
				return gAssetError;
		}
		/* clear temp file flag */
		ap->flags &= ~AssetInTempFile;

		ap->dataOffset = ar->afHeader.dataSize;
		if ((gAssetError = FSWrite(tempRefNum,&count,*th)) != noErr)
			return gAssetError;
		ar->afHeader.dataSize += ap->dataSize;

		
		/* 7/10/95 Only delete handle if asset isn't loaded, otherwise, unlock it */
		if (!(ap->flags & AssetLoaded))
			DisposeHandle(th);
		else
			HSetState(th,hState);
	}
	HUnlock(ar->assetList);
	/* 6/24/95 NEW NEW NEW
	 * Copy contents of Temp File (tempRefNum) into Our File (astRefNum)
	 */
	{
#define BUFSIZE	32000L
		LONG	amt = ar->afHeader.dataSize + ar->afHeader.dataOffset;
		LONG	count;
		Ptr		tbuf;		
		SetFPos(tempRefNum,fsFromStart,0L);
		SetFPos(astRefNum,fsFromStart,0L);
		tbuf = NewPtrClear(BUFSIZE);	/* 7/1/95 changed to NewPtrClear */
		while (amt) {
#if Macintosh
			SpinCursor();
#endif
			if (amt > BUFSIZE)
				count = BUFSIZE;
			else
				count = amt;
			if ((gAssetError = FSRead(tempRefNum,&count,tbuf)) != noErr)
				return gAssetError;
			if ((gAssetError = FSWrite(astRefNum,&count,tbuf)) != noErr)
				return gAssetError;
			amt -= count;
		}
	}
	FSClose(tempRefNum);
	FSpDelete(&tempSpec);
	
	if ((gAssetError = WriteAssetMap(astRefNum)) != noErr)
		return gAssetError;

	/* Reduce length of file... */
	SetEOF(astRefNum,ar->afHeader.assetMapOffset + ar->afHeader.assetMapSize);

	/* 7/2/96 reduce size of temp file */
	if (ar->tempRefNum) {
		ar->tempDataSize = 0L;
		SetEOF(ar->tempRefNum,0L);
	}

#if macintosh			/* 6/24/95 Flush the disc cache to reduce chances of corruption */
	FlushVol(NULL, ar->fsSpec.vRefNum);
#endif
#if unix
	FlushFile(astRefNum);
#endif
	ar->fileNeedsUpdate = false;
	return noErr;
}

/******************************************************************************************/
/* Write out changed assets, and new asset map & header
 * More Efficient, but will leave deleted assets in file
 */
ErrNbr	UpdateAssetFile(void)
{
	FileHandle			oRefNum = curAssetFile->aRefNum;
	AssetFileVars	*ar = curAssetFile;
	AssetRec	huge	*ap;
	LONG			count;
	LONG			i;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}

	if (!(ar->fileNeedsUpdate || (ar->isClosing && ar->tempDataSize)))
		return noErr;

	/* Update Header */
	if ((gAssetError = SetFPos(oRefNum,fsFromStart,0L)) != noErr)
		return gAssetError;
	count = sizeof(AssetFileHeader);
	if ((gAssetError = FSWrite(oRefNum,&count,(Ptr)&ar->afHeader)) != noErr)
		return gAssetError;
	if ((gAssetError = SetFPos(oRefNum,fsFromStart,ar->afHeader.dataOffset+ar->afHeader.dataSize)) != noErr)
		return gAssetError;
	/* Write Data for Each Asset, (clear modified flag first) */
	HLock(ar->assetList);
	ap = (AssetRec huge*) *ar->assetList;
	for (i = 0; i < ar->afMap.nbrAssets; ++i,++ap) {
		if (ar->isClosing && (ap->flags & AssetInTempFile)) {
			/* if using temp file, we need to transfer assets over */
			/* Load it in, if it's not in */
			LoadAsset(ap);
			ap->flags |= AssetModified;
		}
		if (ap->flags & AssetModified) {
			if ((gAssetError = WriteAsset(ap)) != noErr)
				return gAssetError;
		}
	}
	HUnlock(ar->assetList);

	/* 7/2/96 don't write asset map if using temp file (until we close the file) */
	
	if (ar->tempRefNum == 0 || ar->isClosing) {
		if ((gAssetError = WriteAssetMap(oRefNum)) != noErr)
			return gAssetError;
	}
	ar->fileNeedsUpdate = false;
	return noErr;
}

/******************************************************************************************/

FileHandle	CurAssetFile(void)
{
	gAssetError = 0;
	if (curAssetFile)
		return curAssetFile->aRefNum;
	else
		return 0;
}

/******************************************************************************************/

ErrNbr UseAssetFile(FileHandle refNum)
{
	AssetFileVars	*ar;

	gAssetError = 0;

	ar = firstAssetFile;
	while (ar && ar->aRefNum != refNum)
		ar = ar->nextAssetFile;
	if (ar) {
		curAssetFile = ar;
		return noErr;
	}
	else {
		gAssetError = -1;
		return gAssetError;
	}
}

/******************************************************************************************/

ErrNbr CloseAssetFile(FileHandle refNum)
{
	AssetFileVars	*ar,*tr;
	AssetRec	huge  	*ap;
	FileHandle  sr;
	LONG			 i;	/* 11/24/95 */

	gAssetError = 0;
	if (curAssetFile == NULL)
	    return gAssetError = -1;
	if (refNum == curAssetFile->aRefNum)
    {
		if (curAssetFile->nextAssetFile)
			sr = curAssetFile->nextAssetFile->aRefNum;
		else
			sr = 0;
	}
	else
	{
		sr = CurAssetFile();
		UseAssetFile(refNum);
	}
	ar = curAssetFile;
	ar->isClosing = true;
	UpdateAssetFile();
	FSClose(ar->aRefNum);
	/* 7/2/96 delete the temporary file if it exists */
	if (ar->tempRefNum) {
		FSClose(ar->tempRefNum);
		FSpDelete(&ar->tempFSSpec);
	}
	if (ar == firstAssetFile)
		firstAssetFile = ar->nextAssetFile;
	else
  {
		tr = firstAssetFile;
		while (tr && tr->nextAssetFile != ar)
			tr = tr->nextAssetFile;
		tr->nextAssetFile = ar->nextAssetFile;
	}
	/* !! Dispose of Handles */
	HLock(ar->assetList);
	ap = (AssetRec huge *) *ar->assetList;
	for (i = 0; i < ar->afMap.nbrAssets; ++i,++ap) {
		if (ap->flags & AssetLoaded) {
			Handle	th;
			th = GetAssetHandle(ap->rHandle);
			if (th)
				DisposeHandle(th);
		}
	}
	HUnlock(ar->assetList);
	DisposeHandle(ar->typeList);
	DisposeHandle(ar->assetList);
	DisposeHandle(ar->nameList);
#if unix
	DisposeHandle(ar->handleList);
#endif
	DisposePtr((Ptr) ar);
	if (sr)
		UseAssetFile(sr);
	return noErr;
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

AssetRec	*FindAssetRec(AssetType type, LONG id)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at;
	AssetRec	huge	*ap;
	LONG			i,j,nbrTypes,nbrAssets;
	at = (AssetTypeRec *) *ar->typeList;
	nbrTypes = ar->afMap.nbrTypes;
	for (i = 0; i < nbrTypes; ++i,++at) {
	if (at->assetType == type) 		/* esr - 9/8/95 removed unused swap */
    {
			ap = (AssetRec huge *) *ar->assetList;
			ap += at->firstAsset;
			nbrAssets = at->nbrAssets;
			for (j = 0; j < nbrAssets; ++j,++ap) {
				if (ap->idNbr == id) {
					ar->gLastAssetType = i;
					ar->gLastAssetNbr = j + at->firstAsset;
					return ap;
				}
			}
			break;
		}
	}
	return NULL;
}

/******************************************************************************************/

LONG UniqueAssetID(AssetType type, LONG minID)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at;
	AssetRec	huge	*ap;
	LONG			i,j,nbrTypes,uniqueID,nbrAssets;
	Boolean			uniqueFlag;

	at = (AssetTypeRec *) *ar->typeList;
	nbrTypes = ar->afMap.nbrTypes;
	uniqueID = minID;
	for (i = 0; i < nbrTypes; ++i,++at) {
		if (at->assetType == type) {
			do {
				uniqueFlag = true;
				ap = (AssetRec huge *) *ar->assetList;
				ap += at->firstAsset;	
				nbrAssets = at->nbrAssets;
				for (j = 0; j < nbrAssets; ++j,++ap) {
					if (ap->idNbr == uniqueID) {
						uniqueFlag = false;
						++uniqueID;
						break;
					}
				}
			} while (!uniqueFlag);
			break;
		}
	}
	return uniqueID;
}

/******************************************************************************************/
/**
void CheckAssetFlags(char *file, long line);

void CheckAssetFlags(char *file, long line)
{
	AssetFileVars	*ar = curAssetFile;
	AssetRec		*ap;
	LONG			n;
	if (ar == NULL)
		return;
	ap = (AssetRec huge*) *ar->assetList;
	for (n = 0; n < ar->afMap.nbrAssets; ++n,++ap) {
		if (ap->idNbr != 128 && (ap->idNbr < 500 || ap->idNbr > 515) &&
			(ap->flags & (AssetLoaded)) != 0) {
			Debugger();
		}
	}
}

**/

AssetRec	*FindAssetRecByIndex(AssetType type, LONG idx)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at;
	AssetRec	huge	*ap;
	LONG			i,nbrAssets,nbrTypes;
	at = (AssetTypeRec *) *ar->typeList;
	nbrTypes = ar->afMap.nbrTypes;
	for (i = 0; i < nbrTypes; ++i,++at) {
		if (at->assetType == type) {
			ap = (AssetRec huge*) *ar->assetList;
			ap += at->firstAsset;	
			nbrAssets = at->nbrAssets;
			if (idx >= 0 && idx < nbrAssets) {
				return ap + idx;
			}
			else
				return NULL;
			break;
		}
	}
	return NULL;
}

/******************************************************************************************/

/* 7/2/96 New Single Entry point for loading assets */

Handle LoadAsset(AssetRec huge *ap)
{
	AssetFileVars	*ar = curAssetFile;
	Handle			h=NULL;
	LONG			count;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return NULL;
	}

	if (ap->flags & AssetLoaded)
		return GetAssetHandle(ap->rHandle);

	HLock(ar->typeList);
	HLock(ar->assetList);
	h = NewHandleClear(ap->dataSize); /* 7/1/96 JAB Changed to NewHandleClear */
	if (h == NULL) {
		gAssetError = MemError();	/* 8/2/95 */
		return NULL;
	}
	HUnlock(ar->typeList);
	HUnlock(ar->assetList);
	HLock(h);
	if ((ap->flags & AssetInTempFile) && ar->tempRefNum != 0) {
		/* 7/2/96 Load asset from temp file if need be */
		SetFPos(ar->tempRefNum,fsFromStart,ap->dataOffset);
		count = ap->dataSize;
		FSRead(ar->tempRefNum,&count,*h);
	}
	else {
		SetFPos(ar->aRefNum,fsFromStart,ap->dataOffset+ar->afHeader.dataOffset);
		count = ap->dataSize;
		FSRead(ar->aRefNum,&count,*h);
	}
	HUnlock(h);

	HLock(ar->assetList);
	ap->rHandle = AddAssetHandle(h);  /* 7/4/96 */
	HUnlock(ar->assetList);

	ap->flags |= AssetLoaded;
	return h;
}

Handle	GetAsset(AssetType type, LONG id)
{
	AssetFileVars	*ar = curAssetFile;
	AssetRec	huge	*ap;
	Handle			h;
	unsigned LONG	t;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return NULL;
	}
	ap = FindAssetRec(type,id);
	if (ap == NULL) {
		gAssetError = resNotFound;
		return NULL;
	}
	GetDateTime(&t);
	ap->lastUseTime = t;
	h = LoadAsset(ap);
	return h;
}

/******************************************************************************************/

ErrNbr	ChangedAsset(Handle h)
{
	AssetFileVars	*ar = curAssetFile;
	AssetRec	huge *ap = (AssetRec huge *) *curAssetFile->assetList;
	LONG	i,nbrAssets = curAssetFile->afMap.nbrAssets;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}
	for (i = 0; i < nbrAssets; ++i,++ap) {
		if ((ap->flags & AssetLoaded) && GetAssetHandle(ap->rHandle)  == h) {
			ap->flags |= AssetModified;
			ap->dataSize = GetHandleSize(h);
			/* ap->crc = ComputeHandleCRC(ap->rHandle,ap->dataSize);  6/9/95 */
			ar->fileNeedsUpdate = true;
		}
	}
	return noErr;
}

/******************************************************************************************/

Boolean FindAssetByHandle(Handle rHandle, AssetTypeRec **att, AssetRec **app)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at,*fat;
	AssetRec	huge	*ap;
	LONG				i,nbrAssets;

	ap = (AssetRec huge *) *ar->assetList;
	at = (AssetTypeRec *) *ar->typeList;

	/* 3/5/95 optimization - check last loaded asset */
	if (ar->gLastAssetType != -1) {
		AssetRec	huge	*tap;
		AssetTypeRec huge	*tat;
		if (ar->gLastAssetType < ar->afMap.nbrTypes &&
			ar->gLastAssetNbr < ar->afMap.nbrAssets) {
			tat = at + ar->gLastAssetType;
			tap = ap + ar->gLastAssetNbr;
			if ((tap->flags & AssetLoaded) && GetAssetHandle(tap->rHandle) == rHandle) {
				*att = tat;
				*app = tap;
				return true;
			}
		}
	}
	nbrAssets = ar->afMap.nbrAssets;
	fat = at;
	for (i = 0; i < nbrAssets; ++i,++ap) {
		if (i == at->firstAsset) {
			fat = at;
			++at;
		}
		if ((ap->flags & AssetLoaded) && GetAssetHandle(ap->rHandle) == rHandle) {
			*att = fat;
			*app = ap;
			return true;
		}
	}
	return false;
}

/******************************************************************************************/

Boolean AssetExists(AssetType type, LONG id)
{
	return FindAssetRec(type,id) != NULL;
}

Boolean AssetExistsWithCRC(AssetType type, LONG id, unsigned LONG crc)
{
	return FindAssetRecWithCRC(type,id,crc) != NULL;
}

/******************************************************************************************/

ErrNbr	GetAssetInfo(Handle h, AssetType *type, LONG *id, StringPtr name)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at;
	AssetRec	huge 	*ap;
	StringPtr		as;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}
	if (!FindAssetByHandle(h,&at,&ap)) {
		gAssetError = resNotFound;
		return gAssetError;
	}
	*type = at->assetType;
	*id = ap->idNbr;
	if (ap->nameOffset != -1) {
		as = (StringPtr) (*ar->nameList + ap->nameOffset);
		if (as[0] > 0 && as[0] < 32)
			BlockMove(as,name,as[0]+1);
		else
			name[0] = 0;
	}
	else
		name[0] = 0;
	return noErr;
}

void CompressAssetNames()
{
	AssetFileVars	*ar = curAssetFile;
	AssetRec	huge 	*ap;
	Ptr				newNamePtr;
	Handle			newNameSpace;
	LONG			i;  /* 11/24/95 changed from short */
	LONG			lenNewNames;
	StringPtr		name;

	newNameSpace = NewHandleClear(ar->afMap.lenNames);
	if (newNameSpace == NULL) {
		gAssetError = MemError();
		return;	/* not enough memory to compress names */
	}

	newNamePtr = *newNameSpace;

	/* Write Name for Each Asset */
	lenNewNames = 0;
	ap = (AssetRec huge  *) *ar->assetList;
	for (i = 0; i < ar->afMap.nbrAssets; ++i,++ap) {
		if (ap->nameOffset != -1) {
			name = (StringPtr) (*ar->nameList + ap->nameOffset);
			BlockMove(name, newNamePtr, name[0]+1);
			ap->nameOffset = lenNewNames;
			lenNewNames += name[0]+1;
			newNamePtr += name[0]+1;
		}
	}
	SetHandleSize(newNameSpace, lenNewNames);
	DisposeHandle(ar->nameList);
	ar->afMap.lenNames = lenNewNames;
	ar->nameList = newNameSpace;
}

/******************************************************************************************/
/* Fix to allow changes of type */
ErrNbr	SetAssetInfo(Handle h, LONG id, StringPtr name)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at;
	AssetRec	 huge *ap;
	StringPtr		as;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}
	if (!FindAssetByHandle(h,&at,&ap)) {
		gAssetError = resNotFound;
		return gAssetError;
	}
	ap->idNbr = id;
	if (name[0]) {
		if (ap->nameOffset != -1) {
			as = (StringPtr) (*ar->nameList + ap->nameOffset);
			if (as[0] >= name[0]) {
				BlockMove(name,as,name[0]+1);
			}
			else {
				/* New String */
				SetHandleSize(ar->nameList,ar->afMap.lenNames+name[0]+1);
				ap->nameOffset = ar->afMap.lenNames;
				as = (StringPtr) (*ar->nameList + ap->nameOffset);
				BlockMove(name,as,name[0]+1);
				ar->afMap.lenNames += name[0]+1;
			}
		}
		else {
			/* New String */
			SetHandleSize(ar->nameList,ar->afMap.lenNames+name[0]+1);
			ap->nameOffset = ar->afMap.lenNames;
			as = (StringPtr) (*ar->nameList + ap->nameOffset);
			BlockMove(name,as,name[0]+1);
			ar->afMap.lenNames += name[0]+1;
		}
	}
	else
		ap->nameOffset = -1;
	ar->fileNeedsUpdate = true;
	return noErr;
}

/******************************************************************************************/

LONG	CountAssets(AssetType type)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at;
	LONG			i,nbrTypes;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}
	at = (AssetTypeRec *) *ar->typeList;
	nbrTypes = ar->afMap.nbrTypes;
	for (i = 0; i < nbrTypes; ++i,++at) {
		if (at->assetType == type) {
			return at->nbrAssets;
		}
	}
	return 0L;
}

/******************************************************************************************/

Handle	GetIndAsset(AssetType type, LONG nbr)
{
	AssetFileVars	*ar = curAssetFile;
	AssetRec	huge 	*ap;
	Handle			h;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return NULL;
	}
	ap = FindAssetRecByIndex(type,nbr);
	if (ap == NULL) {
		gAssetError = resNotFound;
		return NULL;
	}
	h = LoadAsset(ap);
	return h;
}

/******************************************************************************************/

LONG	CountAssetTypes()
{
	AssetFileVars	*ar = curAssetFile;
	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}
	return ar->afMap.nbrTypes;
}

/******************************************************************************************/

AssetType GetIndAssetType(LONG nbr)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at;
	LONG			i,nbrTypes;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}
	at = (AssetTypeRec *) *ar->typeList;
	nbrTypes = ar->afMap.nbrTypes;
	for (i = 0; i < nbrTypes; ++i,++at) {
		if (i == nbr)
			return at->assetType;
	}
	return 0L;
}

/******************************************************************************************/

ErrNbr	AddAsset(Handle h, AssetType type, LONG id, StringPtr name)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at;
	AssetRec	huge 	*ap;
	LONG			i,nbrTypes,typeNbr;
	unsigned LONG	t,curNbr,desNbr;
	StringPtr		as;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}

	/* Find if Type already exists */
	at = (AssetTypeRec *) *ar->typeList;
	nbrTypes = ar->afMap.nbrTypes;
	for (i = 0; i < nbrTypes; ++i,++at) {
		if (at->assetType == type)
			break;
	}
	typeNbr = i;

	/* If not, add & initialize new type record */
	if (i >= nbrTypes) {
		ar->afMap.nbrTypes++;
		SetHandleSize(ar->typeList,ar->afMap.nbrTypes * sizeof(AssetTypeRec));
		at = (AssetTypeRec *) *ar->typeList;
		at += ar->afMap.nbrTypes-1;
		at->assetType = type;
		at->nbrAssets = 0;
		at->firstAsset = ar->afMap.nbrAssets;
	}
	HLock(ar->typeList);

	/* Add new Resource Record */
	++ar->afMap.nbrAssets;
	SetHandleSize(ar->assetList,ar->afMap.nbrAssets * sizeof(AssetRec));
	ap = (AssetRec huge *) *ar->assetList;
	ap += ar->afMap.nbrAssets-1;
	HLock(ar->assetList);

	/* Initialize it */
	ap->idNbr = id;
	ap->rHandle = AddAssetHandle(h);
	ap->dataOffset = 0;
	ap->flags = AssetLoaded | AssetModified;
	ap->dataSize = GetHandleSize(h);
	GetDateTime(&t);
	ap->lastUseTime = t;
	/* ap->crc = ComputeHandleCRC(h,ap->dataSize);	JAB 6/9/95 */
	if (name && name[0]) {
		/* New String */
		SetHandleSize(ar->nameList,ar->afMap.lenNames+name[0]+1);
		ap->nameOffset = ar->afMap.lenNames;
		as = (StringPtr) (*ar->nameList + ap->nameOffset);
		BlockMove(name,as,name[0]+1);
		ar->afMap.lenNames += name[0]+1;
	}
	else
		ap->nameOffset = -1L;
	/* ap->reserved = 0L; */

	/* Swap it into place, updating relevant type records (firstAsset) field */
	curNbr = ar->afMap.nbrAssets-1;
	desNbr = at->firstAsset;
	/* desNbr = at->firstAsset + at->nbrAssets; 3/4/96 optmization */
	if (curNbr != desNbr) {
		AssetRec	huge *app,tempRec;
		tempRec = *ap;
		app = ap;
		while (curNbr > desNbr) {
			*app = *(app-1);
			--app;
			--curNbr;
		}
		*app = tempRec;
	}

	/* Update Type Record */
	at->nbrAssets++;

	/* Update firstAsset address of remaining type records */
	++typeNbr;
	++at;
	while (typeNbr < nbrTypes) {
		at->firstAsset++;
		++at;
		++typeNbr;
	}
	HUnlock(ar->typeList);
	HUnlock(ar->assetList);
	ar->fileNeedsUpdate = true;
	return noErr;
}

/******************************************************************************************/

/* there is some circumstanstial evidence that this routine
   is bugged - it is currently unused by server & client
 */
ErrNbr RmveAsset(Handle rHandle)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at,*fat=NULL;
	AssetRec	huge 	*ap;
	LONG			nbrAssets,nbrTypes,i,j;
	
	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}
	ap = (AssetRec huge  *) *ar->assetList;
	at = (AssetTypeRec *) *ar->typeList;
	nbrAssets = ar->afMap.nbrAssets;
	nbrTypes = ar->afMap.nbrTypes;
	j = -1;
	for (i = 0; i < nbrAssets; ++i,++ap) {
		if (i == at->firstAsset) {
			fat = at;
			++j;
			++at;
		}
		if ((ap->flags & AssetLoaded) && GetAssetHandle(ap->rHandle) == rHandle) {

			/* Got it!
			 *
			 * Dispose of Asset Record
			 */
			if (i < nbrAssets-1)		/* 6/9/95 */
				BlockMove((Ptr)(ap+1),(Ptr)ap,(nbrAssets-(i+1))*sizeof(AssetRec));	
			ar->afMap.nbrAssets--;

			/* Update Asset Type */
			fat->nbrAssets--;

			/* Dispose of Asset Type, possibly */
			if (fat->nbrAssets <= 0) {
				if (j < nbrTypes-1)		/* 6/9/95 bug fix */
					BlockMove((Ptr)(fat+1),(Ptr)fat,(nbrTypes-(j+1))*sizeof(AssetTypeRec));
				ar->afMap.nbrTypes--;
				--at;
			}
			else
				++j;

			/* Update Remaining Type Records */
			while (j < ar->afMap.nbrTypes) {
				at->firstAsset--;
				++j;
				++at;
			}

			/* Tag the file for update */
			ar->fileNeedsUpdate = true;

			return noErr;
		}
	}
	gAssetError = resNotFound;
	return gAssetError;
}



/******************************************************************************************/

ErrNbr DetachAsset(Handle h)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at;
	AssetRec		huge  *ap;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}
	if (!FindAssetByHandle(h,&at,&ap)) {
		gAssetError = resNotFound;
		return gAssetError;
	}
	if (ap->flags & AssetModified) {
		WriteAsset(ap);
	}
	ap->rHandle = NullAssetHandle;
	ap->flags &= ~AssetLoaded;
	return noErr;
}

/******************************************************************************************/

ErrNbr ReleaseAsset(Handle h)
{
	AssetRec	huge	  *ap;
	AssetTypeRec	*at;
	AssetFileVars	*ar = curAssetFile;

	gAssetError = 0;
	if (ar == NULL)
		return (gAssetError = -1);

	if (!FindAssetByHandle(h,&at,&ap))
		return (gAssetError = resNotFound);

	if (ap->flags & AssetModified)
		WriteAsset(ap);

	ClearAssetHandle(ap->rHandle);
	ap->rHandle = NullAssetHandle;
	ap->flags  &= ~AssetLoaded;
	ar->gLastAssetType = -1;
	ar->gLastAssetNbr = -1;
	return noErr;
}

#if !unix
ErrNbr ReleaseAssets(AssetType assetType)
{
	long	n,i;
	AssetFileVars	*ar = curAssetFile;
	AssetRec	huge 	*ap;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}
	n = CountAssets(assetType);

	for (i = 0; i < n; ++i) {
		ap = FindAssetRecByIndex(assetType,i);
		if (ap == NULL) {
			continue;
		}
		if ((ap->flags & AssetLoaded) > 0) {
			if (ap->flags & AssetModified)
				WriteAsset(ap);
		
			ClearAssetHandle(ap->rHandle);
			ap->rHandle = NULL;
			ap->flags  &= ~AssetLoaded;
		}
	}
	return noErr;
}
#endif

/******************************************************************************************/
/* 6/7/95 JAB */
#define CRC_MAGIC	0xd9216290L
unsigned LONG ComputeCRC(Ptr pt, LONG len)
{
	unsigned LONG crc = CRC_MAGIC;
	unsigned char *p = (unsigned char *) pt;
	while (len--)
		crc = ((crc << 1L) | ((crc & 0x80000000L)? 1 : 0)) ^ *(p++);
	return crc;
}

/* 6/7/95 JAB - Get Asset Record that matches ID and CRC */
AssetRec	*FindAssetRecWithCRC(AssetType type, LONG id, unsigned LONG crc)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at;
	AssetRec	huge	*ap;
	LONG			i,j,nbrTypes,nbrAssets;
	at = (AssetTypeRec *) *ar->typeList;
	nbrTypes = ar->afMap.nbrTypes;
	for (i = 0; i < nbrTypes; ++i,++at) {
		if (at->assetType == type) /*esr - 9/8/95 removed unused swap */
	    {
			ap = (AssetRec huge *) *ar->assetList;
			ap += at->firstAsset;
			nbrAssets = at->nbrAssets;
			for (j = 0; j < nbrAssets; ++j,++ap) {
				if (ap->idNbr == id && (ap->crc == crc || crc == 0L)) {
					ar->gLastAssetType = i;	/* 3/5/95 remember last asset */
					ar->gLastAssetNbr = j + at->firstAsset;
					return ap;
				}
			}
			break;
		}
	}
	return NULL;
}

/* 6/7/95 JAB - Get Asset that matches ID and CRC
 *            - If CRC is zero, it acts like GetAsset
 */
Handle	GetAssetWithCRC(AssetType type, LONG id, unsigned LONG crc)
{
	AssetFileVars	*ar = curAssetFile;
	AssetRec		huge *ap;
	Handle			h;
	unsigned LONG	t;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return NULL;
	}
	ap = FindAssetRecWithCRC(type,id,crc);
	if (ap == NULL) {
		gAssetError = resNotFound;
		return NULL;
	}
	GetDateTime(&t);
	ap->lastUseTime = t;
	h = LoadAsset(ap);
	return h;
}

/* 6/7/95 */
unsigned LONG GetAssetCRC(Handle h)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at;
	AssetRec		huge *ap;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return 0L;
	}
	if (!FindAssetByHandle(h,&at,&ap)) {
		gAssetError = resNotFound;
		return 0L;
	}
	return ap->crc;
}

ErrNbr SetAssetCRC(Handle h, unsigned LONG crc)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at;
	AssetRec	huge	*ap;

	gAssetError = 0;
	if (ar == NULL)
		return gAssetError = -1;
	if (!FindAssetByHandle(h,&at,&ap))
		return gAssetError = resNotFound;
	ap->crc = crc;
	ar->fileNeedsUpdate = true;
	return noErr;
}

AssetRec	*FindAssetRecByName(AssetType type, StringPtr name)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at;
	AssetRec	huge	*ap, huge *findP = NULL;
	LONG			i,j,nbrTypes,nbrAssets;

	if (name == NULL || name[0] == 0)
		return NULL;

	at = (AssetTypeRec *) *ar->typeList;
	nbrTypes = ar->afMap.nbrTypes;
	for (i = 0; i < nbrTypes; ++i,++at) {
		if (at->assetType == type) /*esr - removed unused swap */
	    {
			ap = (AssetRec huge*) *ar->assetList;
			ap += at->firstAsset;

			/* current algorithm is:
			 *		if there is a matching guest prop, use the first one encountered
			 *			(order has no meaning, due to usage sort)
			 *			first one is usually (but not always) the last one added
			 *			with matching name
			 *
			 *		if there is no guest prop, use the first member prop encountered
			 *		
			 */
			nbrAssets = at->nbrAssets;
			for (j = 0; j < nbrAssets; ++j,++ap) {
				if (ap->nameOffset >= 0 && EqualPString(name,(StringPtr) (*ar->nameList + ap->nameOffset),false)) {
					if (ap->idNbr >= 0 && ap->idNbr <= 99999999L) {
						ar->gLastAssetType = i;
						ar->gLastAssetNbr = j + at->firstAsset;
						return ap;
					}
					else {
						if (findP == NULL) {
							findP = ap;
							ar->gLastAssetType = i;
							ar->gLastAssetNbr = j + at->firstAsset;
						}
					}
				}
			}
			break;
		}
	}
	return findP;
}

Handle	GetAssetByName(AssetType type, StringPtr name)
{
	AssetFileVars	*ar = curAssetFile;
	AssetRec	huge	*ap;
	Handle			h;
	unsigned LONG	t;

	gAssetError = 0;
	if (ar == NULL) {
		gAssetError = -1;
		return NULL;
	}
	ap = FindAssetRecByName(type,name);
	if (ap == NULL) {
		gAssetError = resNotFound;
		return NULL;
	}
	GetDateTime(&t);
	ap->lastUseTime = t;
	h = LoadAsset(ap);
	return h;
}

/* 7/27/95 Returns true for all duplicate assets except the first one... */
Boolean	AssetIsDuplicate(Handle h)
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at;
	AssetRec	huge	*ap;
	LONG			i,j,nbrAssets,nbrTypes;
	Str63			name;
	LONG			type,id,size;
	unsigned LONG 	crc;

	if (h == NULL)
		return false;

	GetAssetInfo(h, &type, &id, name);
	crc = GetAssetCRC(h);
	size = GetHandleSize(h);

	at = (AssetTypeRec *) *ar->typeList;
	nbrTypes = ar->afMap.nbrTypes;
	for (i = 0; i < nbrTypes; ++i,++at) {
		if (at->assetType == type) {
			ap = (AssetRec huge*) *ar->assetList;
			ap += at->firstAsset;	
			nbrAssets = at->nbrAssets;
			for (j = 0; j < nbrAssets; ++j,++ap) {
				if (ap->idNbr == id && ap->crc == crc && ap->dataSize == size) {
					if (GetAssetHandle(ap->rHandle) == h)
						return false;
					else
						return true;
				}
			}
			return false;
		}
	}
	return false;
}

#if unix
Handle GetAssetHandle(LONG handleID)
{
	AssetFileVars	*ar = curAssetFile;
	if (handleID > 0 && handleID <= ar->nbrLoadedAssets)
		return (Handle) ((void **) (*ar->handleList))[handleID-1];
	else
		return NULL;	
}

LONG AddAssetHandle(Handle h)
{
	AssetFileVars	*ar = curAssetFile;
	int				i;
	if (ar->nbrLoadedAssets+1 > ar->nbrAllocHandles) {
		ar->nbrAllocHandles = ar->nbrLoadedAssets+64;
		SetHandleSize(ar->handleList, ar->nbrAllocHandles*sizeof(void *));
	}
	for (i = 0; i < ar->nbrLoadedAssets; ++i) {
		if (((void **) (*ar->handleList))[i] == NULL) {
			((void **) (*ar->handleList))[i] = h;
			return i + 1;
		}
	}
	((void **) (*ar->handleList))[ar->nbrLoadedAssets] = h;
	++ar->nbrLoadedAssets;
	return ar->nbrLoadedAssets;
}

void ClearAssetHandle(LONG handleID)
{
	Handle	h;
	AssetFileVars	*ar = curAssetFile;
	if (handleID > 0 && handleID <= ar->nbrLoadedAssets) {
		h = (Handle) ((void **) (*ar->handleList))[handleID-1];
		if (h)
			DisposeHandle(h);
		((void **) (*ar->handleList))[handleID-1] = NULL;
		while (ar->nbrLoadedAssets && ((void **) (*ar->handleList))[ar->nbrLoadedAssets-1] == NULL) {
			--ar->nbrLoadedAssets;
		}
	}
}
#endif

#include <stdlib.h>

/* reverse numeric sort - most recent records first */
int AssetRecCompare(const void *n1, const void *n2);
int AssetRecCompare(const void *n1, const void *n2)
{
	return ( (unsigned long) ((AssetRec *) n2)->lastUseTime -  
			 (unsigned long) ((AssetRec *) n1)->lastUseTime );
}

ErrNbr SortAssetsByUsage()
{
	AssetFileVars	*ar = curAssetFile;
	AssetTypeRec	*at;
	AssetRec		huge *ap;
	int				i,nbrTypes;

	if (ar == NULL) {
		gAssetError = -1;
		return gAssetError;
	}

	at = (AssetTypeRec *) *ar->typeList;
	nbrTypes = ar->afMap.nbrTypes;
	/* For each type record */
	for (i = 0; i < nbrTypes; ++i,++at) {
		/* Sort Assets in this group */
		ap = (AssetRec *) *ar->assetList;
		ap += at->firstAsset;		
		qsort(ap, at->nbrAssets, sizeof(AssetRec), AssetRecCompare);
	}
	return noErr;
}

