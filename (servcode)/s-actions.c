/* s-actions.c */
#include "s-server.h"

/* These routines allow you to define one or more modifier flags from one user to another,
 * such as Mute, HideFrom, Follow etc.
 *
 *
 *
 */

UserActionPtr AddUserActionRecord(ServerUserPtr cUser, ServerUserPtr tUser)
{
	UserActionPtr	newRec;
	newRec = (UserActionPtr) NewPtrClear(sizeof(UserActionRec));
	if (newRec) {
		newRec->nextRec = cUser->actionList;
		newRec->target = tUser->user.userID;
		newRec->actionFlags = 0;
		cUser->actionList = newRec;
	}
	return newRec;
}


void DeleteUserActionRecord(ServerUserPtr cUser, ServerUserPtr tUser)
{
	UserActionPtr	lastRec = NULL,curRec,nextRec;
	for (curRec = cUser->actionList; curRec; curRec = nextRec) {
		nextRec = curRec->nextRec;
		if (curRec->target == tUser->user.userID) {
			if (lastRec)
				lastRec->nextRec = curRec->nextRec;
			else
				cUser->actionList = curRec->nextRec;
			DisposePtr((Ptr) curRec);
		}
		else
			lastRec = curRec;
	}
}

void DeleteAllUserActionRecords(ServerUserPtr cUser)
{
	UserActionPtr	curRec,nextRec;

#if BISERVER
	Boolean			gotOne=cUser->actionList != NULL;
#endif

	for (curRec = cUser->actionList; curRec; curRec = nextRec) {
		nextRec = curRec->nextRec;
		DisposePtr((Ptr) curRec);
	}
	cUser->actionList = NULL;

#if BISERVER
	/* 5/16/96 tell front end to clear action records */

	if (gotOne) {
		LONG	cmds[3];
		cmds[0] = cUser->user.userID;
		cmds[1] = 0;
		cmds[2] = 0;
		GlobalFrontEndEvent(0,0,bi_delaction,(Ptr) &cmds[0],sizeof(LONG)*3);
		cmds[0] = 0;
		cmds[1] = cUser->user.userID;
		cmds[2] = 0;
		GlobalFrontEndEvent(0,0,bi_delaction,(Ptr) &cmds[0],sizeof(LONG)*3);
	}

#endif

}

UserActionPtr FindUserActionRecord(ServerUserPtr cUser, ServerUserPtr tUser)
{
	UserActionPtr	curRec;
	for (curRec = cUser->actionList; curRec; curRec = curRec->nextRec) {
		if (curRec->target == tUser->user.userID)
			return curRec;
	}
	return NULL;
}

void SetUserAction(ServerUserPtr cUser, char *arg, LONG action, Boolean setFlag)
{
	UserActionPtr	aRec;
	ServerUserPtr	tUser;
	char			*actionStr=NULL;

	CtoPstr(arg);
	tUser = GetServerUserByName((StringPtr) arg);
	if (tUser == NULL) {
		UserMessage(cUser,"can't find %s",CvtToCString((StringPtr) arg));
		return;
	}
	if (tUser == cUser)
		return;
	if ((aRec = FindUserActionRecord(cUser,tUser)) == NULL)
		aRec = AddUserActionRecord(cUser,tUser);
	if (aRec == NULL)
		return;
	if (setFlag)
		aRec->actionFlags |= action;
	else
		aRec->actionFlags &= ~action;
	if (aRec->actionFlags == 0)
		DeleteUserActionRecord(cUser, tUser);

#if BISERVER
	if (action)	/* communciate MUTE/HIDE info to front ends */
	{
		LONG	cmds[3];
		cmds[0] = cUser->user.userID;
		cmds[1] = tUser->user.userID;
		cmds[2] = action;
		GlobalFrontEndEvent(0,0,setFlag? bi_addaction : bi_delaction,
							(Ptr) &cmds[0],sizeof(LONG)*3);
	}
#endif

	switch (action) {
	case UA_HideFrom:	actionStr = "hiding from";		break;
	case UA_Mute:		actionStr = "muting";			break;
	case UA_Follow:		actionStr = "following";		break;
	case UA_Kick:		actionStr = "kicking";			break;
	}
	if (actionStr) {
		UserMessage(cUser, "You are %s%s %s",
									setFlag? "" : "not ",actionStr,
									CvtToCString(tUser->user.name));
		TimeLogMessage("%s is %s%s %s\r",
									CvtToCString(cUser->user.name),
									setFlag? "" : "not ",actionStr,
									CvtToCString(tUser->user.name));
	}
}

Boolean ActionExists(ServerUserPtr cUser, ServerUserPtr tUser, LONG action)
{
	UserActionPtr	curRec;
	for (curRec = cUser->actionList; curRec; curRec = curRec->nextRec) {
		if (curRec->target == tUser->user.userID &&
			UserRank(cUser) >= UserRank(tUser))
			return (curRec->actionFlags & action) > 0;
	}
	return false;
}