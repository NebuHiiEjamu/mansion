// BotResp.c
//
// Wheel of Cheese
//
// Bot responses to Palace Events
//
// Note: #define BOTCODE		1 is defined in LOCAL.H
//
//
// The two principal entry points are ProcessBotMacro() and BotEntryPoint()

/*
todo:
	  add Menu Hooks (Carrot, StartBot, etc- examine original menu code)
	  	  
*/

#include "WOFBot.h"

// Globals

Boolean	gCarrot;	// Set from Menu
Boolean gWhisperFlag;
Boolean	gInitialized;
LONG	gWhoID,gMyID;
char	content[512];
char 	tokens[MaxTokens][TokLength];

void ShowStatus();
void Randomize(void);

LONG DoMyStandAloneCode(LONG selector)
{
	switch (selector) {
	case PPA_LAUNCHNOTIFY:
		break;
	case PPA_BEGIN:
		if (!gInitialized) {
			InitializeBot();
			gInitialized = true;
		}
		gCtl->flags |= PPAF_NEEDSNETEVENTS;
		gCtl->idleTime = 10;
		DoCallback(PC_LogMessage,0,"** Rat Bot Installed **\r");
		break;
	case PPA_IDLE:
		IdleBot();
		break;
	case PPA_REFRESH:
		break;
	case PPA_LOCALEVENT:
		break;
	case PPA_NETWORKEVENT:
		NetworkEntryPoint(gCtl->networkEventType, 
							gCtl->networkEventRefcon,
							gCtl->networkEventLength,
							gCtl->networkEventPtr);

		break;
	case PPA_PALACEEVENT:
		switch (gCtl->palaceEventType) {
		case PE_OutChat:
			break;
		case PE_InChat:
			ProcessBotChat(gCtl->palaceEventData,gCtl->palaceEventMessage);
			break;
		case PE_PPAMacro:
			ProcessBotMacro(gCtl->palaceEventMessage);
			break;
		}
		break;
	case PPA_TERMINATE:
		break;
	}
	return PPA_OK;
}

void LogMessage(char *template,...)
{
	va_list args;
	char	str[256];
	va_start(args,template);
	vsprintf(str,template,args);
	va_end(args);
	DoCallback(PC_LogMessage,0,str);
}

void DoUserScript(char *str)
{
	DoCallback(PC_ExecuteIptscrae,0,str);
}

PersonID GetMyID(void)
{
	// If we have it, return global, otherwise use callback
	if (gMyID == 0) {
		gMyID = DoCallback(PC_GetMyID,0,NULL);
	}
	return gMyID;
}

LONG GetNumberPeople()
{
	return DoCallback(PC_GetNumberPeople, 0, NULL);
}

UserRecPtr	GetPersonByIndex(LONG i)
{
	return (UserRecPtr) DoCallback(PC_GetUserByIndex,i,NULL);
}

UserRecPtr	GetPersonByID(LONG id)
{
	return (UserRecPtr) DoCallback(PC_GetUserByID,id,NULL);
}

UserRecPtr	GetPersonByName(char *name)
{
	return (UserRecPtr) DoCallback(PC_GetUserByName,0L,name);
}

void InitWOF()
{
	ShowStatus();
	LoadTopScores();
	LoadAutoOps();
	gWOF.gameState = G_Inactive;
	Randomize();
}


void EndWOF()
{
}

void ShowStatus()
{
}


void tokenize(char *buff)
{
	char *dp,c;
	short	i;
	i = 0;
	dp = tokens[i];
	while (isspace(*buff))
		++buff;
	while (*buff) {
		if (!isspace(*buff)) {
			if (isalpha(*buff) || isdigit(*buff) || *buff == '_' || *buff == '-') {
				c = *(buff++);
				if (isalpha(c))
					c = tolower(c);
				*(dp++) = c;
			}
			else
				++buff;
		}
		else {
			*dp = 0;
			buff++;
			while (isspace(*buff))
				++buff;
			++i;
			if (i >= MaxTokens-1)
				break;
			dp = tokens[i];
		}
	}
	*dp = 0;
	++i;
	tokens[i][0] = 0;
}

void RegisterCuteNick(PlayerRecord *pr, char *newNick)
{
	// Make this permanent!!
	if (pr){
		strcpy(pr->cutenick,newNick);
		AddCuteNick(GetPlayerName(pr->playerID),newNick);
	}
}

void ProcessBotMacro(char *str)
{
	while (*str && isspace(*str))
		++str;
	if (*str == 0)
		return;
	tokenize(str);
	if (strcmp(tokens[0],"nick") == 0) {
		PlayerRecord *pr;
		pr = GetPlayerRecordByName(tokens[1]);
		if (pr)
			RegisterCuteNick(pr,tokens[2]);
		else {
			char tbuf[32];
			sprintf(tbuf,"%s %s",tokens[1],tokens[2]);
			pr = GetPlayerRecordByName(tbuf);
			if (pr)
				RegisterCuteNick(pr,tokens[2]);
		}
	}
	else if (strcmp(tokens[0],"vanna") == 0) {
		gCarrot = !gCarrot;
		LogMessage(";Vanna %s\r",gCarrot? "on" : "off");
	}
	else if (strcmp(tokens[0],"on") == 0) {
		gWOF.gameState = G_Idle;
		LogMessage(";%s Active\r",PatBotName);
		ResetPlayers();
	}
	else if (strcmp(tokens[0],"off") == 0) {
		gWOF.gameState = G_Inactive;
		CommPrintf(":Wrrrr.... %s's eyes go dim...",PatBotName);
	}
}

Boolean RightPerson(PersonID whoID)
{
	if (whoID == gWOF.curPlayer->playerID)
		return true;
	return false;
}

Boolean PuzzleMatch(char *str1, char *str2) 
{
	char	buf1[128],buf2[128];
	char *sp,*dp;
	sp = str1;
	dp = buf1;
	while (*sp) {
		if (isalpha(*sp))
			*(dp++) = tolower(*sp);
		++sp;
	}
	*dp = 0;

	sp = str2;
	dp = buf2;
	while (*sp) {
		if (isalpha(*sp))
			*(dp++) = tolower(*sp);
		++sp;
	}
	*dp = 0;

	if (strcmp(buf1,buf2) == 0)
		return true;
	else
		return false;
}

Boolean BotFloodControl(short maxPerSec)
{
	static long lastCheck;
	static long	ctr;
	long	t;
	Boolean	retCode = false;
	t = time(NULL);
	if (t - lastCheck < 2) {
		if (++ctr > maxPerSec)
			retCode = true;
	}
	else {
		ctr = 1;
	}
	lastCheck = t;
	return retCode;
}

void ProcessSpudTalk()		// argStr[0] contains nick, content contains msg
{
	PlayerRecord	*pr;

	tokenize(content);

	if (gWhoID == GetMyID())
		return;

	pr = GetPlayerRecord(gWhoID);
	if (pr) {
		pr->lastActive = TickCount();
	}

	if (tokens[1][0] == 0 && tokens[0][0] != 0) {
		if (strcmp(tokens[0],"help") == 0 ||		// Global?
			strcmp(tokens[0],"help2") == 0 ||
			strcmp(tokens[0],"recap") == 0 ||
			strcmp(tokens[0],"money") == 0 ||
			strcmp(tokens[0],"newbie") == 0 ||
			strcmp(tokens[0],"play") == 0 ||
			strcmp(tokens[0],"join") == 0 ||
			strcmp(tokens[0],"stop") == 0 ||
			strcmp(tokens[0],"out") == 0 ||
			strcmp(tokens[0],"ratings") == 0 ||
			strcmp(tokens[0],"top10") == 0)
		{
			ProcessGlobal(tokens[0],gWhoID,0);
			return;
		}
		if (tokens[0][1] == 0) {					// Single Letter?
			if (RightPerson(gWhoID)) {
				if (gWOF.gameState == G_WaitSpin) {
					if (strchr("AEIOU",toupper(tokens[0][0])) != NULL) {
						if (gWOF.curPlayer->money >= 250)
							BuyVowel(toupper(tokens[0][0]));
						else {
							EmoteBot(E_Sad);
							ShowPuzzle(0,"Sorry %s, you need at least $250 - you only have $%ld",RandNick(gWOF.curPlayer),gWOF.curPlayer->money);
						}
					}
					else {
						if (gWOF.noCons || gWOF.nbrSolved == gWOF.nbrLetters)
							ShowPuzzle(0,"There are no consonants, %s, I suggest you \"solve\"",RandNick(gWOF.curPlayer));
						else
							ShowPuzzle(0,"Gotta spin first, %s",RandNick(gWOF.curPlayer));
					}
				}
				if (gWOF.gameState == G_WaitVowel) {
					if (strchr("AEIOU",toupper(tokens[0][0])) != NULL) {
						BuyVowel(toupper(tokens[0][0]));
					}
					else {
						ShowPuzzle(0,"That's not a vowel, %s.",RandNick(gWOF.curPlayer));
						BeginWaiting(G_WaitVowel);
					}
				}
				else if (gWOF.gameState == G_WaitCons) {
					if (strchr("BCDFGHJKLMNPQRSTVWXYZ",toupper(tokens[0][0])) != NULL) {
						GuessCons(toupper(tokens[0][0]));
					}
					else {
						ShowPuzzle(0,"That's not a consonant, %s.  Pick a consonant or pass.",RandNick(gWOF.curPlayer));
						// BeginWaiting(G_WaitCons);    don't allow extra time
					}
				}
				else if (gWOF.gameState == G_WaitFreeSpin) {
					if (toupper(tokens[0][0]) == 'Y') {
						SpinWheel();
					}
					else if (toupper(tokens[0][0]) == 'N'){
						NextPlayer();
					}
				}
				else if (gWOF.gameState == G_WaitFinal1) {
					ShowPuzzle(0,"All on one line please, %s",RandNick(gWOF.curPlayer));
					BeginWaiting(G_WaitFinal1);
				}
			}
			return;
		}
		// Geek Added check for gWOF.noCons to prevent spinning with only vowels left 08/22/96
		// Geek says the check should move inside of the statement and put out some useful
		// statement like "There are no consonants left, try 'vowel' or 'solve'
		if (gWOF.gameState == G_WaitSpin && stricmp(tokens[0],"spin") == 0 ) {
			if (RightPerson(gWhoID)) {
				if (gWOF.noCons)
					ShowPuzzle(0,"Sorry %s, there are no consonants left, try 'vowel' or 'solve'",RandNick(gWOF.curPlayer));
				else
					SpinWheel();
			}
			return;
		}
		if (gWOF.gameState >= G_WaitSpin && gWOF.gameState <= G_WaitFreeSpin && stricmp(tokens[0],"pass") == 0) {
			if (RightPerson(gWhoID))
				NextPlayer();
			return;
		}
		if (gWOF.gameState == G_WaitCons && stricmp(tokens[0],"vowel") == 0) {
			if (RightPerson(gWhoID)) {
				EmoteBot(E_Sad);
				ShowPuzzle(0,"Sorry %s, you can only buy vowels before you spin, please pick a consonant or pass.",RandNick(gWOF.curPlayer));
				// BeginWaiting(G_WaitCons);
			}
			return;
		}
		if (gWOF.gameState == G_WaitSpin && (stricmp(tokens[0],"vowel") == 0 || stricmp(tokens[0],"buy") == 0)) {
			if (RightPerson(gWhoID)) {
				if (gWOF.curPlayer->money < 250) {
					EmoteBot(E_Sad);
					ShowPuzzle(0,"I'm sorry %s, you need at least $250 to buy a vowel",RandNick(gWOF.curPlayer));
				}
				else if ((gWOF.letterFlags & VowelMask) == VowelMask) {
					EmoteBot(E_Sad);
					ShowPuzzle(0,"I'm sorry %s, we're fresh out of vowels!",RandNick(gWOF.curPlayer));
				}
				else {
					ShowPuzzle(0,"Which vowel would you like to buy, %s?",RandNick(gWOF.curPlayer));
					EmoteBot(E_Question);
					BeginWaiting(G_WaitVowel);
				}
			}
			return;
		}
		if (gWOF.gameState == G_WaitSpin && 
				 (stricmp(tokens[0],"guess") == 0 ||
				  stricmp(tokens[0],"solve") == 0)) {
			if (RightPerson(gWhoID)) {
				EmoteBot(E_Question);
				switch (MyRandom(2)) {
				case 0: ShowPuzzle(0,"What's your guess, %s?",RandNick(gWOF.curPlayer)); break;
				case 1: ShowPuzzle(0,"What's the answer, %s?",RandNick(gWOF.curPlayer)); break;
				}
				BeginWaiting(G_WaitSolve);
			}
			return;
		}
		if (gWOF.gameState == G_WaitFreeSpin && stricmp(tokens[0],"spin") == 0) {
			if (RightPerson(gWhoID)) {
				gWOF.curPlayer->freeSpins--;
				SpinWheel();
			}
			return;
		}
		if (gWOF.gameState == G_WaitFreeSpin &&  stricmp(tokens[0],"pass") == 0) {
			if (RightPerson(gWhoID))
				NextPlayer();
			return;
		}
	}
	if (gWOF.gameState == G_WaitSpin && RightPerson(gWhoID) && PuzzleMatch(content,gWOF.puzzle)) {
			EmoteBot(E_Maniacal);
			CommPrintf("!@500,20)applause %s, that is CORRECT!!",gWOF.curPlayer->nick);
			if (gCarrot)
				CommPrintf("%s:Clap! Clap!",CarrotPrefix);
			FinishRound();
			return;
	}
	if (gWOF.gameState == G_WaitFinal1 && RightPerson(gWhoID)) {
		short	n=0;
		short	i;
		short	sC=0,sV=0;

		if (TickCount() - gWOF.startWait < 4*60L)
			return;

		for (i = 0; content[i]; ++i) {
			if (isalpha(content[i])) {
				if (strchr("AEIOU",toupper(content[i])) != NULL)
					++sV;
				else
					++sC;
			}
		}
		if (sC != 3 || sV != 1) {
			if (PuzzleMatch(content,gWOF.puzzle)) {
				EmoteBot(E_Maniacal);
				CommPrintf("!@500,20)applause %s, that is CORRECT!!",gWOF.curPlayer->nick);
				if (gCarrot)
					CommPrintf("%s:Clap! Clap!",CarrotPrefix);
				EndGame(1);
			}
			else if ((stricmp(tokens[0],"solve") == 0 || stricmp(tokens[0],"pass") == 0) &&
					strlen(tokens[0]) > 3) {
				EmoteBot(E_Question);
				ShowPuzzle(0,"OK, what's the answer %s?",RandNick(gWOF.curPlayer));
				BeginWaiting(G_WaitFinal2);
			}
			else {
				ShowPuzzle(0,"3 letters and 1 vowel on one line please, %s, or \"solve\"",RandNick(gWOF.curPlayer));
				BeginWaiting(G_WaitFinal1);
			}
			return;
		}
		for (i = 0; content[i]; ++i) {
			if (isalpha(content[i])) {
				for (n = 0; n < gWOF.puzLength; ++n) {
					if (gWOF.puzzle[n] == toupper(content[i])) {
						gWOF.solved[n] = gWOF.puzzle[n];
					}
				}
			}
		}
		gWOF.letterFlags = 0;
		ShowPuzzle(0,"Ok %s, here's the final puzzle.\rYou've got 40 seconds to solve it - GO",RandNick(gWOF.curPlayer));
		BeginWaiting(G_WaitFinal2);
		return;
	}

	if ((gWOF.gameState == G_WaitSolve || gWOF.gameState == G_WaitFinal2) && RightPerson(gWhoID)) {
		// clear type ahead on final
		if (gWOF.gameState == G_WaitFinal2 && TickCount() - gWOF.startWait < 5*60L)
			return;
		if (PuzzleMatch(content,gWOF.puzzle)) {
			EmoteBot(E_Maniacal);
			CommPrintf("!@500,20)applause %s, that is CORRECT!!",gWOF.curPlayer->nick);
			if (gCarrot)
				CommPrintf("%s:Clap! Clap!",CarrotPrefix);
			if (gWOF.gameState == G_WaitFinal2)
				EndGame(1);
			else
				FinishRound();
		}
		else if (stricmp(tokens[0],"pass") == 0) {
			if (gWOF.gameState == gWOF.gameState)
				EndGame(0);
			else
				NextPlayer();
		}
		else if (stricmp(tokens[0],"solve") == 0 || strlen(content) < strlen(gWOF.puzzle)-3) {
			ShowPuzzle(0,"OK, what's the answer %s?",RandNick(gWOF.curPlayer));
			EmoteBot(E_Question);
			BeginWaiting(gWOF.gameState);
		}
		else {
			EmoteBot(E_Sad);
			CommPrintf("Sorry %s, that is incorrect.",RandNick(gWOF.curPlayer));
			if (gWOF.gameState == G_WaitFinal2)
				EndGame(0);
			else
				NextPlayer();
		}
		return;
	}


	ProcessChat();
}

void ProcessChat()
{
	Boolean	spudFlag,carrotFlag;
	short	i;
	char	reply[256];

	spudFlag = carrotFlag = 0;
	for (i = 0; tokens[i][0]; ++i) {
		if (stricmp(VannaName1,tokens[i]) == 0 ||
			stricmp(VannaName2,tokens[i]) == 0) {
			carrotFlag = true;
		}
		if (stricmp(PatName1,tokens[i]) == 0 ||
			stricmp(PatName2,tokens[i]) == 0 ||
			stricmp(PatBotName,tokens[i]) == 0) {
			spudFlag = true;
		}
	}
	if (BotFloodControl(8))
		return;
	for (i = 0; tokens[i][0]; ++i) {
		if (stricmp(tokens[i],VannaName1) == 0 ||
			stricmp(tokens[i],VannaName2) == 0) {
			if (!gCarrot) {
				switch (MyRandom(4)) {
				case 0:	CommPrintf("%s's a little sick today",VannaName1);					break;
				case 1:	CommPrintf("%s just got her head enlarged",VannaName1);	break;
				case 2:	CommPrintf("Doesn't %s look like a big Barbie doll?",VannaName1);	break;
				default: CommPrintf("%s's out with the baby today...",VannaName1);	break;
				}
			}
			else {
				CommPrintf("Doesn't %s look like a big Barbie doll?",VannaName1);
			}
			return;
		}
		if (strcmp(tokens[i],"jeopardy") == 0 || strcmp(tokens[i],"alex") == 0) {
			EmoteBot(E_Sad);
			switch (MyRandom(3)) {
			case 0:	CommString("I wish I had the smarts for Jeopardy...");	break;
			case 1:	CommString("Jeopardy is a truly great game");	break;
			default: CommPrintf("%s doesn't know how to play Jeopardy",VannaName1);	break;
			}
			return;
		}
		if (strcmp(tokens[i],"fuck") == 0 || strcmp(tokens[i],"shit") == 0 ||
			strcmp(tokens[i],"cunt") == 0) {
			PlayerRecord	*pr;
			EmoteBot(E_Mad);
			pr = GetPlayerRecord(gWhoID);
			pr->cussCount++;
			if (pr->cussCount >= 3) {
				CommPrintf("bye bye %s",GetPlayerName(gWhoID));
			}
			else {
				switch (MyRandom(3)) {
				case 0:	CommString("Shhh... this is a family show");			break;
				case 1:	CommString("I love you too");							break;
				default: CommPrintf("I don't believe \"%s\" is in the puzzle",tokens[i]); break;
				}
			}
			return;
		}
		if ((spudFlag || carrotFlag) && 
			 (strcmp(tokens[i],"hello") == 0 || strcmp(tokens[i],"hi") == 0)) {
			EmoteBot(E_Smile);
			switch (MyRandom(3)) {
			case 0:	CommPrintf("Hi %s! You'll be up soon, I hope.",GetPlayerName(gWhoID));	break;
			case 1:	CommString("Top 'o the morning, or whatever the hell it is.");			break;
			default: CommPrintf("Howdy %s!",GetPlayerName(gWhoID)); break;
			}
			return;
		}
		if (strcmp(tokens[i],"vowel") == 0 && RightPerson(gWhoID)) {
			if (gWOF.gameState == G_WaitSpin)
				CommString("Say \"vowel\" to buy a vowel");
			else
				CommString("It's too late to buy a vowel.");
			return;
		}
		if (strcmp(tokens[i],"spin") == 0 && RightPerson(gWhoID)) {
			if (gWOF.gameState == G_WaitSpin)
				CommString("Say \"spin\" to spin");
			else
				CommString("You can't spin right now.");
			return;
		}
		if ((spudFlag || carrotFlag) && 
			(strcmp(tokens[i],"bye") == 0 || strcmp(tokens[i],"goodbye") == 0)) {
			EmoteBot(E_Smile);
			switch (MyRandom(3)) {
			case 0:	CommPrintf("Bye %s! See you soon, I hope.",GetPlayerName(gWhoID));			break;
			case 1:	CommString("Top 'o the evening, or whatever the hell it is.");							break;
			default: CommPrintf("Seeya %s!",GetPlayerName(gWhoID)); break;
			}
			return;
		}
		if ((spudFlag || carrotFlag) && 
			(stricmp("love",tokens[i]) == 0 || stricmp("hate",tokens[i]) == 0)) {
			EmoteBot(E_Smile);
			if (carrotFlag && gCarrot)
				CommString("\msg carrotbot zxcv ");
			switch (MyRandom(3)) {
			case 0:	CommPrintf(")kiss I love you %s! **smooch**",GetPlayerName(gWhoID));		break;
			case 1:	CommPrintf(":hugs %s  {{}}",GetPlayerName(gWhoID));				break;
			default: CommPrintf(":I'm in love with %s!",GetPlayerName(gWhoID)); 				break;
			}
			return;
		}
		if (stricmp("punch",tokens[i]) == 0 ||
			stricmp("kick",tokens[i]) == 0) {
			EmoteBot(E_Sad);
			if (carrotFlag && gCarrot)
				CommString("\msg carrotbot zxcv ");
			switch (MyRandom(3)) {
			case 0:	CommString(":Ouch.");						break;
			case 1:	CommString("I love you too");				break;
			default: CommPrintf("Get a life %s!",GetPlayerName(gWhoID)); break;
			}
			return;
		}
	}
	if (!spudFlag && !carrotFlag)
		return;
	if (strcmp(tokens[0],PatBotName) == 0)
		strcpy(content,&content[strlen(PatBotName)+1]);
	else if (strcmp(tokens[0],PatName1) == 0)
		strcpy(content,&content[strlen(PatName1)+1]);
	else if (strcmp(tokens[0],PatName2) == 0)
		strcpy(content,&content[strlen(PatName2)+1]);
	else if (strcmp(tokens[0],VannaName1) == 0)
		strcpy(content,&content[strlen(VannaName1)+1]);
	else if (strcmp(tokens[0],VannaName2) == 0)
		strcpy(content,&content[strlen(VannaName2)+1]);
	if (*content == ',')
		strcpy(content,content+1);
	if (*content == ' ')
		strcpy(content,content+1);
	while (content[0] && isspace(content[strlen(content)-1]))
		content[strlen(content)-1] = 0;
	Eliza(content,reply);
	if (reply[0]) {
		CommPrintf("%s",reply);
	}
}

void ProcessGlobal(char *cmd, PersonID whoID, Boolean tellFlag)
{
	PlayerRecord	*pr;
	PersonID		privateWho = 0;

	if (whoID == GetMyID())
		return;

	if (tellFlag)
		privateWho = gWhoID;

	if (BotFloodControl(8))
		return;

	if (strcmp(cmd,"help") == 0) {
		PrivatePrintf(gWhoID,"; You may say \"spin\", \"solve\", \"vowel\" or say a letter");
		PrivatePrintf(gWhoID,"; Say help2 for extended help & other commands");
		if (gWOF.gameState == G_WaitNextGame)
			gWOF.startWait += 15*60L;
	}
	else if (strcmp(cmd,"help2") == 0) {
		if (BotFloodControl(2))
			return;
		PrivatePrintf(gWhoID,"; *** Wheel of Cheese Help ***");
		PrivatePrintf(gWhoID,";The following commands are used during play");
		PrivatePrintf(gWhoID,";spin      - spin the wheel");
		PrivatePrintf(gWhoID,";solve     - solve the puzzle");
		PrivatePrintf(gWhoID,";vowel     - buy a vowel");
		PrivatePrintf(gWhoID,";x         - guess the letter 'X'");
		PrivatePrintf(gWhoID,";a         - buy the vowel 'A'");
		PrivatePrintf(gWhoID,"");
		PrivatePrintf(gWhoID,";The following global commands can be used at anytime");
		PrivatePrintf(gWhoID,";play      - include me in the game");
		PrivatePrintf(gWhoID,";out       - i'm not here to play");
		PrivatePrintf(gWhoID,";recap     - show current puzzle");
		PrivatePrintf(gWhoID,";money     - show my money");
		PrivatePrintf(gWhoID,";top10     - show the top 10 list");
		if (gWOF.gameState == G_WaitNextGame)
			gWOF.startWait += 15*60L;
	}
	else if (strcmp(tokens[0],"recap") == 0) {
		QueueElement	*qptr;
		PlayerRecord	*cp;
		short			i;

		if (BotFloodControl(2))
			return;

		for (i = 0,qptr = gWOF.playerList.firstElement; i < gWOF.playerList.nbrElements; ++i,qptr = qptr->nextElement) {
			cp = (PlayerRecord *) qptr;
			if (cp->playing && cp->present)
				PrivatePrintf(privateWho,"%s  current: $%ld    total: $%ld",cp->nick,cp->money,cp->gameWinnings);
		}

		if (gWOF.gameState == G_Idle)
			PrivatePrintf(privateWho,"No game in progress");
		PrivatePrintf(privateWho,"%s is up",gWOF.curPlayer->nick);
	}
	else if (strcmp(tokens[0],"newbie") == 0) {
		pr = GetPlayerRecord(gWhoID);
		if (pr) {
			pr->newbie = (pr->newbie? 0 : 4);
			PrivatePrintf(privateWho,"Newbie mode %s",pr->newbie? "on" : "off");
		}
	}
	else if (strcmp(tokens[0],"money") == 0) {
		pr = GetPlayerRecord(gWhoID);
		if (pr) {
			if (pr->gameWinnings != pr->money)
				PrivatePrintf(privateWho,"%s, you currently have $%ld, total for this game is $%ld",GetPlayerName(gWhoID),pr->money,pr->gameWinnings);
			else
				PrivatePrintf(privateWho,"%s, you currently have $%ld",GetPlayerName(gWhoID),pr->money);
		}
	}
	else if (strcmp(tokens[0],"play") == 0 || strcmp(tokens[0],"join") == 0) {
		pr = GetPlayerRecord(gWhoID);
		if (pr) {
			pr->playing = true;
			CountActivePlayers();
			PrivatePrintf(gWhoID, "%s is in",GetPlayerName(gWhoID));
		}
		if (gWOF.gameState == G_WaitNextGame)
			gWOF.gameState = G_Idle;
	}
	else if (strcmp(tokens[0],"stop") == 0 || strcmp(tokens[0],"out") == 0) {
		pr = GetPlayerRecord(gWhoID);
		if (pr) {
			pr->playing = false;
			CountActivePlayers();
			PrivatePrintf(gWhoID, "%s is out",GetPlayerName(gWhoID));
			if (pr == gWOF.curPlayer)
				NextPlayer();
		}
	}
	else if (strcmp(tokens[0],"top10") == 0 || strcmp(tokens[0],"ratings") == 0) {
		if (BotFloodControl(2))
			return;
		DisplayTopScores(gWhoID);
		if (gWOF.gameState == G_WaitNextGame)
			gWOF.startWait += 15*60L;
	}
	else if (strcmp(tokens[0],"spudzxcv") == 0) {
		ProcessBotMacro(&content[9]);
	}
}

void ProcessPrivateMsg()	// argStr[0] contains nick, content contains msg
{
	if (gWhoID == GetMyID())
		return;

	tokenize(content);

	ProcessGlobal(tokens[0],gWhoID,1);
}

void ProcessPlayerLeft(short how)		// arg1 is player nick
{
	PlayerRecord	*pr;
	// set inactive flag, keep around in case he comes back
	if (gWhoID == GetMyID())
		return;
	pr = GetPlayerRecord(gWhoID);
	if (pr) {
		if (pr->present) {
			pr->present = 0;
			CountActivePlayers();
			pr->lastPresent = TickCount();
			if (pr == gWOF.curPlayer && gWOF.gameState != G_Idle) {
/*				if (how == 1 && (gWOF.curPlayer->permMoney || gWOF.curPlayer->gameWinnings)) {*/
/*					CommPrintf("Let's what a few seconds for %s to return...\r",pr->nick);*/
/*					gWOF.startWait = TickCount();*/
/*				}*/
/*				else*/
					NextPlayer();
			}
		}
	}
}


void ProcessPlayerJoined()		// arg1 is player nick, arg2 is player mail address
{
	PlayerRecord	*pr;
	if (gWhoID == GetMyID() || gWhoID == 0)
		return;
	pr = GetPlayerRecord(gWhoID);
	if (pr) {
		if (!pr->present) {
			pr->present = 1;
			CountActivePlayers();
			CheckAutoOps(GetPlayerName(pr->playerID));
			if (TickCount() - pr->lastPresent > 60L*30) {
				PrivatePrintf(gWhoID, "Welcome back %s!%s",GetPlayerName(pr->playerID),pr->playing? "" : "  Say \"play\" if you want in.");
			}
			pr->lastPresent = TickCount();
			pr->lastActive = TickCount();
			if (pr == gWOF.curPlayer) {
				CommPrintf("%s, you're still up",GetPlayerName(pr->playerID));
				switch (gWOF.gameState) {
				case G_WaitSpin:	ShowPuzzle(0,"Spin or Solve... %s",GetPlayerName(pr->playerID));		
									break;
				case G_WaitVowel:	ShowPuzzle(0,"Which vowel, %s?",GetPlayerName(pr->playerID));			
									EmoteBot(E_Question);
									break;
				case G_WaitCons:	ShowPuzzle(0,"Which letter, %s?",GetPlayerName(pr->playerID));			
									EmoteBot(E_Question);
									break;
				case G_WaitSolve:	ShowPuzzle(0,"What's your solution, %s?",GetPlayerName(pr->playerID));	
									EmoteBot(E_Question);
									break;
				case G_WaitFinal1:	ShowPuzzle(0,"Give me 3 letters, %s.",GetPlayerName(pr->playerID));	
									break;
				case G_WaitFinal2:	ShowPuzzle(0,"What's the answer, %s?",GetPlayerName(pr->playerID));	
									EmoteBot(E_Question);
									break;
				case G_WaitFreeSpin: ShowPuzzle(0,"Wanna use your free spin, %s?",GetPlayerName(pr->playerID));
									EmoteBot(E_Question);
									break;
				}
				BeginWaiting(gWOF.gameState);
			}
		}
	}
	else {
		pr = NewPlayer(gWhoID);
		if (gWOF.nbrActivePlayers == 1 && gWOF.gameState == G_Idle) {
			CommPrintf("Welcome %s!",GetPlayerName(pr->playerID));
/*			CommPrintf("You're the only player...  We sure could use one more!");
			CommPrintf("A new game will start shortly... say \"play\" to start now");
*/
			BeginWaiting(G_WaitNextGame);
			CheckAutoOps(GetPlayerName(pr->playerID));
		}
		else {
			PrivatePrintf(gWhoID,"Welcome %s!  You're in the game.",GetPlayerName(pr->playerID));
			CheckAutoOps(GetPlayerName(pr->playerID));
			if (gWOF.gameState == G_WaitNextGame)
				gWOF.gameState = G_Idle;
		}
	}
}

void ResetPlayers()
{
	PlayerRecord	*pr;
	long			saveCurPlayer = 0;
	short			i,n;
	UserRecPtr		up;

	if (gWOF.curPlayer)
		saveCurPlayer = gWOF.curPlayer->playerID;
	ClearPlayers();
	n = GetNumberPeople();
	for (i = 0; i < n; ++i) {
		up = GetPersonByIndex(i);
		if (up && up->userID != GetMyID())
			pr = NewPlayer(up->userID);
	}
	pr = GetPlayerRecord(saveCurPlayer);
	if (pr)
		gWOF.curPlayer = pr;
	else {
		gWOF.curPlayer = (PlayerRecord *) gWOF.playerList.firstElement;
		if (!gWOF.curPlayer)
			gWOF.gameState = G_Idle;
		else if (gWOF.gameState != G_Idle)
			NextPlayer();
	}	
}

// User Supplied Routines Follow

Boolean	gInitialized;

void InitializeBot()
{
	InitWOF();
}

void CleanupBot()
{
	EndWOF();
}

Boolean NetworkEntryPoint(long palaceEvent, long refCon, long length, char *buffer)
{
	static long lastTickle;
	static long lastEvent;

	if (gWOF.gameState == G_Inactive) {
		lastEvent = 0;
		return false;
	}
	switch (lastEvent) {
	case MSG_USERNEW:	ProcessPlayerJoined();	break;
	case MSG_LOGOFF:	ProcessPlayerLeft(1);	break;
	case MSG_USEREXIT:	ProcessPlayerLeft(0);	break;
	}

	lastEvent = palaceEvent;

	// Pre Processing
	switch (palaceEvent) {
	case MSG_USERNEW:
		gWhoID = *((long *) buffer);
		break;
  	case MSG_LOGOFF:
		gWhoID = refCon;
		break;
	case MSG_USEREXIT:
		gWhoID = refCon;
		break;
	case MSG_TALK:
	case MSG_XTALK:
		gWhoID = refCon;
		gWhisperFlag = false;
		break;
	case MSG_WHISPER:
	case MSG_XWHISPER:
		gWhoID = refCon;
		gWhisperFlag = true;
		break;
	}
	return false;
}

void ProcessBotChat(LONG whoChat, char *str)
{
	if (!gWhisperFlag) {
		// Extract Content
		//
		// Figure out who said it
		strcpy(content,str);
		if (whoChat != 0L) {
			// gWhoID = whoChat;
			ProcessSpudTalk();
		}
	}
	else {
		strcpy(content,str);
		if (whoChat) {
			// gWhoID = whoChat;
			ProcessPrivateMsg();
		}
	}
}

void IdleBot()
{
	// Idle Handling
	switch (gWOF.gameState) {
	case G_Idle:
		if (gWOF.nbrActivePlayers >= 1)
			BeginGame();
		break;
	case G_WaitSpin:	
	case G_WaitVowel:
	case G_WaitCons:
	case G_WaitSolve:
	case G_WaitFinal1:
	case G_WaitFinal2:
	case G_WaitFreeSpin:
	case G_WaitNextGame:
		WaitIdle();
		break;
	}

	CheckResponses();
/**
	if (gWOF.gameState != G_Inactive && time(NULL) - lastTickle > 120L) {
		lastTickle = time(NULL);
		switch (MyRandom(3)) {
		case 0:	CarrotPrintf(":*smile*");	break;
		case 1:	CarrotPrintf(":*giggle*");	break;
		case 2:	CarrotPrintf(":*yawn*");	break;
		}
	}
**/
}

void EmoteBot(short emotion)
{
	switch (emotion) {
	case E_Sad: 
			DoUserScript("[ \"pat2\" \"vanna2\" ] SETPROPS");	
			break;
	case E_Mad: 
			DoUserScript("[ \"pat3\" \"vanna2\" ] SETPROPS");	
			break;
	case E_Smile:
			if (MyRandom(2) == 0)
				DoUserScript("[ \"pat1\" \"vanna1\" ] SETPROPS");
			else
				DoUserScript("[ \"pat1\" \"vanna4\" ] SETPROPS");
			break;
	case E_Maniacal:
			DoUserScript("[ \"pat6\" \"vanna1\" ] SETPROPS");
			DoUserScript(" { [ \"pat6\" \"vanna3\" ] SETPROPS } 60 ALARMEXEC");
			DoUserScript(" { [ \"pat6\" \"vanna1\" ] SETPROPS } 80 ALARMEXEC");
			DoUserScript(" { [ \"pat6\" \"vanna3\" ] SETPROPS } 100 ALARMEXEC");
			DoUserScript(" { [ \"pat6\" \"vanna1\" ] SETPROPS } 120 ALARMEXEC");
			DoUserScript(" { [ \"pat6\" \"vanna3\" ] SETPROPS } 140 ALARMEXEC");
			DoUserScript(" { [ \"pat6\" \"vanna1\" ] SETPROPS } 160 ALARMEXEC");
			DoUserScript(" { [ \"pat6\" \"vanna3\" ] SETPROPS } 180 ALARMEXEC");
			DoUserScript(" { [ \"pat6\" \"vanna1\" ] SETPROPS } 200 ALARMEXEC");
			DoUserScript(" { [ \"pat6\" \"vanna3\" ] SETPROPS } 220 ALARMEXEC");
			DoUserScript(" { [ \"pat6\" \"vanna1\" ] SETPROPS } 240 ALARMEXEC");
			DoUserScript(" { [ \"pat6\" \"vanna3\" ] SETPROPS } 260 ALARMEXEC");
			DoUserScript(" { [ \"pat6\" \"vanna1\" ] SETPROPS } 280 ALARMEXEC");
			DoUserScript(" { [ \"pat6\" \"vanna3\" ] SETPROPS } 300 ALARMEXEC");
			DoUserScript(" { [ \"pat1\" \"vanna1\" ] SETPROPS } 340 ALARMEXEC");
			break;
	case E_Question: 
			switch (MyRandom(4)) {
			case 0:
				DoUserScript("[ \"pat5\" \"vanna1\" ] SETPROPS");
				break;
			case 1:
				DoUserScript("[ \"pat5\" \"vanna4\" ] SETPROPS");
				break;
			case 2:
				DoUserScript("[ \"pat4\" \"vanna1\" ] SETPROPS");
				break;
			case 3:
				DoUserScript("[ \"pat4\" \"vanna4\" ] SETPROPS");
				break;
			}
			break;
	case E_Normal:
			if (MyRandom(2) == 0)
				DoUserScript("[ \"pat4\" \"vanna1\" ] SETPROPS");
			else
				DoUserScript("[ \"pat4\" \"vanna4\" ] SETPROPS");
			break;
	default: DoUserScript("[ \"pat1\" \"vanna1\" ] SETPROPS");	break;
	}
}

int stricmp(const char *str1,const char *str2)
{
	while (*str1) {
		if (isalpha(*str1)) {
			if (toupper(*str1) != toupper(*str2))
				return 1;
		}
		else if (*str1 != *str2)
			return 1;
		++str1;
		++str2;
	}
	if (*str1 == 0 && *str2 == 0)
		return 0;
	else
		return 1;
}

/******************************************************************************************/
/*  Random number generator
 *       Source:  Stephen K. Park and Keith W. Miller, 
 *                "Random Number Generators:  Good Ones Are Hard to Find", 
 *                Communications of the ACM,
 *                vol. 31, p. 1192  (October 1988).
 */
#define	R_A	16807L
#define	R_M	2147483647L
#define R_Q	127773L
#define R_R	2836L

LONG 	gSeed = 1;

/******************************************************************************************/

LONG GetSRand(void)
{
	return gSeed;
}

/******************************************************************************************/

void MySRand(LONG s)
{
	gSeed = s;
	if (gSeed == 0)
		gSeed = 1;
}

/******************************************************************************************/

void Randomize(void)
{
	unsigned LONG t;

	GetDateTime(&t);
	MySRand(t);
}

/******************************************************************************************/

LONG LongRandom(void)
{
	LONG	hi,lo,test;

	hi   = gSeed / R_Q;
	lo   = gSeed % R_Q;
	test = R_A * lo - R_R * hi;
	if (test > 0)
    gSeed = test;
	else
	  gSeed = test + R_M;
	return gSeed;
}

/******************************************************************************************/

double DoubleRandom(void)
{
	return LongRandom() / (double) R_M;
}

/******************************************************************************************/

short MyRandom(short max)
{
	return (short) (DoubleRandom() * max);
}
