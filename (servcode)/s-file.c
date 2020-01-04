/* S-FILE.C */

#include "s-server.h"

#define MaxFileSends	64L
#define MaxBlockSize	2048L

typedef struct {
	LONG			userID;
	Str63			name;
	FileHandle		iFile;		/* JAB 6/14/96 Changed to FileHandle datatype */
	LONG			fileSize;
	FileBlockHeader	fBlock;
} FSendRec, huge *FSendPtr;

typedef struct {
	FileBlockHeader	fBlock;
	char			data[MaxBlockSize];
} FBlock;

LONG		gTransID=1L;
short		gNbrFileSends = 0;
FSendPtr	gFileSendPtr = NULL;
FBlock		gSendBlock;

/* Call this in response to MSG_FILEQUERY
 * This is called from S-EVENTS to initiate a file transfer
 */
void ProcessFileSend(ServerUserPtr cUser, StringPtr fName)
{
	FileHandle	iFile;
	Str255		pName;
	StringPtr	sp;
	/* Check if file can be opened... */
	sp = BuildPictureName(fName);
	if (sp == NULL) {
		LogMessage("Can't find file to send: %.*s\r",fName[0],&fName[1]);
		return;
	}
	BlockMove(sp, pName, sp[0]+1);
	if (OpenFileReadOnly(pName, 0, &iFile) != noErr) {
		LogMessage("Failed File Send: %.*s\r",fName[0],&fName[1]);
		return;
	}
	NewFileSend(cUser->user.userID,fName, iFile);
}

/* Allocate new file send record, send first block
 */
void NewFileSend(LONG userID, StringPtr fName, FileHandle fRefNum)
{
	FSendPtr		ap;
	LONG			fileSize,blockSize;
	ServerUserPtr	cUser;

	cUser = GetServerUser(userID);
	if (cUser == NULL)
		return;
	if (gFileSendPtr == NULL) {
		gFileSendPtr = (FSendPtr) NewPtrClear(sizeof(FSendRec) * MaxFileSends);
		if (gFileSendPtr == NULL)
			return;
	}
	if (gNbrFileSends >= MaxFileSends) {
		LogMessage("Failed File Send (Exceeded Max): %.*s\r",fName[0],&fName[1]);
		return;
	}
	
	ap = &gFileSendPtr[gNbrFileSends];
	gNbrFileSends++;

	memset(ap, 0, sizeof(FSendRec));
	BlockMove(fName, ap->name, fName[0]+1);
	ap->userID = userID;
	ap->iFile = fRefNum;
	ap->fBlock.transactionID = gTransID++;
	/* Send first block */
	GetEOF(fRefNum, &fileSize);
	ap->fileSize = fileSize;
	ap->fBlock.varBlock.firstBlockRec.size = fileSize;
	BlockMove(fName, ap->fBlock.varBlock.firstBlockRec.name, fName[0]+1);
	ap->fBlock.nbrBlocks = fileSize / MaxBlockSize;
	if (fileSize % MaxBlockSize)
		++ap->fBlock.nbrBlocks;
	ap->fBlock.blockNbr = 0;
	blockSize = (fileSize < MaxBlockSize? fileSize : MaxBlockSize);
	ap->fBlock.blockSize = blockSize;
	ap->fileSize -= blockSize;
	/* !!!! */
	BlockMove(&ap->fBlock,&gSendBlock.fBlock,sizeof(FileBlockHeader));
	FSRead(ap->iFile,&blockSize,&gSendBlock.fBlock.varBlock.firstBlockRec.data[0]);
	PostUserEvent(cUser, MSG_FILESEND, 0L, (Ptr) &gSendBlock, sizeof(FileBlockHeader)+blockSize-1);
}

/* Called during idle from S-EVENTS.C  ServerIdle()
 *		Note; ServerIdle() should be called frequently!!
 *
 * !!!! don't bother if user buffer is full
 * For each pending file request, send next block, 
 * if done, close file, delete record
 */
void FileSendIdle()
{
	short			i;
	FSendPtr		ap;
	LONG			blockSize;
	ServerUserPtr	cUser;

	/* if (GetTicks() - gLastFileSend < FILESENDTICKS)
	 *	return;
     */
	for (i = 0; i < gNbrFileSends; ++i) {
		ap = &gFileSendPtr[i];
		cUser = GetServerUser(ap->userID);
		if (cUser == NULL || (cUser->flags & U_Kill) > 0) {
			FSClose(ap->iFile);
			gFileSendPtr[i] = gFileSendPtr[gNbrFileSends-1];
			--i;
			--gNbrFileSends;
			continue;
		}
		/* Don't overload user buffer...
		 * This only allows one send-ahead in the queue
		 * !!!! this prevents the user's send buffer from
		 *		overflowing
		 *    you don't want to have lots of 2K file packets waiting
		 *    to go out because it will prevent the user from doing anything
		 *    else
		 */
		if (PendingSendLength(cUser) > 4000L)
			continue;
		++ap->fBlock.blockNbr;
		blockSize = (ap->fileSize < MaxBlockSize? ap->fileSize : MaxBlockSize);
		ap->fBlock.blockSize = blockSize;
		ap->fileSize -= blockSize;
		/* !!! */
		BlockMove(&ap->fBlock,&gSendBlock.fBlock,sizeof(FileBlockHeader));
		FSRead(ap->iFile,&blockSize,&gSendBlock.fBlock.varBlock.nextBlockRec.data[0]);
		PostUserEvent(cUser, MSG_FILESEND, 0L, (Ptr) &gSendBlock, sizeof(FileBlockHeader)+blockSize-1);
		/* File finished sending? */
		if (ap->fileSize <= 0) {
			FSClose(ap->iFile);		/* Close the file */
			/* Swap in last entry in to this position (delete the entry) */
			gFileSendPtr[i] = gFileSendPtr[gNbrFileSends-1];
			--i;
			--gNbrFileSends;
		}
	}
	/* gLastFileSend = GetTicks(); */
}

