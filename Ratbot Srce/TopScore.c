/****************************************************************************
 * TopScores.c
 *
 *		TopScore Stuff
 *
 * JAB 4/28/93 - 5/93
 ****************************************************************************/
#include "WOFBot.h"

#define TopScoreResType	'tScr'
#define MaxScoreRecords	10		// Number of top scores shown

typedef struct {
	long	score;
	long	res1,res2,res3;
	Str63	name;
} ScoreRecord;

ScoreRecord	scoreRecord[MaxScoreRecords+1];
short		gLastHighScore=-1;

void LoadTopScores()
{
	Handle	h;
	Ptr		p;
	long	hSize;
	short	i;
	h = Get1Resource(TopScoreResType,128);
	if (h != NULL) {
		hSize = GetHandleSize(h);
		p = *h;
		for (i = 0; i < MaxScoreRecords; ++i) {
			scoreRecord[i].score = *((long *) p);	p += sizeof(long);
			scoreRecord[i].res1 = *((long *) p);	p += sizeof(long);
			scoreRecord[i].res2 = *((long *) p);	p += sizeof(long);
			scoreRecord[i].res3 = *((long *) p);	p += sizeof(long);
			BlockMove(p,scoreRecord[i].name,p[0]+1);
			p += p[0]+1+((p[0]&1)? 0 : 1);
			if ((long) p - (long) *h >= hSize)
				break;
		}
		ReleaseResource(h);
	}
}

#define AutoOps	200
#define	CuteNix	201
Handle	gAutoOps,gCuteNix;

void LoadAutoOps()
{
	gAutoOps = Get1Resource('STR#',AutoOps);
	gCuteNix = Get1Resource('STR#',CuteNix);
}

void AddCuteNick(char *orig, char *alias)
{
	char	tbuf[128];
	long	len;
	char	*p;
	sprintf(tbuf,"%s>%s",orig,alias);
	len = GetHandleSize(gCuteNix);
	SetHandleSize(gCuteNix,len + strlen(tbuf) + 1);
	p = *gCuteNix;
	*((short *) p) += 1;
	p += len;
	strcpy(p,tbuf);
	CtoPstr(p);
	ChangedResource(gCuteNix);
}

void CheckCuteNick(PlayerRecord *player)
{
	short	nbrCuteNix,i,len;
	char	*p;

	p = *gCuteNix;
	nbrCuteNix = *((short *) p);	p += sizeof(short);
	len = strlen(player->nick);
	for (i = 0; i < nbrCuteNix; ++i) {
		if (strncmp(player->nick,p+1,len) == 0 && p[len+1] == '>') {
			strncpy(player->cutenick,&p[len+2],(p[0]-len)-1);
			player->cutenick[(p[0]-len)-1] = 0;
			break;
		}
		p += p[0]+1;
	}
}


char *RandNick(PlayerRecord *player)
{
	if (player->cutenick[0] == 0)
		return GetPlayerName(player->playerID);
	else if (MyRandom(2))
		return GetPlayerName(player->playerID);
	else
		return player->cutenick;
}

void CheckAutoOps(char *nick)
{
	char	*p;
	short	nbrAutoOps,i;
	Str63	pNick;

	strcpy((char *) pNick,nick);
	CtoPstr((char *) pNick);
	p = *gAutoOps;
	nbrAutoOps = *((short *) p);	p += sizeof(short);
	for (i = 0; i < nbrAutoOps; ++i) {
		if (EqualString((StringPtr) p,pNick,false,false)) {
/*			if (gCarrot)
				CommPrintf("%s /mode #WOF +o %s\r",CarrotPrefix,nick);
			else
				CommPrintf("/mode #WOF +o %s\r",nick);
*/
		}
		p += p[0]+1;
	}
}

void IntegrateScore(long score, char *name)
{
	Handle		h,h2;
	Ptr			p;
	short		i,recno;
	Str63		pName;
	ScoreRecord	temp;

	strcpy((char *) pName,name);
	CtoPstr((char *) pName);


	recno = MaxScoreRecords;
	for (i = 0; i < MaxScoreRecords; ++i) {
		if (EqualString(pName,scoreRecord[i].name,false,false)) {
			recno = i;
			if (scoreRecord[i].score > score)
				return;
			break;
		}
	}
	if (recno < MaxScoreRecords)
		CommPrintf("A new high score for %s!\r",name);
	scoreRecord[recno].score = score;
	BlockMove(pName,scoreRecord[recno].name,pName[0]+1);

	// Sort recno in...
	while (recno > 0 && scoreRecord[recno].score > scoreRecord[recno-1].score) {
		temp = scoreRecord[recno];
		scoreRecord[recno] = scoreRecord[recno-1];
		scoreRecord[recno-1] = temp;
		--recno;
	}

	if (recno < MaxScoreRecords) {
		CommPrintf("%s is #%d in the Top Ten List!\r",name,recno+1);
		gLastHighScore = recno;
		// Save Scores
		h = NewHandle(sizeof(ScoreRecord) * MaxScoreRecords);
		if (h) {
			while ((h2 = GetResource(TopScoreResType, 128)) != NULL) {
				RmveResource(h2);
				DisposHandle(h2);
			}

			p = *h;
			for (i = 0; i < MaxScoreRecords; ++i) {
				*((long *) p) = scoreRecord[i].score;	p += sizeof(long);
				*((long *) p) = scoreRecord[i].res1;	p += sizeof(long);
				*((long *) p) = scoreRecord[i].res2;	p += sizeof(long);
				*((long *) p) = scoreRecord[i].res2;	p += sizeof(long);
				BlockMove(scoreRecord[i].name,p,scoreRecord[i].name[0]+1);
				p += p[0]+1+((p[0]&1)? 0 : 1);
			}
			SetHandleSize(h,(long)p-(long) *h);
			AddResource(h,TopScoreResType,128,"\pTop Scores");
			WriteResource(h);
			ReleaseResource(h);
		}
		DisplayTopScores(0);
	}
}

void DisplayTopScores(PersonID who)
{
	short		i;

	PrivatePrintf(who,"; WOF Top Ten"); 
	for (i = 0; i < MaxScoreRecords; ++i) {
		if (scoreRecord[i].score) {
			PrivatePrintf(who,";%-2d %-20.*s  $%ld",
				i+1,
				scoreRecord[i].name[0],&scoreRecord[i].name[1],
				scoreRecord[i].score);
		}
	}
}

