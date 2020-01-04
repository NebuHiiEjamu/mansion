////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "U-USER.H"       // esr 5/19 Renamed from User.h
#include "M-ASSETS.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FileHandle	gUserAssetFile;
Boolean	gReadOnlyAssets;

#ifdef WIN32
extern BOOL 	gNoProps;
#endif

Boolean RegisterAssetRequest(long type, long id, unsigned long crc);
void RegisterAssetReception(long type, long id, unsigned long crc);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// We've recieved an acknoledgement for an asset
// translate appropriate IDs that are already in use.
/*****
void TranslateAsset(AssetType type, long oldID, long newID)
{
	LocalUserRecPtr	up;
	short	i,j;

	switch (type)
  {
	  case RT_PROP:
		  // !!! Also check for prop in fave list with oldID...
		  up = gRoomWin->userList;
		  for (i = 0; i < gRoomWin->curRoom.nbrPeople; ++i,++up) {
			  for (j = 0; j < 3; ++j) {
				  if (up->user.propID[j] == oldID)
				  {
					  up->user.propID[j] = newID;
					  break;
				  }
			  }
		  }
		  for (i = 0; i < gRoomWin->nbrFaveProps; ++i)
			  if (((long *) *gRoomWin->faveProps)[i] == oldID)
				  ((long *) *gRoomWin->faveProps)[i] = newID;
  		if (gSelectProp == oldID)
	  		gSelectProp = newID;
		  break;
	}
}
*****/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 6/7/95 JAB modified to use AssetSpecs

void RefreshForAsset(AssetType type, long id, long crc)
{
	LocalUserRecPtr	up;
	short		i,j;
	Rect		rr,ur={0,0,0,0};
	LPropPtr	lp;
	RoomRec		*rp = &gRoomWin->curRoom;
	
	switch (type) {
	case RT_PROP:
		// !!! Also check for prop in fave list with oldID...
		up = gRoomWin->userList;
		for (i = 0; i < rp->nbrPeople; ++i,++up) {
			for (j = 0; j < up->user.nbrProps; ++j) {						// 6/7/95
				if (up->user.propSpec[j].id == id && (crc == 0 || up->user.propSpec[j].crc == 0 || up->user.propSpec[j].crc == (unsigned long)crc))	// 6/7/95
				{
					up->propHandle[j] = GetAssetWithCRC(RT_PROP,id,crc);	// 6/7/95
					CheckUserFace(up);	// 6/22
					ComputePropRect(j,up,&rr);
					UnionRectMac(&rr,&ur,&ur);
					break;
				}
			}
		}
		lp = (LPropPtr) &rp->varBuf[rp->firstLProp];
		for (i = 0; i < rp->nbrLProps; ++i) {
			if (lp->propSpec.id == id && (crc == 0 || lp->propSpec.crc == 0 || lp->propSpec.crc == (unsigned long)crc)) {				// 6/7/95
				SetRectMac(&rr,0,0,PropWidth,PropHeight);
				OffsetRectMac(&rr,lp->loc.h,lp->loc.v);
				gRoomWin->lPropHandles[i] = GetAssetWithCRC(RT_PROP,id,crc);	// 6/7/95
				UnionRectMac(&rr,&ur,&ur);
			}
			if (lp->link.nextOfst)
				lp = (LPropPtr) &rp->varBuf[lp->link.nextOfst];
			else
				break;
		}
		break;
	}
	if (!IsRectEmptyMac(&ur)) {
		RefreshRoom(&ur);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InitUserAssets(void)
{
	OSErr	oe;
	void	InitializeEncryption();

  #ifdef macintosh
	FSSpec	propSpec = {0,0L,"\pPalace.prp"};
  #else
	FSSpec	propSpec = {0,0L,"\x0aPalace.prp"};
  #endif
	if ((oe = OpenAssetFile(&propSpec,&gUserAssetFile)) != noErr) {
		ErrorExit("Can't open props file: %d",oe);
	}
	/* 7/2/96 - use temporary file for storing assets */
	UseTempAssetFile();

  #ifdef macintosh			// 7/7/95 confirm we have read/write access
	gReadOnlyAssets = AssetsAreReadOnly(gUserAssetFile);
  #endif

	/* 7/20/96 Initalize encryption random number table */
	/* Not a great place to stick this, but it's in the shared code... */
	InitializeEncryption();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CloseUserAssets(void)
{
	if (gUserAssetFile) {
		CloseAssetFile(gUserAssetFile);
		gUserAssetFile = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 6/7/95 JAB modified to use assetspec

void RequestAsset(AssetType type, AssetSpec *spec)
{
	long	rRec[3];
	rRec[0] = type;
	rRec[1] = spec->id;
	rRec[2] = spec->crc;
	if (!RegisterAssetRequest(type,spec->id,spec->crc))	{ // 7/17/95
		PostServerEvent(MSG_ASSETQUERY,gRoomWin->meID,(Ptr) &rRec[0],sizeof(long)*3);
		if (gDebugFlag)
			LogMessage("Requested %.4s %lxH\r",&type,spec->id);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ReceiveAsset(Ptr assetMsg)	// Server is sending asset block
{
	AssetBlockPtr	asp;
	Handle			h = NULL,h2;
	asp = (AssetBlockPtr) assetMsg;

	if (gReadOnlyAssets)
		return;

	if (gDebugFlag)
		LogMessage("-->Prop ID# %lx Block Length %[%ld]\r",asp->spec.id,asp->blockSize);


	if (asp->blockNbr == 0) {
		if (asp->blockSize > asp->varBlock.firstBlockRec.size)
			return;

		RegisterAssetReception(asp->type,asp->spec.id,asp->spec.crc); // 7/17/95

		// 7/7/95 Make sure we aren't replacing old one...
		h2 = GetAssetWithCRC(asp->type, asp->spec.id, asp->spec.crc);

		// 7/20/96 Make sure we aren't replacing a guest prop with
		// a "forged" one - JAB
		//
		if (h2 == NULL && asp->type == RT_PROP &&
			asp->spec.id >= MinReservedProp &&
			asp->spec.id <= MaxReservedProp)
			h2 = GetAsset(asp->type, asp->spec.id);

		if (h2 == NULL) {
			h = NewHandleClear(asp->varBlock.firstBlockRec.size);
			if (h == NULL)
				return;
			if (h) {
				BlockMove(&asp->varBlock.firstBlockRec.data[0],*h,asp->blockSize);
				AddAsset(h,asp->type,asp->spec.id,(StringPtr) asp->varBlock.firstBlockRec.name);
				SetAssetCRC(h,asp->spec.crc);		// 6/9/95
			}
		}
	}
	else {
		h = GetAssetWithCRC(asp->type,asp->spec.id,asp->spec.crc);	// 6/9/95
		if (h) {
			BlockMove(&asp->varBlock.nextBlockRec.data[0],*h + asp->blockOffset,asp->blockSize);
			ChangedAsset(h);
		}
	}
	if (h && asp->blockNbr == asp->nbrBlocks-1) {
		ReleaseAsset(h);
		/* UpdateAssetFile(); New - reduces prop lag - update is called periodically  2/28/96 */
		RefreshForAsset(asp->type,asp->spec.id,asp->spec.crc);
		if (gDebugFlag)
			LogMessage("Received %.4s %lxH\r",&asp->type,asp->spec.id);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// JAB 6/7/95 Changed to not bother with translation...
//     6/8/95 no longer needed
/**
void ServerAcknowledgeAsset(long *assetMsg)	// Server is acknowledging registered asset
{
	AssetType	type;
	long		id,crc;	// 6/7/95 JAB

	type  = assetMsg[0];
	id = assetMsg[1];
	crc = assetMsg[2];
	// TranslateAsset(type,id,crc);
	RefreshForAsset(type,id,crc);
}
***/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Register Asset
void RegisterAsset(AssetType type, AssetSpec *spec)
{
	Handle	h;
	Ptr		p;
	long	size,id;
	AssetBlockPtr	asp;

	h = GetAssetWithCRC(type,spec->id,spec->crc);
	if (h) {
		size = GetHandleSize(h);
		/* 7/1/96 JAB Changed to NewPtrClear */
		p = NewPtrClear(sizeof(AssetBlockHeader)+size-1);
		if (p) {
			asp = (AssetBlockPtr) p;
			asp->type        = type;
			asp->spec        = *spec;
			asp->spec.crc = GetAssetCRC(h);
			asp->blockSize   = size;
			asp->blockOffset = 0L;
			asp->blockNbr    = 0;
			asp->nbrBlocks   = 1;
			asp->varBlock.firstBlockRec.flags = 0;
			asp->varBlock.firstBlockRec.size  = size;
			GetAssetInfo(h,&type,&id,(StringPtr) asp->varBlock.firstBlockRec.name);
			BlockMove(*h,&asp->varBlock.firstBlockRec.data[0],size);
			PostServerEvent(MSG_ASSETREGI,gRoomWin->meID,p,sizeof(AssetBlockHeader)+size-1);
			DisposePtr(p);
		}
		// We aren't disposing of asset here, since it might be in use.
		// will need to deal with this at some point...
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Modify this to support multiple pages...
void ServerAssetQuery(long *assetMsg)	// Server is asking for asset
{
	AssetType		type;
	AssetSpec		spec;
	Boolean			gotItFlag;
	short			i;

	type = assetMsg[0];
	spec.id = assetMsg[1];
	spec.crc = assetMsg[2];		// 6/7/95
	RegisterAsset(type,&spec);


	if (gRoomWin->mePtr) {		// 7/5/95
		gotItFlag = false;
		for (i = 0; i < gRoomWin->mePtr->user.nbrProps; ++i) {
			if (gRoomWin->mePtr->user.propSpec[i].id == spec.id) {
				gotItFlag = true;
				break;
			}
		}
		if (gotItFlag) {
			ChangeProp(gRoomWin->mePtr->user.nbrProps, &gRoomWin->mePtr->user.propSpec[0]);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 6/7/95 JAB modified to use assetspec
Handle GetUserAsset(AssetType type, AssetSpec *spec)
{
	Handle	h;
	h = GetAssetWithCRC(type,spec->id,spec->crc);
#ifdef WIN32
	if (h == NULL && !gReadOnlyAssets && !gNoProps)			// If asset isn't in our asset library,
#else
	if (h == NULL && !gReadOnlyAssets)			// If asset isn't in our asset library,
#endif
		RequestAsset(type,spec);				// request it from server
	return h;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MaxAssetRequests	64
#define MaxRequestTimeLimit	(3*60L*TICK_SECONDS)		// 3 minutes 3*60L*60L

typedef struct {
	long		type;
	long		id;
	unsigned long crc;
	unsigned long timeStamp;
} AReqRec, *AReqPtr;

short		gNbrAssetRequests = 0;
AReqPtr		gAssetRequestPtr = NULL;

// 7/17/95 Maintain a list of assets that have been requested from server
// (to avoid multiple requests for the same asset)
//
// Returns true if asset has already been requested, otherwise, adds
// to list of pending asset requests
Boolean RegisterAssetRequest(long type, long id, unsigned long crc)
{
	AReqPtr			ap;
	short			i;
	unsigned long	t;
	if (gAssetRequestPtr == NULL) {
		gAssetRequestPtr = (AReqPtr) NewPtrClear(sizeof(AReqRec) * MaxAssetRequests);
		if (gAssetRequestPtr == NULL)
			return false;
	}
	// Check if asset has already been requested
	ap = gAssetRequestPtr;
	for (i = 0; i < gNbrAssetRequests; ++i,++ap) {
		if (ap->type == type && ap->id == id && (ap->crc == 0 || crc == 0 || ap->crc == crc))
			return true;
	}
	// If List is full, kill oldest record
	if (gNbrAssetRequests == MaxAssetRequests) {
		BlockMove((Ptr)&gAssetRequestPtr[1],(Ptr)&gAssetRequestPtr[0],(gNbrAssetRequests-1)*sizeof(AReqRec));
		--gNbrAssetRequests;
		// Kill bottom one in list
	}
	// Kill Records with old Timestamps
	ap = gAssetRequestPtr;
	t = GetTicks();
	for (i = 0; i < gNbrAssetRequests; ++i,++ap) {
		if (t - ap->timeStamp > MaxRequestTimeLimit) {
			BlockMove((Ptr)&ap[1],(Ptr)ap,sizeof(AReqRec)*((gNbrAssetRequests-1)-i));
			--gNbrAssetRequests;
			--i;
			--ap;
		}
	}
	// Add asset to list
	ap = &gAssetRequestPtr[gNbrAssetRequests];
	ap->type = type;
	ap->id = id;
	ap->crc= crc;
	ap->timeStamp = GetTicks();
	++gNbrAssetRequests;
	return false;
}


// 7/17/95 Once asset is received, remove it from the list
//
void RegisterAssetReception(long type, long id, unsigned long crc)
{
	AReqPtr	ap;
	short	i;
	if (gAssetRequestPtr == NULL || gNbrAssetRequests == 0)
		return;
	ap = gAssetRequestPtr;
	for (i = 0; i < gNbrAssetRequests; ++i,++ap) {
		if (ap->type == type && ap->id == id && (ap->crc == 0 || crc == 0 || ap->crc == crc)) {
			BlockMove((Ptr)&ap[1],(Ptr)ap,sizeof(AReqRec)*((gNbrAssetRequests-1)-i));
			--gNbrAssetRequests;
			return;
		}
	}
}

