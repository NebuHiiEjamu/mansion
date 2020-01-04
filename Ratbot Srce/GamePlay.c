// GamePlay.c
//
// Wheel of CHEESE game logic
//

#include "WOFBot.h"

WOFVars gWOF;

void BeginGame()
{
	short	i;
	QueueElement	*qptr;
	Boolean		gotOne;

	if (gWOF.playerList.firstElement != NULL) {
		// Remove inactive players..., reset turnNbr if necessary
		do {
			gotOne = false;
			for (i = 0,qptr = gWOF.playerList.firstElement; i < gWOF.playerList.nbrElements; ++i,qptr = qptr->nextElement) {
				if (!((PlayerRecord *) qptr)->present && TickCount() - ((PlayerRecord *) qptr)->lastActive > 10L*60*60L) {
					RemoveFromQueue(&gWOF.playerList, i);
					gotOne = true;
					break;
				}
				else {
					((PlayerRecord *) qptr)->money = 0;
					((PlayerRecord *) qptr)->gameWinnings = 0;
					((PlayerRecord *) qptr)->freeSpins = 0;
					((PlayerRecord *) qptr)->prizeFlags = 0;
					((PlayerRecord *) qptr)->nbrPrizes = 0;
				}
			}
		} while (gotOne);
	}

	// Count present players, if less than 2, go to idle
	if (gWOF.nbrActivePlayers < 1)
		gWOF.gameState = G_Idle;

	gWOF.roundNbr = 0;
	gWOF.catFlags = 0;
	gWOF.nbrPrizes = 0;

	// Announce Game
/**
	switch (MyRandom(4)) {
	case 0: CommString("!Look at this showroom FILLED with glamorous PRIZES!!    Its...");	break;
	case 1: CommString("!LIVE, from beautiful downtown Burbank California!       Its...");	break;
	case 2: CommString("!There's cash to give away and hundreds of glamorous prizes on...");	break;
	case 3: CommString("!LIVE, from beautiful downtown cyberspace!!   Its...");	break;
	}
***/
	CommPrintf("!@500,20 WHEEL\r OF\rCHEESE\r\rwith your host... %s!",PatBotName);
/**
	CommString("!Here's your host... spud!!");
	switch (MyRandom(6)) {
	case 0: CommString("How ya doin!  **fake smile**");						break;
	case 1: CommString("Good evening! or whatever the hell time it is...");	break;
	case 2: CommString("Is my fly open?   I hate when that happens...");		break;
	case 3: CommString("Hello. Did you know I have an IQ of 72?");			break;
	case 4: CommPrintf("Hi everybody!  Did you know I've hosted %ld games?",100000L+Random()); break;
	case 5:	CommPrintf("Carrot you look lovely today :)");
			CommPrintf("%sSo do you Spud!",CarrotPrefix);					break;
	}
**/
	/* CommPrintf("There are %d players",gWOF.nbrActivePlayers); */
/*
	for (i = 0,qptr = gWOF.playerList.firstElement; i < gWOF.playerList.nbrElements; ++i,qptr = qptr->nextElement) {
		if (((PlayerRecord *) qptr)->present && 
			((PlayerRecord *) qptr)->playing)
			CommPrintf("%s",GetPlayerName(((PlayerRecord *) qptr)->playerID));
	}
	switch (MyRandom(3)) {
	case 0: CommString("...and whoever else drops by during the game");	break;
	case 1: CommString("...and whoever else wants to play");	break;
	case 2: CommString("...and anybody else who drops by");	break;
	}
*/
	BeginRound();
}

void SelectPuzzle()
{
	short	nbrCats,nbrPuzzles,cat,puzz,i,catID;
	ResType	catType;
	Str63	catName;
	Str255	puzzle;
	Ptr		p;
	Handle	puzzList;

	gWOF.letterFlags = 0;
	gWOF.solvedFlags = 0;
	gWOF.nbrSolved = 0;
	gWOF.noCons = false;

	// !! Load in a random puzzle
	nbrCats = Count1Resources('PUZZ');
	do {
		cat = MyRandom(nbrCats);	
	} while (gWOF.catFlags & (1L << cat));
	gWOF.catFlags |= (1L << cat);
	puzzList = Get1IndResource('PUZZ',cat+1);
	if (puzzList == NULL) {
		SysBeep(1);
	}
	else {
		GetResInfo(puzzList, &catID, &catType, catName);
		PtoCstr(catName);
		strcpy(gWOF.clue,(char *) catName);
		p = *puzzList;
		nbrPuzzles = *((short *) p);	p += sizeof(short);
		puzz = MyRandom(nbrPuzzles);
		for (i = 0; i < puzz-1; ++i)
			p += p[0]+1;
		BlockMove(p,puzzle,p[0]+1);
		PtoCstr(puzzle);
		strcpy(gWOF.puzzle,(char *) puzzle);
		gWOF.puzLength = strlen((char *) puzzle);
		gWOF.nbrLetters = 0;
		for (i = 0; i < gWOF.puzLength; ++i) {
			if (isalpha(gWOF.puzzle[i]))
				gWOF.puzzle[i] = toupper(gWOF.puzzle[i]);
			if (isalpha(gWOF.puzzle[i])) {
				gWOF.solved[i] = '_';
				gWOF.nbrLetters++;
			}
			else
				gWOF.solved[i] = gWOF.puzzle[i];
		}
		gWOF.solved[gWOF.puzLength] = 0;
		ReleaseResource(puzzList);
	}
}

int SelectPrize(Boolean grandFlag)
{
	short	prizeStrIdx,i,nbrPrizes;
	long	prizeValue;
	Str255	prizeName;
	Ptr		p;
	Handle	prizeList;

	prizeList = Get1Resource('STR#',GrandPrizeSTRs+grandFlag);
	if (prizeList == NULL) {
		SysBeep(1);
	}
	else {
		p = *prizeList;
		nbrPrizes = *((short *) p);	p += sizeof(short);
		prizeStrIdx = MyRandom(nbrPrizes);
		for (i = 0; i < prizeStrIdx-1; ++i)
			p += p[0]+1;
		BlockMove(p,prizeName,p[0]+1);
		PtoCstr(prizeName);
		prizeValue = atol((char *) prizeName);
		p = strchr((char *) prizeName,',');
		p = p+1;
		strcpy(gWOF.prizeName[gWOF.nbrPrizes],p);
		gWOF.prizeValue[gWOF.nbrPrizes] = prizeValue;
		++gWOF.nbrPrizes;
		ReleaseResource(prizeList);
		StatusPrintf(0,"%s",gWOF.puzzle);
	}
	
	return gWOF.nbrPrizes-1;
}



void BeginWaiting(short state)
{
	gWOF.gameState = state;
	gWOF.startWait = TickCount();
	gWOF.timeWarning = 0;
}

void WaitIdle()
{
	if (gWOF.startWait == 0) {
		gWOF.startWait = TickCount();
		return;
	}
	if (gWOF.gameState >= G_WaitSpin && gWOF.gameState < G_WaitFinal1) {
		if (!gWOF.curPlayer->present && TickCount() - gWOF.startWait > 15*60L) {
			EmoteBot(E_Normal);
			CommString("OK... Let's go on to the next player");
			NextPlayer();
		}
		else if (gWOF.timeWarning == 0 && TickCount() - gWOF.startWait > 30*60L) {
			if (gWOF.curPlayer->money == 0 && gWOF.curPlayer->permMoney == 0 && gWOF.gameState == G_WaitSpin)
			{
				EmoteBot(E_Normal);
				if (gWOF.noCons || gWOF.nbrSolved == gWOF.nbrLetters)
					ShowPuzzle(0,"I suggest you \"solve\", %s",RandNick(gWOF.curPlayer));
				else
					ShowPuzzle(0,"say \"spin\", %s",RandNick(gWOF.curPlayer));
			}
			else {
				EmoteBot(E_Question);
				switch (MyRandom(3)) {
				case 0:	
					if (gCarrot)
						CommPrintf("%s Go %s!",CarrotPrefix,RandNick(gWOF.curPlayer));	
					else
						CommPrintf(":looks at %s expectantly",RandNick(gWOF.curPlayer));	
					break;
				case 1:	CommPrintf(":waits for %s...",RandNick(gWOF.curPlayer));		break;
				case 2:	CommPrintf("anytime %s...",RandNick(gWOF.curPlayer));						break;
				}
			}
			gWOF.timeWarning = 1;
		}
		else if (gWOF.timeWarning == 1 && TickCount() - gWOF.startWait > 45*60L) {
			EmoteBot(E_Mad);
			if (gWOF.curPlayer->lastActive < gWOF.startWait)
				CommPrintf("!  %s!  ",GetPlayerName(gWOF.curPlayer->playerID));
			else if (gWOF.curPlayer->money == 0 && gWOF.curPlayer->permMoney == 0 && gWOF.gameState == G_WaitSpin)
				CommPrintf("you can \"spin\" or \"solve\", %s",RandNick(gWOF.curPlayer));
			else
				CommPrintf("we don't have all day %s.",RandNick(gWOF.curPlayer));
			gWOF.timeWarning = 2;
		}
		else if (TickCount() - gWOF.startWait > 60*60L) {
			EmoteBot(E_Normal);
			if (gWOF.curPlayer->lastActive < gWOF.startWait)
				CommPrintf("Let's move on...");
			else
				CommPrintf("Sorry %s, your time is up!",RandNick(gWOF.curPlayer));
			if (gWOF.curPlayer->lastActive < gWOF.startWait)
				gWOF.curPlayer->playing = false;
			CountActivePlayers();
			NextPlayer();
		}
	}
	if (gWOF.gameState == G_WaitFinal2 && TickCount() - gWOF.startWait > 40*60L) {
		EmoteBot(E_Sad);
		CommPrintf("Sorry, time's up %s!",RandNick(gWOF.curPlayer));
		EndGame(0);
		return;
	}
	if (gWOF.gameState == G_WaitFinal1 && TickCount() - gWOF.startWait > 60*60L) {
		EmoteBot(E_Sad);
		CommPrintf("Sorry, time's up %s!",RandNick(gWOF.curPlayer));
		EndGame(0);
		return;
	}
	if (gWOF.gameState == G_WaitNextGame && TickCount() - gWOF.startWait > 45*60L) {
		gWOF.gameState = G_Idle;
	}
}

Boolean FreeSpinCheck()
{
	if (gWOF.curPlayer->freeSpins <= 0)
		return false;
	else {
		ShowPuzzle(0,"%s - You've got a free spin. Spin or Pass.",RandNick(gWOF.curPlayer));
		BeginWaiting(G_WaitFreeSpin);
		return true;
	}
}

char *ShowPlayerMoney(PlayerRecord *pr, Boolean concatFlag)
{
	static char output[256]="";
	if (pr == gWOF.curPlayer) {
		if (pr->money != 0) {
			if (concatFlag)
				sprintf(output,"You've got $%ld, ",pr->money);
			else
				sprintf(output,"You've got $%ld, %s",pr->money,GetPlayerName(pr->playerID));
		}
	}
	else {
		if (pr->money != 0)
			sprintf(output,"%s has $%ld",GetPlayerName(pr->playerID), pr->money);
		else
			sprintf(output,"%s has no money",GetPlayerName(pr->playerID));
	}
	return output;
}

short hiliteMode;

// Which indicates letter to highlight, -1 for blanks
void ShowPuzzle(short which, char *str,...)
{
	short	i;

	char	tbuf[256],pbuf[256],*sp;
	va_list args;

	va_start(args,str);
	vsprintf(pbuf,str,args);
	va_end(args);

	sp = tbuf;

	if (gCarrot) {
		strcpy(sp,CarrotPrefix);
		sp += strlen(sp);
	}
	sprintf(sp,"%s:\r",gWOF.clue);
	sp += strlen(sp);
	for (i = 0; i < gWOF.puzLength; ++i) {
		if (gWOF.solved[i] == '_') {
			if (which == -1) {
				*(sp++) = '_';
				*(sp++) = ' ';
			}
			else {
				*(sp++) = '_';
				*(sp++) = ' ';
			}
		}
		else {
			if (gWOF.solved[i] == which) {
				*(sp++) = gWOF.solved[i];
				*(sp++) = ' ';
			}
			else if (gWOF.solved[i] == ' ') {
				*(sp++) = '\r';
			}
			else {
				*(sp++) = gWOF.solved[i];
				*(sp++) = ' ';
			}
		}
	}
	*sp = 0;
	if (gWOF.letterFlags != gWOF.solvedFlags) {
		sprintf(sp,"\r\r(~");
		sp += strlen(sp);
		for (i = 0; i < 32; ++i) {
			if ((gWOF.letterFlags & (1L << i)) && !(gWOF.solvedFlags & (1L << i))) {
				*(sp++) = (char) (i+'a');
			}
		}
		*(sp++) = ')';
		*sp = 0;
	}
	CommPrintf("@10,10^%s\r%s",tbuf,pbuf);
}

void NextPlayer()
{
	if (gWOF.nbrActivePlayers < 1) {
		CommPrintf("Looks like everybody's left.  Resetting");
		gWOF.gameState = G_Idle;
		return;
	}
	
	SelectNextPlayer();
	switch (MyRandom(3)) {
	case 0: ShowPuzzle(0,"Next up is %s, with $%ld",GetPlayerName(gWOF.curPlayer->playerID),gWOF.curPlayer->money);	break;
	case 1: ShowPuzzle(0,"OK %s, it's your turn.  You've got $%ld",GetPlayerName(gWOF.curPlayer->playerID),gWOF.curPlayer->money);	break;
	case 2: ShowPuzzle(0,"%s is up next.  You've got $%ld",GetPlayerName(gWOF.curPlayer->playerID),gWOF.curPlayer->money);	break;
	}

	BeginWaiting(G_WaitSpin);
}

short SolveLetter(short letter)
{
	short 	sum,i;

	sum = 0;
	for (i = 0; i < gWOF.puzLength; ++i) {
		if (gWOF.puzzle[i] == letter) {
			gWOF.solved[i] = letter;
			++gWOF.nbrSolved;
			++sum;
			gWOF.solvedFlags |= (1L << (letter-'A'));
		}
	}
	gWOF.letterFlags |= (1L << (letter-'A'));
	if (sum == 0) {
		EmoteBot(E_Sad);
		switch (MyRandom(3)) {
		case 0:		CommPrintf(")no Sorry %s, there are no \'%c\'s",RandNick(gWOF.curPlayer),(char) letter);	break;
		case 1:		CommPrintf(")no No \'%c\'s!",(char) letter);	break;
		case 2:		CommPrintf(")no Nope!  No \'%c\'s",(char) letter);	break;
		}
	}
	else {
		EmoteBot(E_Smile);
		switch (sum) {
		case 1:		CommPrintf(")yes There is one %c!",letter);			break;
		case 2:		CommPrintf(")yes There are two %cs",letter);			break;
		case 3:		CommPrintf(")yes There are three %cs",letter);			break;
		case 4:		CommPrintf(")yes There are four %cs",letter);			break;
		case 5:		CommPrintf(")yes There are five %cs",letter);			break;
		case 6:		CommPrintf(")yes There are six %cs",letter);			break;
		case 7:		CommPrintf(")yes There are seven %cs",letter);			break;
		case 8:		CommPrintf(")yes There are eight %cs",letter);			break;
		case 9:		CommPrintf(")yes There are nine %cs",letter);			break;
		default:	CommPrintf(")yes There are %d %cs",sum,letter);		break;
		}
	}
	return sum;
}

void BuyVowel(short vowel)
{
	short	sum;
	if (gWOF.letterFlags & (1L << (vowel-'A')))  {
		EmoteBot(E_Sad);
		ShowPuzzle(0,"I'm sorry %s, \'%c\' has already been guessed, try again",RandNick(gWOF.curPlayer),(char) vowel);
	}
	else {
		gWOF.curPlayer->money -= 250;
		sum = SolveLetter(vowel);
		if (sum == 0) {
			// ShowPlayerMoney(gWOF.curPlayer,false);
			if (!FreeSpinCheck())
				NextPlayer();
		}
		else {
			ShowPuzzle(vowel,"");
			if (gWOF.nbrSolved == gWOF.nbrLetters) {
				switch (MyRandom(3)) {
				case 0: CommString(":Hmmm..  I wonder what it is...  :)");	break;
				case 1: CommString(":I think I know it now...  :)");	break;
				case 2: CommString("Uh... you gonna spin again? :)");	break;
				}
			}
			;
			switch (MyRandom(4)) {
			case 0:	ShowPuzzle(0,"%s\rNow what, %s?",ShowPlayerMoney(gWOF.curPlayer,true),
													RandNick(gWOF.curPlayer));	break;
			case 1:	ShowPuzzle(0,"%s\rWhat next, %s?",ShowPlayerMoney(gWOF.curPlayer,true),
										RandNick(gWOF.curPlayer));	break;
			case 2:	ShowPuzzle(0,"%s\rWhat now, %s?",ShowPlayerMoney(gWOF.curPlayer,true),
													RandNick(gWOF.curPlayer));	break;
			default: ShowPuzzle(0,"%s\rGo again.",ShowPlayerMoney(gWOF.curPlayer,true),
													RandNick(gWOF.curPlayer));	break;
			}
			EmoteBot(E_Question);
			BeginWaiting(G_WaitSpin);
		}
	}
}

void GuessCons(short cons)
{
	short	sum,i;
	Boolean	nocons;
	if (gWOF.letterFlags & (1L << (cons-'A')))  {
		EmoteBot(E_Sad);
		ShowPuzzle(0,"I'm sorry %s, \'%c\' has already been guessed, try again",RandNick(gWOF.curPlayer),(char) cons);
	}
	else {
		sum = SolveLetter(cons);
		if (sum == 0) {
			// ShowPlayerMoney(gWOF.curPlayer,false);
			if (!FreeSpinCheck())
				NextPlayer();
		}
		else {
			ShowPuzzle(cons,"");
			nocons = true;
			for (i = 0; i < gWOF.puzLength; ++i) {
				if (gWOF.solved[i] == '_' && strchr("AEIOU",toupper(gWOF.puzzle[i])) == NULL) {
					nocons = false;
					break;
				}
			}
			if (nocons) {
				gWOF.noCons = true;
				CommString("!@500,20)FazeIn There are no consonants left!");
			}
			if (gWOF.nbrSolved == gWOF.nbrLetters) {
				switch (MyRandom(3)) {
				case 0: CommString(":Kinda pointless to spin now, isn't it?  :)");	break;
				case 1: CommString("What do you think it is?  :)");				break;
				case 2: CommString(":Hmmm I wonder what it is? :)");				break;
				}
				
			}
			if (gWOF.letterMode == W_Money) {
				gWOF.curPlayer->money += gWOF.letterValue*sum;
				if (gWOF.letterValue*sum > 5000) {
					EmoteBot(E_Maniacal);
					switch (MyRandom(3)) {
					case 0:	CommPrintf(")applause My my!");		break;
					case 1:	CommPrintf(")applause Wow!");		break;
					case 2:	CommPrintf(")applause Very nice!");	break;
					}
					if (gCarrot)
						CommPrintf("%s:Clap! Clap! *smile*",CarrotPrefix);
				}
				else
					EmoteBot(E_Smile);
			}
			else if (gWOF.letterMode == W_Prize) {
				gWOF.curPlayer->prizeFlags |= (1L << gWOF.letterValue);
				gWOF.curPlayer->nbrPrizes++;
				EmoteBot(E_Maniacal);
				if (MyRandom(2) == 0) {
					switch (MyRandom(3)) {
					case 0:	CommPrintf(")applause All right!");		break;
					case 1:	CommPrintf(")applause Nice job!");		break;
					case 2:	CommPrintf(")applause Very nice!");		break;
					}
				}
				CommPrintf("You've won %s!",gWOF.prizeName[gWOF.letterValue]);
				if (gCarrot)
					CommPrintf("%s:Clap! Clap! Clap!",CarrotPrefix);
			}
			switch (MyRandom(4)) {
			case 0:		ShowPuzzle(0,"%s\rNow what, %s?",ShowPlayerMoney(gWOF.curPlayer,true),RandNick(gWOF.curPlayer));		break;
			case 1:		ShowPuzzle(0,"%s\rWhat next, %s?",ShowPlayerMoney(gWOF.curPlayer,true),RandNick(gWOF.curPlayer));	break;
			case 2:		ShowPuzzle(0,"%s\rWhat now, %s?",ShowPlayerMoney(gWOF.curPlayer,true),RandNick(gWOF.curPlayer));		break;
			default:	ShowPuzzle(0,"%s\rGo again.",ShowPlayerMoney(gWOF.curPlayer,true),RandNick(gWOF.curPlayer));			break;
			}
			BeginWaiting(G_WaitSpin);
		}
	}
}


WheelList wList[] = {
	W_Prize,	0,
	W_Bankrupt,	0,
	W_Bankrupt,	0,
	W_FreeSpin,	0,
	W_LoseTurn,	0,
	W_Money,	100,
	W_Money,	100,
	W_Money,	100,
	W_Money,	150,
	W_Money,	150,
	W_Money,	150,
	W_Money,	200,
	W_Money,	200,
	W_Money,	200,
	W_Money,	300,
	W_Money,	400,
	W_Money,	500,
	W_Money,	750,
	W_Money,	1000,
	W_Money,	1200,
	W_Money,	1500,
	W_Money,	2000,
	W_Money,	2500,
	W_Money,	5000
};

#define NbrWheelSlots	(sizeof(wList)/sizeof(WheelList))

void SpinWheel()
{
	short	r,n;
	char	*randStr;
	/* CommPrintf("!Whrrrrrr r  r   r    r"); */
ReSpin:
	r = MyRandom(NbrWheelSlots);
	switch (wList[r].type) {
	case W_Money:
		EmoteBot(E_Smile);
		CommPrintf("!@500,20 $%ld",wList[r].value);
		if (wList[r].value >= 2000 && MyRandom(3) == 0) {
			EmoteBot(E_Maniacal);
			switch (MyRandom(3)) {
			case 0:	CommString(":)Applause All right!");	break;
			case 1:	CommString(":)Applause Wow!");	break;
			case 2:	CommString(":)Applause Hiyo!");	break;
			}
			if (gCarrot)
				CommPrintf("%s:Clap! Clap!",CarrotPrefix);
		}
		switch (MyRandom(3)) {
		case 0:	ShowPuzzle(0,"$%ld\rWhich letter, %s?",wList[r].value,RandNick(gWOF.curPlayer));		break;
		case 1:	ShowPuzzle(0,"$%ld\rYour letter?",wList[r].value,RandNick(gWOF.curPlayer));			break;
		case 2:	ShowPuzzle(0,"$%ld\rWhat's your letter?",wList[r].value,RandNick(gWOF.curPlayer));	break;
		}
		gWOF.letterMode = W_Money;
		gWOF.letterValue = wList[r].value;
		BeginWaiting(G_WaitCons);
		break;
	case W_Prize:
		EmoteBot(E_Smile);
		CommString("!@500,20)Debut PRIZE!!");
		if (gWOF.nbrPrizes >= 31)
			goto ReSpin;
		n = SelectPrize(0);
		switch (MyRandom(4)) {
		case 0:	randStr = "Wow!";	break;
		case 1: randStr = "Ooh!";	break;
		case 2:	randStr = "Hey!";	break;
		default:	randStr = "My my!";	break;
		}
		CommCache(")applause %s - %s worth $%ld!!\r\r",randStr,gWOF.prizeName[n],gWOF.prizeValue[n]);
		switch (MyRandom(3)) {
		case 0:	CommPrintf("Get the next letter right and you'll win it, %s!",RandNick(gWOF.curPlayer));	break;
		case 1:	CommPrintf("Get this letter right and it's yours!");	break;
		case 2:	CommPrintf("It could be yours, %s!",RandNick(gWOF.curPlayer));	break;
		}
		if (gCarrot)
			CommPrintf("%s:Clap! Clap!",CarrotPrefix);

		switch (MyRandom(3)) {
		case 0:	ShowPuzzle(0,"Which letter, %s?",RandNick(gWOF.curPlayer));	break;
		case 1:	ShowPuzzle(0,"Your letter?",RandNick(gWOF.curPlayer));	break;
		case 2:	ShowPuzzle(0,"What's your letter?",RandNick(gWOF.curPlayer));	break;
		}
		gWOF.letterMode = W_Prize;
		gWOF.letterValue = n;
		EmoteBot(E_Question);
		BeginWaiting(G_WaitCons);
		break;
	case W_Bankrupt:
		EmoteBot(E_Sad);
		CommString("!@500,20 BANKRUPT!");
		// Geek Added )laughter just to make them feel bad (MP sound) 08/22/96
		switch (MyRandom(4)) {
		case 0:	CommPrintf(")laughter Sorry %s...",RandNick(gWOF.curPlayer));		break;
		case 1:	CommPrintf(")laughter Ouch!");									break;
		case 2:	CommPrintf(")laughter Haha!  I mean... Sorry %s",RandNick(gWOF.curPlayer));	break;
		case 3:	CommPrintf(")laughter Jeez %s...",RandNick(gWOF.curPlayer));	break;
		}
		gWOF.curPlayer->money = 0;
		if (gWOF.curPlayer->prizeFlags)
			CommPrintf("...at least you get to keep your prize%s",
				gWOF.curPlayer->nbrPrizes > 1? "s" : "");
		if (!FreeSpinCheck())
			NextPlayer();
		break;
	case W_LoseTurn:
		EmoteBot(E_Sad);
		CommString("!@500,20 LOSE TURN!");
		switch (MyRandom(3)) {
		case 0:	CommString("Oh! You lose your turn!  Too bad...");	break;
		case 1: CommPrintf("%s, you lose your turn...",RandNick(gWOF.curPlayer));	break;
		case 2:	CommString("Shoot!  You lose your turn!");				break;
		}
		if (!FreeSpinCheck())
			NextPlayer();
		break;
	case W_FreeSpin:
		EmoteBot(E_Smile);
		CommString("!@500,20 A FREE SPIN!");
		switch (MyRandom(3)) {
		case 0:	ShowPuzzle(0,"Hold on to that and spin again, %s!",RandNick(gWOF.curPlayer)); break;
		case 1:	ShowPuzzle(0,"Spin again!"); break;
		case 2:	ShowPuzzle(0,"Spin again, %s!",RandNick(gWOF.curPlayer)); break;
		}
		gWOF.curPlayer->freeSpins++;
		BeginWaiting(G_WaitSpin);
		break;
	}
}


void SelectNextPlayer()
{
	QueueElement	*firstPlayer,*qp;
	short			n;
	PlayerRecord	*pr;
	char			*playerName;
	// Put front queue element in rear.

	if (gWOF.nbrActivePlayers < 1) {
		gWOF.gameState = G_Idle;
		return;
	}
	if (gWOF.playerList.firstElement == NULL)
		return;

	if (gWOF.playerList.firstElement->nextElement == NULL) {
		gWOF.curPlayer = (PlayerRecord *) gWOF.playerList.firstElement;
		return;
	}

	n = 0;
	do {
		firstPlayer = gWOF.playerList.firstElement;
		gWOF.playerList.firstElement = firstPlayer->nextElement;
		qp = gWOF.playerList.firstElement;
		while (qp->nextElement)
			qp = qp->nextElement;
		qp->nextElement = firstPlayer;
		firstPlayer->nextElement = NULL;
		++n;
		if (n >= 200) {
			short			i;
			QueueElement	*qptr;
			CommString("; Resetting...");
			gWOF.gameState = G_Idle;
			for (i = 0,qptr = gWOF.playerList.firstElement; i < gWOF.playerList.nbrElements; ++i,qptr = qptr->nextElement) {
				((PlayerRecord *) qptr)->present = 0;
			}
			gWOF.nbrActivePlayers = 0;
			return;
		}
		pr = (PlayerRecord *) gWOF.playerList.firstElement;
		playerName = GetPlayerName(pr->playerID);
		if (playerName == NULL || *playerName == 0 || GetPersonByID(pr->playerID) == NULL)
			pr->present = 0;
	} while (!pr->present || !pr->playing);
	CountActivePlayers();
	gWOF.curPlayer = (PlayerRecord *) gWOF.playerList.firstElement;
}

/**- unneeded - we got one
short MyRandom(short max)	// Returns number 0 <= n  < max
{
	long range;
	unsigned short qdRdm;
	qdRdm = Random();
	range = max;
	return (qdRdm * range) / 65536L;
}
**/

void BeginRound()
{
	// If > round 1, announce points
	short			i;
	QueueElement	*qptr;
	char			*randStr,*promptStr;
	gWOF.turnNbr = 0;

	switch (gWOF.roundNbr) {
	case 0:	randStr = "our first round";		break;
	case 1:	randStr = "round number two";		break;
	case 2:	randStr = "round number three";		break;
	}
	CommCache("!@500,20 Let's begin %s.\r",randStr);

	// Zero out Money
	for (i = 0,qptr = gWOF.playerList.firstElement; i < gWOF.playerList.nbrElements; ++i,qptr = qptr->nextElement) {
		((PlayerRecord *) qptr)->money = 0;
		((PlayerRecord *) qptr)->freeSpins = 0;
		((PlayerRecord *) qptr)->prizeFlags = 0;
		((PlayerRecord *) qptr)->nbrPrizes = 0;
	}

	SelectPuzzle();	// Select and initialize a random puzzle

	switch (gWOF.roundNbr) {
	case 0:	CommPrintf("%s our first puzzle.",gCarrot? "Carrot, show us" : "Here's");		break;
	case 1:	CommPrintf("%s our second puzzle.",gCarrot? "Carrot, show us" : "Here's");		break;
	case 2:	CommPrintf("%s our third puzzle.",gCarrot? "Carrot, show us" : "Here's");		break;
	}
	
	SelectNextPlayer();

	if (gWOF.roundNbr == 0)
		promptStr = "  Say \"spin\"";
	else
		promptStr = "";
	switch (MyRandom(3)) {
	case 0:		ShowPuzzle(-1,"First up is %s.%s",GetPlayerName(gWOF.curPlayer->playerID),promptStr);	break;
	case 1:		ShowPuzzle(-1,"%s is up first.%s",GetPlayerName(gWOF.curPlayer->playerID),promptStr);	break;
	case 2:		ShowPuzzle(-1,"Let's start with %s.%s",GetPlayerName(gWOF.curPlayer->playerID),promptStr);	break;
	}
	BeginWaiting(G_WaitSpin);
}

void FinishRound()
{
	short	i;

	CommCache("This game you won $%ld in cash.",gWOF.curPlayer->money);
	gWOF.curPlayer->permMoney += gWOF.curPlayer->money;
	gWOF.curPlayer->gameWinnings += gWOF.curPlayer->money;
	if (gWOF.curPlayer->nbrPrizes) {
		short	n = 0;
		for (i = 0; i < 31; ++i) {
			if (gWOF.curPlayer->prizeFlags & (1L << i)) {
				++n;
				CommCache("%s %s, worth $%ld ",
					(n == 1)? "  You've also won" : (n == gWOF.curPlayer->nbrPrizes? "and" : ""),
					gWOF.prizeName[i],gWOF.prizeValue[i]);
				gWOF.curPlayer->permMoney += gWOF.prizeValue[i];
				gWOF.curPlayer->gameWinnings += gWOF.prizeValue[i];
			}
		}
	}
	if (gWOF.curPlayer->gameWinnings > gWOF.curPlayer->money) {
		CommCache("for a total of $%ld!!",gWOF.curPlayer->gameWinnings);
	}
	CommPrintf("");

	++gWOF.roundNbr;
	if (gWOF.roundNbr < 3)		// was 3
		BeginRound();
	else
		FinalRound();
}

void FinalRound()
{
	QueueElement	*qptr;
	PlayerRecord	*maxPlayer,*cp;
	long	max;
	short	i,n;

	CommCache("That was our last game");
	max = 0;
	maxPlayer = (PlayerRecord *) gWOF.playerList.firstElement;
	for (i = 0,qptr = gWOF.playerList.firstElement; i < gWOF.playerList.nbrElements; ++i,qptr = qptr->nextElement) {
		if (((PlayerRecord *) qptr)->gameWinnings > max && 
			((PlayerRecord *) qptr)->present &&
			((PlayerRecord *) qptr)->playing) {
			max = ((PlayerRecord *) qptr)->gameWinnings;
			maxPlayer = (PlayerRecord *) qptr;
		}
	}
	for (i = 0,qptr = gWOF.playerList.firstElement; i < gWOF.playerList.nbrElements; ++i,qptr = qptr->nextElement) {
		if (((PlayerRecord *) qptr)->gameWinnings && maxPlayer != (PlayerRecord *) qptr) {
			cp = (PlayerRecord *) qptr;
			CommCache("\r%s won $%ld",GetPlayerName(cp->playerID),cp->gameWinnings);
		}
	}
	gWOF.curPlayer = maxPlayer;
	cp = maxPlayer;
	CommPrintf("");
	CommCache("!@500,20 Today's Grand Prize Finalist is %s who's already won $%ld\r",GetPlayerName(cp->playerID),cp->gameWinnings);
	CommCache("%s has a chance of winning today's Grand Prize:\r",GetPlayerName(cp->playerID));
	n = SelectPrize(1);
	CommPrintf("%s worth $%ld!!",gWOF.prizeName[n],gWOF.prizeValue[n]);
	SelectPuzzle();	// Select and initialize a random puzzle
	{
		for (i = 0; i < gWOF.puzLength; ++i) {
			if (strchr("STRLNE",gWOF.puzzle[i]) != NULL) {
				gWOF.solved[i] = gWOF.puzzle[i];
			}
		}
		gWOF.letterFlags |= (1L << ('S'-'A'));
		gWOF.letterFlags |= (1L << ('T'-'A'));
		gWOF.letterFlags |= (1L << ('R'-'A'));
		gWOF.letterFlags |= (1L << ('L'-'A'));
		gWOF.letterFlags |= (1L << ('N'-'A'));
		gWOF.letterFlags |= (1L << ('E'-'A'));
	}
/*	if (gCarrot)
		CommPrintf("Carrot, show us the final puzzle.");
	else
		CommPrintf("All you have to do is solve this final puzzle.");
*/
	ShowPuzzle(-1,"We've already provided the letters S,T,R,L,N,E\rGive me 3 letters and 1 vowel, %s, on a single line.",GetPlayerName(cp->playerID));
	BeginWaiting(G_WaitFinal1);
}


void EndGame(Boolean wonFlag)
{
	PlayerRecord *cp;
	cp = gWOF.curPlayer;
	if (wonFlag) {
		EmoteBot(E_Maniacal);
		if (gCarrot)
			CommPrintf("%s Clap! Clap! *smile* Clap!",CarrotPrefix);
		CommCache("!@500,20)applause Congratulations %s, you're our Grand Prize Winner\r",GetPlayerName(cp->playerID));
		CommCache("You've won %s worth $%ld ",gWOF.prizeName[gWOF.nbrPrizes-1],gWOF.prizeValue[gWOF.nbrPrizes-1]);
		cp->gameWinnings += gWOF.prizeValue[gWOF.nbrPrizes-1];
		cp->permMoney += gWOF.prizeValue[gWOF.nbrPrizes-1];
		CommPrintf("Giving you a total of $%ld in cash and prizes!!",cp->gameWinnings);
	}
	else {
		EmoteBot(E_Sad);
		strcpy(gWOF.solved,gWOF.puzzle);
		if (cp->nbrPrizes)
			CommPrintf("!@500,20 Too bad %s, but you still have won $%ld in cash and prizes!!",GetPlayerName(cp->playerID), cp->gameWinnings);
		else if (cp->gameWinnings)
			CommPrintf("!@500,20 Too bad %s, but you still have won $%ld!!",GetPlayerName(cp->playerID), cp->gameWinnings);
		else
			CommPrintf("!@500,20 Too bad %s!!",GetPlayerName(cp->playerID));
	}
	IntegrateScore(cp->gameWinnings, GetPlayerName(cp->playerID));
	ShowPuzzle(-1,"Here's the correct answer\rThat's it for Wheel of Cheese.  See ya in a few seconds!");
	BeginWaiting(G_WaitNextGame);
}

