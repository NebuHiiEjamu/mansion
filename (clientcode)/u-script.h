/******************************************************************************
 ScriptMgr.h

	The Script Manager.

	Author - Jim Bumgardner
	©1994 Warner New Media
 ******************************************************************************/

#ifndef __ScriptMgr__
#define __ScriptMgr__

#include "MANSION.H"

#define MaxStack			256		// increased from 64, 2/29/96
#define MaxVars				64
#define MaxTBuf				4096
#define VF_Global			1
#define VF_EGlobal			2		// 6/6/95
#define MaxScriptNesting	16
#define MaxArrayElements	256

#define OP_LESSEQUAL	0xB2
#define OP_GTREQUAL		0xB3
#define OP_INEQUALITY	0xAD

enum ParseModes {
	P_GetAtom,			// Normal Parsing
	P_CollectString,	// Collect Proc between {} for Pushing
	P_NumberParseModes
};

enum AtomTypes {	
	V_Error = -1, 		// Used when there is a parsing error
	V_None, 			// Probably unused, use to detect errors
	V_LongLiteral,		// Literal Long Value value = value
	V_Symbol,			// Symbol 			value = address in string table
	V_AtomList,			// Bracketed Code	value = address in string table
	V_StringLiteral,	// String Literal	value = address in string table
	V_ArrayMark,		// Marker to beginning of array
	V_Array,			// Array			value = address in string table of array
	V_NbrAtomTypes
};

enum GlobalStrings {
	GS_ChatString,
	GS_NbrGlobalStrings
};

typedef struct {
	long	value;
	short	type;
} PStack, *PStackPtr;

typedef struct {
	long	nbrItems;
	PStack	firstItem[1];
} ArrayRec, *ArrayPtr;

typedef struct {
	char	name[32];
	long	value;
	short	type;
	short	flag;	// VF_Global
} Variable, *VariablePtr;

typedef struct GlobalVar {
	struct GlobalVar **nextVar;
	Variable	v;
	char		data[1];
} GlobalVar, *GlobalVarPtr, **GlobalVarHandle;

enum {A_OK, A_Break, A_Return, A_Exit, A_UserBreak, A_Abort};	// Levels of script abort

#define SCHAR(offset)	gPV.strList[gPV.so + offset]
#define SC				gPV.strList[gPV.so]
#define INC_SC			++gPV.so
#define SCHAR_INC		gPV.strList[gPV.so++]

typedef struct {
	// Nested
	long		tsp[MaxScriptNesting];	// Nested script Ptrs
	long		so;						// script Offset

	// Local (cleared after script is done executing)
	PStack		stack[MaxStack];	// Stack
	short		sLevel;				// Stack Pointer
	Variable	vList[MaxVars];		// Local Variable Array
	GlobalVarHandle	gList;			// Global Variable List
	short		nbrVars;			// Number Local Vars
	short		nLevel;				// Script Nesting Level {}

									// 5/16/96 Changed strListLen to a LONG
	LONG		strListLen;			// length of string table
	short		pMode;				// Parse Mode

	// Global
	Ptr			tBuf;				// Temp Buffer
	Ptr			strList;
									// 5/16/96 Changed allocStrList to LONG
	LONG		allocStrList;		// allocated space for string table
	short		abortScript;
	Boolean		scriptRunning;
	HotspotPtr	curSpot;
} ParseVars;

extern ParseVars gPV;
extern char	gGlobalStrings[GS_NbrGlobalStrings][256];
extern long	gWhoChat;

// u-Script.c
GlobalVarHandle NewGlobal         (VariablePtr v);
Variable       *NewVariable       (char *sym);
Variable       *GetVariable       (char *sym);
void            RetrieveGlobal    (VariablePtr v);
void            UpdateGlobal      (VariablePtr v);
void            GlobalizeVariable (VariablePtr v);
void            AssignVariable    (VariablePtr v, short type, long value);
void 			RetrieveExternStringGlobal(VariablePtr v, short nbr);
void 			UpdateExternStringGlobal(VariablePtr v);
void            InitScriptMgr     (void);
void 						CloseScriptMgr		(void);
long            DoHotspotScript   (char *sBuf, HotspotPtr ab);
long            DoUserScript      (char *sBuf);
void            InitInterpreter   (void);
void            AtomToValue       (short *type, long *value);
void            PushAtom          (short type, long value);
short           PopAtom           (long *value);
void            ParseNumber       (void);
long            AddToStringTable  (char *str);
Boolean	        FindInStringTable (char *str, long *offset);
void            ParseAtomList     (void);
void            ParseStringLiteral(void);
void            ParseSymbol       (void);
void            BinaryOp          (unsigned char opType,short type1, long v1,short type2, long v2);
void            UnaryOp           (unsigned char opType, short type1, long v1);
void            UnaryAssignmentOp (unsigned char opType, short type1, long v1);
void            BinaryAssignmentOp (unsigned char opType, short type1, long v1,short type2, long v2);
long            RunMessageScript  (char *msg);
void            RunScript         (void);
void            CallSubroutine    (long offset);
void            GetExternStringGlobal(char *string, short nbr);
void            SetExternStringGlobal(char *string, short nbr);
long 			Substr(long frag, long whole);
long 			grepstr(long pattern, long string);
long 			grepsub(long replace);

void 			SetScriptAlarm(char *script, long futureTime);
void 			LoadRoomScripts(void);
void 			AddDefaultEventHandlers(HotspotPtr  hp);
void			AddEventHandler(HotspotPtr hp, short type, char *str,...);
void			ParseSpotEventHandler(HotspotPtr hsl);
short 			GetEventType(char *token);

void			ClearAlarms(void);

void			ClearUserScript(void);
long			AllocateArray(short nbrItems, PStackPtr ps);
long			PopArray(void);
long			AddToDataTable(Ptr str, long len);
void			AbortScript(char *msg);	// 4/16/96 helpful script error function

extern void (*SFuncArray[])(void);

#endif
