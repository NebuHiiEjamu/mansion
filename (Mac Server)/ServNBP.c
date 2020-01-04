/* This routine finds the socket used by the PPC Toolbox (it's the one with
** type 'PPCToolBox') and gives it a NBP alias (it registers a new NBP name
** on that socket) with the type passed in newNBPType.  The NameTableEntry
** record passed to this routine must be allocated globally (or must be a
** locked block on the heap until RemoveNBPAlias is called).  The variable
** newEntity is filled in with the new entity name and is returned to the
** function's caller so it can be passed to the RemoveNBPAlias function
** (below).  You'll get an error if any call fails, if an entity of type
** 'PPCToolBox' is not found (usually because either Program Linking isn't
** enabled or AppleTalk is disabled), or if RegisterName fails because >100
** copies of your application are running on the one machine. */
#include "S-SERVER.H"

#define		kTupleSize	104
#define		kMaxTuples	1
#define		pcpy(dst,src)	BlockMove(src,dst,(src)[0]+1)
#include <AppleTalk.h>

#if THINK_C
#define	NBPPtr	Ptr
#else
#define NBPPtr	StringPtr
#endif

void pcatdec(StringPtr str, short num)
{
	sprintf((char *) &str[str[0]+1],"%d",num);
	str[0] = strlen((char *) &str[1]);
}

OSErr	AddPPCNBPAlias(NamesTableEntry *theNTE, Str32 newNBPType, EntityName *newEntity)
{
	OSErr			err;
	MPPParamBlock	pb;
	char			keepSelfFlag;
	short			keepResFile, origLen, num, len;

	EntityName		myEntityName;

	AddrBlock		myAddrBlock;
	char			myRetBuff[kTupleSize];
	Handle			machineNameHndl;
	Str32			machineName;

	pb.SETSELF.newSelfFlag = 1;
	err = PSetSelfSend(&pb, false);						/* Turn on self-send. */
	if (err) return(err);

	keepSelfFlag = pb.SETSELF.oldSelfFlag;				/* Keep old self-send flag. */

	keepResFile = CurResFile();
	UseResFile(0);
	machineNameHndl = GetResource('STR ', -16413);		/* Get machine name. */
	UseResFile(keepResFile);

	if (!machineNameHndl) {
		pb.SETSELF.newSelfFlag = keepSelfFlag;
		PSetSelfSend(&pb, false);
		return(ResError());
	}

	pcpy(machineName, (StringPtr)*machineNameHndl);		/* Keep a copy of the machine name.   */
	ReleaseResource(machineNameHndl);					/* Release the machine name resource. */

	NBPSetEntity((Ptr)&myEntityName, (NBPPtr)machineName, (NBPPtr)"\pPPCToolBox", (NBPPtr)"\p*");

	pb.NBPinterval    = 1;					/* We want to build the entity name using */
	pb.NBPcount       = 1;					/* the machine name and 'PPCToolBox'.     */
	pb.NBPentityPtr   = (Ptr)&myEntityName;
	pb.NBPretBuffPtr  = myRetBuff;
	pb.NBPretBuffSize = (kTupleSize * kMaxTuples);
	pb.NBPmaxToGet    = kMaxTuples;

	if (!(err = PLookupName(&pb, false))) {		/* If lookup was okay... */
		if (pb.NBPnumGotten) {					/* If entity was found... */
									/* ...we found the socket used by the PPC Toolbox. */
									/* This means that there is a socket, due to program */
									/* linking being turned on. */

			NBPExtract(myRetBuff, pb.NBPnumGotten, 1, &myEntityName, &myAddrBlock);
				/* Break the tuple into component parts. */

			for (origLen = machineName[num = 0]; num < 100;) {

				pb.NBPinterval   = 7;
				pb.NBPcount      = 5;
				pb.NBPentityPtr  = (Ptr)theNTE;
				pb.NBPverifyFlag = 1;

				NBPSetNTE((Ptr)theNTE, (NBPPtr)machineName, (NBPPtr)newNBPType, (NBPPtr)"\p*", myAddrBlock.aSocket);

				NBPSetEntity((Ptr)newEntity, (NBPPtr)machineName,(NBPPtr) newNBPType, (NBPPtr)"\p*");
				err = PRegisterName(&pb, false);

				if (err != nbpDuplicate) break;
					/* We registered the name, (or badness happened), so we are done. */

				len = 31;		/* The name we tried already exists, so make up an alternate. */
				if (++num > 9)
					--len;
				if (origLen > len)
					origLen = len;
				machineName[0] = origLen;
				pcatdec(machineName, num);
			}
		}
		else
			LogMessage("Turn on program linking (via the Sharing Setup Control Panel) to permit local connections\r");
	}

	pb.SETSELF.newSelfFlag = keepSelfFlag;
	PSetSelfSend(&pb, false);

	return(err);
}


/*****************************************************************************/



/* This function removes the entity specified by theEntity from the registered
** names queue.  You'll get an error if theEntity hasn't been registered on
** this Macintosh with RegisterName. */

OSErr	RemoveNBPAlias(EntityPtr theEntity)
{
	MPPParamBlock	pb;

	pb.MPPioCompletion = nil;
	pb.NBPentityPtr    = (Ptr)theEntity;
	return(PRemoveName(&pb, false));
}


