/* s-memrooms.c - functions pertaining to maintainence of member-created rooms */

#include "s-server.h"

Boolean	gRoomHasBeenDeleted;
extern RoomRecPtr      gRoom;

void ScheduleRoomDeletionCheck()
{
	gRoomHasBeenDeleted = true;
}


void ProcessRoomDeletions()
{
	ServerRoomPtr	cRoom,nextRoom=NULL;

	if (!gRoomHasBeenDeleted)
		return;

	gRoomHasBeenDeleted = false;

	for (cRoom = gRoomList; cRoom; cRoom = nextRoom) {
		nextRoom = cRoom->nextRoom;
		if (cRoom->memberOwner && cRoom->room.nbrPeople == 0 && GetServerUserByCounter(cRoom->memberOwner) == NULL) {
			DeleteRoom(cRoom->room.roomID);
		}
	}
}

void NewMemberRoom(ServerUserPtr cUser, char *name)
{
	RoomID			roomID;
	ServerRoomPtr	room;
	/* make sure user is not a guest */
	if (cUser->flags & U_Guest)
		return;
	/* make sure we don't already own one, or we're a wizard */
	if (!(cUser->flags & U_SuperUser)) {
		ServerRoomPtr	cRoom;
		for (cRoom = gRoomList; cRoom; cRoom = cRoom->nextRoom) {
			if (cRoom->memberOwner == cUser->counter) {
				UserMessage(cUser, "Sorry - you may only create one room at a time");
				return;
			}
		}
	}
	if (*name && !LegalRoomName(name)) {
		UserMessage(cUser, "Sorry - that name is illegal");
		return;
	}
	roomID = MakeNewRoom(cUser, true, name);
	if (roomID == 0)
		return;
	room = GetRoom(roomID);
	room->memberOwner = cUser->counter;
	cUser->lastOwnedRoom = roomID;
	UserMessage(cUser, "You have created a new room - type \'help for a list of useful commands");
}

Boolean LegalRoomName(char *name)
{
	char	*p;
	Boolean	alphaFlag=false;
	if (strlen(name) < 2 || strlen(name) > 31)
		return false;
	for (p = name; *p; ++p) {
		/* Name may contain no control characters */
		if (*p < ' ')
			return false;
		/* Name must contain at least one alphanumeric */
		if (isalnum(*p)) {
			alphaFlag = true;
			break;
		}
	}
	if (!alphaFlag || isspace(*name) || *name == '-')
		return false;

	/* insert future room name censorship here */	

	return true;
}

void UpdateRoomStatus(ServerRoomPtr room)
{
	PostRoomEvent(room->room.roomID,MSG_ROOMSETDESC,0L,(Ptr)&room->room, sizeof(RoomRec)+room->room.lenVars);
}


void MemberRoomModification(ServerUserPtr cUser, int modNumber, char *arg)
{
	ServerRoomPtr	cRoom,newRoom;
	cRoom = GetRoom(cUser->user.roomID);
	/* room exists? */
	if (cRoom == NULL)
		return;
	/* user is a member? */
	if (cUser->flags & U_Guest)
		return;
	/* cRoom is owned by somebody? */
	/* check if cUser is owner, or a wizard */
	if (cRoom->memberOwner == 0) {
		UserMessage(cUser, "Sorry - this is a public room");
		return;
	}
	if (!(cUser->flags & U_SuperUser) && cRoom->memberOwner != cUser->counter)
	{
		UserMessage(cUser, "Sorry - you are not the owner of this room");
		return;
	}
	/* okay, all is cool */
	switch (modNumber) {
	case MR_Title:
		if (*arg && LegalRoomName(arg)) {
			BeginRoomCreation();
			CopyRoom(&cRoom->room);
			/* affect the change */
			gRoom->roomNameOfst = AddRoomCtoPString(arg);
			newRoom = EditRoomToNewRoom();
			EndRoomCreation();
			ReplaceRoom(cRoom,newRoom,true);
			UserMessage(cUser,"Room name changed to %s",arg);
		}
		else
			UserMessage(cUser,"That room name is illegal");
		break;
	case MR_Picture:
		if (*arg && LegalRoomName(arg)) {
			BeginRoomCreation();
			CopyRoom(&cRoom->room);
			/* affect the change */
			gRoom->pictNameOfst = AddRoomCtoPString(arg);
			newRoom = EditRoomToNewRoom();
			EndRoomCreation();
			ReplaceRoom(cRoom,newRoom,true);
			UserMessage(cUser,"Room picture changed to %s",arg);
		}
		else
			UserMessage(cUser,"That picture name is illegal");
		break;
	case MR_Owner:
		{
			ServerUserPtr	tUser;

			if (*arg == 0) {
				UserMessage(cUser, "You didn't specify a name");
				return;
			}

			CtoPstr(arg);
			tUser = GetServerUserByName((StringPtr) arg);
			if (tUser) {
				if (tUser->flags & U_Guest) {
					UserMessage(cUser, "Sorry, you can't transfer ownership to a guest");
				}
				else {
					StringPtr	rName;
					cRoom->memberOwner = tUser->counter;
					if (cRoom->room.roomNameOfst)
						rName = (StringPtr) &cRoom->room.varBuf[cRoom->room.roomNameOfst];
					else
						rName = (StringPtr) "";
					RoomMessage(cRoom->room.roomID, "Ownership tranferred to %s",CvtToCString(tUser->user.name));
					TimeLogMessage("%s transferred ownership of room %s to %s\r",
									CvtToCString(cUser->user.name),
									CvtToCString(rName),
									CvtToCString(tUser->user.name));
					tUser->lastOwnedRoom = cRoom->room.roomID;
				}
			}
			else {
				PtoCstr((StringPtr) arg);
				UserMessage(cUser, "Can't find %s",arg);
			}
		}
		break;
	case MR_Kick:
		{
			ServerUserPtr	tUser;
			ServerRoomPtr	hellRoom;

			if (*arg == 0) {
				UserMessage(cUser, "You didn't specify a name");
				return;
			}

			CtoPstr(arg);
			tUser = GetServerUserByName((StringPtr) arg);
			if (tUser && tUser->user.roomID == cRoom->room.roomID) {
				/* !!! Bug - won't work if front gate is full */
				/* !!! if so and so is in room, find a room to kick him too, and kick him */
				/* !!! find a room which is public, not closed or private, has an OK occupancy */
				if ((hellRoom = GetRoom(777)) != NULL)
					ChangeUserRoom(tUser, 777, true);
				else
					ChangeUserRoom(tUser, gEntrance, true);
				
				/* add so and so to kick record */
				PtoCstr((StringPtr) arg);
				SetUserAction(cUser, arg, UA_Kick, true);
				/* feedback to users in room: so and so has been kicked out */
				RoomMessage(cRoom->room.roomID,"%s kicked out by %s",
						CvtToCString(tUser->user.name),
						CvtToCString(cUser->user.name));
				/* feedback to target: you have been kicked */
				UserMessage(tUser,"You have been kicked out by %s",
						CvtToCString(cUser->user.name));
			}
			else {
				PtoCstr((StringPtr) arg);
				UserMessage(cUser, "Can't find %s",arg);
			}
		}
		/* requires mods in user->room navigation */
		break;
	case MR_Unkick:
		{
			ServerUserPtr	tUser;

			if (*arg == 0) {
				UserMessage(cUser, "You didn't specify a name");
				return;
			}

			CtoPstr(arg);
			tUser = GetServerUserByName((StringPtr) arg);
			if (tUser) {
				/* remove so and so from kick record */
				PtoCstr((StringPtr) arg);
				SetUserAction(cUser, arg, UA_Kick, false);
				/* feedback to users in room: so and so has been unkicked */
				RoomMessage(cRoom->room.roomID,"%s unkicked by %s",
						CvtToCString(tUser->user.name),
						CvtToCString(cUser->user.name));
				/* feedback to target: you have been unkicked */
				UserMessage(tUser,"You have been unkicked by %s",
						CvtToCString(cUser->user.name));
			}
			else {
				PtoCstr((StringPtr) arg);
				UserMessage(cUser, "Can't find %s",arg);
			}
		}
		break;
	case MR_Password:
		/* no password opens it up */
		if (*arg) {
			/* set the password */
			if (LegalRoomName(arg)) {
				strcpy(cRoom->roomPassword,arg);
				RoomMessage(cRoom->room.roomID, "%s has set a password for this room",CvtToCString(cUser->user.name));
			}
			else
				UserMessage(cUser, "That password is illegal - no change made");
		}
		else {
			/* clear the password */
			cRoom->roomPassword[0] = 0;
			RoomMessage(cRoom->room.roomID, "%s has cleared the password for this room",CvtToCString(cUser->user.name));
		}
		break;
	case MR_Close:
		/* double check how closed flag works */
		cRoom->room.roomFlags |= RF_Closed | RF_Private;
		RoomMessage(cRoom->room.roomID, "%s has closed the room",CvtToCString(cUser->user.name));
		break;
	case MR_Open:
		cRoom->room.roomFlags &= ~(RF_Closed | RF_Private);
		RoomMessage(cRoom->room.roomID, "%s has opened the room",CvtToCString(cUser->user.name));
		break;
	case MR_Scripts:
		if (strcmp(arg,"off") == 0) {
			cRoom->room.roomFlags |= RF_CyborgFreeZone;
			RoomMessage(cRoom->room.roomID, "%s has turned scripts off",CvtToCString(cUser->user.name));
		}
		else {
			cRoom->room.roomFlags &= ~RF_CyborgFreeZone;
			RoomMessage(cRoom->room.roomID, "%s has turned scripts on",CvtToCString(cUser->user.name));
		}
		UpdateRoomStatus(cRoom);
		break;
	case MR_Painting:
		if (strcmp(arg,"off") == 0) {
			cRoom->room.roomFlags |= RF_NoPainting;
			RoomMessage(cRoom->room.roomID, "%s has turned painting off",CvtToCString(cUser->user.name));
		}
		else {
			cRoom->room.roomFlags &= ~RF_NoPainting;
			RoomMessage(cRoom->room.roomID, "%s has turned painting on",CvtToCString(cUser->user.name));
		}
		UpdateRoomStatus(cRoom);
		break;
	case MR_Guests:
		if (strcmp(arg,"off") == 0) {
			cRoom->room.roomFlags |= RF_NoGuests;
			RoomMessage(cRoom->room.roomID, "%s is not allowing guests to enter",CvtToCString(cUser->user.name));
		}
		else {
			cRoom->room.roomFlags &= ~RF_NoGuests;
			RoomMessage(cRoom->room.roomID, "%s is allowing guests to enter",CvtToCString(cUser->user.name));
		}
		break;
	case MR_Hide:
	case MR_Unhide:
		if (strcmp(arg,"off") == 0 || modNumber == MR_Unhide) {
			cRoom->room.roomFlags &= ~RF_Hidden;
			RoomMessage(cRoom->room.roomID, "%s has stopped hiding the room",CvtToCString(cUser->user.name));
		}
		else {
			cRoom->room.roomFlags |= RF_Hidden;
			RoomMessage(cRoom->room.roomID, "%s has hidden the room",CvtToCString(cUser->user.name));
		}
		break;
	case MR_Delete:
		DeleteRoom(cRoom->room.roomID);
		break;
	default:
		break;
	}
}

/* used to select a password prior to navigating */
void SetUserNavPassword(ServerUserPtr cUser, char *arg)
{
	if (arg && *arg && LegalRoomName(arg)) {
		strcpy(cUser->navPassword,arg);
		if (cUser->lastPasswordRoom) {
			/* go there */
			ServerRoomPtr	pRoom;
			RoomID			roomID;
			roomID = cUser->lastPasswordRoom;
			pRoom = GetRoom(cUser->lastPasswordRoom);
			if (pRoom->memberOwner && pRoom->roomPassword[0] && !IsRoomOwner(cUser, pRoom)) {
				/* check for password validity and go there */
				if (strcmp(pRoom->roomPassword, cUser->navPassword) == 0) {
					UserMessage(cUser, "Password Succeeded");
					cUser->lastPasswordRoom = 0;
					ChangeUserRoom(cUser, roomID, false);
				}
				else
					UserMessage(cUser, "Password Incorrect");
			}
			else {
				/* room is no longer password protected, or we own it, try going there */
				UserMessage(cUser, "Navigation Password Set");
				cUser->lastPasswordRoom = 0;
				ChangeUserRoom(cUser, roomID, false);
			}
		}
		else
			UserMessage(cUser, "Navigation Password Set");
	}
	else {
		UserMessage(cUser, "That is not a valid password");
	}
}

Boolean	IsRoomOwner(ServerUserPtr cUser, ServerRoomPtr cRoom)
{
	if (cUser == NULL || cRoom == NULL)
		return false;
	if (cRoom->memberOwner == 0)
		return false;
	if (cUser->flags & U_God)
		return true;
	if (cRoom->memberOwner != cUser->counter)
		return false;
	return true;
}
