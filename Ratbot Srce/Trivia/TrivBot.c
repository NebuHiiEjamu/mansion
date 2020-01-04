// An alternate Trivia Game that I grafted onto the Wheel of Cheese code.
//

// TriviaBot.c

#include "WOFBot.h"

extern char content[];
extern LONG	gWhoID;
Handle		triviaBaseH;
long		gTriviaAlloc,gTriviaLen;

void EnableTriviaGame()
{
	gWOF.gameState = G_WaitTriviaGame;
	gWOF.startWait = TickCount() - 60*60L;
	LoadTriviaDatabase();
}

void StartTriviaGame()
{
	QueueElement	*qp;
	PlayerRecord	*pr;
	short			i;

	// load database
	CommPrintf("@500,10 !It's CHEEZARDY - with your host, Ratbot!");
	LoadTriviaDatabase();
	
	// reset counters
	gWOF.triviaQuestionNumber = 1;

	// For each player
	for (i = 0,qp = gWOF.playerList.firstElement; 
		 i < gWOF.playerList.nbrElements; 
		 ++i,qp = qp->nextElement)
	{
		pr = (PlayerRecord *) qp;
		pr->triviaScore = 0;
		pr->triviaActive = 0;
	}
	NextTriviaQuestion();
}

void NextTriviaQuestion()
{
	QueueElement	*qp;
	PlayerRecord	*pr;
	short			i;
	// For each player
	for (i = 0,qp = gWOF.playerList.firstElement; 
		 i < gWOF.playerList.nbrElements; 
		 ++i,qp = qp->nextElement)
	{
		pr = (PlayerRecord *) qp;
		pr->triviaCache = 0;
		pr->playerQRank = 0;
		pr->playerBBet = 0;
	}
	// Reset Question Counters
	gWOF.triviaQRank = 0;

	// Formulate basic question, display it
	LoadRandomQuestion();
	CommPrintf("You have 20 seconds to answer");
	CommPrintf("@10,10^Question #%d: %s\r\r%s",
				gWOF.triviaQuestionNumber,
				gWOF.curQuestion,gWOF.curAnswers);
	gWOF.gameState = G_WaitTriviaAnswer;
	gWOF.startWait = 0;
}

void EndTriviaQuestion()
{
	QueueElement	*qp;
	PlayerRecord	*pr;
	short			i;
	char			standingsStr[256] = "";

	// For each player, Tally Score, Accumulate List for Display
	for (i = 0,qp = gWOF.playerList.firstElement; 
		 i < gWOF.playerList.nbrElements; 
		 ++i,qp = qp->nextElement)
	{
		pr = (PlayerRecord *) qp;
		pr->triviaScore += pr->triviaCache;
		if (pr->triviaCache)
			pr->triviaActive = true;
		// Send out private messagees indicating correctness and score if QRank is non zero
		if (pr->playerQRank == -1) {
			PrivatePrintf(pr->playerID,"; %s, your answer was incorrect.  You lost 200 pts for a total of %ld",
						GetPlayerName(pr->playerID),pr->triviaScore);
		}
		else if (pr->playerQRank > 0) {
			switch (pr->playerQRank) {
			case 1:
				PrivatePrintf(pr->playerID,"; %s, your answer was correct and you answered 1st.\rYou won %ld pts for a total of %ld",
						GetPlayerName(pr->playerID),pr->triviaCache,pr->triviaScore);
				break;
			case 2:
				PrivatePrintf(pr->playerID,"; %s, your answer was correct and you answered 2nd.\rYou won %ld pts for a total of %ld",
						GetPlayerName(pr->playerID),pr->triviaCache,pr->triviaScore);
				break;
			case 3:
				PrivatePrintf(pr->playerID,"; %s, your answer was correct and you answered 3rd.\rYou won %ld pts for a total of %ld",
						GetPlayerName(pr->playerID),pr->triviaCache,pr->triviaScore);
				break;
			default:
				PrivatePrintf(pr->playerID,"; %s, your answer was correct and you answered %ldth!\rYou won %ld pts for a total of %ld",
						GetPlayerName(pr->playerID),pr->playerQRank,pr->triviaCache,pr->triviaScore);
			}
		}
	}
	FormatStandings(standingsStr);

	// Make public message showing answer, and scores
	if (gWOF.triviaQuestionNumber <= 2)
		CommPrintf("Check your Log Window to see your current score");
	if (++gWOF.triviaQuestionNumber >= 5) {
		StartBonusBetting();
		CommPrintf("@10,10 ^The correct answer was %s\r\r%s\r\rWinners place your bets!",
					gWOF.correctAnswer,standingsStr);
	}
	else {
		CommPrintf("@10,10 ^The correct answer was %s\r\r%s\r\rPlease stand by for the next question.",
					gWOF.correctAnswer,standingsStr);
		gWOF.gameState = G_WaitTriviaQuestion;
		gWOF.startWait = 0;
	}
}

void StartBonusBetting()
{
	// Message: place your bets!
	CommPrintf("The bonus question is coming up - top 5 winners place your bets (privately to me).  You may bet any amount up to your current score.");
	gWOF.gameState = G_WaitBonusBetting;
	gWOF.startWait = 0;
}

void BonusQuestion()
{
	QueueElement	*qp;
	PlayerRecord	*pr;
	short			i;
	// For each player
	for (i = 0,qp = gWOF.playerList.firstElement; 
		 i < gWOF.playerList.nbrElements; 
		 ++i,qp = qp->nextElement)
	{
		pr = (PlayerRecord *) qp;
		pr->triviaCache = 0;
		pr->playerQRank = 0;
	}
	// Reset Question Counters
	gWOF.triviaQRank = 0;

	// Formulate basic question, display it
	LoadRandomQuestion();
	CommPrintf("Winners - you have 20 seconds to answer");
	CommPrintf("@10,10^Bonus Question: %s\r\r%s",gWOF.curQuestion,gWOF.curAnswers);
	gWOF.gameState = G_WaitBonusAnswer;
	gWOF.startWait = 0;
}

void BonusAnswer()
{
	QueueElement	*qp;
	PlayerRecord	*pr;
	short			i;
	char			standingsStr[256] = "";

	// For each player
	for (i = 0,qp = gWOF.playerList.firstElement; 
		 i < gWOF.playerList.nbrElements; 
		 ++i,qp = qp->nextElement)
	{
		pr = (PlayerRecord *) qp;
		pr->triviaScore += pr->triviaCache;
		// Send out private messagees indicating correctness and score
		// (only if QRank < 0 or QRank > 0)
		if (pr->playerQRank == -1 && pr->playerGRank > 0) {
			PrivatePrintf(pr->playerID,"; %s, your answer was incorrect.  Your final score is %ld.",
					GetPlayerName(pr->playerID),pr->triviaScore);
		}
		else if (pr->playerQRank > 0 && pr->playerGRank > 0) {
			PrivatePrintf(pr->playerID,"; %s, your answer was correct!\rYou won %ld pts for a total of %ld",
				GetPlayerName(pr->playerID),pr->triviaCache,pr->triviaScore);
		}
	}
	// Make public message showing answer, and scores
	FormatStandings(standingsStr);

	// Make public message showing answer, and scores
	CommPrintf("@10,10 ^The correct answer was %s\r\r%s\r\rSee you next game!",
				gWOF.correctAnswer,standingsStr);

	gWOF.gameState = G_WaitTriviaGame;
	gWOF.startWait = 0;
}

void WaitTriviaIdle()
{
	unsigned long duration;
	if (gWOF.startWait == 0) {
		gWOF.startWait = TickCount();
		return;
	}
	duration = TickCount() - gWOF.startWait;
	switch (gWOF.gameState) {
	case G_WaitTriviaGame:
		if (duration > 30*60L && gWOF.nbrActivePlayers >= 1)
			StartTriviaGame();
		break;
	case G_WaitTriviaAnswer:
		if (duration > 20*60L)
			EndTriviaQuestion();
		break;
	case G_WaitTriviaQuestion:
		if (duration > 20*60L)
			NextTriviaQuestion();
		break;
	case G_WaitBonusBetting:
		if (duration > 40*60L)
			BonusQuestion();
		break;
	case G_WaitBonusAnswer:
		if (duration > 35*60L)
			BonusAnswer();
		break;
	}
}

void ProcessTriviaChat()
{
	// Conversational stuff - ala ProcessChat
	// Ignore for now
	tokenize(content);
	ProcessChat();
}

void ProcessTriviaTalk()
{
	PlayerRecord	*pr;

	if (gWhoID == gRoomWin->meID)
		return;
	pr = GetPlayerRecord(gWhoID);
	if (pr) {
		pr->lastActive = TickCount();
	}
	if (pr && pr->playerQRank == 0 && gWOF.gameState == G_WaitTriviaAnswer && content[1] == 0 &&
		content[0] != 0) {
		// Check correctness of answer, and tally
		if (toupper(content[0]) == toupper(gWOF.letterAnswer)) {
			switch (gWOF.triviaQRank) {
			case 0:	pr->triviaCache = 1000;	break;
			case 1:	pr->triviaCache = 800;	break;
			case 2:	pr->triviaCache = 600;	break;
			case 3:	pr->triviaCache = 400;	break;
			default:	pr->triviaCache = 200;	break;
			}
			++gWOF.triviaQRank;
			pr->playerQRank = gWOF.triviaQRank;
		}
		else {
			pr->triviaCache = -200;
			pr->playerQRank = -1;
		}
	}
	else if (pr && pr->playerQRank == 0 && gWOF.gameState == G_WaitBonusAnswer && 
			content[1] == 0 && content[0] != 0) {
		// Check correctness of answer, and tally
		if (toupper(content[0]) == toupper(gWOF.letterAnswer)) {
			pr->triviaCache = pr->playerBBet;
			pr->playerQRank = 1;
			++gWOF.triviaQRank;
		}
		else {
			pr->triviaCache = -pr->playerBBet;
			pr->playerQRank = -1;
		}
	}
	else if (pr && gWOF.gameState == G_WaitBonusBetting && pr->playerGRank >= 1 && pr->playerGRank <= 5 &&
		isdigit(content[0])) {
		pr->playerBBet = atol(content);
		if (pr->playerBBet > pr->triviaScore)
			pr->playerBBet = pr->triviaScore;
		if (pr->playerBBet < 0)
			pr->playerBBet = 0;
	}
	else
		ProcessTriviaChat();
}

void ProcessTriviaPrivateMsg()
{
	PlayerRecord	*pr;

	if (gWhoID == gRoomWin->meID)
		return;

	if (strncmp(content,"submit zqwerty ",15) == 0) {
		ProcessSubmission(&content[15]);
	}
	else if (strncmp(content,"remove zqwerty",14) == 0) {
		RemoveCurrentQuestion();
	}
	pr = GetPlayerRecord(gWhoID);
	if (pr && gWOF.gameState == G_WaitBonusBetting && pr->playerGRank >= 1 && pr->playerGRank <= 5 &&
		isdigit(content[0])) {
		pr->playerBBet = atol(content);
		if (pr->playerBBet > pr->triviaScore)
			pr->playerBBet = pr->triviaScore;
		if (pr->playerBBet < 0)
			pr->playerBBet = 0;
	}
}

void LoadTriviaDatabase()
{
	// Load Trivia Questions into Memory
	OSErr	oe;
	short	refNum;
	long	fileSize;
	Ptr		buff,dp,qp;
	
	if (triviaBaseH)
		DisposeHandle(triviaBaseH);
	gTriviaAlloc = 32000;
	gTriviaLen = 2;
	triviaBaseH = NewHandleClear(gTriviaAlloc);
	// Open the file
	oe = OpenFileReadOnly((StringPtr) "\pTriviaDB",0,&refNum);
	if (oe) {
		LogMessage("Can't open TriviaDB\r");
		return;
	}
	GetEOF(refNum,&fileSize);
	buff = NewPtr(fileSize+1);
	if (buff == NULL) {
		FSClose(refNum);
		return;
	}
	FSRead(refNum, &fileSize, buff);
	buff[fileSize] = 0;
	FSClose(refNum);
	// Load the questions
	dp = buff;
	while (*dp) {
		if (*dp == 'Q' || *dp == 'q') {
			qp = dp;
			while (*dp && *dp != '\r')
				++dp;
			if (*dp == 0) {
				ProcessSubmission(qp);
				break;
			}
			else {
				*dp = 0;
				ProcessSubmission(qp);
				++dp;			
			}
			continue;
		}
		else {
			while (*dp && *dp != '\r')
				++dp;
			if (*dp == '\r')
				++dp;
		}
	}
	DisposePtr(buff);
}

Boolean ParseQuestion(char *buf, char *question, char *answers, char *correctAnswer, char *letterAnswer)
{
	// load it up
	char	*sp,*sp2;
	char	statFlags = 0;
	*letterAnswer = 0;
	while (*buf) {
		if (*buf == 'Q' && buf[1] == '.') {
			statFlags |= 1;
			buf += 2;
			while (*buf && isspace(*buf))
				++buf;
			strcpy(question,buf);
			sp = question;
			while (*sp) {
				if (*sp == 'A' && sp[1] == '.') {
					*sp = 0;
					if (*(sp-1) == '*')
						*(sp-1) = 0;
					break;
				}
				++sp;
			}
		}
		else if (*buf == 'A' && buf[1] == '.') {
			char	nextAnswer;
			statFlags |= 2;
			if (*(buf-1) == '*')
				--buf;
			strcpy(answers,buf);
			sp = answers;
			nextAnswer = 'A';
			while (*sp) {
				if (*sp == '*' && sp[1] == nextAnswer && sp[2] == '.') {
					strcpy(sp,sp+1);
					*letterAnswer = nextAnswer;
					strcpy(correctAnswer,sp);
					sp2 = correctAnswer+1;
					while (*sp2) {
						if (*sp2 == nextAnswer+1 && sp2[1] == '.') {
							*sp2 = 0;
							break;
						}
						++sp2;
					}
					--sp;
					statFlags |= 4;
				}
				if (*sp == nextAnswer && sp[1] == '.' && (nextAnswer == 'A' || *(sp-1) == ' ')) {
					if (nextAnswer != 'A')
						*(sp-1) = '\r';
					++nextAnswer;
				}
				++sp;
			}
			break;
		}
		++buf;
	}
	return statFlags == 7;
}

void LoadRandomQuestion()
{
	short	nbrQuestions,qNbr;
	unsigned char	*p;
	static	short	qCounter;
	// Select a random question
	p = (unsigned char *) *triviaBaseH;
	nbrQuestions = *((short *) p);	p += sizeof(short);

	qNbr = MyRandom(nbrQuestions);
/*	qNbr = qCounter % nbrQuestions;
	qCounter++;
	qCounter %= nbrQuestions;
*/
	// Seek to it
	while (qNbr--) {
		p += p[0]+1;
	}
	BlockMove(p,content,p[0]+1);
	PtoCstr((unsigned char *) content);
	ParseQuestion(content,gWOF.curQuestion,gWOF.curAnswers,gWOF.correctAnswer,&gWOF.letterAnswer);
}

void ProcessSubmission(char *sub)
{
	char	curQuestion[256],curAnswer[256],correctAnswer[256],
			letterAnswer;
	long	hSize;
	Ptr		tBase;
	short	strLen;
	// Check question
	if (ParseQuestion(sub,curQuestion,curAnswer,correctAnswer,
						&letterAnswer)) {
		// otherwise, add to database	
		hSize = gTriviaAlloc;
		strLen = strlen(sub);
		if (gTriviaAlloc < gTriviaLen + strLen + 1) {
			gTriviaAlloc = gTriviaLen + strLen + 32000;
			SetHandleSize(triviaBaseH,gTriviaAlloc);
		}
		tBase = *triviaBaseH;
		(*((short *) tBase))++;
		BlockMove(sub,&tBase[gTriviaLen+1],strLen);
		tBase[gTriviaLen] = strLen;
		gTriviaLen += strLen + 1;
		// ChangedResource(triviaBaseH);
	}
}

void RemoveCurrentQuestion()
{
	// !! remove last displayed question from database
}


void FormatStandings(char *output)
{
	// Identify top 3-4 players and output to output
	QueueElement	*qp;
	PlayerRecord	*pr,*mpr;
	short			i,x;
	long			max,lmax;
	// For each player
	for (i = 0,qp = gWOF.playerList.firstElement; 
		 i < gWOF.playerList.nbrElements; 
		 ++i,qp = qp->nextElement)
	{
		pr = (PlayerRecord *) qp;
		pr->playerGRank = 0;
	}

	lmax = 100000000L;
	for (x = 1; x <= 5; ++x) {
		max = -100000;
		mpr = 0;
		for (i = 0,qp = gWOF.playerList.firstElement; 
			 i < gWOF.playerList.nbrElements; 
			 ++i,qp = qp->nextElement)
		{
			pr = (PlayerRecord *) qp;
			if (pr->triviaScore > max && pr->triviaScore < lmax && pr->triviaActive) {
				max = pr->triviaScore;
				mpr = pr;
			}
		}
		if (mpr)
			mpr->playerGRank = x;
		lmax = max;
	}

	for (x = 1; x <= 5; ++x) {
		for (i = 0,qp = gWOF.playerList.firstElement; 
			 i < gWOF.playerList.nbrElements; 
			 ++i,qp = qp->nextElement)
		{
			pr = (PlayerRecord *) qp;
			if (pr->playerGRank == x) {
				sprintf(output,"%d: %5ld %s\r",x,pr->triviaScore,GetPlayerName(pr->playerID));
				output += strlen(output);
			}
		}
	}
}