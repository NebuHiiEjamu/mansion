/* S-SECURE.C */

#include "s-server.h"
#include "s-secure.h"
#include "s-timout.h"	/* 1/17/97 JAB */
#include <time.h>

/* 9/8/95 - moved Swap routines to M-UTILS.c */

#include "m-magic.h"

unsigned LONG ComputeLicenseCRC(unsigned LONG ctr)
{
	unsigned LONG crc = CRC_MAGIC;
	unsigned char *p = (unsigned char *) &ctr;
	Boolean LittleEndian();
    if (LittleEndian())
		SwapLong((unsigned LONG *) &ctr);
	crc = ((crc << 1L) | ((crc & 0x80000000L)? 1 : 0)) ^ gCRCMask[p[0]];
	crc = ((crc << 1L) | ((crc & 0x80000000L)? 1 : 0)) ^ gCRCMask[p[1]];
	crc = ((crc << 1L) | ((crc & 0x80000000L)? 1 : 0)) ^ gCRCMask[p[2]];
	crc = ((crc << 1L) | ((crc & 0x80000000L)? 1 : 0)) ^ gCRCMask[p[3]];
	return crc;
}

Boolean ValidUserSerialNumber(unsigned LONG crc, unsigned LONG counter)
{
	counter ^= (MAGIC_LONG ^ crc);
	return crc == ComputeLicenseCRC(counter);
}

Boolean NewbieUserSerialNumber(unsigned LONG crc, unsigned LONG counter)
{
	counter ^= (MAGIC_LONG ^ crc);
	if (crc != ComputeLicenseCRC(counter))
		return true;
	if (counter < 500)	/* Counter^magic_long on newbie serial# is 0 */
		return true;
	return false;
}

void UserOldSerialCheck(ServerUserPtr user)
{
}

Boolean UserSecurityCheck(AuxRegistrationRec *lPtr, ServerUserPtr	user)
{
	ServerUserPtr	curUser;
	LONG			oldIP,newIP;
	IPBanRecPtr		ipRec;
	LONG			penaltyTime = gPrefs.deathPenaltyMinutes*60L;

	if ((ipRec = GetIPBanRecByIP(GetIPAddress(user),0)) != NULL) {
		unsigned LONG	curTime;
		curTime = time(NULL);
		penaltyTime = ipRec->penaltyTime;
		/* If time remaining and
		 * all-inclusive or member is a guest
		 */
		if (curTime - ipRec->timeLastPenalized < penaltyTime &&
			(!(ipRec->banFlags & (BR_SiteBan1 | BR_SiteBan2)) ||
				NewbieUserSerialNumber(lPtr->crc, lPtr->counter))) {
			ScheduleUserKill(user, K_DeathPenaltyActive, 0);
			return true;
		}
	}

	if (!ValidUserSerialNumber(lPtr->crc, lPtr->counter)) {
		ScheduleUserKill(user, K_InvalidSerialNumber, 0);
		return true;
	}
#if HIGHSECURITY
	else {
		unsigned LONG counter;
		/* check for valid range */
		/* valid ranges are 0-0
		                    500-1499
		                    100000-199999
		                    500000000-500100000
		 */
		counter = lPtr->counter ^ (MAGIC_LONG ^ lPtr->crc);
		if ((counter >= 1 && counter < 500) ||
			(counter >= 1500 && counter <= 99999) ||
			(counter >= 200000 && counter < 500000000L) ||
			counter > 500100000L) 
		{
			ScheduleUserKill(user,K_InvalidSerialNumber, 3000);
			return true;
		} 
	}
#endif
	user->crc = lPtr->crc;
	user->counter = lPtr->counter;

	/* 1/17/97 JAB */
	user->puidCtr = lPtr->puidCtr;
	user->puidCRC = lPtr->puidCRC;
	user->demoElapsed = lPtr->demoElapsed;
	user->totalElapsed = lPtr->totalElapsed;

	/* Check if newbie */
	if (NewbieUserSerialNumber(user->crc,user->counter)) {
		user->flags |= U_Guest;
		user->user.faceNbr = FACE_Smile;
		user->user.colorNbr = GuestColor;

		/* 1/27/97 JAB Don't automatically kill guests if demomembers are allowed (kill later...) */
		if (!(gPrefs.permissions & PM_AllowGuests) && !(gPrefs.serverOptions & SO_AllowDemoMembers)) {	/* 8/17/95 */
			ScheduleUserKill(user, K_NoGuests, 0);
			return true;
		}
		/* 1/14/97 JAB New Style Guest... */
		if (user->puidCtr) {
			Handle				h;
			AltBasePtr			ap;

			/* Kick off if the guest has been banned */
			if ((ipRec = GetIPBanRecByPuidKey(user->puidCtr,0)) != NULL) {
				unsigned LONG	curTime;
				curTime = time(NULL);
				penaltyTime = ipRec->penaltyTime;
				if (curTime - ipRec->timeLastPenalized < penaltyTime) {
					ScheduleUserKill(user, K_DeathPenaltyActive, 0);
					return true;
				}
			}
			/* Check for AltBaseRec */
			if ((h = GetAltBaseRec(user->puidCtr)) != NULL) {
				/* Additional security checks */
				unsigned LONG	curTime;
				GetDateTime(&curTime);

				HLock(h);
				ap = (AltBasePtr) *h;
				/* clear bad flags */
				ap->lastFlags &= ~(U_Penalized | U_Banished | U_Kill | U_CommError | U_Pin);
				/* automatically ungag if 2 hours have passed */
				if (curTime - ap->timeLastActive > 120*60L) {
					ap->lastFlags &= ~(U_Gag);
					ap->lastFlags &= ~(U_PropGag);
				}
				user->flags = ap->lastFlags;
				user->flags &= ~(U_SuperUser | U_God);
				/* 1/27/97 JAB force guest mode here */
				user->flags |= U_Guest;
				if (ap->totalElapsed > user->totalElapsed)
					user->totalElapsed = ap->totalElapsed;
				if (ap->demoElapsed > user->demoElapsed)
					user->demoElapsed = ap->demoElapsed;
				HUnlock(h);
				DisposeHandle(h);
			}

#if USEGUESTTIMEOUT
			if (user->totalElapsed > GuestTimeOut) 
			{
				/* Kick off */
				ScheduleUserKill(user, K_DemoExpired, 0);
				return true;
			}
#endif
			/* 1/27/97 Allow Member Access if permission is set and timer is small enough */
			if (user->demoElapsed < DemoTimeOut && (gPrefs.serverOptions & SO_AllowDemoMembers)) {
				/* Permit Member Access */
				user->flags &= ~U_Guest;
			}
			
			/* 1/27/97 JAB Kick off if still a guest, and guests aren't allowed */
			if (!(gPrefs.permissions & PM_AllowGuests) && (user->flags & U_Guest))
			{
				ScheduleUserKill(user, K_NoGuests, 0);
				return true;
			}

			/* 8/4/95 Check for Duplicates */
			for (curUser = gUserList; curUser; curUser = curUser->nextUser) {
				if (curUser != user && curUser->puidCtr == user->puidCtr) {
					oldIP = GetIPAddress(curUser);
					newIP= GetIPAddress(user);
					if (curUser->connectionType != C_AppleTalk)
						ScheduleUserKill(curUser, K_DuplicateUser, 0);
				}
			}
		}
	}
	else {	/* Not a newbie */
		/* If not, check if duplicate user */
		Handle			h;
		UserBasePtr		up;
		AltBasePtr		ap;

		/* 1/16/97 JAB */
		if (user->puidCtr && (ipRec = GetIPBanRecByPuidKey(user->puidCtr,0)) != NULL) {
			unsigned LONG	curTime;
			curTime = time(NULL);
			penaltyTime = ipRec->penaltyTime;
			if (curTime - ipRec->timeLastPenalized < penaltyTime) {
				ScheduleUserKill(user, K_DeathPenaltyActive, 0);
				return true;
			}
		}

		if ((ipRec = GetIPBanRecByCounter(user->counter,0)) != NULL) {
			unsigned LONG	curTime;
			curTime = time(NULL);
			penaltyTime = ipRec->penaltyTime;
			if (curTime - ipRec->timeLastPenalized < penaltyTime) {
				ScheduleUserKill(user, K_DeathPenaltyActive, 0);
				return true;
			}
		}

		/* 1/16/97 JAB */
		if (user->puidCtr && (h = GetAltBaseRec(user->puidCtr)) != NULL) {
			/* Additional security checks */
			unsigned LONG	curTime;
			GetDateTime(&curTime);

			HLock(h);
			ap = (AltBasePtr) *h;
			/* clear bad flags */
			/* 1/27/97 Force Member Mode Here */
			ap->lastFlags &= ~(U_Penalized | U_Banished | U_Kill | U_CommError | U_Pin | U_Guest);
			/* automatically ungag if 2 hours have passed */
			if (curTime - ap->timeLastActive > 120*60L) {
				ap->lastFlags &= ~(U_Gag);
				ap->lastFlags &= ~(U_PropGag);
			}
			user->flags = ap->lastFlags;
			user->flags &= ~(U_SuperUser | U_God);
			if (ap->totalElapsed > user->totalElapsed)
				user->totalElapsed = ap->totalElapsed;
			if (ap->demoElapsed > user->demoElapsed)
				user->demoElapsed = ap->demoElapsed;
			HUnlock(h);
			DisposeHandle(h);
		}
		else if ((h = GetUserBaseRec(user->counter)) != NULL) {
			/* Additional security checks */
			unsigned LONG	curTime;
			GetDateTime(&curTime);

			HLock(h);
			up = (UserBasePtr) *h;
			/* clear bad flags */
			up->lastFlags &= ~(U_Penalized | U_Banished | U_Kill | U_Guest | U_CommError | U_Pin);
			/* automatically ungag if 2 hours have passed */
			if (curTime - up->timeLastActive > 120*60L) {
				up->lastFlags &= ~(U_Gag);
				up->lastFlags &= ~(U_PropGag);
			}
			user->flags = up->lastFlags;
			/* New - deny auto-wizard or superuser */
			user->flags &= ~(U_SuperUser | U_God);
			HUnlock(h);
			DisposeHandle(h);
		}
		/* 8/4/95 Check for Duplicates AFTER Banishments */
		/* 1/16/97 JAB removed unused DuplicateUserCode */
		for (curUser = gUserList; curUser; curUser = curUser->nextUser) {
			if (curUser != user && curUser->counter == user->counter) {
				oldIP = GetIPAddress(curUser);
				newIP= GetIPAddress(user);
				if (curUser->connectionType != C_AppleTalk)
					ScheduleUserKill(curUser, K_DuplicateUser, 0);
			}
		}
	}
	return false;
}

