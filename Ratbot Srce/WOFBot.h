// JBot.h

#define DEBUG	0
#include "PPA.h"
#include "PPAPalace.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#define MaxTokens	32
#define TokLength	32
#define GrandPrizeSTRs	202

#define CarrotPrefix	"@10,10"

#define PatName1	"rat"
#define PatName2	"spud"
#define PatBotName	"ratbot"
#define VannaName1	"bananna"
#define VannaName2	"banana"

// Palace Defs from Mansion.h
extern PPARecord	*gCtl;

extern char tokens[MaxTokens][TokLength];

extern Boolean	gCarrot;

enum WOFEmotes  {E_Smile, E_Sad, E_Mad, E_Normal, E_Question, E_Maniacal};

enum WOFStates {G_Inactive, G_Idle, G_WaitNextGame, G_WaitSpin, G_WaitVowel, G_WaitSolve, 
				G_WaitCons, G_WaitFreeSpin, G_WaitFinal1, G_WaitFinal2
				};

enum WOFEvents {E_None, E_SpudTalk, E_GenTalk, E_PrivateMsg, E_PlayerLeft, E_PlayerSignoff, 
				E_PlayerJoined, E_UserList, E_NewNick, E_ChanNames};

enum WheelContents {W_Money, W_FreeSpin, W_Bankrupt, W_LoseTurn, W_Prize};

typedef struct WheelList {
	short	type;
	long	value;
} WheelList;

typedef struct QueueElement {
	struct QueueElement *nextElement;
} QueueElement;

typedef struct {
	short			nbrElements;
	QueueElement	*firstElement;
} QueueHeader;

typedef struct {
	QueueElement	qHdr;
	PersonID	playerID;
	char	nick[32];
	char	cutenick[32];
	Boolean	present,playing;
	long	lastActive,lastPresent;
	Boolean	newbie;		// Counter - initially set to 4 when player joins
	short	freeSpins;	// FreeSpins init to 0 at start of game
	long	prizeFlags;	// Flags init to 0 at start of game
	short	nbrPrizes;
	long	money;		// Current Money, init to 0 at start
	long	gameWinnings;
	long	permMoney;	// Permanent Money + Prize Winnings
	short	cussCount;
	Boolean	inUse;
	Boolean	heapRecord;
	// Trivia Stuff
//	long	triviaScore;
//	long	triviaCache;
//	long	playerGRank;
//	long	playerQRank;
//	long	playerBBet;
//	long	triviaActive;
} PlayerRecord;

typedef struct {
	short		gameState;
	short		roundNbr,turnNbr;
	QueueHeader	playerList;
	PlayerRecord	*curPlayer;
	short		nbrActivePlayers;
	short		nbrPrizes;
	char		prizeName[32][128];
	long		prizeValue[32];
	char		grandPrizeName[128];
	long		grandPrizeValue;
	short		curPlayerIndex;
	long		startWait;
	Boolean		timeWarning;
	Boolean		noCons;
	char		clue[64];
	char		puzzle[128];
	char		solved[128];
	short		puzLength;
	short		nbrSolved,nbrLetters;
	long		letterFlags,solvedFlags;
	long		catFlags;
	short		letterMode;
	long		letterValue;	// Next Letter is worth this (from spin)
	long		lastNamesRequest;
	// Trivia Game Vars
	short		triviaQuestionNumber;
	short		triviaQRank;
	char		curQuestion[256];
	char		curAnswers[256];
	char		correctAnswer[256];
	char		letterAnswer;	// single letter
} WOFVars;

#define VowelMask		0x00104111L

extern WOFVars	gWOF;

void StatusPrintf(short n, char *str, ...);
void CommPrintf(char *str,...);
void CommString(char *str);
void tokenize(char *buff);
void OpenWOFPort();
void BuyVowel(short vowel);
void GuessCons(short cons);
void BeginWaiting(short state);
void ProcessGlobal(char *cmd, PersonID whoID, Boolean tellFlag);
void ShowPuzzle(short which,char *str,...);
void IntegrateScore(long score, char *name);
void DisplayTopScores(PersonID who);
void EndGame(Boolean wonFlag);
char *RandNick(PlayerRecord *player);

PlayerRecord *GetPlayerRecord(PersonID playerID);
PlayerRecord *GetPlayerRecordByName(char *str);
PlayerRecord *NewPlayer(PersonID playerID);

short MyRandom(short max);

void InitWOF();
void LoadTopScores();
void LoadAutoOps();
void EndWOF();
Boolean RightPerson(PersonID who);
Boolean PuzzleMatch(char *str1, char *str2) ;
void ProcessSpudTalk();
void SpinWheel();
void NextPlayer();
void ProcessBotMacro(char *str);
void FinishRound();
void ProcessChat();
void Eliza(char *str, char *reply);
void RespPrintf(char *temp,...);
void CountActivePlayers();
void ProcessPrivateMsg();
void ProcessPlayerLeft(short how);
void ClearPlayers();
void ProcessPlayerJoined();
void CheckAutoOps(char *name);
void ProcessBotUserList(short n, UserRecPtr ary);
void BeginGame();
void WaitIdle();
void CheckResponses();
void CarrotPrintf(char *temp,...);
void ResetPlayers();

// Queue Stuff
void RemoveFromQueue(QueueHeader *qh, short nbr);
void AddToQueue(QueueHeader *qh, QueueElement *pr);
void CheckCuteNick(PlayerRecord *pr);
char *GetPlayerName(PersonID userID);
void DiscardVacantPlayers();

// GamePlay stuff
void BeginRound();
void SelectPuzzle();
int SelectPrize(Boolean grandFlag);
Boolean FreeSpinCheck();
char *ShowPlayerMoney(PlayerRecord *pr,Boolean concatFlag);
void SelectNextPlayer();
short SolveLetter(short letter);
void SpinWheel();
void NextPlayer();
void FinalRound();

void 	ToggleBot(Boolean onFlag);	// Used from Menu
void 	InitializeBot();
void	CleanupBot();
Boolean NetworkEntryPoint(long eventType, long refCon, long length, char *buffer);
void ProcessBotChat(LONG whoChat, char *str);
void IdleBot();
UserRecPtr	GetPersonByIndex(LONG idx);
UserRecPtr	GetPersonByName(char *name);
UserRecPtr	GetPersonByID(LONG id);


// Comm
void PrivatePrintf(PersonID whoID, char *str,...);
void CommCache(char *str,...);

// Topscores
void AddCuteNick(char *orig, char *alias);
void RegisterCuteNick(PlayerRecord *pr, char *newNick);
void EmoteBot(short emotion);
Boolean BotFloodControl(short maxPerSec);

// Trivia Game
void ProcessTriviaPrivateMsg();
void ProcessTriviaTalk();
void ProcessTriviaChat();
void WaitTriviaIdle();
void BonusAnswer();
void BonusQuestion();
void StartBonusBetting();
void EndTriviaQuestion();
void NextTriviaQuestion();
void StartTriviaGame();
void EnableTriviaGame();
void ProcessSubmission(char *sub);
void RemoveCurrentQuestion();
void LoadTriviaDatabase();
void LoadRandomQuestion();
void FormatStandings(char *output);
Boolean ParseQuestion(char *buf, char *question, char *answers, char *correctAnswer,
					char *letterAnswer);


