#include "s-server.h"

/* Tap Functions */
LONG	tapper,tappee,taproom;

void TAPOn(ServerUserPtr cUser, char *arg)
{
	ServerUserPtr tUser;
	ServerRoomPtr tRoom;
	short		roomID = 0;
	tapper = tappee = taproom = 0;
	roomID = atoi(arg);
	CtoPstr(arg);
	tUser = GetServerUserByName((StringPtr) arg);
	if (tUser) {
		tapper = cUser->user.userID;
		tappee = tUser->user.userID;
		UserMessage(cUser,"; %s Tapped",CvtToCString(tUser->user.name));
	}
	else if (roomID && (tRoom = GetRoom(roomID)) != NULL) {
		tapper = cUser->user.userID;
		taproom = tRoom->room.roomID;
	    UserMessage(cUser,"; Room %d Tapped",(int) taproom);
	}
	else
		UserMessage(cUser,"; can't find %s",CvtToCString((StringPtr) arg));
}

void TAPOff(ServerUserPtr cUser, char *arg)
{
	tapper = 0;
	UserMessage(cUser,"; Tap Disabled");
}

void TAPMessage(ServerUserPtr cUser, ServerUserPtr tUser, char *buffer, 
Boolean encrypted)
{
	char			tbuf[256];
	short			len;
	ServerUserPtr	tapUser,tapVictim=NULL;

	if (tapper == 0)
		return;
	if (cUser == NULL)
		return;

	tapUser = GetServerUser(tapper);
	if (tapUser == NULL) {
		tapper = 0;
		return;
	}

	if (cUser == tapUser || tUser == tapUser)
		return;

	if (tappee) {
		tapVictim = GetServerUser(tappee);
		if (tapVictim == NULL) {
			UserPrivateMessage(cUser, "Tappee Left");
			tapper = 0;
			return;
		}
		if (cUser != tapVictim && tUser != tapVictim)
			return;
	}
	if (taproom) {
		if (cUser->user.roomID != taproom)
			return;
	}
	if (encrypted) {
		len = *((short *) buffer) - 3;
		if (len > 0 && len < 256)
			DecryptCString((unsigned char*) buffer+2, 
				(unsigned char *) tbuf, len);
	}
	else
		strcpy(tbuf,buffer);
	if (tUser)
		UserPrivateMessage(tapUser,"; Message from %s to %s: %.170s",
							CvtToCString(cUser->user.name),
							CvtToCString(tUser->user.name),
							tbuf);
	else
		UserPrivateMessage(tapUser,"; %s: %.220s",
							CvtToCString(cUser->user.name),
							tbuf);
}


