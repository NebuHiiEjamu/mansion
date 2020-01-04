// Queues.c
//
// Routines for keeping track of players
//

#include "WOFBot.h"


void RemoveFromQueue(QueueHeader *qh, short nbr)
{
	short			i;
	QueueElement	*qp,*lq;
	lq = NULL;
	for (i = 0,qp = qh->firstElement; i < qh->nbrElements; ++i,qp = qp->nextElement)
	{
		if (i == nbr) {
			if (lq)
				lq->nextElement = qp->nextElement;
			else
				qh->firstElement = qp->nextElement;
			if (((PlayerRecord *) qp)->heapRecord) {
				// Free Heap Record
				DisposePtr((Ptr) qp);
			}
			else {
				// Free up for use in freeplayerpool
				((PlayerRecord *) qp)->inUse = false;
			}
			--qh->nbrElements;
			return;
		}
		lq = qp;
	}
}

void AddToQueue(QueueHeader *qh, QueueElement *pr)
{
	QueueElement	*qp;

	if (qh->firstElement == NULL) {
		qh->firstElement = pr;		
	}
	else {
		qp = qh->firstElement;
		while (qp->nextElement)
			qp = qp->nextElement;
		qp->nextElement = pr;
	}
	qh->nbrElements++;
}

void CountActivePlayers()
{
	QueueElement	*qp;
	short			i;
	gWOF.nbrActivePlayers = 0;
	for (i = 0,qp = gWOF.playerList.firstElement; i < gWOF.playerList.nbrElements; ++i,qp = qp->nextElement)
	{
		if (((PlayerRecord *) qp)->present && ((PlayerRecord *) qp)->playing)
			++gWOF.nbrActivePlayers;
	}
}

#define MaxPlayerPool	32

PlayerRecord	gPlayerPool[MaxPlayerPool];

PlayerRecord	*GetFreePlayerPool();

PlayerRecord	*GetFreePlayerPool()
{
	short	i;
	for (i = 0; i < MaxPlayerPool; ++i) {
		if (!gPlayerPool[i].inUse) {
			memset(&gPlayerPool[i],0,sizeof(PlayerRecord));
			gPlayerPool[i].inUse = true;
			return &gPlayerPool[i];
		}
	}
	return NULL;
}

PlayerRecord *NewPlayer(PersonID userID)
{
	PlayerRecord	*pr;

	pr = GetFreePlayerPool();
	if (pr == NULL) {
		pr = (PlayerRecord *) NewPtrClear(sizeof(PlayerRecord));
		pr->heapRecord = true;
	}
	pr->playerID = userID;
	pr->present = true;
	pr->playing = true;
	pr->lastPresent = TickCount();
	pr->lastActive = TickCount();
	pr->newbie = 4;
	AddToQueue(&gWOF.playerList, (QueueElement *) pr);
	gWOF.nbrActivePlayers++;
	CheckCuteNick(pr);
	return pr;
}

PlayerRecord *GetPlayerRecord(PersonID whoID)
{
	QueueElement	*qp;
	short			i;

	for (i = 0,qp = gWOF.playerList.firstElement; 
		 i < gWOF.playerList.nbrElements; 
		 ++i,qp = qp->nextElement)
	{
		if (((PlayerRecord *) qp)->playerID == whoID)
			return (PlayerRecord *) qp;
	}
	return NULL;
}

PlayerRecord *GetPlayerRecordByName(char *name)
{
	UserRecPtr	up = NULL;
	up = GetPersonByName(name);
	if (up) {
		return GetPlayerRecord(up->userID);
	}
	else
		return NULL;
}

char *GetPlayerName(PersonID userID)
{
	UserRecPtr	up;
	PlayerRecord	*pr;
	pr = GetPlayerRecord(userID);
	if (pr == NULL)
		return "";
	up = GetPersonByID(userID);
	if (up) {
		BlockMove(up->name,pr->nick,up->name[0]+1);
		PtoCstr((StringPtr) pr->nick);
	}
	return pr->nick;
}

void ClearPlayers()
{
	while (gWOF.playerList.nbrElements) {
		RemoveFromQueue(&gWOF.playerList, 0);
	}
}
