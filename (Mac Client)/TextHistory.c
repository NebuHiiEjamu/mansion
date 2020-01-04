// Text History Routines
#include "U-USER.H"

#define MaxTextLimit	32000L

typedef struct TextHistory {
	struct TextHistory	**next,**last;
	unsigned char text[1];		// pascal string
} TextHistory,*TextHistoryPtr,**TextHistoryHandle;

static TextHistoryHandle	gLastHist,gFirstHist,gCurHist;
Boolean						inRecall;
static long					gTotalTHLength;

void RetrieveHistoryText(TextHistoryHandle th, StringPtr buf);
void KillFirstText();
void PushHistoryText(TEHandle	teH);
void HistoryRecall();
void HistoryBackward();
void HistoryForward();

void RetrieveHistoryText(TextHistoryHandle	th, StringPtr buf)
{
	if (th == NULL)
		buf[0] = 0;
	else {
		BlockMove((*th)->text,buf,(*th)->text[0]+1);
	}
}

void KillFirstText()
{
	if (gFirstHist) {
		(*gLastHist)->next = (*gFirstHist)->next;
		(*(*gFirstHist)->next)->last = gLastHist;
		if (gCurHist == gFirstHist)
			gCurHist = (*gFirstHist)->next;
		gTotalTHLength -= (*gFirstHist)->text[0];
		DisposeHandle((Handle) gFirstHist);
	}
}

void PushHistoryText(TEHandle	teH)
{
	long				len;
	Str255				text,lastText;
	TextHistoryHandle	th;

	len = (*teH)->teLength;
	if (len > 250)
		len = 250;
	BlockMove(*(*teH)->hText,&text[1],len);
	text[0] = len;
	RetrieveHistoryText(gLastHist, lastText);
	if (EqualString(text,lastText,true,true)) {
		inRecall = false;
		return;
	}
	while (gTotalTHLength + len > MaxTextLimit && gFirstHist != gLastHist)
		KillFirstText();
	th = (TextHistoryHandle) NewHandleClear(8 + len + 1);
	gTotalTHLength += len;
	BlockMove(text, (*th)->text, text[0]+1);

	if (th == NULL)
		return;
	(*th)->last = gLastHist;
	(*th)->next = gFirstHist;
	if (gLastHist)
		(*gLastHist)->next = th;
	if (gFirstHist)
		(*gFirstHist)->last = th;
	gLastHist = th;
	gCurHist = th;
	inRecall = false;
	if (gFirstHist == NULL) {
		gFirstHist = th;
		gLastHist = th;
		gCurHist = th;
		(*th)->last = th;
		(*th)->next = th;
	}
}

void HistoryRecall()
{
	Str255	temp;
	TextStyle	ts;
	short		lineHeight,ascent;
	if (gCurHist == NULL)
		return;
	PrepareTextColors();
	RetrieveHistoryText(gCurHist, temp);
	TESetText(&temp[1],temp[0],gRoomWin->msgTEH);
	TESetSelect(0,32767,gRoomWin->msgTEH);
	TEGetStyle((*gRoomWin->msgTEH)->selStart,&ts,&lineHeight,&ascent,gRoomWin->msgTEH);
	ts.tsColor = gWhiteColor;
	TESetStyle(doColor,&ts,false,gRoomWin->msgTEH);
	InvalRect(&gRoomWin->msgRect);
	RestoreWindowColors();
}

void HistoryBackward()
{
	if (gCurHist) {
		if (inRecall)
			gCurHist = (*gCurHist)->last;
		HistoryRecall();
		inRecall = true;
	}
}

void HistoryForward()
{
	if (gCurHist) {
		gCurHist = (*gCurHist)->next;
		HistoryRecall();
		inRecall = true;
	}
}