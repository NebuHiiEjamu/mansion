// UserTCPSwap.c

#include "U-USER.H"
// #include "UserTCP.h"

// Byte Ordering...
void SwapUserList(long nbrUsers, Ptr p);
void SwapRoomList(long nbrRooms, Ptr p);


/* moved to M-UTILS.c

unsigned long SwapLong(unsigned long *n)
{
	*n =  ((*n >> 24) & 0x000000FFL) |
			((*n >> 8)  & 0x0000FF00L) |
			((*n << 8)  & 0x00FF0000L) |
			((*n << 24) & 0xFF000000L);
	return *n;
}
**/

/* moved to M-UTILS.c
unsigned short SwapShort(unsigned short *n)
{
	*n = ((*n >> 8) & 0x00FF) |
		 ((*n << 8) & 0xFF00);
	return *n;
}
**/

#define SSwapLong(n)	SwapLong((unsigned long *) n)
#define SSwapShort(n)	SwapShort((unsigned short *) n)

// Data on TCP/IP and over modems is swapped to use LSB data structures
// so that PCs can use the data more effectively.
//
// The following routines perform swapping of incoming and outgoing packets
// for the client.  A similar set of routines will be used by the server.
//

// Will work with either incoming or outgoing asset...


void SwapAsset(AssetBlockPtr p)
{
	SSwapLong(&p->type);
	SSwapLong(&p->spec.id);
	SwapLong(&p->spec.crc);
	SSwapLong(&p->blockSize);
	SSwapLong(&p->blockOffset);
	SSwapShort(&p->blockNbr);
	SSwapShort(&p->nbrBlocks);
	if (p->blockNbr == 0) {
		SSwapLong(&p->varBlock.firstBlockRec.flags);
		SSwapLong(&p->varBlock.firstBlockRec.size);
		if (p->type == RT_PROP || p->type == (long)LongSwap(RT_PROP)) {
			PropHeaderPtr	lp;
			lp = (PropHeaderPtr) &p->varBlock.firstBlockRec.data[0];
			SSwapShort(&lp->width);
			SSwapShort(&lp->height);
			SSwapShort(&lp->hOffset);
			SSwapShort(&lp->vOffset);
			SSwapShort(&lp->scriptOffset);		// 6/23/95
			SSwapShort(&lp->flags);				// 6/23/95
		}
	}
}

void SwapFile(FileBlockPtr p)
{
	SSwapLong(&p->transactionID);
	SSwapLong(&p->blockSize);
	SSwapShort(&p->blockNbr);
	SSwapShort(&p->nbrBlocks);
	if (p->blockNbr == 0)
		SSwapLong(&p->varBlock.firstBlockRec.size);
}


// Works with incoming and outgoing
void SwapIncomingUser(UserRecPtr p)
{
	short	i;
	SSwapLong(&p->userID);
	SSwapShort(&p->roomPos.h);
	SSwapShort(&p->roomPos.v);
	SSwapShort(&p->roomID);
	SSwapShort(&p->faceNbr);
	SSwapShort(&p->colorNbr);
	SSwapShort(&p->nbrProps);
	for (i = 0;i < p->nbrProps; ++i) {
		SSwapLong(&p->propSpec[i].id);
		SSwapLong(&p->propSpec[i].crc);
	}
	// Bool bool string
}

void SwapUserList(long nbrUsers, Ptr p)
{
	UserListPtr	up;
	while (nbrUsers--) {
		up = (UserListPtr) p;
		SSwapLong(&up->userID);
		SSwapShort(&up->flags);
		SSwapShort(&up->roomID);
		p += 8 + up->name[0] + (4 -(up->name[0] & 3));	/* 9/6/95 */
	}
}

void SwapRoomList(long nbrRooms, Ptr p)
{
	RoomListPtr	rp;
	while (nbrRooms--) {
		rp = (RoomListPtr) p;
		SSwapLong(&rp->roomID);
		SSwapShort(&rp->flags);
		SSwapShort(&rp->nbrUsers);
		p += 8 + rp->name[0] + (4 - (rp->name[0] & 3));	/* 9/6/95 */
	}
}


// Swap Outgoing DrawCmd Buffer
void SwapDrawCmdOutBuffer(short cmd, Ptr cp)
{
	short	n;
	switch (cmd & 0x00FF) {
	case DC_Ellipse:
		SSwapShort((short *) cp);	cp += 2;		// pensize
		SSwapShort((short *) cp);	cp += 2;		// ptCount
		SSwapShort((short *) cp);	cp += 2;		// foreColor
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;		// AnchorPt
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;		// pt
		SSwapShort((short *) cp);	cp += 2;
		break;	
	
	case DC_Path:	// 7 shorts, N shorts
		SSwapShort((short *) cp);	cp += 2;		// pensize
		n = *((short *) cp);
		SSwapShort((short *) cp);	cp += 2;		// ptCount
		SSwapShort((short *) cp);	cp += 2;		// foreColor
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;		// AnchorPt
		SSwapShort((short *) cp);	cp += 2;
		while (n--) {
			SSwapShort((short *) cp);	cp += 2;	// DataPts
			SSwapShort((short *) cp);	cp += 2;
		}
		break;	
	case DC_Shape:	// 8 shorts, skip 2 chars, 4 shorts
		SSwapShort((short *) cp);	cp += 2;		// shapetype
		SSwapShort((short *) cp);	cp += 2;		// pensize
		SSwapShort((short *) cp);	cp += 2;		// forecolor
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;		// backcolor
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;
		cp += 2;									// fillShape,fillPattern
		SSwapShort((short *) cp);	cp += 2;		// BoundsRect
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);
		break;
	}
}

void SwapDrawCmdInBuffer(short cmd, Ptr cp)
{
	short	n;
	switch (cmd & 0x00FF) {
	case DC_Ellipse:
		SSwapShort((short *) cp);	cp += 2;		// pensize
		SSwapShort((short *) cp);	cp += 2;		// ptCount
		SSwapShort((short *) cp);	cp += 2;		// foreColor
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;		// AnchorPt
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;		// pt
		SSwapShort((short *) cp);	cp += 2;
		break;	
	
	case DC_Path:	// 7 shorts, N shorts
		SSwapShort((short *) cp);	cp += 2;		// pensize
		SSwapShort((short *) cp);	
		n = *((short *) cp);
		cp += 2;		// ptCount
		SSwapShort((short *) cp);	cp += 2;		// foreColor
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;		// AnchorPt
		SSwapShort((short *) cp);	cp += 2;
		while (n--) {
			SSwapShort((short *) cp);	cp += 2;	// DataPts
			SSwapShort((short *) cp);	cp += 2;
		}
		break;	
	case DC_Shape:	// 8 shorts, skip 2 chars, 4 shorts
		SSwapShort((short *) cp);	cp += 2;		// shapetype
		SSwapShort((short *) cp);	cp += 2;		// pensize
		SSwapShort((short *) cp);	cp += 2;		// forecolor
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;		// backcolor
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;
		cp += 2;									// fillShape,fillPattern
		SSwapShort((short *) cp);	cp += 2;		// BoundsRect
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);	cp += 2;
		SSwapShort((short *) cp);
		break;
	}
}


// Works with Outgoing draw commmands only...
//
void SwapDrawCmdOut(DrawRecPtr p, Ptr cp)	// for outgoing drawcmds only
{
	short	cmd;
	cmd = p->drawCmd;

	SSwapShort(&p->drawCmd);
	SSwapShort(&p->cmdLength);
	SSwapShort(&p->dataOfst);
	SSwapShort(&p->link.nextOfst);
	SwapDrawCmdOutBuffer(cmd,cp);
}

// Works with Incoming draw commmands only...
//
void SwapDrawCmdIn(DrawRecPtr p, Ptr cp)	// for incoming drawcmds only
{
	SSwapShort(&p->drawCmd);
	SSwapShort(&p->cmdLength);
	SSwapShort(&p->dataOfst);
	SSwapShort(&p->link.nextOfst);
	SwapDrawCmdInBuffer(p->drawCmd,cp);
}


void SwapIncomingRoom(RoomRecPtr rp)
{
	PictureRecPtr	prp;	// Picture Rec Pointer
	LPropPtr		lpp;	// Loose Prop Pointer
	HotspotPtr		hsp;	// Hot Spot Pointer
	DrawRecPtr		drp;	// Draw Rec Pointer
	short			i;
	short			n;
	Point			*pp;	// Point Pointer
	EventHandlerPtr	ehp;	// Event Handler Ptr
	StateRecPtr		srp;	// State Rec Ptr
	
	SSwapShort(&rp->roomID);
	SSwapLong(&rp->roomFlags);
	SSwapShort(&rp->roomNameOfst);
	SSwapShort(&rp->pictNameOfst);
	SSwapShort(&rp->artistNameOfst);
	SSwapShort(&rp->passwordOfst);
	SSwapShort(&rp->nbrPeople);
	SSwapLong(&rp->facesID);
	SSwapShort(&rp->lenVars);
	// Swap Hotspots
	//
	SSwapShort(&rp->nbrHotspots);
	SSwapShort(&rp->hotspotOfst);
	hsp = (HotspotPtr) &rp->varBuf[rp->hotspotOfst];
	for (i = 0; i < rp->nbrHotspots; ++i,++hsp) {
		SSwapLong(&hsp->scriptEventMask);
		SSwapLong(&hsp->flags);
		SSwapLong(&hsp->secureInfo);
		SSwapLong(&hsp->refCon);
		SSwapShort(&hsp->loc.v);
		SSwapShort(&hsp->loc.h);
		SSwapShort(&hsp->id);
		SSwapShort(&hsp->dest);
		SSwapShort(&hsp->type);
		SSwapShort(&hsp->groupID);
		SSwapShort(&hsp->state);
		SSwapShort(&hsp->nameOfst);		// 8/23/95
		SSwapShort(&hsp->scriptTextOfst);
		// Swap Point List
		//
		SSwapShort(&hsp->ptsOfst);
		SSwapShort(&hsp->nbrPts);
		pp = (Point *) &rp->varBuf[hsp->ptsOfst];
		for (n = 0; n < hsp->nbrPts; ++n,++pp) {
			SSwapShort(&pp->v);
			SSwapShort(&pp->h);
		}

		// Swap Event Handlers
		//
		SSwapShort(&hsp->scriptRecOfst);
		SSwapShort(&hsp->nbrScripts);
		ehp = (EventHandlerPtr) &rp->varBuf[hsp->scriptRecOfst];
		for (n = 0; n < hsp->nbrScripts; ++n,++ehp) {
			SSwapLong(&ehp->refCon);
			SSwapShort(&ehp->eventType);
			SSwapShort(&ehp->scriptTextOfst);
		}

		// Swap State Records
		SSwapShort(&hsp->stateRecOfst);
		SSwapShort(&hsp->nbrStates);
		srp = (StateRecPtr) &rp->varBuf[hsp->stateRecOfst];
		for (n = 0; n < hsp->nbrStates; ++n,++srp) {
			SSwapShort(&srp->pictID);
			SSwapShort(&srp->picLoc.v);
			SSwapShort(&srp->picLoc.h);
		}
	}

	// Swap Pictures
	//
	SSwapShort(&rp->nbrPictures);
	SSwapShort(&rp->pictureOfst);
	prp = (PictureRecPtr) &rp->varBuf[rp->pictureOfst];
	for (i = 0; i < rp->nbrPictures; ++i,++prp) {
		SSwapLong(&prp->refCon);
		SSwapShort(&prp->picID);
		SSwapShort(&prp->picNameOfst);
		SSwapShort(&prp->transColor);
	}

	// Swap DrawCmds  modified 7/14/95
	//
	SSwapShort(&rp->nbrDrawCmds);
	SSwapShort(&rp->firstDrawCmd);						// 7/14/95
	drp = (DrawRecPtr) &rp->varBuf[rp->firstDrawCmd];	// 7/14/95
	for (i = 0; i < rp->nbrDrawCmds; ++i) {
		SSwapShort(&drp->link.nextOfst);				// 7/14/95
		SSwapShort(&drp->drawCmd);
		SSwapShort(&drp->cmdLength);
		SSwapShort(&drp->dataOfst);
		SwapDrawCmdInBuffer(drp->drawCmd,(Ptr) &rp->varBuf[drp->dataOfst]);
		if (drp->link.nextOfst)							// 7/14/95
			drp = (DrawRecPtr) &rp->varBuf[drp->link.nextOfst];
		else
			break;
	}

	// Swap LProps
	//
	SSwapShort(&rp->nbrLProps);
	SSwapShort(&rp->firstLProp);
	lpp = (LPropPtr) &rp->varBuf[rp->firstLProp];
	for (i = 0; i < rp->nbrLProps; ++i) {
		SSwapShort(&lpp->link.nextOfst);
		SSwapLong(&lpp->propSpec.id);
		SSwapLong(&lpp->propSpec.crc);
		SSwapLong(&lpp->flags);
		SSwapLong(&lpp->refCon);
		SSwapShort(&lpp->loc.v);
		SSwapShort(&lpp->loc.h);
		if (lpp->link.nextOfst)
			lpp = (LPropPtr) &rp->varBuf[lpp->link.nextOfst];
		else
			break;
	}

	// Swap FProps
	//
	// SSwapShort(&rp->nbrFProps);
	// SSwapShort(&rp->fPropOfst);
	// fpp = (long *) &rp->varBuf[rp->fPropOfst];
	// for (i = 0; i < rp->nbrFProps; ++i,++fpp) {
	// 	SSwapLong(fpp);
	// }
}

void SwapOutgoingRoom(RoomRecPtr rp)
{
	PictureRecPtr	prp;	// Picture Rec Pointer
	LPropPtr		lpp;	// Loose Prop Pointer
	HotspotPtr		hsp;	// Hot Spot Pointer
	DrawRecPtr		drp;	// Draw Rec Pointer
	short			i;
	short			n;
	Point			*pp;	// Point Pointer
	EventHandlerPtr	ehp;	// Event Handler Ptr
	StateRecPtr		srp;	// State Rec Ptr
	short			tempOfst;

	SSwapShort(&rp->roomID);
	SSwapLong(&rp->roomFlags);
	SSwapShort(&rp->roomNameOfst);
	SSwapShort(&rp->pictNameOfst);
	SSwapShort(&rp->artistNameOfst);
	SSwapShort(&rp->passwordOfst);
	SSwapShort(&rp->nbrPeople);
	SSwapLong(&rp->facesID);

	// Swap Hotspots
	//
	hsp = (HotspotPtr) &rp->varBuf[rp->hotspotOfst];
	for (i = 0; i < rp->nbrHotspots; ++i,++hsp) {
		SSwapLong(&hsp->scriptEventMask);
		SSwapLong(&hsp->flags);
		SSwapLong(&hsp->secureInfo);
		SSwapLong(&hsp->refCon);
		SSwapShort(&hsp->loc.v);
		SSwapShort(&hsp->loc.h);
		SSwapShort(&hsp->id);
		SSwapShort(&hsp->dest);
		SSwapShort(&hsp->type);
		SSwapShort(&hsp->groupID);
		SSwapShort(&hsp->state);
		SSwapShort(&hsp->nameOfst);		// 8/23/95
		SSwapShort(&hsp->scriptTextOfst);

		// Swap Point List
		//0
		pp = (Point *) &rp->varBuf[hsp->ptsOfst];
		for (n = 0; n < hsp->nbrPts; ++n,++pp) {
			SSwapShort(&pp->v);
			SSwapShort(&pp->h);
		}
		SSwapShort(&hsp->ptsOfst);
		SSwapShort(&hsp->nbrPts);

		// Swap Event Handlers
		//
		ehp = (EventHandlerPtr) &rp->varBuf[hsp->scriptRecOfst];
		for (n = 0; n < hsp->nbrScripts; ++n,++ehp) {
			SSwapLong(&ehp->refCon);
			SSwapShort(&ehp->eventType);
			SSwapShort(&ehp->scriptTextOfst);
		}
		SSwapShort(&hsp->scriptRecOfst);
		SSwapShort(&hsp->nbrScripts);


		// Swap State Records
		srp = (StateRecPtr) &rp->varBuf[hsp->stateRecOfst];
		for (n = 0; n < hsp->nbrStates; ++n,++srp) {
			SSwapShort(&srp->pictID);
			SSwapShort(&srp->picLoc.v);
			SSwapShort(&srp->picLoc.h);
		}
		SSwapShort(&hsp->stateRecOfst);
		SSwapShort(&hsp->nbrStates);
	}
	SSwapShort(&rp->nbrHotspots);
	SSwapShort(&rp->hotspotOfst);

	// Swap Pictures
	//
	prp = (PictureRecPtr) &rp->varBuf[rp->pictureOfst];
	for (i = 0; i < rp->nbrPictures; ++i,++prp) {
		SSwapLong(&prp->refCon);
		SSwapShort(&prp->picID);
		SSwapShort(&prp->picNameOfst);
		SSwapShort(&prp->transColor);
	}
	SSwapShort(&rp->nbrPictures);
	SSwapShort(&rp->pictureOfst);

	// Swap DrawCmds
	//
	drp = (DrawRecPtr) &rp->varBuf[rp->firstDrawCmd];
	for (i = 0; i < rp->nbrDrawCmds; ++i) {
		tempOfst = drp->link.nextOfst;
		SwapDrawCmdOutBuffer(drp->drawCmd,(Ptr) &rp->varBuf[drp->dataOfst]);
		SSwapShort(&drp->drawCmd);
		SSwapShort(&drp->cmdLength);
		SSwapShort(&drp->dataOfst);
		SSwapShort(&drp->link.nextOfst);
		if (tempOfst)
			drp = (DrawRecPtr) &rp->varBuf[tempOfst];
		else
			break;
	}
	SSwapShort(&rp->nbrDrawCmds);
	SSwapShort(&rp->firstDrawCmd);

	// Swap LProps
	//
	lpp = (LPropPtr) &rp->varBuf[rp->firstLProp];
	for (i = 0; i < rp->nbrLProps; ++i) {
		tempOfst = lpp->link.nextOfst;
		SSwapLong(&lpp->propSpec.id);
		SSwapLong(&lpp->propSpec.crc);
		SSwapLong(&lpp->flags);
		SSwapLong(&lpp->refCon);
		SSwapShort(&lpp->loc.v);
		SSwapShort(&lpp->loc.h);
		SSwapShort(&lpp->link.nextOfst);
		if (tempOfst)
			lpp = (LPropPtr) &rp->varBuf[tempOfst];
		else
			break;
	}
	SSwapShort(&rp->nbrLProps);
	SSwapShort(&rp->firstLProp);

	// Swap FProps
	//
	// fpp = (long *) &rp->varBuf[rp->fPropOfst];
	// for (i = 0; i < rp->nbrFProps; ++i,++fpp) {
	// 	SSwapLong(fpp);
	// }
	// SSwapShort(&rp->nbrFProps);
	// SSwapShort(&rp->fPropOfst);


	SSwapShort(&rp->lenVars);
}

void SwapBlowThru(char *p, Boolean incomingFlag);

/*------------------------------------------------------------------------
 * Function: SwapBlowThru
 *
 * Swaps the header fields of BlowThru events
 *
 * 2/12/97	cdm		Added this header, and corrected to swap correctly
 *
 *	InComing blowthru:
 *		long	signature
 *		char	userData[] -- any length of data
 *
 *	Outgoing blowthru:
 *		long	flag
 *		long	numUsers	if > 0 then userlist follows
 *		long	userlist[]	{optional}
 *		long	signature
 *		char	userData[] -- any length of data
 */

void SwapBlowThru(char *p, Boolean incomingFlag)

{
	if (!incomingFlag)
	{
		unsigned long	*flag = (unsigned long*)p;
		unsigned long	*user = (unsigned long*)((long)flag + sizeof(long));
		unsigned long	*sign = (unsigned long*)((long)user + sizeof(long));
		unsigned long	*userids = 0;
		short			i = 0;
		
		if (*user > 0)
		{
			// only when we have a list of users.
			userids = sign;
			sign = (unsigned long*)((long)sign + (sizeof(long) * (*user)));
		}
		
		/*
			In the packet swap the contents from above the signature. (i.e. all
			those that would be interpreted by the server.
		*/
		if (*user > 0)
		{
			for (i = 0; i < (short)*user; i++)
				SwapLong(&userids[i]);
		}
		SwapLong(user);
		SwapLong(flag);
		/*
			We don't swap the signature as it is not being interpreted by the server.
			Anyway, it has to be in LittleEndian format.
		*/
	}
	else
	{
		/*
			On an incoming packet the signature is the only known field, and it's
			littleEndian anyway
		*/
	}
	
#ifdef OLDCODE
	long cnt,i;



	SSwapLong((long *)p);

	p += sizeof(LONG);

	if (!incomingFlag)

		cnt = *((long *)p);

	SSwapLong((long *)p);

	if (incomingFlag)

		cnt = *((long *)p);

	p += sizeof(LONG);

	for (i=0;i<cnt;i++)

	{

		SSwapLong((long *)p);

		p += sizeof(LONG);

	}
#endif
}


// Should only be performed by MSB machines
//
// Client -> Server (convert from MSB to LSB)
//
void SwapOutgoingClientTCPPacket(Ptr	buf)
{
	Ptr	p;
	long	eventType; // not used ,refNum;
	p = buf;
	eventType = *((long *) p);	SSwapLong((long *) p);		p += 4;
								SSwapLong((long *) p);		p += 4;
	// not used refNum = *((long *) p);
	SSwapLong((long *) p);		p += 4;

	switch (eventType) {
	case MSG_GMSG:	// Nothing
	case MSG_SMSG:
	case MSG_RMSG:
	case MSG_LOGOFF:
	case MSG_USERNAME:
	case MSG_TALK:
	case MSG_PING:
	case MSG_PONG:
	case MSG_ROOMNEW:
	case MSG_SPOTNEW:
	case MSG_SUPERUSER:
		// nada
		break;
	// case MSG_USERINFO:	// 6/9/95 what was this? color?
	case MSG_USERCOLOR:	// yup...
	case MSG_XTALK:
	case MSG_USERFACE:	// 1 short
	case MSG_SPOTDEL:	// 1 short
	case MSG_ROOMGOTO:	// 1 short
		SSwapShort((short *) p);
		break;
	case MSG_USERMOVE:	// 2 shorts
	case MSG_DOORLOCK:	// 2 shorts
	case MSG_DOORUNLOCK:// shorts
		SSwapShort((short *) p);
		SSwapShort((short *) (p+2));
		break;
	case MSG_SPOTSTATE:		// 3 shorts
		SSwapShort((short *) p);
		SSwapShort((short *) (p+2));
		SSwapShort((short *) (p+4));
		break;
	case MSG_PICTMOVE:	// 4 shorts
	case MSG_SPOTMOVE:	// 4 shorts
		SSwapShort((short *) p);
		SSwapShort((short *) (p+2));
		SSwapShort((short *) (p+4));
		SSwapShort((short *) (p+6));
		break;
	case MSG_KILLUSER:	// 1 long
	case MSG_WHISPER:	// 1 long
	case MSG_PROPDEL:	// 1 long
		SSwapLong((long *) p);
		break;
	/***  1/23/97 JAB see below...
	case MSG_LOGON:		// 2 longs
		SSwapLong((long *) p);
		SSwapLong((long *) (p+4));
		break;
	 ***/
	case MSG_XWHISPER:	// 1 long + 1 short
		SSwapLong((long *) p);
		SSwapShort((short *) (p+4));
		break;
	case MSG_USERDESC:	// short + short + long + N AssetRecs	 6/7/95
		{
			long	n;
			SSwapShort((short *) p); p += 2;
			SSwapShort((short *) p); p += 2;
			n = *((long *) p);
			SSwapLong((long *) p); p += 4;
			while (n--) {
				SSwapLong((long *) p); p += 4;
				SSwapLong((long *) p); p += 4;
			}
		}
		break;
	case MSG_USERPROP:	// 1 long + N AssetRecs					6/7/95
		{
			long	n;
			n = *((long *) p);
			SSwapLong((long *) p); p += 4;
			while (n--) {
				SSwapLong((long *) p); p += 4;
				SSwapLong((long *) p); p += 4;
			}
		}
		break;
	case MSG_ASSETQUERY:	// 3 longs  6/7/95
		SSwapLong((long *) p);
		SSwapLong((long *) (p+4));
		SSwapLong((long *) (p+8));
		break;
	case MSG_PROPNEW:	// long long short short		6/7/95
		SSwapLong((long *) p);
		SSwapLong((long *) (p+4));
		SSwapShort((short *) (p+8));
		SSwapShort((short *) (p+10));
		break;
	case MSG_PROPMOVE:	// long short short
		SSwapLong((long *) p);
		SSwapShort((short *) (p+4));
		SSwapShort((short *) (p+6));
		break;
	// 1/14/97 JAB Modified logon event
	case MSG_LOGON:
		SSwapLong((long *) p);			// crc
		SSwapLong((long *) (p+4));		// counter
		SSwapLong((long *) (p+72));		// auxFlags
		SSwapLong((long *) (p+76));		// puidCtr
		SSwapLong((long *) (p+80));		// puidCRC
		SSwapLong((long *) (p+84));		// demoElapsed
		SSwapLong((long *) (p+88));		// totalelapsed
		SSwapLong((long *) (p+92));		// demoLimit
		SSwapShort((short *) (p+96));	// desiredRoom		
		/* 1/27/97 JAB */
		SSwapLong((long *) (p+104));	// ulRequestedProtocolVersion
		SSwapLong((long *) (p+108));	// ulUploadCaps
		SSwapLong((long *) (p+112));	// ulDownloadCaps
		SSwapLong((long *) (p+116));	// ul2DEngineCaps
		SSwapLong((long *) (p+120));	// ul2DGraphicsCaps
		SSwapLong((long *) (p+124));	// ul3DEngineCaps
		break;
	case MSG_ASSETREGI:		// AssetBlockPtr
		SwapAsset((AssetBlockPtr) p);
		break;
	case MSG_ROOMSETDESC:	// RoomRecord
		SwapOutgoingRoom((RoomRecPtr) p);
		break;
	case MSG_DRAW:			// Draw Command
		SwapDrawCmdOut((DrawRecPtr) p,p + sizeof(DrawRecord));
		break;
	case MSG_BLOWTHRU:
    	SwapBlowThru(p, false);
    	break;
	}
}


// Client swaps packets coming from server over TCP (LSB->MSB)
//
void SwapIncomingClientTCPPacket(Ptr	buf)
{
	Ptr	p;
	long	eventType,refNum,length;
	p = buf;
	SSwapLong((long *) p); eventType = *((long *) p); p += 4;

	// 1/27/97 JAB - need Length to properly Kevin's modified packets...

	SSwapLong((long *) p); length = *((long *) p);	 p += 4;
	SSwapLong((long *) p); refNum = *((long *) p);	 p += 4;

	switch (eventType) {
	// 1/27/97 JAB added optional fields... case MSG_VERSION:		// nada
	case MSG_SERVERDOWN:	// nada
	case MSG_USEREXIT:		// nada
	case MSG_ROOMDESCEND:
	case MSG_USERNAME:
	case MSG_TALK:
	case MSG_WHISPER:
	case MSG_PING:
	case MSG_TIYID:		// 1 short
		break;

	case MSG_USERSTATUS:
	case MSG_USERCOLOR:	// 1 short
	case MSG_USERFACE:	// 1 short
	case MSG_XTALK:		// 1 short
	case MSG_XWHISPER:	// 1 short
		SSwapShort((short *) p);
		break;
	case MSG_USERMOVE:	// 2 shorts (v,h))
	case MSG_DOORLOCK:	// 2 shorts (room,door)
	case MSG_DOORUNLOCK: // 2 shorts (room,door)
		SSwapShort((short *) p);
		SSwapShort((short *) (p+2));
		break;
	case MSG_SPOTSTATE:	// 3 shorts (room, spot, state)
		SSwapShort((short *) p);
		SSwapShort((short *) (p+2));
		SSwapShort((short *) (p+4));
		break;
	case MSG_PICTMOVE:	// 4 shorts(room, spot, v, h)
	case MSG_SPOTMOVE:	// 4 shorts(room, spot, v, h)
		SSwapShort((short *) p);
		SSwapShort((short *) (p+2));
		SSwapShort((short *) (p+4));
		SSwapShort((short *) (p+6));
		break;
	case MSG_USERLOG:	// 1 long
	case MSG_LOGOFF:	// 1 long
	case MSG_PROPDEL:	// 1 long		// 6/28
	case MSG_SERVERINFO:	// 1 long	// 7/25/95
		SSwapLong((long *) p);
		// 1/27/97 JAB Added Kevin's fields...
		if (length >= 80) {
			SSwapLong((long *) (p+68));	// serverOptions
			SSwapLong((long *) (p+72)); // ulUploadCaps
			SSwapLong((long *) (p+76)); // ulDownloadCaps
		}
		break;
	// 1/27/97 JAB Added Kevin's fields...
	case MSG_VERSION:
		if (length >= 8) {
			SSwapLong((long *) p);	// ulMaxProtocolVersion
			SSwapLong((long *) (p+4)); // ulNegotiatedProtocolVersion
		}
		break;
	case MSG_ASSETQUERY: // 3 longs 6/7/95
	case MSG_ASSETNEW:	// 3 longs (type,old,new)
		SSwapLong((long *) p);
		SSwapLong((long *) (p+4));
		SSwapLong((long *) (p+8));
		break;
	case MSG_USERDESC:	// short + short + long + N assetspecs
		{
			long	n;
			SSwapShort((short *) p);	p += 2;
			SSwapShort((short *) p);	p += 2;
			SSwapLong((long *) p);
			n = *((long *) p);			p += 4;
			while (n--) {
				SSwapLong((long *) p);	p += 4;
				SSwapLong((long *) p);	p += 4;
			}
		}
		// SysBeep(1);
		break;
	case MSG_USERPROP:	// 1 long + N assetspecs
		{
			long	n;
			SSwapLong((long *) p);
			n = *((long *) p);	p += 4;
			while (n--) {
				SSwapLong((long *) p);	p += 4;
				SSwapLong((long *) p);	p += 4;
			}
		}
		break;
	case MSG_PROPNEW:	// long long short short		// 6/28
		SSwapLong((long *) p);
		SSwapLong((long *) (p+4));
		SSwapShort((short *) (p+8));
		SSwapShort((short *) (p+10));
		break;
	case MSG_PROPMOVE:	// long, short, short (propID, v, h)
		SSwapLong((long *) p);
		SSwapShort((short *) (p+4));
		SSwapShort((short *) (p+6));
		break;
	case MSG_USERLIST:	// Array of UserRecs
		while (refNum--) {
			SwapIncomingUser((UserRecPtr) p);
			p += sizeof(UserRec);
		}
		break;
	case MSG_ASSETSEND:	// AssetBlock
		SwapAsset((AssetBlockPtr) p);
		break;
	case MSG_FILESEND:
		SwapFile((FileBlockPtr) p);
		break;
	case MSG_USERNEW:	// UserRecPtr
		SwapIncomingUser((UserRecPtr) p);
		break;
	case MSG_ROOMSETDESC:	// RoomRecPtr
		SwapIncomingRoom((RoomRecPtr) p);
		break;
	case MSG_ROOMDESC:	// RoomRecPtr
		SwapIncomingRoom((RoomRecPtr) p);
		break;
	case MSG_LISTOFALLUSERS:
		SwapUserList(refNum, p);
		break;
	case MSG_LISTOFALLROOMS:
		SwapRoomList(refNum, p);
		break;
	case MSG_DRAW:			// Draw Command
		SwapDrawCmdIn((DrawRecPtr) p,p + sizeof(DrawRecord));
		break;
	case MSG_BLOWTHRU:
    	SwapBlowThru(p, true);
    	break;
	// 1/16/97 JAB
	case MSG_ALTLOGONREPLY:
		SSwapLong((long *) p);			// crc
		SSwapLong((long *) (p+4));		// counter
		SSwapLong((long *) (p+72));		// auxFlags
		SSwapLong((long *) (p+76));		// puidCtr
		SSwapLong((long *) (p+80));		// puidCRC
		SSwapLong((long *) (p+84));		// demoElapsed
		SSwapLong((long *) (p+88));		// totalelapsed
		SSwapLong((long *) (p+92));		// demoLimit
		SSwapShort((short *) (p+96));	// desiredRoom
		/* 1/27/97 JAB Kevin's Additions... */
		if (length >= 128) {
			SSwapLong((long *) (p+104));	// ulRequestedProtocolVersion
			SSwapLong((long *) (p+108));	// ulUploadCaps
			SSwapLong((long *) (p+112));	// ulDownloadCaps
			SSwapLong((long *) (p+116));	// ul2DEngineCaps
			SSwapLong((long *) (p+120));	// ul2DGraphicsCaps
			SSwapLong((long *) (p+124));	// ul3DEngineCaps
		}

		break;
	}
}
