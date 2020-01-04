// MacLocalUtils.c

#include "MANSION.H"

#define ErrorALRT			10000
#define LongErrorALRT		10001
#define VerboseErrorDLOG	702
#define MessageSTR			131

void ErrorMessage(char *str,...);
void ReportError(short code, char *name);
void ErrorExit(char *str,...);
int MyFGets(short refNum, char *buf);
Boolean MembersOnly(Boolean flag);
Boolean CanPaste(long type);
void ReportMessage(short strCode);
void ReportVerboseMessage(short strCode);
OSErr OpenFileReadOnly(StringPtr name, short volNum, FileHandle *refNum);
void CreateTemporaryFileName(StringPtr seedName, StringPtr tempName);
void RenameFile(StringPtr oldName, StringPtr newName);
StringPtr	BuildMediaFolderName(StringPtr pictureName, int catalogNumber, int indexNumber);

static char gTBuf[512];


#define VE_OKItem	1
#define VE_TextItem	2


static TEHandle	gTextH;

pascal void DrawTextItem(DialogPtr dp, short itemNbr);
pascal void DrawTextItem(DialogPtr dp, short itemNbr)
{
	short			t;
	Handle			h;
	Rect			r;

	if (itemNbr == VE_TextItem) {
		GetDItem(dp, VE_TextItem, &t, &h, &r);
		if (gTextH)
			TEUpdate(&r,gTextH);
	}
}

void ReportVerboseMessage(short textIdx)
{
	GrafPtr			savePort;
	DialogPtr		dp;
	UserItemUPP		drawTextProc = NewUserItemProc(DrawTextItem);
	short			t;
	Handle			h;
	Rect			r;
	short			itemHit;
	Handle			styleH,textH;

	GetPort(&savePort);
	styleH = GetResource('styl',128+textIdx);
	if (styleH == NULL)
		return;
	textH = GetResource('TEXT',128+textIdx);
	if (textH == NULL)
		return;
	if ((dp = GetNewDialog (VerboseErrorDLOG, NULL, (WindowPtr) -1)) == NULL)
		return;

	GetDItem(dp, VE_TextItem, &t, &h, &r);
	SetDItem(dp, VE_TextItem, t, (Handle) drawTextProc, &r);
	// Use DLOG 702, 'TEXT/styl' 128+itemNbe

	SetDialogDefaultItem(dp, VE_OKItem);
	ShowWindow(dp);
	SetPort(dp);
	gTextH = TEStylNew(&r,&r);
	if (gTextH == NULL)
		goto Err1;

	HLock(textH);
	TEActivate(gTextH);
	TEStyleInsert(*textH,GetHandleSize(textH),(StScrpRec **) styleH,gTextH);
	TEDeactivate(gTextH);

	do {
		ModalDialog(NULL, &itemHit);
	} while (itemHit != VE_OKItem);
	
	TEDispose(gTextH);
Err1:
	DisposDialog(dp);
	SetPort(savePort);
}

void ErrorMessage(char *str,...)
{
	va_list args;
	va_start(args,str);
	vsprintf(gTBuf,str,args);
	va_end(args);

	// Debugger();	/* JAB test */

	CtoPstr(gTBuf);
	ParamText((StringPtr) gTBuf,"\p","\p","\p");
	if ((unsigned char) gTBuf[0] > 160)
		Alert(LongErrorALRT,NULL);
	else
		Alert(ErrorALRT,NULL);
}

void ReportMessage(short strCode)
{
	Str255	theString;
	GetIndString(theString,MessageSTR,strCode);
	if (theString[0])
		ErrorMessage("%.*s",theString[0],&theString[1]);
}

void ReportError(short code, char *name)
{
	Handle		h;
	StringPtr	hs;
	if ((h = GetResource('STR ',code)) != NULL) {
		HLock(h);
		hs = (StringPtr) *h;
		if (code > 0 || name == NULL || *name == 0)
			ErrorMessage("%.*s",hs[0],&hs[1]);
		else
			ErrorMessage("%.*s in routine %s",hs[0],&hs[1],name);
		HUnlock(h);
		ReleaseResource(h);
	}
	else {
		if (name == NULL || *name == 0)
			ErrorMessage("Error %d occurred",code);
		else
			ErrorMessage("Error %d occurred in routine %s",code,name);
	}
}

void ErrorExit(char *str,...)
{
	va_list args;
	va_start(args,str);
	vsprintf(gTBuf,str,args);
	va_end(args);

	CtoPstr(gTBuf);
	ParamText((StringPtr) gTBuf,"\p","\p","\p");
	Alert(ErrorALRT,NULL);
	exit(1);
}

int MyFGets(short refNum, char *buf)
{

#define EOL			13
#define MaxBufSize	256

	IOParam	pb;
	OSErr	oe;

	pb.ioCompletion = NULL;
	pb.ioRefNum = refNum;
	pb.ioBuffer = (Ptr) buf;
	pb.ioReqCount = MaxBufSize;
	pb.ioPosMode = fsAtMark | 0x80 | ('\r' << 8);
	pb.ioPosOffset = 0;

	oe = PBRead((ParmBlkPtr) &pb, false);
	if (oe == noErr || oe == eofErr && pb.ioActCount > 0) {
		buf[pb.ioActCount] = 0;	// Add a nul to the end
		return 1;
	}
	else
		return 0;
}

Boolean CanPaste(long type)
{
	long	scrapOffset;
	return GetScrap(NULL,type,&scrapOffset) > 0;
}

OSErr OpenFileReadOnly(StringPtr name, short volNum, FileHandle *refNum)
{
	short	sRefNum;
	OSErr	oe;
	oe = HOpen(volNum, 0L, name, fsRdPerm, &sRefNum);
	*refNum = sRefNum;
	return oe;
}

void CreateTemporaryFileName(StringPtr seedName, StringPtr tempName)
{
	BlockMove(seedName,tempName,seedName[0]+1);
	BlockMove(".tmp",&tempName[tempName[0]+1],4);
	tempName[0] += 4;
}


void RenameFile(StringPtr oldName, StringPtr newName)
{
	FSSpec	fSpec;
	char	cName[127] /* ,*p */;
	OSErr	oe;
	fSpec.vRefNum = 0;
	fSpec.parID = 0L;
	BlockMove(oldName, fSpec.name, oldName[0]+1);
	BlockMove(newName,cName,newName[0]+1);
	oe = FSpRename(&fSpec, (StringPtr) cName);
}

void MidiPlay(char *fileName);				// note filename is a C-String!!
void MidiLoop(char *fileName, short iters);	// -1 means infinite
void MidiStop(void);

void MidiPlay(char *fileName)
{
}

void MidiLoop(char *fileName, short iters)
{
}

void MidiStop(void)
{
}

#undef DisposeHandle
#undef DisposePtr
#undef FSClose
#undef ReleaseResource
#undef NewHandleClear
#undef NewPtrClear

void MyDisposeHandle(Handle *h)
{
	if (*h == NULL) {
		ErrorMessage("%s: Dispose NULL Handle",__FILE__);
		return;
	}
	DisposeHandle(*h);
	*h = NULL;
}

void MyDisposePtr(Ptr *p)
{
	if (*p == NULL) {
		ErrorMessage("%s: Dispose NULL Ptr",__FILE__);
		return;
	}
	DisposePtr(*p);
	*p = NULL;
}

void MyReleaseResource(Handle *h)
{
	if (*h == NULL) {
		ErrorMessage("%s: Release NULL Resource",__FILE__);
		return;
	}
	ReleaseResource(*h);
	*h = NULL;
}

OSErr MyFSClose(FileHandle *f)
{
	OSErr	oe;
	if (*f == 0) {
		ErrorMessage("%s: Close NULL File",__FILE__);
		return 0;
	}
	oe = FSClose(*f);
	*f = 0;
	return oe;
}

Handle MyNewHandleClear(Size size)
{
	Handle	h;
	h = NewHandleClear(size);
	if (h == NULL)
		ErrorMessage("%s: New Handle Failed [size=%ld]",__FILE__,(long) size);
	return	h;	
}

Ptr MyNewPtrClear(Size size)
{
	Ptr	p;
	p = NewPtrClear(size);
	if (p == NULL)
		ErrorMessage("%s: New Ptr Failed [size=%ld]",__FILE__,(long) size);
	return	p;
}
