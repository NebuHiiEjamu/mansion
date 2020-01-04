/******************************************************************************************/
/* esr - redoing
 * esr - Changes:       * Replaced all 'evnt' with MSG_xxxx.
 *                                                 
 *             * #ifdef FSSpec filenames. Mostly cuz its to LONG.
 */
/******************************************************************************************/

#include "s-server.h"
#include "m-assets.h"
#include "m-events.h"

/******************************************************************************************/

short   gServAssetFile;

void RegisterFailedAssetRequest(LONG userID, LONG type, LONG id, unsigned LONG crc);
void RegisterServerAssetReception(LONG type, LONG id, unsigned LONG crc);
Boolean AssetHasBeenRequested(LONG type, LONG id, unsigned LONG crc);

/******************************************************************************************/

void InitServerAssets(void)
{
#ifdef macintosh
        FSSpec  propSpec = {0,0L,"\pPServer.prp"};	/* 6/7/95 - note pascal string! */
#else
        FSSpec  propSpec = {0,0,"\x0bPServer.prp"};
#endif

        if (OpenAssetFile(&propSpec,&gServAssetFile) != noErr) {
                ErrorExit("Can't open props file %s\n",
                		CvtToCString(propSpec.name));
        }
		
}

/******************************************************************************************/

void CloseServerAssets(void)
{
        if (gServAssetFile)
                CloseAssetFile(gServAssetFile);
}

/******************************************************************************************/

void CheckForProp(ServerUserPtr cUser, LONG propID, unsigned LONG crc)		/* 6/8/95 JAB */
{
    if (propID && !AssetExistsWithCRC(RT_PROP,propID,crc)) {		/* 6/8/95 */
		/* 7/8/96 - don't request guest props from non-gods */
		if (propID >= MinReservedProp && propID <= MaxReservedProp &&
			!(cUser->flags & U_God)) {
			return;
		}
		/* 7/27/95 - don't request assets multiple times */
		if (!AssetHasBeenRequested(RT_PROP, propID, crc)) {
            LONG            msg[3];
            msg[0] = RT_PROP;
            msg[1] = propID;
            msg[2] = crc;										/* 6/8/95 */
			RegisterFailedAssetRequest(0L,RT_PROP,propID,crc);
            PostUserEvent(cUser,MSG_ASSETQUERY,0L,(Ptr) &msg[0],sizeof(LONG) * 3); /*6/8/95 */
		}
    }
}

/******************************************************************************************/


/******************************************************************************************/
/* 6/8/95 modified to use new id/crc system.  Translation functions removed. 
 *
 * 6/9/95 modified to use receiving crc, multiblock sends should work now...
 *
 * 7/24/95 modified to not save duplicates...
 *
 */

/* JAB 10/28/96 copied this in from U-USER.H - it really should
   be in a shared header file */
typedef struct {
	short	width,height,hOffset,vOffset;
	short	scriptOffset;
	short	flags;
} PropHeader,*PropHeaderPtr;


Boolean IsBadProp(Ptr p);
Boolean IsBadProp(Ptr p)
{
	PropHeaderPtr pp = (PropHeaderPtr) p;
	if (pp->width != 44 || pp->height != 44 ||
		/* JAB 10/31/96  Note: multiple prop paste can result in props on right side with
		   offsets up to 66 - consider a multi-prop with width 89 */
		pp->hOffset < -44 || pp->hOffset > 66 ||
		pp->vOffset < -44 || pp->vOffset > 66)
	{
		return 1;
	}
	else
		return 0;
}

void ReceiveAsset(ServerUserPtr   cUser, Ptr assetMsg)        /* User is sending asset block */
{
    AssetBlockPtr   asp;
    Handle          h = NULL,h2;
	Ptr				p;

	
    asp = (AssetBlockPtr) assetMsg;

	/* 7/8/96 - don't save guest props from non-gods */
	if (asp->type == RT_PROP && asp->spec.id >= MinReservedProp &&
		asp->spec.id <= MaxReservedProp && !(cUser->flags & U_God)) {
		return;
	}

    if (asp->blockNbr == 0) {
		/* 1/28/96 additional limits */
        if (asp->blockSize > asp->varBlock.firstBlockRec.size ||
        	asp->blockSize > 5000 || asp->blockSize < 0 ||
        	asp->varBlock.firstBlockRec.size < 1 ||
        	asp->varBlock.firstBlockRec.size > 5000)
            return;
		h2 = GetAssetWithCRC(asp->type, asp->spec.id, asp->spec.crc);	/* 7/27/95 */
		if (h2 == NULL) {
			p = &asp->varBlock.firstBlockRec.data[0];
			/* 1/13/95 Check Prop CRC, don't save if bad */
			if (asp->type == RT_PROP && 
				/* JAB 10/28/96 Added additional bad prop checks
				   for too-long name, and bad width,height,hoffset,voffset */
				(asp->blockSize < 12 || 
				 ComputeCRC(p + sizeof(PropHeader),asp->blockSize - sizeof(PropHeader)) != asp->spec.crc ||
				 asp->varBlock.firstBlockRec.name[0] < 0 ||
				 asp->varBlock.firstBlockRec.name[0] > 31 ||
				 IsBadProp(p))
				 ) 
			{
				TimeLogMessage("%s sent bad prop\r",CvtToCString(cUser->user.name));
				/* UserBadAssert(cUser, 0); */
				return;
			}
	        h = NewHandleClear(asp->varBlock.firstBlockRec.size);
	        if (h == NULL)
	            return;
	        if (h) {
	            BlockMove(p,*h,asp->blockSize);
	            AddAsset(h,asp->type,asp->spec.id,(StringPtr) asp->varBlock.firstBlockRec.name);
	            SetAssetCRC(h,asp->spec.crc);		/* 6/9/95 */
	        }
		}
		else
			ReleaseAsset(h2);
    }
    else {
        /* newID = GetAssetTrans(asp->type,asp->id); */
/** shouldn't happen
        h = GetAssetWithCRC(asp->type,asp->spec.id,asp->spec.crc);
        if (h) {
            BlockMove(&asp->varBlock.nextBlockRec.data[0],*h + asp->blockOffset,asp->blockSize);
            ChangedAsset(h);
        }
 **/
    }
    if (h && asp->blockNbr == asp->nbrBlocks-1) {
        ReleaseAsset(h);
		RegisterServerAssetReception(asp->type,asp->spec.id,asp->spec.crc);
		UpdateAssetFile();	/* 8/10/95 */
    }
}

/******************************************************************************************/

void UserAssetQuery(ServerUserPtr cUser, LONG *assetMsg)    /* User is asking for asset */
{
        AssetType       type;
        LONG            id;
		unsigned LONG	crc;
        Handle          h;
        Ptr             p;
        LONG            size;
        AssetBlockPtr   asp;
		Boolean			badProp = 0;
		Str255			assetName;	/* JAB 10/28/96 */
		
		if (gDebugFlag)
			LogMessage("User @%s requested an asset", cUser->verbalIP);
		
        type = assetMsg[0];
        id = assetMsg[1];
		crc = assetMsg[2];						/* 6/8/95 */
        h = GetAssetWithCRC(type,id,crc);		/* 6/8/95 */
        if (h) {
                size = GetHandleSize(h);

				/* 1/13/95 don't propogate bad props */
				if (type == RT_PROP && crc && 
					(size < 12 || ComputeCRC(*h + sizeof(PropHeader),size - sizeof(PropHeader)) != crc ||
					 IsBadProp(*h))) 
				{
					badProp = 1;
				}

				p = gBigBuffer;  /* 12/11/95 */
                /* p = NewPtr(sizeof(AssetBlockHeader)+size-1); */
                if (p && !badProp) {
                        asp = (AssetBlockPtr) p;
                        asp->type = type;
                        asp->spec.id = id;					/* 6/8/95 */
                        asp->spec.crc = GetAssetCRC(h);		/* 6/8/95 */
                        asp->blockSize = size;
                        asp->blockOffset = 0L;
                        asp->blockNbr = 0;
                        asp->nbrBlocks = 1;
                        asp->varBlock.firstBlockRec.flags = 0;
                        asp->varBlock.firstBlockRec.size = size;
						/* 10/28/96 JAB - truncate name if necessary */
                        GetAssetInfo(h,&type,&id,assetName);
                        if (assetName[0] > 31)
                        	assetName[0] = 31;
                        BlockMove(assetName,(StringPtr) asp->varBlock.firstBlockRec.name,assetName[0]+1);
                        BlockMove(*h,&asp->varBlock.firstBlockRec.data[0],size);
						if (gDebugFlag)
	                        TimeLogMessage("Sent %.4s %lxH to %s\r",&asp->type,asp->spec.id,CvtToCString(cUser->user.name));
                        if (cUser)
                                PostUserEvent(cUser,MSG_ASSETSEND,0L,p,sizeof(AssetBlockHeader)+size-1);
                        /* DisposePtr(p); */
                }
                ReleaseAsset(h);
        }
        else {
			RegisterFailedAssetRequest(cUser->user.userID,type,id,crc);
        }
}

/******************************************************************************************/


#define MaxAssetRequests	256

/* 10/28/96 JAB switched to time(NULL) because clock() isn't real time */
#define MaxRequestTimeLimit	(30L)		/* changd from 3 minutes to 30 seconds */

typedef struct {
	LONG			type;
	LONG			id;
	unsigned LONG 	crc;
	unsigned LONG 	timeStamp;
	LONG			userID;
} AFReqRec, *AFReqPtr;

short			gNbrAssetRequests = 0;
AFReqPtr		gAssetRequestPtr = NULL;

/* 7/17/95 Maintain a list of assets that have been requested by users
 * and have failed.  If we receive this asset later, send it to the user
*/
void RegisterFailedAssetRequest(LONG userID, LONG type, LONG id, unsigned LONG crc)
{
	AFReqPtr			ap;
	short			i;
	unsigned LONG	t;
	
	if (gAssetRequestPtr == NULL) {
		gAssetRequestPtr = (AFReqPtr) NewPtrClear(sizeof(AFReqRec) * MaxAssetRequests);
		if (gAssetRequestPtr == NULL)
			return;
	}

	if (gDebugFlag)
		LogMessage("Asset request failed");

	/* Return if it's already in list (shouldn't happen, but just in case...) */
	ap = gAssetRequestPtr;
	for (i = 0; i < gNbrAssetRequests; ++i,++ap) {
		if (ap->userID == userID && ap->type == type && ap->id == id && (ap->crc == 0 || crc == 0 || ap->crc == crc)) {
			return;
		}
	}

	/* If List is full, kill most recent record */
	if (gNbrAssetRequests == MaxAssetRequests) {
		BlockMove(&gAssetRequestPtr[1],&gAssetRequestPtr[0],(gNbrAssetRequests-1)*sizeof(AFReqRec));
		--gNbrAssetRequests;
		/* Kill bottom one in list */
	}
	/* Kill Records with old Timestamps or absent users */
	ap = gAssetRequestPtr;
	t = time(NULL);
	for (i = 0; i < gNbrAssetRequests; ++i,++ap) {
		if (t - ap->timeStamp > MaxRequestTimeLimit || 
			(ap->userID && GetServerUser(ap->userID) == NULL)) {
			BlockMove(&ap[1],ap,sizeof(AFReqRec)*((gNbrAssetRequests-1)-i));
			--gNbrAssetRequests;
			--i;
			--ap;
		}
	}
	/* Add asset to list */
	ap = &gAssetRequestPtr[gNbrAssetRequests];
	ap->type = type;
	ap->id = id;
	ap->crc= crc;
	ap->userID = userID;
	ap->timeStamp = time(NULL);
	++gNbrAssetRequests;
	return;
}


/* 7/17/95 Once asset is received, send it to users who have requested it and
 * remove it from the list
*/
void RegisterServerAssetReception(LONG type, LONG id, unsigned LONG crc)
{
	AFReqPtr	ap;
	short	i;
	if (gAssetRequestPtr == NULL || gNbrAssetRequests == 0)
		return;
	ap = gAssetRequestPtr;
	for (i = 0; i < gNbrAssetRequests; ++i,++ap) {
		if (ap->type == type && ap->id == id && (ap->crc == 0 || crc == 0 || ap->crc == crc)) {
			/* Send to user */
			if (ap->userID) {
				ServerUserPtr	cUser;
				cUser = GetServerUser(ap->userID);
				if (cUser)
					UserAssetQuery(cUser,(LONG *) ap);
			}
			BlockMove(&ap[1],ap,sizeof(AFReqRec)*((gNbrAssetRequests-1)-i));
			--gNbrAssetRequests;
			--i;
			--ap;
		}
	}
}

Boolean AssetHasBeenRequested(LONG type, LONG id, unsigned LONG crc)
{
	AFReqPtr	ap;
	short	i;
	if (gAssetRequestPtr == NULL || gNbrAssetRequests == 0)
		return false;
	ap = gAssetRequestPtr;
	for (i = 0; i < gNbrAssetRequests; ++i,++ap) {
		if (ap->type == type && ap->id == id && (ap->crc == 0 || crc == 0 || ap->crc == crc) &&
			ap->userID == 0) {
			return true;
		}
	}
	return false;
}

LONG PurgeServerProps(short nDays)
{
	extern AssetFileVars	*curAssetFile;		/* current resource file/map */
	LONG					nbrPurged=0;
	unsigned LONG			curTime,cutoffTime;
	LONG					i,n,j;
	AssetFileVars			*ar;
	AssetTypeRec			*at;
	AssetRec				*ap,*apN;			/* 7/20/96 */
	LONG					nbrTypeAssets;

	if (nDays == 0)
		nDays = gPrefs.purgePropDays;
	GetDateTime(&curTime);
	cutoffTime = curTime - nDays*60L*60L*24L;

	ar = curAssetFile;
	if (ar == NULL)
		return 0L;
	at = (AssetTypeRec *) *ar->typeList;
	ap = (AssetRec huge  *) *ar->assetList;
	apN = ap;	/* New variable */
	n = 0;
	nbrPurged = 0;

	/* 7/20/96 JAB
	   Significantly improved performance of purge by eliminating
	   redundant memcpys (blockmoves) (order (N*N)/2) in place of a single compaction
	   operation (order N)
	 */

	for (j = 0; j < ar->afMap.nbrTypes; ++j,++at) {
		at->firstAsset = n;

		/* JAB 7/20/96 changed logic here */
		/* use loop for props and all asset types following rops */
		if (ap == apN && at->assetType != RT_PROP) {
			n += at->nbrAssets;
			ap += at->nbrAssets;
			apN += at->nbrAssets;
		}
		else {
			nbrTypeAssets = at->nbrAssets;
			for (i = 0; i < nbrTypeAssets; ++i) {
				*ap = *apN;	/* 7/20/96 Added this - copy current asset in */
				if (at->assetType == RT_PROP) {
					if (!(ap->flags & AssetLoaded) && 
						 ap->lastUseTime < cutoffTime && 
						(ap->idNbr < 0 || ap->idNbr > MaxReservedProp)) 
					{
						/* 7/20/96 removed this */
						/* if (n < ar->afMap.nbrAssets-1)
						 *	BlockMove((Ptr)(ap+1),(Ptr)ap,(ar->afMap.nbrAssets-(n+1))*sizeof(AssetRec));
						 */
						ar->afMap.nbrAssets--;
						at->nbrAssets--;
						ar->fileNeedsUpdate = true;
						++nbrPurged;
						++apN;	/* 7/20/96 */
					}
					else {
						++n;
						++ap;
						++apN;	/* 7/20/96 */
					}
				}
				else {
					++n;
					++ap;
					++apN;	/* 7/20/96 */
				}
			}
		}
	}
	if (nbrPurged)
		WriteAssetFile();
	return nbrPurged;
}

