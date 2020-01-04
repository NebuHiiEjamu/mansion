// Script Pre Parser.c
//
// Todo: Stop using strlen...LONGs and other things will cause nulls to appear
//
// reverse meaning of assigment and DEF operators
//
// reverse WHILE, IF, IFELSE
//
// Bugs: Can't parse complex expressions (probably need ; and ()  )(
// bugs: can't parse WHILE statement properly (doesn't know that initial atom list
//		is supposed to leave something on the stack
//
#if 0

#include "U-USER.H"
#include "U-SCRIPT.H"

typedef struct {
	char	*resWord;
	short	type;
	short	cmd;
	short	nbrArgs;
} ResWordTable;


typedef struct {
	char			*gSP;
	char			gToken[2048];
	Boolean			ungetFlag;
	Ptr				scriptBuf,varBuf;
	long			eventMask;
	short			nbrScripts;
	long			lenVars;
	EventHandlerRec	spotEvents[32];
} ScriptParser, *ScriptParserPtr;


extern ScriptParserPtr		gSPPtr;


// data types
enum	{	DT_SYMBOL=1,	DT_LONGLITERAL, DT_STRINGLITERAL, DT_OPERATOR, DT_COMMAND, DT_FUNCTION, DT_ATOMLIST,
			DT_SUBEXPRESSION};

// Operator, Command and Function Types
enum	{	
			OT_EQUALITY=1, OT_ADDITION, OT_SUBTRACTION, OT_MULTIPLICATION,
			OT_DIVISION, OT_LESSEQUAL, OT_GREATEREQUAL, OT_LESS, OT_GREATER,
			OT_STRINGCONCAT, OT_NOT, OT_NOTEQUAL, OT_ASSIGNMENT,
			
			CT_EXEC,	CT_ALARMEXEC, CT_IF, CT_IFELSE, CT_WHILE, CT_RETURN,
			CT_DEF, CT_GLOBAL, CT_LAUNCHAPP, CT_SHELLCMD, CT_GOTOROOM,
			CT_LOCK, CT_SETPICLOC, CT_SETLOC, CT_SETSPOTSTATE,	CT_SETSPOTSTATELOCAL,
			CT_SETALARM, CT_GETSPOTSTATE, CT_ADDLOOSEPROP, CT_DOFFPROP, CT_DONPROP, 
			CT_REMOVEPROP, CT_NAKED, CT_SETCOLOR, CT_SETFACE, CT_UNLOCK, CT_GLOBALMSG, 
			CT_ROOMMSG, CT_SUSRMSG, CT_LOCALMSG, CT_LOGMSG, CT_SAY, CT_DELAY, CT_KILLUSER, 
			CT_GOTOURL, CT_MACRO, CT_MOVE, CT_SETPOS, 
// Functions	
			FT_ISLOCKED, FT_RANDOM, FT_SUBSTR, FT_GREPSTR, FT_GREPSUB, 
			FT_DEST, FT_ME, FT_INSPOT, FT_ITOA,

			CT_NbrCommands};

// Special Treatment:
// # ; comments
// {   } brackets delimit atom lists
// 0-9 or -0=9  numbers
// " string literals
//
// Symbols begin with letter and contain letters, numbers and/or underscore
//
ResWordTable	gResWordTable[] =
{
	"==",			DT_OPERATOR,	OT_EQUALITY,			2,
	"+",			DT_OPERATOR,	OT_ADDITION,			2,
	"-",			DT_OPERATOR,	OT_SUBTRACTION,			2,
	"*",			DT_OPERATOR,	OT_MULTIPLICATION,		2,
	"/",			DT_OPERATOR,	OT_DIVISION,			2,
	"<=",			DT_OPERATOR,	OT_LESSEQUAL,			2,
	">=",			DT_OPERATOR,	OT_GREATEREQUAL,		2,
	"<",			DT_OPERATOR,	OT_LESS,				2,
	">",			DT_OPERATOR,	OT_GREATER,				2,
	"&",			DT_OPERATOR,	OT_STRINGCONCAT,		2,
	"!",			DT_OPERATOR,	OT_NOT,					1,
	"NOT",			DT_OPERATOR,	OT_NOT,					1,
	"<>",			DT_OPERATOR,	OT_NOTEQUAL,			2,
	"!=",			DT_OPERATOR,	OT_NOTEQUAL,			2,
	"=",			DT_OPERATOR,	OT_ASSIGNMENT,			2,
	// Generic commands
	"DEF",			DT_COMMAND,		CT_DEF,					2,
	"GLOBAL",		DT_COMMAND,		CT_GLOBAL,				1,
	"EXEC",			DT_COMMAND,		CT_EXEC,				1,
	"ALARMEXEC",	DT_COMMAND,		CT_ALARMEXEC,			2,
	"IF",			DT_COMMAND,		CT_IF,					2,
	"IFELSE",		DT_COMMAND,		CT_IFELSE,				3,
	"WHILE",		DT_COMMAND,		CT_WHILE,				2,
	"LAUNCHAPP",	DT_COMMAND,		CT_LAUNCHAPP,			1,
	"SHELLCMD",		DT_COMMAND,		CT_SHELLCMD,			1,

	// Palace specific commands
	"GOTOROOM",		DT_COMMAND,		CT_GOTOROOM,			1,
	"LOCK",			DT_COMMAND,		CT_LOCK,				1,
	"SETPICLOC",	DT_COMMAND,		CT_SETPICLOC,			3,
	"SETLOC",		DT_COMMAND,		CT_SETLOC,				3,
	"SETSPOTSTATE",	DT_COMMAND,		CT_SETSPOTSTATE,		2,
	"SETSPOTSTATELOCAL", DT_COMMAND, CT_SETSPOTSTATELOCAL, 	2,
	"SETALARM",		DT_COMMAND,		CT_SETALARM,			2,
	"GETSPOTSTATE",	DT_COMMAND,		CT_GETSPOTSTATE,		1,
	"ADDLOOSEPROP",	DT_COMMAND,		CT_ADDLOOSEPROP,		3,
	"DOFFPROP",		DT_COMMAND,		CT_DOFFPROP,			0,
	"DONPROP",		DT_COMMAND,		CT_DONPROP,				1,
	"REMOVEPROP",	DT_COMMAND,		CT_REMOVEPROP,			1,
	"CLEARPROPS",	DT_COMMAND,		CT_NAKED,				0,
	"NAKED",		DT_COMMAND,		CT_NAKED,				0,
	"SETCOLOR",		DT_COMMAND,		CT_SETCOLOR,			1,
	"SETFACE",		DT_COMMAND,		CT_SETFACE,				1,
	"UNLOCK",		DT_COMMAND,		CT_UNLOCK,				1,
	"GLOBALMSG",	DT_COMMAND,		CT_GLOBALMSG,			1,
	"ROOMMSG",		DT_COMMAND,		CT_ROOMMSG,				1,
	"SUSRMSG",		DT_COMMAND,		CT_SUSRMSG,				1,
	"LOCALMSG",		DT_COMMAND,		CT_LOCALMSG,			1,
	"LOGMSG",		DT_COMMAND,		CT_LOGMSG,				1,
	"SAY",			DT_COMMAND,		CT_SAY,					1,
	"CHAT",			DT_COMMAND,		CT_SAY,					1,
	"DELAY",		DT_COMMAND,		CT_DELAY,				1,
	"KILLUSER",		DT_COMMAND,		CT_KILLUSER,			1,
	"GOTOURL",		DT_COMMAND,		CT_GOTOURL,				1,
	"NETGOTO",		DT_COMMAND,		CT_GOTOURL,				1,
	"MACRO",		DT_COMMAND,		CT_MACRO,				1,
	"MOVE",			DT_COMMAND,		CT_MOVE,				2,
	"SETPOS",		DT_COMMAND,		CT_SETPOS,				2,
		
	// FUNCTIONS
	"RANDOM",		DT_FUNCTION,	FT_RANDOM,				1,
	"ISLOCKED",		DT_FUNCTION,	FT_ISLOCKED,			1,
	"SUBSTR",		DT_FUNCTION,	FT_SUBSTR,				2,
	"GREPSTR",		DT_FUNCTION,	FT_GREPSTR,				2,
	"GREPSUB",		DT_FUNCTION,	FT_GREPSUB,				1,
	"DEST",			DT_FUNCTION,	FT_DEST,				0,
	"ME",			DT_FUNCTION,	FT_ME,					0,
	"INSPOT",		DT_FUNCTION,	FT_INSPOT,				1,
	"ITOA",			DT_FUNCTION,	FT_ITOA,				1,
	NULL,0,0,0,
};

typedef struct {
	short	type;
	short	nbrArgs;
	short	argCtr;
	short	command;
} CommandStack;

typedef struct {
	short	type;
	char	*begPtr;
} ParseStack;

CommandStack	cStack[32];
ParseStack		pStack[32];
short			cStackCtr;
short			pStackCtr;

void PushCommandStack(short type, short nbrArgs, short command);
char *PushParseStack(short type, char *dp);
char *PopParseStack(char *dp);
char *IncArgCounter(char *dp);
void ParseScript(char *inStr, char *outStr, long *len);
Boolean GetScriptToken();

void PushCommandStack(short type, short nbrArgs, short command)
{
	cStack[cStackCtr].type = type;
	cStack[cStackCtr].nbrArgs = nbrArgs;
	cStack[cStackCtr].command = command;
	cStack[cStackCtr].argCtr = 0;
	++cStackCtr;
}

char *PushParseStack(short type, char *dp)
{
	pStack[pStackCtr].type = type;
	pStack[pStackCtr].begPtr = dp;
	if (type == DT_ATOMLIST)
		dp += 4;
	++pStackCtr;
	return dp;
}

char *PopParseStack(char *dp)
{
	char	*sp;

	pStackCtr--;
	// !!! Concat atom list in...
	if (pStack[pStackCtr].type == DT_ATOMLIST) {
		sp = pStack[pStackCtr].begPtr;
		*((short *) sp) = DT_ATOMLIST;	sp += 2;
		*((short *) sp) = ((long) dp - (long) sp) - 2;
	}
	return sp;
}

char *IncArgCounter(char *dp)
{
	if (cStackCtr == 0)	// argument without command - probably prefix arg for operator
		return;
	cStack[cStackCtr-1].argCtr++;
	if (cStack[cStackCtr-1].argCtr >= cStack[cStackCtr-1].nbrArgs)
	{
		--cStackCtr;
		*((short *) dp) = cStack[cStackCtr].type;		dp += 2;
		*((short *) dp) = cStack[cStackCtr].command;	dp += 2;
		if (cStack[cStackCtr].type == DT_OPERATOR || cStack[cStackCtr].type == DT_FUNCTION)
			dp = IncArgCounter(dp);
	}
	return dp;
}

void ParseScript(char *inStr, char *outStr, long *len)
{
	char	*dp;
	short	i;
	Boolean	gotOne;

	cStackCtr = 0;
	// PushCommandStack(1,CT_EXEC);
	dp = outStr;
	GetScriptToken();	// Skip initial opening {
	
	while (GetScriptToken()) {
		switch (gSPPtr->gToken[0]) {
		case '{':
			dp = PushParseStack(DT_ATOMLIST, dp);
			break;
		case '}':
			if (pStackCtr) {
				dp = PopParseStack(dp);
				dp = IncArgCounter(dp);
			}
			else
				goto DoneParse;
			break;
		case '(':
			dp = PushParseStack(DT_SUBEXPRESSION, dp);
			break;
		case ')':
			*dp = 0;
			dp = PopParseStack(dp);
			if (pStackCtr == 0)
				goto DoneParse;
			break;
		case '\"':
			// Parse String Literal
			{
				short	len = strlen(gSPPtr->gToken);
				*((short *) dp) = DT_STRINGLITERAL;			dp += 2;
				*((short *) dp) = strlen(gSPPtr->gToken);	dp += 2;
				BlockMove(gSPPtr->gToken, dp, len);
				dp += len;
				if (len & 1)
					++dp;
				dp = IncArgCounter(dp);
			}
			break;
		default:
			if ((gSPPtr->gToken[0] == '-' && isdigit(gSPPtr->gToken[1]))  || isdigit(gSPPtr->gToken[0])) {
				*((short *) dp) = DT_LONGLITERAL;			dp += 2;
				*((long *) dp) = atol(gSPPtr->gToken);		dp += 4;
				dp = IncArgCounter(dp);
			}
			else {
				gotOne = false;
				for (i = 0; gResWordTable[i].resWord; ++i) {
					if (stricmp(gResWordTable[i].resWord,gSPPtr->gToken) == 0) {
						gotOne = true;
						break;
					}
				}
				if (gotOne) {
					switch (gResWordTable[i].type){
					case DT_OPERATOR:
						PushCommandStack(DT_OPERATOR, gResWordTable[i].nbrArgs-1,gResWordTable[i].cmd);
						break;
					case DT_FUNCTION:
						if (gResWordTable[i].nbrArgs)
							PushCommandStack(DT_FUNCTION, gResWordTable[i].nbrArgs,gResWordTable[i].cmd);
						else {
							*((short *) dp) = DT_FUNCTION;				dp += 2;
							*((short *) dp) = gResWordTable[i].cmd;		dp += 2;
							dp = IncArgCounter(dp);
						}
						break;
					case DT_COMMAND:
						if (gResWordTable[i].nbrArgs)
							PushCommandStack(DT_COMMAND, gResWordTable[i].nbrArgs,gResWordTable[i].cmd);
						else {
							*((short *) dp) = DT_COMMAND;				dp += 2;
							*((short *) dp) = gResWordTable[i].cmd;		dp += 2;
						}
						break;
					}
					break;
				}
				else {
					// Push Unknown Symbol
					*((short *) dp) = DT_SYMBOL;				dp += 2;
					*((char *) dp) = strlen(gSPPtr->gToken);	dp += 2;
					strcpy(dp,gSPPtr->gToken);
					dp += strlen(dp);
					dp += (strlen(dp) & 1)? 0 : 1;
					dp = IncArgCounter(dp);
				}
			}
			break;
		}
	}
DoneParse:
	*((short *) dp) DT_COMMAND;	dp += 2;
	*((short *) dp) CT_RETURN;	dp += 2;
	*dp = 0;
	*len = (long) dp - (long) outStr;
}

// This should deal with comments, numbers, strings correctly
//
Boolean GetScriptToken()
{
	return false;
}

#endif