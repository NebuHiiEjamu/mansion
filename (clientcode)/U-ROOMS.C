// Room.c
#include "U-USER.H"
#include "m-cmds.H"
#include "U-SNDS.H"
#include "U-SCRIPT.H"	// 7/24/95
#include "U-SECURE.H"
#include "U-TIMOUT.H"   // 1/14/97 JAB

#if PPASUPPORT
#include "PPAMgr.h"
#endif

void ThisIsYourRec(LONG id)
{
	gRoomWin->meID = id;

	// 1/23/97 JAB - using alternate style MSG_LOGON to support timeout feature
	//
	AlternateLogon();

	/** - old method...

		log.crc = gSecure.crc;
		log.counter = gSecure.counter;
		BlockMove(gPrefs.name, log.userName, gPrefs.name[0]+1);
		PostServerEvent(MSG_LOGON, gRoomWin->meID, (Ptr) &log, 8 + log.userName[0]+1);

	 **/

	// 1/14/97 JAB Modified to start timing session
	BeginSessionTimer();
}

void ComputePropsRect(LocalUserRecPtr up, Rect *r)	// 6/8/95 - modified to use variable list
{
	short	        ox,oy;
	Rect	        tmpRect;
	Point	        propOffset;
	Handle	        h;
	PropHeaderPtr	pp;
	short			i;
	if (up->user.nbrProps) {
		for (i = 0; i < up->user.nbrProps; ++i) {
			h = up->propHandle[i];
			if (h == NULL)
				continue;			// 7/27/95
			pp = (PropHeaderPtr) *h;
            /* 9/8/95 eddie isn't using swaps */
			propOffset.h = pp->hOffset;		// 5/23 Changed Name JAB
			propOffset.v = pp->vOffset;
			SetRectMac(&tmpRect,0,0,PropWidth,PropHeight);
			ox = up->user.roomPos.h + propOffset.h-FaceWidth/2;
			oy = up->user.roomPos.v + propOffset.v-FaceHeight/2;
			OffsetRectMac(&tmpRect,ox,oy);
			if (i > 0)
				UnionRectMac(&tmpRect,r,r);
			else
				*r = tmpRect;
		}
	}
}

void ComputePropRect(short propIdx,LocalUserRecPtr up,Rect *r)
{
	short	        ox,oy;
	Point	        propOffset;
	Handle	      h;
	PropHeaderPtr	pp;

	if ((h = up->propHandle[propIdx]) != NULL)
	{
		pp = (PropHeaderPtr) *h;
        /* 9/8/95 eddie wasn't using swaps */
		propOffset.h = pp->hOffset;		// 5/23 Changed Name JAB
		propOffset.v = pp->vOffset;
		SetRectMac(r,0,0,PropWidth,PropHeight);
		ox = up->user.roomPos.h + propOffset.h-FaceWidth/2;
		oy = up->user.roomPos.v + propOffset.v-FaceHeight/2;
		OffsetRectMac(r,ox,oy);
	}
	else
		SetRectMac(r,0,0,0,0);
}


void ComputeNameRect(LocalUserRecPtr up, Rect *r)
{
#if macintosh
	FontInfoMac	fi;
	short		w,h,v;
	OffscreenTextEnable();				// 6/13/95 local function
	GetFontInfoMac(&fi);				// 6/13/95

	// NOTE: This code should be identical to that in RefreshFaces()
	w = StringWidth(up->user.name);
	h = up->user.roomPos.h - w/2;
	v = up->user.roomPos.v + NameGap + fi.ascent;
	if (v+fi.descent > gOffscreenRect.bottom)
		v -= (v+fi.descent)-gOffscreenRect.bottom;
	if (h+w > gOffscreenRect.right)
		h -= (h+w) - gOffscreenRect.right;
	if (v-fi.ascent < 0)
		v += gOffscreenRect.top-(v-fi.ascent);
	if (h < gOffscreenRect.left)
		h = gOffscreenRect.left;
	SetRect(r,h-1,v-(fi.ascent+1),h+w+1,v+fi.descent+1);

/**
	r->left   = (up->user.roomPos.h - StringWidth(up->user.name)/2) - 1;
	r->right  = r->left +  StringWidth(up->user.name) + 2;
	r->top    = up->user.roomPos.v + NameGap;
	r->bottom = r->top+fi.ascent+fi.descent+1;
**/

	OffscreenTextRestore();				// 6/13/95 local function

#else
	GetNameRect(up,r);
#endif
}

void ComputeUserRect(LocalUserRecPtr	up, Rect *r)
{
	SetRectMac(r,0,0,FaceWidth,FaceHeight);
	OffsetRectMac(r,up->user.roomPos.h-FaceWidth/2,up->user.roomPos.v-FaceHeight/2);
}

void ComputeUserRRect(LocalUserRecPtr up, Rect *r)
{
	Rect	r2;

	ComputeUserRect(up,r);
	if (up->user.nbrProps)		// 6/7/95
    {
		ComputePropsRect(up,&r2);
		UnionRectMac(r,&r2,r);
	}
	if (gShowNames)				// 6/13/95
    { 
		ComputeNameRect(up,&r2);
		UnionRectMac(r,&r2,r);
	}
}

void RefreshUser(LocalUserRecPtr cUser)
{
	Rect r;
	ComputeUserRRect(cUser,&r);	// 6/13/95 use whole rect
	RefreshRoom(&r);
}

void ClearRoom(void)
{
	short	i,j;
	LocalUserRecPtr	up;
	HotspotPtr hsl = (HotspotPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];
	PictureRecPtr prl = (PictureRecPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.pictureOfst];

#if PPASUPPORT
	PPAMgrKillAllPPAs(PPAKILL_ROOM);
#endif

	DeletePeople();
	ClearBalloons();
	for (i = 0; i < gRoomWin->curRoom.nbrHotspots; ++i)
		CollapseHotspot(&hsl[i]);
	for (i = 0; i < gRoomWin->curRoom.nbrPictures; ++i)
		CollapsePicture(&prl[i]);
	for (i = 0; i < gRoomWin->curRoom.nbrLProps; ++i) {
		if (gRoomWin->lPropHandles[i]) {
			ReleaseAsset(gRoomWin->lPropHandles[i]);
			gRoomWin->lPropHandles[i] = NULL;
		}
	}
	up = gRoomWin->userList;
	for (i = 0; i < gRoomWin->curRoom.nbrPeople; ++i,++up)
		for (j = 0; j < 3; ++j)
			if (up->propHandle[j])
				ReleaseAsset(up->propHandle[j]);

	// commented out to test if it's causing a bug...
	ReleaseAssets(RT_PROP);		// 10/9/95 Release all other loaded props...
								// Note: face handles have been detached already...
								
	gRoomWin->curRoom.nbrPeople = 0;
}


// 7/24/96 JAB modified to only reload picture information when
// this is called with fullReloadFlag set to false (in this case,
// we've been called by the file download manager, because we've
// received a new image file
//
void ModifyRoomInfo(RoomRecPtr room, Boolean fullReloadFlag)
{
	short			i;
	HotspotPtr 		hsl;
	PictureRecPtr	prl;

	prl = (PictureRecPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.pictureOfst];

	if (fullReloadFlag) {
		hsl = (HotspotPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];
		for (i = 0; i < gRoomWin->curRoom.nbrHotspots; ++i)
			CollapseHotspot(&hsl[i]);
	}

	for (i = 0; i < gRoomWin->curRoom.nbrPictures; ++i)
		CollapsePicture(&prl[i]);

	if (fullReloadFlag)
		BlockMove((Ptr) room,(Ptr) &gRoomWin->curRoom, sizeof(RoomRec) + room->lenVars);

	prl = (PictureRecPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.pictureOfst];
	if (fullReloadFlag) {
		hsl = (HotspotPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];
		for (i = 0; i < gRoomWin->curRoom.nbrHotspots; ++i,++hsl)	// 6/7/95
			ExpandHotspot(hsl);
	}

	for (i = 0; i < gRoomWin->curRoom.nbrPictures; ++i,++prl)	// 6/7/95
		ExpandPicture(prl);

	if (fullReloadFlag)
		ModifyLoosePropHandles();

	// 7/24/96 JAB - preventing a memoryleak from scripts being reloaded
	// when only a picture has been downloaded
	if (fullReloadFlag)
		LoadRoomScripts();		// 7/24/95

	RefreshNewRoom();
}

void ModifyLoosePropHandles()
{
	short			i;
	LPropPtr		lp;
	RoomRec			*rp = &gRoomWin->curRoom;
	for (i = 0; i < gRoomWin->curRoom.nbrLProps; ++i)
		gRoomWin->lPropHandles[i] = NULL;
	lp = (LPropPtr) &rp->varBuf[rp->firstLProp];
	for (i = 0; i < gRoomWin->curRoom.nbrLProps; ++i)	{
		gRoomWin->lPropHandles[i] = GetUserAsset(RT_PROP,&lp->propSpec);
		if (lp->link.nextOfst)
			lp = (LPropPtr) &rp->varBuf[lp->link.nextOfst];
		else
			break;
	}
}

void NewRoom(RoomRecPtr room)
{
	short			i;
	HotspotPtr 		hsl;
	PictureRecPtr	prl;

/**
	if (gDebugFlag) {
		void DumpBufferToLog(char *label, unsigned char *ptr, LONG len, LONG offset);
		DumpBufferToLog("ROOMREC", (unsigned char*) room, sizeof(RoomRec) + room->lenVars,
						(LONG) (LONG) &room->varBuf[0] - (LONG) room);
	}
**/
	gEnteringRoom = true;
	ClearRoom();
	BlockMove((Ptr) room,(Ptr) &gRoomWin->curRoom, sizeof(RoomRec) + room->lenVars);
	gRoomWin->curRoom.nbrPeople = 0;
	hsl = (HotspotPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];
	prl = (PictureRecPtr) &gRoomWin->curRoom.varBuf[gRoomWin->curRoom.pictureOfst];
	for (i = 0; i < gRoomWin->curRoom.nbrHotspots; ++i)
		ExpandHotspot(&hsl[i]);
	for (i = 0; i < gRoomWin->curRoom.nbrPictures; ++i)
		ExpandPicture(&prl[i]);
	ModifyLoosePropHandles();
}

void EndRoomDescription(void)
{
	gRoomWin->mePtr = GetUser(gRoomWin->meID);	// sets to NULL because we haven't entered
	gRoomWin->targetID = 0;
	RefreshNewRoom();
	LoadRoomScripts();	// 7/24/95 - parse room scripts (moved from server code)
	// 4/6/95 JBUM Removed PE_Startup Event passing...
	gEnteringRoom = false;
}


extern Point			gCenterP;
PersonID				gLastLog;
unsigned long			gLastLogTime;	// 11/15

// 5/24/95 JAB 
//
LocalUserRecPtr GetUser(long id)
{
	short	i;
	LocalUserRecPtr	up = gRoomWin->userList;	// 5/24/95
	
	for (i = 0; i < gRoomWin->curRoom.nbrPeople; ++i,++up) {
		if (up->user.userID == id)		// 5/24/95
			return up;
	}
	return NULL;
}

LocalUserRecPtr GetUserByName(char *name)
{
	short	i;
	Str63	pName;

	strcpy((char *) pName, name);
	CtoPstr((char *) pName);
	for (i = 0; i < gRoomWin->curRoom.nbrPeople; ++i) {
		if (EqualPString(pName,gRoomWin->userList[i].user.name,false))
			return &gRoomWin->userList[i];
	}
	return NULL;
}

void DeletePeople(void)
{
	short	i;
	for (i = 0; i < MaxPeoplePerRoom; ++i) {
		memset(&gRoomWin->userList[i],0,sizeof(LocalUserRec));
	}
	gRoomWin->nbrFadeIns = gRoomWin->nbrFadeOuts = 0L;
}

void RoomPerson(UserRecPtr person)
{
	LocalUserRecPtr	cUser;
	short			i;
	cUser = &gRoomWin->userList[gRoomWin->curRoom.nbrPeople];
	cUser->user = *person;
	cUser->userFade = 0;
	cUser->localFlags &= ~(UF_FadeIn | UF_FadeOut | UF_IRCSparkyAware);
	for (i = 0; i < MaxUserProps; ++i) {	// 6/7/95
		if (i < cUser->user.nbrProps)		// 6/7/95
			cUser->propHandle[i] = GetUserAsset(RT_PROP,&cUser->user.propSpec[i]);
		else
			cUser->propHandle[i] = NULL;
	}
	CheckUserFace(cUser);		// 6/22/95
	++gRoomWin->curRoom.nbrPeople;
	gRoomWin->mePtr = GetUser(gRoomWin->meID);
}

void RoomPeople(short n, UserRecPtr ary)
{
	while (n--) {
		RoomPerson(ary);
		// 11/5/95 cancel gLastLog which causes "ping" sound
		if (gLastLog == ary->userID)
			gLastLog = 0;
		ary++;
	}
}


void UpdateUserPosition(PersonID userID, Point *newpos)
{
	LocalUserRecPtr cUser;
	Rect		oldR,newR,tr;
	cUser = GetUser(userID);
	if (cUser) {
		// Erase User in Old Position !!!
		KillUserBalloons(userID);
		ComputeUserRRect(cUser,&oldR);
		cUser->user.roomPos = *newpos;
		ComputeUserRRect(cUser,&newR);
		if (SectRectMac(&oldR,&newR,&tr)) {
			UnionRectMac(&oldR,&newR,&tr);
			RefreshRoom(&tr);
		}
		else {
			RefreshRoom(&oldR);
			RefreshRoom(&newR);
		}

	}
}

void UpdateUserFace(PersonID userID, short faceNbr)
{
	LocalUserRecPtr cUser;
	cUser = GetUser(userID);
	if (cUser) {
		cUser->user.faceNbr = faceNbr;
		RefreshUser(cUser);
	}
}

// 6/7/95 JAB modified to use assetspec, modified command format
//
// 4/24/96 JAB modified to set user prop animation fields correctly
//
void CheckUserFace(LocalUserRecPtr cUser)
{
	short	i,n,nbrAnimateProps=0;
	n = cUser->user.nbrProps;
	cUser->localFlags &= ~UF_PropFace;
	cUser->localFlags &= ~UF_IsAnimating;
	cUser->localFlags &= ~UF_IsPalindrome;

	for (i = 0; i < n; ++i) {	// 6/7/95
		if (cUser->propHandle[i]) {		// 6/7/95
			if (((PropHeaderPtr) *cUser->propHandle[i])->flags & PF_PropFaceFlag) {
				cUser->localFlags |= UF_PropFace;
				// return;
			}
			if (((PropHeaderPtr) *cUser->propHandle[i])->flags & PF_Animate) {
				++nbrAnimateProps;
				gRoomWin->hasFaceAnimations = true;
				cUser->localFlags |= UF_IsAnimating;
				if (((PropHeaderPtr) *cUser->propHandle[i])->flags & PF_Palindrome) {
					cUser->localFlags |= UF_IsPalindrome;
				}
			}
		}
	}

	if (!(gPrefs.userPrefsFlags & UPF_UsePropAnimations))
		gRoomWin->hasFaceAnimations = false;

	if (nbrAnimateProps < 2)
		cUser->localFlags &= ~UF_IsAnimating;

	cUser->nbrAnimateProps = nbrAnimateProps;
	cUser->propFrameNumber = 0;
	cUser->palindromeDirection = false;
	cUser->lastAnimate = GetTicks();
}

void UpdateUserProp(PersonID userID, long *p)
{
	LocalUserRecPtr cUser;
	Rect			r,r2;
	short			i;

	cUser = GetUser(userID);
	if (cUser) {
		ComputeUserRRect(cUser,&r);
		if (p) {  /* 4/17/96 eddie */
			cUser->user.nbrProps = (short)*p;	++p;
			// 6/7/95
			BlockMove((Ptr)p,(Ptr)&cUser->user.propSpec[0],cUser->user.nbrProps * sizeof(AssetSpec));
		}
		for (i = 0; i < MaxUserProps; ++i) {	// 6/7/95
			if (i < cUser->user.nbrProps)		// 6/7/95
				cUser->propHandle[i] = GetUserAsset(RT_PROP,&cUser->user.propSpec[i]);	// 6/7/95
			else
				cUser->propHandle[i] = NULL;
		}
		CheckUserFace(cUser);		// 6/22/95
		ComputeUserRRect(cUser,&r2);
		UnionRectMac(&r,&r2,&r);
		RefreshRoom(&r);
	}
}

void UpdateUserDesc(PersonID userID, short *p)	// 6/7/95 New Function
{
	LocalUserRecPtr cUser;
	Rect			r,r2;
	short			i;
	short			nbrProps;
	long			*lp;


	cUser = GetUser(userID);
	if (cUser) {
		ComputeUserRRect(cUser,&r);
		cUser->user.faceNbr = *p;	++p;
		cUser->user.colorNbr = *p;	++p;
		lp = (long *) p;
		nbrProps = cUser->user.nbrProps = (short)*lp;	++lp;

		BlockMove((Ptr)lp,(Ptr)&cUser->user.propSpec[0],nbrProps * sizeof(AssetSpec));
		for (i = 0; i < MaxUserProps; ++i) {
			if (i < nbrProps)
				cUser->propHandle[i] = GetUserAsset(RT_PROP,&cUser->user.propSpec[i]);
			else
				cUser->propHandle[i] = NULL;
		}
		CheckUserFace(cUser);			// 6/22/95
		ComputeUserRRect(cUser,&r2);
		UnionRectMac(&r,&r2,&r);
		RefreshRoom(&r);
	}
}


void UpdateUserColor(PersonID userID, short colorNbr)
{
	LocalUserRecPtr cUser;
	cUser = GetUser(userID);
	if (cUser) {
		cUser->user.colorNbr = colorNbr;
		RefreshUser(cUser);
	}
}


void UpdateUserName(PersonID userID, StringPtr name)
{
	LocalUserRecPtr cUser;
	Rect			oldR,r;
	cUser = GetUser(userID);
	if (cUser) {
		ComputeUserRRect(cUser,&oldR);	// 6/13/95 use whole rect
		BlockMove(name,cUser->user.name,name[0]+1);
		// User may have shrunk - use union of before and after
		ComputeUserRRect(cUser,&r);	// 6/13/95 use whole rect
		UnionRectMac(&oldR,&r,&r);
		RefreshRoom(&r);
	}
}

void LogonPerson(PersonID userID)
{
	gLastLog = userID;
	gLastLogTime = GetTicks();	// 11/5/95
	ShowRoomStatus();
}

void LogOff(void)
{
	gRoomWin->navInProgress = false;  // 9/12/95
	RefreshSplash();		//  6/15/95
}

void LogoffPerson(PersonID userID)
{
	ShowRoomStatus();
	if (userID == gRoomWin->meID) {
		ExitPerson(userID);
		LogOff();
	}
	// 11/5/95 - don't ping when person aren't in the room
	else if (GetUser(userID) != NULL) {
		FadeOutPerson(userID);
		PlaySnd(S_Fader, 1);
#ifdef WIN32
		LocalLogoffPerson(userID);
#endif
	}
}

void FadeOutPerson(PersonID userID)
{
	Boolean		fadeOut = true;
	
	if (userID == gRoomWin->meID)
		fadeOut = false;
	else if (gRoomWin->noFades || !(gPrefs.userPrefsFlags & UPF_UseFadeEffects))
		fadeOut = false;

	if (fadeOut) {
		LocalUserRecPtr	up;
		short		i;
		up = gRoomWin->userList;
		for (i = 0; i < gRoomWin->curRoom.nbrPeople; ++i,++up) {
			if (up->user.userID == userID) {
				KillUserBalloons(userID);
				up->userFade = 15;
				up->localFlags |= UF_FadeOut;
				if (up->localFlags & UF_FadeIn) {
					up->localFlags &= ~UF_FadeIn;
					gRoomWin->nbrFadeIns--;
				}
				gRoomWin->nbrFadeOuts++;
        		SetIdleTimer(TIMER_FAST);
 			}
		}
	}
	else
		ExitPerson(userID);
}

void ExitPerson(PersonID userID)
{
	short		i;
	Rect		rf;
	LocalUserRecPtr	up;

	if (userID == gRoomWin->meID)
		goto ExitExit;
	if (userID == gRoomWin->targetID) {
		gRoomWin->targetID = 0;
		RefreshRoom(&gOffscreenRect);
	}
	up = gRoomWin->userList;
	for (i = 0; i < gRoomWin->curRoom.nbrPeople; ++i,++up) {
		if (up->user.userID == userID)
			break;
	}
	if (i >= gRoomWin->curRoom.nbrPeople)
		goto ExitExit;

	if (up->localFlags & UF_FadeOut)		// 5/24/95
		--gRoomWin->nbrFadeOuts;			// 5/24/95

	KillUserBalloons(userID);
	ComputeUserRRect(up,&rf);

#ifdef WIN32
	LocalExitPerson(up);
#endif

	for (; i < gRoomWin->curRoom.nbrPeople-1; ++i) {
		gRoomWin->userList[i] = gRoomWin->userList[i+1];
	}
	--gRoomWin->curRoom.nbrPeople;
	gRoomWin->mePtr = GetUser(gRoomWin->meID);
	RefreshRoom(&rf);
ExitExit:
	ShowRoomStatus();
}

void ChangeFace(short faceNbr)
{
	UserRecPtr	cUser;
	if (gRoomWin->mePtr == NULL)
		return;
	cUser = &gRoomWin->mePtr->user;
	if (faceNbr >= gIconSetup->nbrFaces)
		faceNbr = 0;
	if (faceNbr != cUser->faceNbr) {
		PostServerEvent(MSG_USERFACE,gRoomWin->meID,(Ptr) &faceNbr,sizeof(short));
		UpdateUserFace(cUser->userID,faceNbr); // 6/25/95
	}
}

void ChangeColor(short colorNbr)
{
	UserRecPtr	cUser;
	if (gRoomWin->mePtr == NULL)
		return;
	cUser = &gRoomWin->mePtr->user;
	if (colorNbr >= NbrColors)
		colorNbr = 0;
	if (MembersOnly(colorNbr != 3))
		return;
	if (colorNbr != cUser->colorNbr) {
		PostServerEvent(MSG_USERCOLOR,gRoomWin->meID,(Ptr) &colorNbr,sizeof(short));
		UpdateUserColor(cUser->userID,colorNbr);	// 6/25/95
	}
}

// 6/7/95 - all in one user description change
void ChangeUserDesc(short faceNbr, short colorNbr, short nbrProps, AssetSpec *specList)
{
	struct UserDescSpec {
		short	faceNbr;
		short	colorNbr;
		long	nbrProps;
		AssetSpec	assetSpecs[MaxUserProps];
	} uSpec;
	UserRecPtr	cUser;
	if (gRoomWin->mePtr == NULL)
		return;
	cUser = &gRoomWin->mePtr->user;
	if (faceNbr >= gIconSetup->nbrFaces)
		faceNbr = 0;
	uSpec.faceNbr = faceNbr;
	uSpec.colorNbr = colorNbr;
	uSpec.nbrProps = nbrProps;
	BlockMove((Ptr)specList,(Ptr)uSpec.assetSpecs,nbrProps * sizeof(AssetSpec));	// 6/10/95
	PostServerEvent(MSG_USERDESC,gRoomWin->meID,(Ptr) &uSpec,sizeof(short)*2+sizeof(long)+sizeof(AssetSpec)*nbrProps);
	UpdateUserDesc(cUser->userID, (short *) &uSpec);
}


void ChangeProp(short nbrProps, AssetSpec *specList)	// 6/7/95 modified for new command format
{
	struct ChangePropSpec {
		long		nbrProps;
		AssetSpec	assetSpecs[MaxUserProps];
	} uSpec;
	UserRecPtr	cUser;
	if (gRoomWin->mePtr == NULL)
		return;
	cUser = &gRoomWin->mePtr->user;
	uSpec.nbrProps = nbrProps;	
	if (nbrProps)
		BlockMove((Ptr)specList,(Ptr)uSpec.assetSpecs,nbrProps*sizeof(AssetSpec));
	PostServerEvent(MSG_USERPROP,gRoomWin->meID,(Ptr) &uSpec,sizeof(long)+sizeof(AssetSpec)*nbrProps);
	UpdateUserProp(cUser->userID,(long *) &uSpec);
}


// 6/7/95 NEW JAB
void DonProp(long id, unsigned long crc)	// Put on prop with id# !!!
{
	// Append to end
	long		nbrProps;
	AssetSpec	assetSpecs[MaxUserProps];
	UserRecPtr	cUser;
	if (gRoomWin->mePtr == NULL)
		return;
	cUser = &gRoomWin->mePtr->user;
	nbrProps = cUser->nbrProps;
	if (nbrProps >= MaxUserProps)
		return;
	BlockMove((Ptr)&cUser->propSpec[0],(Ptr)&assetSpecs[0],nbrProps*sizeof(AssetSpec));
	assetSpecs[nbrProps].id = id;
	assetSpecs[nbrProps].crc = crc;
	++nbrProps;
	ChangeProp((short)nbrProps,assetSpecs);
}

void DonPropByName(char *cString)	// 6/7/95 
{
	char	pString[255];
	char	len;
	Handle	h;
	len = strlen(cString);
	BlockMove(cString,&pString[1],len+1);
	pString[0] = len;
	if ((h = GetAssetByName(RT_PROP,(StringPtr) pString)) != NULL) {
		long	type,id,crc;
		GetAssetInfo(h,&type,&id,(StringPtr) pString);
		crc = GetAssetCRC(h);
		DonProp(id,crc);
	}
}

// 6/7/95 NEW JAB
void DoffProp(long id, unsigned long crc) // take off prop with id# !!!
{
	// Append to end
	long	nbrProps;
	AssetSpec	assetSpecs[MaxUserProps];
	short	i;
	UserRecPtr	cUser;
	if (gRoomWin->mePtr == NULL)
		return;
	cUser = &gRoomWin->mePtr->user;
	nbrProps = cUser->nbrProps;
	if (nbrProps == 0)
		return;
	BlockMove((Ptr)cUser->propSpec,(Ptr)assetSpecs,nbrProps*sizeof(AssetSpec));
	for (i = 0; i < nbrProps; ++i) {
		if (assetSpecs[i].id == id && (crc == 0L || assetSpecs[i].crc == crc)) {
			// 6/21/95 Bug Fix
			BlockMove((Ptr)&assetSpecs[i+1],(Ptr)&assetSpecs[i],((nbrProps-1)-i)*sizeof(AssetSpec));
			--nbrProps;
			break;
		}
	}
	ChangeProp((short)nbrProps,assetSpecs);
}

void DoffPropByName(char *cString) 		// 6/7/95
{
	char	pString[255];
	char	len;
	Handle	h;
	len = strlen(cString);
	BlockMove(cString,&pString[1],len+1);
	pString[0] = len;
	if ((h = GetAssetByName(RT_PROP,(StringPtr) pString)) != NULL) {
		long	type,id,crc;
		GetAssetInfo(h,&type,&id,(StringPtr) pString);
		crc = GetAssetCRC(h);
		DoffProp(id,crc);
	}
}


void ClearProps(void)		// take off all props# !!!!
{
	ChangeProp(0,NULL);
}

void NewPerson(UserRecPtr person)
{
	LocalUserRecPtr	cUser;

	// 5/24/95  Kill Ghosts
	while ((cUser = GetUser(person->userID)) != NULL)
		ExitPerson(person->userID);

	RoomPerson(person);
	cUser = GetUser(person->userID);
	if (!gRoomWin->noFades) {
		if ((gPrefs.userPrefsFlags & UPF_UseFadeEffects) > 0) {
			cUser->userFade = 1;
			cUser->localFlags |= UF_FadeIn;
			gRoomWin->nbrFadeIns++;
	    	SetIdleTimer(TIMER_FAST);
		}
		if (gLastLog == person->userID) {
			// Only ping if person has logged on recently
			if (GetTicks() - gLastLogTime < 15L*TICK_SECONDS)
				PlaySnd(S_Fader,1);
			gLastLog = false;
		}
	}
	RefreshUser(cUser);
	ShowRoomStatus();

#ifdef WIN32
  LocalNewPerson(cUser);
#endif

	// 4/6/95 JBUM Added PE_Enter Event
	if (person->userID == gRoomWin->meID) {
		gRoomWin->navInProgress = false;  // 9/12/95
		if (gRoomWin->signOn) {
			gRoomWin->signOn = false;
			TriggerHotspotEvents(PE_SignOn);
		}
		TriggerHotspotEvents(PE_Enter);
	}
}

// 5/10/95  Local Processing of Color Changes
//

// no longer used in mac version 10/18/95
void ChangeColorLocal(short colorNbr)
{
	if (gRoomWin->mePtr == NULL)
		return;
	if (colorNbr >= NbrColors)
		colorNbr = 0;
	if (colorNbr != gRoomWin->mePtr->user.colorNbr)
		UpdateUserColor(gRoomWin->meID,colorNbr);
}

// 5/9/95 Simplified to not permit clicking on hotspots.
//
void MoveUser(short hd, short vd)
{
	Point		np;
	Hotspot		*navR;

	if (gEnteringRoom || gRoomWin->mePtr == NULL)
		return;

	np = gRoomWin->mePtr->user.roomPos;
	np.h += hd;
	np.v += vd;
	if (np.h-FaceWidth/2 < 0) {
		np.h = FaceWidth/2;
	}

	if (np.h+FaceWidth/2 > RoomWidth) {
		np.h = RoomWidth-FaceWidth/2;
	}
	if (np.v-FaceHeight/2 < 0) {
		np.v = FaceHeight/2;
	}
	if (np.v+FaceHeight/2 > RoomHeight) {
		np.v = RoomHeight-FaceHeight/2;
	}

	// Can't move out of navigable area - if already out, s'ok
	// cause we need to move back in...
	navR = GetNavArea();
	if (navR &&
		!PtInHotspot(np,navR) &&
		PtInHotspot(gRoomWin->mePtr->user.roomPos,navR))
		return;

	PostServerEvent(MSG_USERMOVE,gRoomWin->meID,(Ptr) &np,sizeof(Point));
	UpdateUserPosition(gRoomWin->meID,&np);
}

// 5/10/95 - Local Processingof Movement (not sent to server)
// 10/19/95 no longer used in mac version
void MoveUserLocal(short hd, short vd)
{
	Point		np;
	Hotspot		*navR;

	if (gEnteringRoom || gRoomWin->mePtr == NULL)
		return;

	np = gRoomWin->mePtr->user.roomPos;
	np.h += hd;
	np.v += vd;
	if (np.h-FaceWidth/2 < 0) {
		np.h = FaceWidth/2;
	}

	if (np.h+FaceWidth/2 > RoomWidth) {
		np.h = RoomWidth-FaceWidth/2;
	}
	if (np.v-FaceHeight/2 < 0) {
		np.v = FaceHeight/2;
	}
	if (np.v+FaceHeight/2 > RoomHeight) {
		np.v = RoomHeight-FaceHeight/2;
	}

	// Can't move out of navigable area - if already out, s'ok
	// cause we need to move back in...
	navR = GetNavArea();
	if (navR &&
		!PtInHotspot(np,navR) &&
		PtInHotspot(gRoomWin->mePtr->user.roomPos,navR))
		return;
	UpdateUserPosition(gRoomWin->meID,&np);
}

short AddRoomString(StringPtr str)
{
	short	retVal;
	retVal = gRoomWin->curRoom.lenVars;
	BlockMove(str,&gRoomWin->curRoom.varBuf[retVal],str[0]+1);
	gRoomWin->curRoom.lenVars += str[0]+1;
	return retVal;
}

short AddRoomBuffer(Ptr str, long len)
{
	short	retVal;
	retVal = gRoomWin->curRoom.lenVars;
	BlockMove(str,&gRoomWin->curRoom.varBuf[retVal],len);
	gRoomWin->curRoom.lenVars += (short)len;
	return retVal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 7/14/95 - New Function to process draw commands...

void ProcessDrawCmd(DrawRecPtr dp)
{
	RoomRec			*rp = &gRoomWin->curRoom;
	DrawRecPtr		dpp;
	DrawRecord		tmp;
	short			drawCmdOfst;
	short			i;
	switch (dp->drawCmd) {
	case DC_Detonate:
		rp->nbrDrawCmds = 0;
		rp->firstDrawCmd = 0;
		break;
	case DC_Delete:
		if (rp->nbrDrawCmds) {
			--rp->nbrDrawCmds;
			if (rp->nbrDrawCmds == 0)
				rp->firstDrawCmd = 0;
		}
		break;
	default:
		tmp.link.nextOfst = 0;
		tmp.drawCmd = dp->drawCmd;
		tmp.cmdLength = dp->cmdLength;
		tmp.dataOfst = AddRoomBuffer((Ptr) &dp[1],dp->cmdLength);
		drawCmdOfst = AddRoomBuffer((Ptr) &tmp, sizeof(DrawRecord));

		if (rp->nbrDrawCmds) {
			dpp = (DrawRecPtr) &rp->varBuf[rp->firstDrawCmd];
			for (i = 0; i < rp->nbrDrawCmds-1; ++i)
				dpp = (DrawRecPtr) &rp->varBuf[dpp->link.nextOfst];
			dpp->link.nextOfst = drawCmdOfst;
		}
		else {
			rp->firstDrawCmd = drawCmdOfst;
		}
		++rp->nbrDrawCmds;
		break;
	}
	RefreshRoom(&gOffscreenRect);
}

void PrivateChatSelect(LONG userID, StringPtr name)
{
	if (gRoomWin->targetID != userID) {
		gRoomWin->targetID = userID;
		BlockMove(name, gRoomWin->targetUserName, name[0]+1);
		RefreshRoom(&gOffscreenRect);
		ShowRoomStatus();
	}
}


