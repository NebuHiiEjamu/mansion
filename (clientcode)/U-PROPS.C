////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "u-User.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern AssetSpec gSelectProp;

Boolean IsRareProp(AssetSpec *propID)
{
  PropHeaderPtr ph;
  Handle    propH;

  propH = GetAssetWithCRC(RT_PROP,propID->id,propID->crc);
  if (propH) {
		ph   = (PropHeaderPtr) *propH;
		return (ph->flags & PF_PropRareFlag) > 0;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Boolean PtInProp(Point p,Point op,AssetSpec *propID)
{
  Handle    propH;
  Ptr       sp;
  short     y,x;
  PropHeaderPtr ph;
  Boolean     yHit;         // 6/8/95
  unsigned    char  cb,mc,pc;
  Ptr	      bitEnd;

  propH = GetAssetWithCRC(RT_PROP,propID->id,propID->crc);

  if (propH)
  {
    bitEnd = *propH+GetHandleSize(propH);
    p.h -= op.h;
    p.v -= op.v;
    ph   = (PropHeaderPtr) *propH;
    sp   = *propH + sizeof(PropHeader);

    for (y=0;y<ph->height && y <= p.v; ++y)
    {
      x    = 0;
      yHit = (p.v >= y-1 && p.v <= y+1);
      while (x < ph->width)
      {
        cb = *((unsigned char *) sp); ++sp;
        mc = cb >> 4;
        pc = cb & 0x0F;
        x += mc;
		if (x > ph->width)		// bad prop check
			return false;

        if (yHit && p.h >= x-1 && p.h <= x + pc)      // 6/8/95 made more forgiving
          return true;
        sp += pc;
        x += pc;
		if (x > ph->width)		// bad prop check
			return false;
      }
    }
    if (sp >= bitEnd)
  	  return false;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Boolean PropInUse(long id, long crc)  // returns index# + 1, or 0 if not in use
{
  short i,nbrProps = gRoomWin->mePtr->user.nbrProps;
  AssetSpec *as = gRoomWin->mePtr->user.propSpec;

  for (i = 0; i < nbrProps; ++i,++as) {
    if (as->id == id &&  (crc == 0 || as->crc == (unsigned long)crc))
      return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SavePropFavorites(void)
{
  ChangedAsset(gRoomWin->faveProps);
  UpdateAssetFile();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ToggleProp(AssetSpec *spec)  // 6/7/95 rewritten to use AssetSpecs JAB
{
 // 6/12/95
 //  AssetSpec assetSpecs[MaxUserProps];
 // short     nbrProps  = gRoomWin->mePtr->user.nbrProps;

 // BlockMove((Ptr)&gRoomWin->mePtr->user.propSpec[0],(Ptr)&assetSpecs[0],nbrProps*sizeof(AssetSpec));
  // If prop is in use, delete it
  if (PropInUse(spec->id,spec->crc))
    DoffProp(spec->id,spec->crc);
  else
    DonProp(spec->id,spec->crc);
  if (gSelectProp.id != spec->id || gSelectProp.crc != spec->crc)
  {
    gSelectProp = *spec;
    RefreshPropPicker();
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LoadPropFavorites(void)
{
  short      i;
  Handle     h = GetAsset(ASSET_FAVE,128L);
  AssetSpec *fSpec;

  if (h == NULL)
  {
    gRoomWin->nbrFaveProps = gIconSetup->nbrProps;
    gRoomWin->faveProps    = NewHandleClear(gIconSetup->nbrProps * sizeof(AssetSpec));
    fSpec = (AssetSpec *) *gRoomWin->faveProps;
    for (i = 0; i < gRoomWin->nbrFaveProps; ++i)
    {
      fSpec[i].id = i+gIconSetup->firstProp;
      fSpec[i].crc = 0;
    }
    AddAsset(gRoomWin->faveProps,ASSET_FAVE,128L,(StringPtr) "");
    SavePropFavorites();
  }
  else
  {
    gRoomWin->nbrFaveProps = GetHandleSize(h)/sizeof(AssetSpec);
    gRoomWin->faveProps    = h;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AddPropFavorite(AssetSpec *spec)
{
  short      i;
  AssetSpec *lp;

  if (gRoomWin == NULL)	/* 7/9/96 */
  		return;
  lp = (AssetSpec *) *gRoomWin->faveProps;
  for (i = 0; i < gRoomWin->nbrFaveProps; ++i,++lp)
    if (lp->id == spec->id && (lp->crc == 0L || spec->crc == 0 || lp->crc == spec->crc))
      return;

  SetHandleSize(gRoomWin->faveProps,(gRoomWin->nbrFaveProps+1)*sizeof(AssetSpec));
  lp = (AssetSpec *) *gRoomWin->faveProps;
  BlockMove((Ptr)lp,(Ptr)&lp[1],gRoomWin->nbrFaveProps*sizeof(AssetSpec));
  lp[0] = *spec;
  gRoomWin->nbrFaveProps++;
  gSelectProp = *spec;
  RefreshPropPicker();
  SavePropFavorites();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ReplacePropFavorite(AssetSpec *oldspec,AssetSpec *newspec)   // 6/9/95
{
  // Find prop in favorites list
  // Replace entry with the new setting
  short   i;
  AssetSpec *as;

  HLock(gRoomWin->faveProps);
  as = (AssetSpec *) *gRoomWin->faveProps;
  for (i = 0; i < gRoomWin->nbrFaveProps; ++i,++as) {
    if (as->id == oldspec->id && as->crc == oldspec->crc) {
      break;
    }
  }
  if (i >= gRoomWin->nbrFaveProps)
    return;
  HUnlock(gRoomWin->faveProps);
  as = (AssetSpec *) *gRoomWin->faveProps;
  as[i] = *newspec;
  gSelectProp = *newspec;
  RefreshPropPicker();
  SavePropFavorites();
}

Boolean IsPropFavorite(AssetSpec *spec)   // 6/9/95
{
  // Find prop in favorites list
  // Replace entry with the new setting
  short   i;
  AssetSpec *as;

  HLock(gRoomWin->faveProps);
  as = (AssetSpec *) *gRoomWin->faveProps;
  for (i = 0; i < gRoomWin->nbrFaveProps; ++i,++as) {
    if (as->id == spec->id && (as->crc == spec->crc || as->crc == 0)) {
 		return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DeletePropFavorite(AssetSpec *spec)  // 6/7/95
{
	short   i;
	AssetSpec *as;

	HLock(gRoomWin->faveProps);       // 6/9/95

    as = (AssetSpec *) *gRoomWin->faveProps;

	for (i = 0; i < gRoomWin->nbrFaveProps; ++i,++as) {
	    if (as->id == spec->id && as->crc == spec->crc) {
	    	BlockMove((Ptr)&as[1],(Ptr)as,((gRoomWin->nbrFaveProps-1)-i)*sizeof(AssetSpec));
			--gRoomWin->nbrFaveProps;
	      	if (i < gRoomWin->nbrFaveProps) {
				gSelectProp = *as;
	     	}
			else
		        gSelectProp.id = gSelectProp.crc = 0;
	      	break;
		}
	}
	HUnlock(gRoomWin->faveProps);     // 6/9/95

	// 10/18/95
	SetHandleSize(gRoomWin->faveProps,gRoomWin->nbrFaveProps*sizeof(AssetSpec));

  	RefreshPropPicker();
	SavePropFavorites();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void DuplicatePropFavorite(AssetSpec *spec)		// 6/9/95
{
	// Find prop in favorites list
	// Add a new entry with the identical settings
	short		i;
	AssetSpec	*as;

	HLock(gRoomWin->faveProps);
	as = (AssetSpec *) *gRoomWin->faveProps;
	for (i = 0; i < gRoomWin->nbrFaveProps; ++i,++as) {
		if (as->id == spec->id && as->crc == spec->crc) {
			break;
		}
	}
	if (i >= gRoomWin->nbrFaveProps)
		return;
	HUnlock(gRoomWin->faveProps);
	SetHandleSize(gRoomWin->faveProps,(gRoomWin->nbrFaveProps+1)*sizeof(AssetSpec));
	as = (AssetSpec *) *gRoomWin->faveProps;
	BlockMove((Ptr)&as[i],(Ptr)&as[i+1],(gRoomWin->nbrFaveProps-i)*sizeof(AssetSpec));
	++gRoomWin->nbrFaveProps;
	RefreshPropPicker();
	SavePropFavorites();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ShowLooseProps();	// To aid in scripting, dump loose props to log
void ShowLooseProps()	// To aid in scripting, dump loose props to log
{
	RoomRec		*rp = &gRoomWin->curRoom;
	LPropPtr 	lp;
	long		type,id,crc;
	Str63		name;
	short		i;

	if (rp->nbrLProps == 0)
		return;
	lp  = (LPropPtr)&rp->varBuf[rp->firstLProp];
	for (i=0;i<rp->nbrLProps;++i)
	{
		if (gRoomWin->lPropHandles[i])
		{
			GetAssetInfo(gRoomWin->lPropHandles[i], &type, &id, name);
			crc = GetAssetCRC(gRoomWin->lPropHandles[i]);
			if (name[0])
				LogMessage("\"%.*s\" %d %d ADDLOOSEPROP\r",name[0],&name[1],lp->loc.h,lp->loc.v);
			if (lp->link.nextOfst)
				lp = (LPropPtr) &rp->varBuf[lp->link.nextOfst];
			else
				break;
		}
	}
}

void RefreshLProps(Rect *rr)
{
	RoomRec	*rp = &gRoomWin->curRoom;

	if (rp->nbrLProps > 0)
	{
		Rect		 r;
		Rect     r2;
		Handle	 h;
		short		 i;
		LPropPtr lp  = (LPropPtr)&rp->varBuf[rp->firstLProp];

		for (i=0;i<rp->nbrLProps;++i)
		{
			SetRectMac(&r,0,0,PropWidth,PropHeight);
			OffsetRectMac(&r,lp->loc.h,lp->loc.v);
			if (SectRectMac(rr,&r,&r2))
				if ((h = gRoomWin->lPropHandles[i]) != NULL)
					DrawProp(h,r.left+LeftMargin,r.top+TopMargin, 0);

			if (lp->link.nextOfst)
				lp = (LPropPtr) &rp->varBuf[lp->link.nextOfst];
			else
				break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MoveLooseProp(long *cmd)
{
	LPropPtr lp;
	Rect		 r;
	Rect     r2;
	short    i;
	short		 propNbr = (short)cmd[0];
	RoomRec *rp      = &gRoomWin->curRoom;

	if (propNbr < rp->nbrLProps)
	{
		lp = (LPropPtr) &rp->varBuf[rp->firstLProp];
		for (i = 0; i < propNbr; ++i)
		{
			if (lp->link.nextOfst)
				lp = (LPropPtr) &rp->varBuf[lp->link.nextOfst];
		}
		SetRectMac(&r,0,0,PropWidth,PropHeight);
		OffsetRectMac(&r,lp->loc.h,lp->loc.v);
		lp->loc = *((Point *) &cmd[1]);
		SetRectMac(&r2,0,0,PropWidth,PropHeight);
		OffsetRectMac(&r2,lp->loc.h,lp->loc.v);
		UnionRectMac(&r,&r2,&r);
		RefreshRoom(&r);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LPropPtr PtInLooseProp(LPropRec *prop, Point p, short index, short *retIndex)
{
	LPropPtr	lp2;
	Rect		r;
	if (prop->link.nextOfst) {
		if ((lp2 = PtInLooseProp((LPropPtr) &gRoomWin->curRoom.varBuf[prop->link.nextOfst],p,index+1,retIndex)) != NULL)
			return lp2;
	}
	SetRectMac(&r,0,0,PropWidth,PropHeight);
	OffsetRectMac(&r,prop->loc.h,prop->loc.v);
	if (PtInRectMac(p,&r) && PtInProp(p,topLeft(r),&prop->propSpec)) {
		*retIndex = index;
		return prop;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DelLooseProp(long *cmd)
{
	LPropPtr	lp,lastLP;
	RoomRec		*rp = &gRoomWin->curRoom;
	short		propDel = (short)cmd[0],i;
	Rect		r;

	if (propDel == -1)
	{
		rp->nbrLProps = 0;
		rp->firstLProp = 0;
		ModifyLoosePropHandles();
		RefreshRoom(&gOffscreenRect);
	}
	else {
		lp = (LPropPtr) &rp->varBuf[rp->firstLProp];
		lastLP = NULL;
		for (i = 0; i < propDel; ++i) {
			if (lp->link.nextOfst) {
				lastLP = lp;
				lp = (LPropPtr) &rp->varBuf[lp->link.nextOfst];
			}
		}
		if (propDel == 0)
			rp->firstLProp = lp->link.nextOfst;
		else
			lastLP->link.nextOfst = lp->link.nextOfst;
		--rp->nbrLProps;
		SetRectMac(&r,0,0,PropWidth,PropHeight);
		OffsetRectMac(&r,lp->loc.h,lp->loc.v);
		ModifyLoosePropHandles();
		RefreshRoom(&r);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AddLooseProp(long *cmd)
{
	LPropRec		lNew;
	RoomRec			*rp = &gRoomWin->curRoom;
	Rect			r;
	long			propID = cmd[0];
	unsigned long	crc = cmd[1];
	Point			propLoc= *((Point *) &cmd[2]);

	memset(&lNew,0,sizeof(LPropRec));
	lNew.propSpec.id = propID;
	lNew.propSpec.crc = crc;
	lNew.loc = propLoc;
	lNew.link.nextOfst = rp->firstLProp;
	rp->firstLProp = AddRoomBuffer((Ptr) &lNew,sizeof(LPropRec));
	rp->nbrLProps++;
	ModifyLoosePropHandles();
	SetRectMac(&r,0,0,PropWidth,PropHeight);
	OffsetRectMac(&r,lNew.loc.h,lNew.loc.v);
	RefreshRoom(&r);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

