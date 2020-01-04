/* s-usrbas.c */
/* */
/* Server User Database Maintainence */


#include "s-server.h"
#include "s-secure.h"

/* 1/14/96 JAB */
#ifdef macintosh
  #define RT_ALTBASE      'Gest'
#else
  #define RT_ALTBASE      0x47657374L    /* 'Gest' */
#endif

/* 1/14/96 */
Handle GetAltBaseRec(LONG counter)
{
	Handle		h;
	/* counter ^= MAGIC_LONG; */
	if (counter) {
		if ((h = GetAsset(RT_ALTBASE, counter)) != NULL) {
			DetachAsset(h);
			return h;
		}
	}
	return NULL;
}

Handle GetUserBaseRec(LONG counter)
{
	Handle		h;
	counter ^= MAGIC_LONG;
	if (counter) {
		if ((h = GetAsset(RT_USERBASE, counter)) != NULL) {
			DetachAsset(h);
			return h;
		}
	}
	return NULL;
}

/* 1/14/97 JAB */
void UpdateAltDatabase(ServerUserPtr user, short whyKilled);
void UpdateAltDatabase(ServerUserPtr user, short whyKilled)
{
	Handle			h,h2;
	AltBasePtr	up;
	unsigned LONG	curTime;
	Boolean			beingPenal = false;
	unsigned LONG	assetID;
	

	if (!ValidUserSerialNumber(user->crc, user->counter) || user->puidCtr == 0)
		return;

	/* If newbie, don't bother */
	if ((h = GetAltBaseRec(user->counter)) != NULL)
		SetHandleSize(h, sizeof(AltBaseRec) + user->user.name[0]);
	else
		h = NewHandleClear(sizeof(AltBaseRec) + user->user.name[0]);
	HLock(h);
	up = (AltBasePtr) *h;

	/* GetIPUserBaseRec(GetIPAddress(user), &ipRec); */
	/* up->crc = user->crc; */
	/* up->counter = user->counter; */
	
	assetID = user->puidCtr;	/* 1/14/97 JAB */

	up->lastIPAddress = GetIPAddress(user);

	GetDateTime(&curTime);
	up->timeLastActive = curTime;
	++up->nbrLogins;

	if (user->deathPenalty)
		beingPenal = true;

	if (user->user.roomID == 0) {
		++up->nbrFailedAttempts;
	}
	else
		++up->nbrLogins;

	up->lastFlags = user->flags;
	up->whyLastKilled = whyKilled;

	up->totalElapsed = user->totalElapsed + (time(NULL) - user->signonTime);
	if (!(user->flags & U_Guest) && NewbieUserSerialNumber(user->crc,user->counter))
		up->demoElapsed = user->demoElapsed + (time(NULL) - user->signonTime);

	if (beingPenal) {
		if (whyKilled != K_KilledByPlayer)
			++up->nbrDishonorableDeaths;
		up->lastFlags |= U_Penalized;
		up->timeLastPenalized = curTime;
	}

	if (whyKilled == K_DeathPenaltyActive)						/* 8/3/95 bug fix */
		up->lastFlags |= U_Penalized;
	if (whyKilled == K_BanishKill || whyKilled == K_Banished)	/* 8/3/95 bug fix */
		up->lastFlags |= U_Banished;

	BlockMove(user->user.name,up->lastName,user->user.name[0]+1);

	/* rewritten 12/3/95 */
	if (beingPenal)
		BanUserByIP(NULL,user,whyKilled, user->deathPenalty, 0);

	HUnlock(h);

	/* Save Userbase Rec if valid user */
	if ((h2 = GetAsset(RT_ALTBASE, assetID)) != NULL) {
		LONG	len;
		len = GetHandleSize(h);
		SetHandleSize(h2, len);
		BlockMove(*h,*h2,len);
		ChangedAsset(h2);
		DisposeHandle(h);
		ReleaseAsset(h2);
	}
	else {
		AddAsset(h,RT_ALTBASE,assetID,NULL);
        ReleaseAsset(h);
	}
	UpdateAssetFile();
}


void UpdateUserDatabase(ServerUserPtr user, short whyKilled)
{
	Handle			h,h2;
	UserBasePtr		up;
	unsigned LONG	curTime;
	Boolean			beingPenal = false;
	unsigned LONG	assetID;
	
	/* 1/14/97 JAB */
	if (user->puidCtr)
	{
		UpdateAltDatabase(user,whyKilled);
		return;
	}

	if ((h = GetUserBaseRec(user->counter)) != NULL)
		SetHandleSize(h, sizeof(UserBaseRec) + user->user.name[0]);
	else
		h = NewHandleClear(sizeof(UserBaseRec) + user->user.name[0]);
	HLock(h);
	up = (UserBasePtr) *h;

	/* GetIPUserBaseRec(GetIPAddress(user), &ipRec); */
	/* up->crc = user->crc; */
	/* up->counter = user->counter; */
	
	assetID = user->counter ^ MAGIC_LONG;

	up->lastIPAddress = GetIPAddress(user);

	GetDateTime(&curTime);
	up->timeLastActive = curTime;
	++up->nbrLogins;

	if (user->deathPenalty)
		beingPenal = true;

	if (user->user.roomID == 0) {
		++up->nbrFailedAttempts;
	}
	else
		++up->nbrLogins;

	up->lastFlags = user->flags;
	up->whyLastKilled = whyKilled;

	if (beingPenal) {
		if (whyKilled != K_KilledByPlayer)
			++up->nbrDishonorableDeaths;
		up->lastFlags |= U_Penalized;
		up->timeLastPenalized = curTime;
	}

	if (whyKilled == K_DeathPenaltyActive)						/* 8/3/95 bug fix */
		up->lastFlags |= U_Penalized;
	if (whyKilled == K_BanishKill || whyKilled == K_Banished)	/* 8/3/95 bug fix */
		up->lastFlags |= U_Banished;
	BlockMove(user->user.name,up->lastName,user->user.name[0]+1);

	/* rewritten 12/3/95 */
	if (beingPenal)
		BanUserByIP(NULL,user,whyKilled, user->deathPenalty, 0);

	HUnlock(h);

	/* Save Userbase Rec if valid user */
	if (ValidUserSerialNumber(user->crc, user->counter) &&
	    !NewbieUserSerialNumber(user->crc, user->counter)) {
		if ((h2 = GetAsset(RT_USERBASE, assetID)) != NULL) {
			LONG	len;
			len = GetHandleSize(h);
			SetHandleSize(h2, len);
			BlockMove(*h,*h2,len);
			ChangedAsset(h2);
			DisposeHandle(h);
			ReleaseAsset(h2);
		}
		else {
			AddAsset(h,RT_USERBASE,assetID,NULL);
	        ReleaseAsset(h);
		}
	}
	else
		DisposeHandle(h);
	UpdateAssetFile();
}


/* Generates a report file of the user database */
void GenerateUserDataFile();
