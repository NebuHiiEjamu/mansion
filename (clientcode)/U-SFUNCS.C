#include "U-USER.H"
#include "m-cmds.H"
#include "U-SCRIPT.H"
#include "U-REGEXP.H"	//  6/2/95 JAB
#include "U-SECURE.H"
#if PPASUPPORT
#include "PPAMgr.h"
#endif

#include <time.h>

#define _U_SFUNC	1

#include "U-SFUNCS.H"

#define P_MatchFail	0x00
#define P_MatchFork	0x80

#include "U-PTABLE.C"

/**
void ParseSymbol()
{
	Ptr		dp,sp;
	char	c;
	short	keyVal;
	dp = gPV.tBuf;
	while ( (SC >= 'a' && SC <= 'z') ||
			(SC >= 'A' && SC <= 'Z') ||
	 		(SC >= '0' && SC <= '9') ||
			SC == '_')	
	{
		c = SCHAR_INC;
		if (islower(c))
			c = toupper(c);
		*(dp++) = c;
	}
	*dp = 0;
	sp = gPV.tBuf;
#include "U-KEYWRD.C"
	if (keyVal == -1) {	// Unknown Word - push as a SYmbol
		long sOffset;
		if (!FindInStringTable(gPV.tBuf,&sOffset))
			sOffset = AddToStringTable(gPV.tBuf);
		PushAtom(V_Symbol, sOffset);
	}
	else {
		(*SFuncArray[keyVal])();
	}
}
**/

void ParseSymbol()
{
	Ptr		dp,sp;
	char	c;
	short	keyVal;
	short	i;
	dp = gPV.tBuf;
	while ( (SC >= 'a' && SC <= 'z') ||
			(SC >= 'A' && SC <= 'Z') ||
	 		(SC >= '0' && SC <= '9') ||
			SC == '_')	
	{
		c = SCHAR_INC;
		if (islower(c))
			c = toupper(c);
		*(dp++) = c;
	}
	*dp = 0;
	sp = gPV.tBuf;
	keyVal = -1L;
	i = 0;
	while (1) {
		switch (gParseTab[i] & 0x80) {
		case P_MatchFork:
			if (*sp == (gParseTab[i] & 0x7F)) {
				i += 2;
				if (*sp == 0) {
					keyVal = (gParseTab[i] & 0x7F);
					break;
				}
				else
					++sp;
			}
			else
				i += gParseTab[i+1];
			continue;
		case P_MatchFail:
			if (*sp == gParseTab[i]) {
				++i;
				if (*sp == 0) {
					keyVal = (gParseTab[i] & 0x7F);
					break;
				}
				else
					++sp;
				continue;
			}
			break;
		}
		break;
	}

	if (keyVal == -1) {	// Unknown Word - push as a SYmbol
		long sOffset;
		if (!FindInStringTable(gPV.tBuf,&sOffset))
			sOffset = AddToStringTable(gPV.tBuf);
		PushAtom(V_Symbol, sOffset);
	}
	else {
		(*SFuncArray[keyVal])();
	}
}

void SF_EXEC()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_AtomList)
		CallSubroutine(value);
}


void SF_ALARMEXEC()
{	// 7/21/95
	short	type1,type2;
	long	value1,value2;
	if ((type1 = PopAtom(&value1)) == V_Error)
		return;
	AtomToValue(&type1,&value1);
	if ((type2 = PopAtom(&value2)) == V_Error)
		return;
	AtomToValue(&type2,&value2);
	if (type2 == V_AtomList && type1 == V_LongLiteral) {
		SetScriptAlarm(gPV.strList+value2,value1);
	}
}

void SF_IF()
{
	short	sType,eType;
	long	sValue,eValue;
	if ((eType = PopAtom(&eValue)) == V_Error)
		return;
	if ((sType = PopAtom(&sValue)) == V_Error)
		return;
	AtomToValue(&eType,&eValue);
	if (eValue) {
		AtomToValue(&sType,&sValue);
		if (sType == V_AtomList)
			CallSubroutine(sValue);
	}
}

void SF_IFELSE()
{
	short	sType,seType,eType;
	long	sValue,seValue,eValue;
	if ((eType = PopAtom(&eValue)) == V_Error)
		return;
	if ((seType = PopAtom(&seValue)) == V_Error)
		return;
	if ((sType = PopAtom(&sValue)) == V_Error)
		return;
	AtomToValue(&eType,&eValue);
	if (eValue) {
		AtomToValue(&sType,&sValue);
		if (sType == V_AtomList)
			CallSubroutine(sValue);
	}
	else {
		AtomToValue(&seType,&seValue);
		if (seType == V_AtomList)
			CallSubroutine(seValue);
	}
}

void SF_WHILE()
{
	short	sType,eType,rType;
	long	sValue,eValue,rValue;
	if ((eType = PopAtom(&eValue)) == V_Error)	// Pop Expression [atomlist]
		return;
	if ((sType = PopAtom(&sValue)) == V_Error)	// Pop Subroutine [atomlist]
		return;
	AtomToValue(&eType,&eValue);			// Dereference variable, if any
	do {
		if (eType == V_AtomList) {
			CallSubroutine(eValue);	// Execute Expression
			if ((rType = PopAtom(&rValue)) == V_Error)
				return;
			AtomToValue(&rType,&rValue);
		}
		else {
			rType = eType;
			rValue = eValue;
		}
		if (rValue) {
			if (sType == V_AtomList)
				CallSubroutine(sValue);
		}
	} while (rValue && !gPV.abortScript);
	if (gPV.abortScript == A_Break)	// BREAK statement
		gPV.abortScript = A_OK;
}

void SF_GOTOROOM()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral) {
		DoPalaceCommand(PC_GotoRoom, value, NULL);
		gPV.abortScript = A_Exit; // 9/12/95
	}
}

void SF_LOCK()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral) {
		DoPalaceCommand(PC_Lock, value, NULL);
	}
}

void SF_NOT()
{	// 6/27/95
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		value = !value;
	else
		value = 0;
	PushAtom(V_LongLiteral, value);
}

void SF_AND()
{	// 7/27/95
	short	type1,type2;
	long	value1,value2,value;
	if ((type1 = PopAtom(&value1)) == V_Error)
		return;
	AtomToValue(&type1,&value1);
	if ((type2 = PopAtom(&value2)) == V_Error)
		return;
	AtomToValue(&type2,&value2);
	if (type2 == V_LongLiteral && type2 == V_LongLiteral)
		value = value1 && value2;
	else
		value = 0;
	PushAtom(V_LongLiteral, value);
}

void SF_OR()
{	// 7/27/95
	short	type1,type2;
	long	value1,value2,value;
	if ((type1 = PopAtom(&value1)) == V_Error)
		return;
	AtomToValue(&type1,&value1);
	if ((type2 = PopAtom(&value2)) == V_Error)
		return;
	AtomToValue(&type2,&value2);
	if (type2 == V_LongLiteral && type2 == V_LongLiteral)
		value = value1 || value2;
	else
		value = 0;
	PushAtom(V_LongLiteral, value);
}

void SF_SETPICLOC()
{
	short	ary[4];
	short	type;
	long	value;
	ary[0] = gRoomWin->curRoom.roomID;
	ary[1] = ary[2] = 0;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		ary[1] = (short)value;				// hotspot id
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		ary[2] = (short)value;				// y
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		ary[3] = (short)value;				// x
	DoPalaceCommand(PC_SetPicOffset, (long) &ary[0], NULL);
}

void SF_SETLOC()
{
	short	ary[4];
	short	type;
	long	value;
	ary[0] = gRoomWin->curRoom.roomID;
	ary[1] = ary[2] = 0;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		ary[1] = (short)value;				// hotspot id
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		ary[2] = (short)value;				// y
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		ary[3] = (short)value;				// x
	DoPalaceCommand(PC_SetLoc, (long) &ary[0], NULL);
}

void SF_SETSPOTSTATE()
{
	short	ary[3];
	short	type;
	long	value;
	ary[0] = gRoomWin->curRoom.roomID;
	ary[1] = ary[2] = 0;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		ary[1] = (short)value;				// hotspot id
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		ary[2] = (short)value;				// state
	DoPalaceCommand(PC_SetSpotState, (long) &ary[0], NULL);
}

void SF_SETSPOTSTATELOCAL()
{
	short	ary[3];
	short	type;
	long	value;
	ary[0] = gRoomWin->curRoom.roomID;
	ary[1] = ary[2] = 0;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		ary[1] = (short)value;				// hotspot id
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		ary[2] = (short)value;				// state
	DoPalaceCommand(PC_SetSpotStateLocal, (long) &ary[0], NULL);
}

void SF_SETALARM()
{	// 6/28/95
	short	spotID=0;
	long	futureTime=0;
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		spotID = (short)value;				// hotspot id
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		futureTime = (short)value;			// future Time
	if ((gPV.curSpot && spotID > 0) ||
		(gPV.curSpot == NULL && spotID == 0)) {
		SetSpotAlarm(spotID,futureTime);
	}
}

void SF_GETSPOTSTATE()
{	// 6/27/95
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);		// spotID
	if (type == V_LongLiteral)
		value = DoPalaceCommand(PC_GetSpotState, value, NULL);
	else
		value = 0;
	PushAtom(V_LongLiteral, value);
}

void SF_ADDLOOSEPROP()
{	// 6/7/95 modified
	long	ary[3];
	short	type;
	long	value;
	Point	pt;

	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		pt.v = (short)value;
		// ary[2] = value << 16;			// y coord 6/7/95
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		pt.h = (short)value;
		// ary[2] |= value;				// x coord 6/7/95

	ary[2] = *((long *) &pt);

	if ((type = PopAtom(&value)) == V_Error)	// prop id#
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral) {
		// JAB 11/7/96
		ary[0] = (long) value;				// propID
		ary[1] = 0;						// figure out crc#... 6/7/95
		DoPalaceCommand(PC_AddLooseProp, (long) &ary[0], NULL);
	}
	else if (type == V_StringLiteral) {	// 7/6/95
		Handle	h;
		short	len;
		char	*p;
		p = gPV.strList+value;
		len = strlen(p);
		BlockMove(p,&gPV.tBuf[1],len+1);
		gPV.tBuf[0] = (char)len;
		if ((h = GetAssetByName(RT_PROP,(StringPtr) gPV.tBuf)) != NULL)
		{
			long	type,id,crc;
			GetAssetInfo(h,&type,&id,(StringPtr) gPV.tBuf);
			crc = GetAssetCRC(h);
			ary[0] = id;
			ary[1] = crc;
			DoPalaceCommand(PC_AddLooseProp, (long) &ary[0], NULL);
		}			
	}
}

void SF_TOPPROP();
void SF_TOPPROP()
{
	UserRecPtr	cUser;
	long		id;
	cUser = &gRoomWin->mePtr->user;
	if (cUser->nbrProps > 0)
		id = cUser->propSpec[cUser->nbrProps-1].id;
	else
		id = 0;
	PushAtom(V_LongLiteral, id);
}

void SF_DROPPROP();
void SF_DROPPROP()
{	// 6/7/95 modified
	long	x=0,y=0;
	short	type;
	long	value;
	Point	pt;
	long	cmd[3];
	UserRecPtr	cUser;

	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		y = value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		x = value;

	cUser = &gRoomWin->mePtr->user;
	if (cUser->nbrProps > 0) {
		pt.h = (short)x;
		pt.v = (short)y;
		cmd[0] = cUser->propSpec[cUser->nbrProps-1].id;
		cmd[1] = cUser->propSpec[cUser->nbrProps-1].crc;
		cmd[2] = *((long *) &pt);
		ToggleProp(&cUser->propSpec[cUser->nbrProps-1]);
		DoPalaceCommand(PC_AddLooseProp, (long) &cmd[0], NULL);
	}
}

void SF_DOFFPROP()
{	// 7/20/95
	// Clears most recent prop
	UserRecPtr	cUser;
	cUser = &gRoomWin->mePtr->user;
	if (cUser->nbrProps > 0)
		DoffProp(cUser->propSpec[cUser->nbrProps-1].id,
			cUser->propSpec[cUser->nbrProps-1].crc);
}

void SF_DONPROP()
{		// 6/7/95
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		DonProp(value,0L);
	else if (type == V_StringLiteral)
		DonPropByName(gPV.strList+value);
}

void SF_REMOVEPROP()
{		// 6/7/95
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		DoffProp(value,0L);
	else if (type == V_StringLiteral)
		DoffPropByName(gPV.strList+value);
}

void SF_CLEARPROPS()
{
	ClearProps();
}

void SF_CLEARLOOSEPROPS()
{		// 6/7/95
	long	pcmd[1];
	pcmd[0] = -1L;
	DoPalaceCommand(PC_DelLooseProp,(long) &pcmd[0],NULL);
}
	// 4/6/95 JBUM NEW

void SF_SETCOLOR()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	ChangeColor((short)value);
}

	// 4/6/95 JBUM NEW
void SF_SETFACE()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	ChangeFace((short)value);
}

void SF_UNLOCK()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral) {
		DoPalaceCommand(PC_Unlock, value, NULL);
	}
}

void SF_ISLOCKED()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		value = DoPalaceCommand(PC_IsLocked, value, NULL);
	else
		value = 0;
	PushAtom(V_LongLiteral, value);
}

void SF_GLOBALMSG()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_StringLiteral)
		DoPalaceCommand(PC_GlobalMsg, 0, gPV.strList+value);
}

void SF_SAY()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_StringLiteral)
		DoPalaceCommand(PC_Chat, 0, gPV.strList+value);
}

void SF_ROOMMSG()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_StringLiteral)
		DoPalaceCommand(PC_RoomMsg, 0, gPV.strList+value);
}

void SF_SUSRMSG()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_StringLiteral)
		DoPalaceCommand(PC_SusrMsg, 0, gPV.strList+value);
}

void SF_LOCALMSG()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_StringLiteral)
		DoPalaceCommand(PC_LocalMsg, 0, gPV.strList+value);
}

void SF_DELAY()
{
	short	type;
	long	value;
	// Once in Idaho, make this asynchronous...
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral) {
		long	dmy;
		Delay(value*60L,&dmy);
	}
}

void SF_GLOBAL()
{
	long		sOffset;
	Variable	*v;
	if (PopAtom(&sOffset) != V_Symbol)
		return;
	if ((v = GetVariable(gPV.strList+sOffset)) == NULL)
		return;
	GlobalizeVariable(v);
}

void SF_DEF()
{
	// Function Definition
	short		type;
	long		sOffset;
	long		value;
	Variable	*v;
	if ((type = PopAtom(&sOffset)) != V_Symbol)
		return;
	if ((v = GetVariable(gPV.strList+sOffset)) == NULL)
		return;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AssignVariable(v,type,value);
}


void SF_RANDOM()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		value = MyRandom((short) value);
	else {
		type = V_LongLiteral;
		value = 0;
	}
	PushAtom(type,value);
}
	// 4/6/95 JBUM NEW

void SF_SUBSTR()
{
	short	type1,type2;
	long	value1,value2;
	// Once in Idaho, make this asynchronous...
	if ((type1 = PopAtom(&value1)) == V_Error)
		return;
	AtomToValue(&type1,&value1);
	if ((type2 = PopAtom(&value2)) == V_Error)
		return;
	AtomToValue(&type2,&value2);
	if (type1 != V_StringLiteral || type2 != V_StringLiteral)
		PushAtom(V_LongLiteral,0);
	else
		PushAtom(V_LongLiteral,Substr(value1,value2));
}
	// 6/2/95 JBUM NEW

void SF_GREPSTR()
{
	short	type1,type2;
	long	pattern,string;
	// Once in Idaho, make this asynchronous...
	if ((type1 = PopAtom(&pattern)) == V_Error)		// pattern
		return;
	AtomToValue(&type1,&pattern);
	if ((type2 = PopAtom(&string)) == V_Error)		// string
		return;
	AtomToValue(&type2,&string);
	if (type1 != V_StringLiteral || type2 != V_StringLiteral)
		PushAtom(V_LongLiteral,0);
	else
		PushAtom(V_LongLiteral,grepstr(pattern,string));
}
	// 6/2/95 JBUM NEW

void SF_GREPSUB()
{
	short	type1;
	long	replace;
	// Once in Idaho, make this asynchronous...
	if ((type1 = PopAtom(&replace)) == V_Error)		// pattern
		return;
	AtomToValue(&type1,&replace);
	if (type1 != V_StringLiteral)
		PushAtom(V_LongLiteral,0);
	else
		PushAtom(V_StringLiteral,grepsub(replace));
}

void SF_DEST()
{
	PushAtom(V_LongLiteral,(long) gPV.curSpot->dest);
}

void SF_ME() 	// also ID 11/21
{
	long	id = 0;
	if (gPV.curSpot)
		id = gPV.curSpot->id;
	PushAtom(V_LongLiteral,id);
}

void SF_LAUNCHAPP()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_StringLiteral)
		DoPalaceCommand(PC_LaunchApp, 0, gPV.strList+value);
}

void SF_SHELLCMD()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_StringLiteral)
		DoPalaceCommand(PC_LaunchApp, 0, gPV.strList+value);
}

void SF_KILLUSER()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral) {
		if ((gRoomWin->serverInfo.serverPermissions & PM_CyborgsMayKill) > 0)
			DoPalaceCommand(PC_KillUser, value, NULL);
	}
}
	// 6/15/95

void SF_NETGOTO()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_StringLiteral)
		DoPalaceCommand(PC_GotoURL, 0, gPV.strList+value);
}

void SF_MACRO()	// called from SCRIPT (Macro Keys call GenerateUserEvent)
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral) {
		// Play recursively properly   7/21/95
		// 5/9/96 Only low numbered macros can be scripted
		if (value >= 0 && value < MaxScriptMacros &&
			(gRoomWin->userEventMask & (1L << (PE_Macro0 + value))))
			RunMessageScript(gRoomWin->userScriptPtrs[PE_Macro0 + value]);
		else
			PlayMacro((short) value);
	}
}

void SF_MOVE()
{	// 7/10/95
	short	type1,type2;
	long	value1,value2;
	if ((type1 = PopAtom(&value1)) == V_Error)
		return;
	AtomToValue(&type1,&value1);
	if ((type2 = PopAtom(&value2)) == V_Error)		// string
		return;
	AtomToValue(&type2,&value2);
	MoveUser((short)value2,(short)value1);
}

void SF_SETPOS()
{	// 7/10/95
	short	type1,type2;
	long	value1,value2;
	Point	np;
	if ((type1 = PopAtom(&value1)) == V_Error)
		return;
	AtomToValue(&type1,&value1);
	if ((type2 = PopAtom(&value2)) == V_Error)		// string
		return;
	AtomToValue(&type2,&value2);
	np = gRoomWin->mePtr->user.roomPos;
	MoveUser(value2-np.h, value1-np.v);
}

void SF_INSPOT()
{
	short		type;
	long		value,result;
	Point		np;
	HotspotPtr	hs;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	np = gRoomWin->mePtr->user.roomPos;
	hs = GetHotspot((short)value);
	if (hs && gRoomWin->mePtr)
		result = PtInHotspot(np, hs);
	else
		result = 0;
	PushAtom(V_LongLiteral,result);
}

void SF_ITOA()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral) {
		sprintf(gPV.tBuf,"%ld",value);
		value = MyRandom((short) value);
		PushAtom(V_StringLiteral, AddToStringTable(gPV.tBuf));
	}
	else
		PushAtom(V_StringLiteral, AddToStringTable(""));
}

void SF_LOGMSG()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_StringLiteral)
#ifdef WIN32
		LogMsg(0,PubChatText,"%s\r",gPV.strList+value);
#else
		LogMessage("%s\r",gPV.strList+value);
#endif
}

void SF_SHOWLOOSEPROPS()
{
	void ShowLooseProps();
	ShowLooseProps();
}

void SF_SERVERNAME()
{	// 7/25/95
	sprintf(gPV.tBuf,"%.*s",gRoomWin->serverInfo.serverName[0],&gRoomWin->serverInfo.serverName[1]);
	PushAtom(V_StringLiteral, AddToStringTable(gPV.tBuf));
}

void SF_USERNAME()
{	// 7/25/95
	StringPtr	name;
	if (gRoomWin->mePtr)
		name = gRoomWin->mePtr->user.name;
	else
		name = gPrefs.name;
	sprintf(gPV.tBuf,"%s",CvtToCString(name));
	PushAtom(V_StringLiteral, AddToStringTable(gPV.tBuf));
}

void SF_SETPROPS()
{
	short		aType,iType;
	long		aValue,iValue;
	ArrayPtr	ap;
	long		i;
	long		n=0;
	AssetSpec	aSpec[MaxUserProps];
	if ((aType = PopAtom(&aValue)) == V_Error)		// array
		return;
	AtomToValue(&aType,&aValue);
	if (aType == V_Array) {
		ap = (ArrayPtr) (gPV.strList+aValue);
		for (i = 0; i < ap->nbrItems; ++i) {
			iType = ap->firstItem[i].type;
			iValue = ap->firstItem[i].value;
			AtomToValue(&iType,&iValue);
			switch (iType) {
			case V_LongLiteral:
				aSpec[n].id = iValue;
				aSpec[n].crc = 0;
				++n;
				break;
			case V_StringLiteral:
				{
					char	pString[255];
					char	*cString = gPV.strList + iValue;
					char	len;
					Handle	h;
					len = strlen(cString);
					BlockMove(cString,&pString[1],len+1);
					pString[0] = len;
					if ((h = GetAssetByName(RT_PROP,(StringPtr) pString)) != NULL) {
						long	type,id,crc;
						GetAssetInfo(h,&type,&id,(StringPtr) pString);
						crc = GetAssetCRC(h);
						aSpec[n].id = id;
						aSpec[n].crc = crc;
						++n;
					}
				}
				break;
			}
		}
	}
	if (gRoomWin->mePtr && n) {
		ChangeUserDesc(gRoomWin->mePtr->user.faceNbr,
						gRoomWin->mePtr->user.colorNbr,
						n,
						&aSpec[0]);
	}
}

void SF_GET()
{
	short		iType,aType;
	long		iValue, aValue;
	ArrayPtr	ap;

	if ((iType = PopAtom(&iValue)) == V_Error)		// index
		return;
	AtomToValue(&iType,&iValue);

	if ((aType = PopAtom(&aValue)) == V_Error)		// array
		return;
	AtomToValue(&aType,&aValue);

	if (iType == V_LongLiteral && aType == V_Array) {
		ap = (ArrayPtr) (gPV.strList+aValue);
		if (iValue >= 0 && iValue < ap->nbrItems) {
			PushAtom(ap->firstItem[iValue].type,ap->firstItem[iValue].value);
			return;
		}
	}
	PushAtom(V_LongLiteral, 0L);
}

void SF_PUT()
{
	short		iType,aType,vType;
	long		iValue, aValue,vValue;
	ArrayPtr	ap;

	if ((iType = PopAtom(&iValue)) == V_Error)		// index
		return;
	AtomToValue(&iType,&iValue);

	if ((aType = PopAtom(&aValue)) == V_Error)		// array
		return;
	AtomToValue(&aType,&aValue);

	if ((vType = PopAtom(&vValue)) == V_Error)		// item
		return;
	AtomToValue(&vType,&vValue);

	if (iType == V_LongLiteral && aType == V_Array) {
		ap = (ArrayPtr) (gPV.strList+aValue);
		if (iValue >= 0 && iValue < ap->nbrItems) {
			ap->firstItem[iValue].type = vType;
			ap->firstItem[iValue].value = vValue;
		}
	}
}

void SF_ARRAY()
{
	short		iType;
	long		iValue;
	ArrayPtr	ap;
	long		size;
	
	if ((iType = PopAtom(&iValue)) == V_Error)		// index
		return;
	AtomToValue(&iType,&iValue);
	if (iType == V_LongLiteral) {
		if (iValue >= 0 && iValue < MaxArrayElements) {
			size = sizeof(long)+iValue*sizeof(PStack);
			ap = (ArrayPtr) gPV.tBuf;
			memset(ap,0,size);
			ap->nbrItems = iValue;
			PushAtom(V_Array,  AddToDataTable((Ptr) &ap, size));
			return;
		}
	}
	PushAtom(V_LongLiteral, 0L);
}

// !!

void SF_FOREACH()
{
	short		fType,aType;
	long		fValue, aValue;
	ArrayPtr	ap;
	short		i;

	if ((aType = PopAtom(&aValue)) == V_Error)		// array
		return;
	AtomToValue(&aType,&aValue);

	if ((fType = PopAtom(&fValue)) == V_Error)		// atomlist
		return;
	AtomToValue(&fType,&fValue);


	if (fType == V_AtomList && aType == V_Array) {
		ap = (ArrayPtr) (gPV.strList+aValue);
		for (i = 0; i < ap->nbrItems && !gPV.abortScript; ++i) {
			PushAtom(ap->firstItem[i].type,ap->firstItem[i].value);
			CallSubroutine(fValue);
		}
		if (gPV.abortScript == A_Break)
			gPV.abortScript = A_OK;
	}
}

void SF_LENGTH()
{
	short		aType;
	long		aValue;
	ArrayPtr	ap;
	short		nbrItems = 0;

	if ((aType = PopAtom(&aValue)) == V_Error)		// array
		return;
	AtomToValue(&aType,&aValue);

	if (aType == V_Array) {
		ap = (ArrayPtr) (gPV.strList+aValue);
		nbrItems = (short)ap->nbrItems;
	}
	PushAtom(V_LongLiteral, nbrItems);
}


void SF_SELECT()	// Select a hotspot by id#
{
	short		iType;
	long		iValue;
	HotspotPtr	hs;
	if ((iType = PopAtom(&iValue)) == V_Error)		// index
		return;
	AtomToValue(&iType,&iValue);
	if (iType == V_LongLiteral) {
		hs = GetHotspot((short)iValue);\
		if (hs)
			PassHotspotEvent(hs,PE_Select);
	}
}

void SF_NBRSPOTS()	// Number of Spots
{
	PushAtom(V_LongLiteral, gRoomWin->curRoom.nbrHotspots);
}

void SF_NBRDOORS()	// Number of Doors
{
	HotspotPtr hs = (HotspotPtr)&gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];
	short	n = 0;
	short	i;

	for (i=0;i<gRoomWin->curRoom.nbrHotspots;i++,++hs) {
	    switch (hs->type)
	    {
	      case HS_Door:
	      case HS_ShutableDoor:
	      case HS_LockableDoor:
				++n;
				break;
		}
	}
	PushAtom(V_LongLiteral, n);
}

void SF_DOORIDX()	// Returns doorID of door index # n
{
	short		iType;
	long		iValue;
	long		doorID = 0;
	HotspotPtr 	hs = (HotspotPtr)&gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];
	short		n = 0,i;

	if ((iType = PopAtom(&iValue)) == V_Error)		// index
		return;
	AtomToValue(&iType,&iValue);

	if (iType == V_LongLiteral) {
		for (i=0;i<gRoomWin->curRoom.nbrHotspots;i++,++hs) {
		    switch (hs->type) {
		      case HS_Door:
		      case HS_ShutableDoor:
		      case HS_LockableDoor:
		      		if (n == iValue) {
						doorID = hs->id;
						goto Done;
					}
					++n;
					break;
			}
		}
	}
Done:
	PushAtom(V_LongLiteral, doorID);
}

void SF_SPOTIDX()	// Returns id of spot index # n
{
	short		iType;
	long		iValue;
	long		spotID = 0;
	HotspotPtr 	hs = (HotspotPtr)&gRoomWin->curRoom.varBuf[gRoomWin->curRoom.hotspotOfst];

	if ((iType = PopAtom(&iValue)) == V_Error)		// index
		return;
	AtomToValue(&iType,&iValue);

	if (iType == V_LongLiteral) {
		if (iValue >= 0 && iValue < gRoomWin->curRoom.nbrHotspots)
			spotID = hs[iValue].id;
	}
	PushAtom(V_LongLiteral, spotID);
}

void SF_EXIT()	// Returns id of spot index # n
{
	gPV.abortScript = A_Exit;
}


void SF_RETURN()	// Returns id of spot index # n
{
	gPV.abortScript = A_Return;
}

void SF_BREAK()	// Returns id of spot index # n
{
	gPV.abortScript = A_Break;
}

void SF_DUP()
{
	if (gPV.sLevel) {
		gPV.stack[gPV.sLevel] = gPV.stack[gPV.sLevel-1];
		++gPV.sLevel;
	}
}

void SF_SWAP()
{
	PStack	temp;
	if (gPV.sLevel > 1) {
		temp = gPV.stack[gPV.sLevel-1];
		gPV.stack[gPV.sLevel-1] = gPV.stack[gPV.sLevel-2];
		gPV.stack[gPV.sLevel-2] = temp;
	}
}

void SF_POP()
{
	long	value;
	PopAtom(&value);
}

void SF_BEEP()
{
	SysBeep(1);
}

// 7/29/95

void SF_WHOCHAT()		// who generated inChat event
{
	PushAtom(V_LongLiteral, gWhoChat);
}

void SF_WHOME()		// who generated inChat event
{
	long	id;
	if (gRoomWin->mePtr)
		id = gRoomWin->mePtr->user.userID;
	else
		id = 0;
	PushAtom(V_LongLiteral,id);
}

void SF_POSX()		// who generated inChat event
{
	long	x;
	if (gRoomWin->mePtr)
		x = gRoomWin->mePtr->user.roomPos.h;
	else
		x = 256;
	PushAtom(V_LongLiteral, x);
}

void SF_POSY()		// who generated inChat event
{
	long	y;
	if (gRoomWin->mePtr)
		y = gRoomWin->mePtr->user.roomPos.v;
	else
		y = 192;
	PushAtom(V_LongLiteral, y);
}


void SF_PRIVATEMSG()
{
	short	iType,sType;
	long	iValue,sValue;
	Ptr		p;
	char	*text;
	short	len;
	if ((iType = PopAtom(&iValue)) == V_Error)		// who id#
		return;
	AtomToValue(&iType,&iValue);
	if ((sType = PopAtom(&sValue)) == V_Error)		// string
		return;
	AtomToValue(&sType,&sValue);
	if (iType == V_LongLiteral && sType == V_StringLiteral) {
		text = gPV.strList+sValue;
		len = strlen(text);
		if (len > 248) {	// 11/1 jab
			len = 248;
		}
		p = gPV.tBuf;
		*((long *) p) = iValue;
		p += 4;
		*((short *) p) = len + 3;
		p += 2;
		EncryptCString((unsigned char *)text,(unsigned char *) p,len);
		p[len] = 0;
		PostServerEvent(MSG_XWHISPER,gRoomWin->meID,gPV.tBuf,len+7);
	}
}

void SF_STATUSMSG()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_StringLiteral)
		StatusMessage(gPV.strList+value,0);
}

void SF_SPOTDEST()	// Returns dest of spot id n
{
	short		iType;
	long		iValue;
	HotspotPtr	hs;
	if ((iType = PopAtom(&iValue)) == V_Error)
		return;
	AtomToValue(&iType,&iValue);
	if (iType == V_LongLiteral) {
		hs = GetHotspot((short)iValue);\
		if (hs)
			PushAtom(V_LongLiteral,hs->dest);
	}
}

void SF_UPPERCASE()		// converts string to upper
{
	short		sType;
	long		sValue;
	char		*sp,*dp;
	if ((sType = PopAtom(&sValue)) == V_Error)
		return;
	AtomToValue(&sType,&sValue);
	if (sType == V_StringLiteral) {
		sp = gPV.strList+sValue;
		dp = gPV.tBuf;
		while (*sp) {
			if (islower(*sp))
				*dp = toupper(*sp);
			else
				*dp = *sp;
			++dp;
			++sp;
		}
		*dp = 0;
		PushAtom(V_StringLiteral,AddToStringTable(gPV.tBuf));
	}
}

void SF_LOWERCASE()	// converts string to lowercase
{
	short		sType;
	long		sValue;
	char		*sp,*dp;
	if ((sType = PopAtom(&sValue)) == V_Error)
		return;
	AtomToValue(&sType,&sValue);
	if (sType == V_StringLiteral) {
		sp = gPV.strList+sValue;
		dp = gPV.tBuf;
		while (*sp) {
			if (isupper(*sp))
				*dp = tolower(*sp);
			else
				*dp = *sp;
			++dp;
			++sp;
		}
		*dp = 0;
		PushAtom(V_StringLiteral,AddToStringTable(gPV.tBuf));
	}
}


void SF_ISGUEST()
{
	PushAtom(V_LongLiteral, gSecure.guestAccess > 0);
}

void SF_ISWIZARD()
{
	PushAtom(V_LongLiteral, (gRoomWin->userFlags & U_SuperUser) > 0);
}

void SF_ISGOD()
{
	PushAtom(V_LongLiteral, (gRoomWin->userFlags & U_God) > 0);
}

void SF_DIMROOM()
{
	short	iType;
	long	iValue;
	void SetLightLevel(short n);

	if ((iType = PopAtom(&iValue)) == V_Error)		// index
		return;
	AtomToValue(&iType,&iValue);
	if (iType == V_LongLiteral)
		SetLightLevel((short) iValue);
}

void SF_DATETIME()
{
	time_t	dateSecs;
	time(&dateSecs);
	PushAtom(V_LongLiteral, dateSecs);
}

void SF_TICKS()
{
	long ticks;
	ticks = TickCount();
	PushAtom(V_LongLiteral, ticks);
}

void SF_SPOTNAME()
{
	short		iType;
	long		iValue;
	HotspotPtr	hs;
	if ((iType = PopAtom(&iValue)) == V_Error)		// spot id#
		return;
	AtomToValue(&iType,&iValue);
	gPV.tBuf[0] = 0;
	if (iType == V_LongLiteral) {
		hs = GetHotspot((short)iValue);
		GetSpotName(hs, (StringPtr) gPV.tBuf);
		PtoCstr((StringPtr) gPV.tBuf);
	}
		
	PushAtom(V_StringLiteral, AddToStringTable(gPV.tBuf));
}

void SF_SOUND()
{
	short		sType;
	long		sValue;
	if ((sType = PopAtom(&sValue)) == V_Error)
		return;
	AtomToValue(&sType,&sValue);
	if (sType == V_StringLiteral) {
		DoPalaceCommand(PC_PlaySound, 0L, gPV.strList+sValue);
	}
}

void SF_MIDIPLAY()
{
	short		sType;
	long		sValue;
	if ((sType = PopAtom(&sValue)) == V_Error)
		return;
	AtomToValue(&sType,&sValue);
	if (sType == V_StringLiteral) {
		MidiPlay(gPV.strList+sValue);
	}
}

void SF_MIDILOOP()
{
	short		sType,iType;
	long		sValue,iValue;
	if ((sType = PopAtom(&sValue)) == V_Error)
		return;
	AtomToValue(&sType,&sValue);
	if ((iType = PopAtom(&iValue)) == V_Error)		// loop v#
		return;
	AtomToValue(&iType,&iValue);


	if (sType == V_StringLiteral && iType == V_LongLiteral) {
		MidiLoop(gPV.strList+sValue, (short) iValue);
	}
}

void SF_MIDISTOP()
{
	MidiStop();
}

// "name" HASPROP
// N HASPROP
// returns true if user is wearing the prop indicated by N or NAME

// "prop" HASPOPR
// propID HASPROP
// returns TRUE if user is wearing prop

void SF_HASPROP()
{
	short		sType;
	long		sValue,bResult=0;

	if ((sType = PopAtom(&sValue)) == V_Error)
		return;
	AtomToValue(&sType,&sValue);
	if (sType == V_StringLiteral) {
		Handle	h;
		short	len;
		len = strlen(gPV.strList+sValue);
		BlockMove(gPV.strList+sValue,&gPV.tBuf[1],len+1);
		gPV.tBuf[0] = (char)len;
		if ((h = GetAssetByName(RT_PROP,(StringPtr) gPV.tBuf)) != NULL)
		{
			long	type,id;
			GetAssetInfo(h,&type,&id,(StringPtr) gPV.tBuf);
			bResult = PropInUse(id,0) > 0;
		}
	}
	else if (sType == V_LongLiteral) {
		bResult = PropInUse(sValue,0) > 0;
	}
	PushAtom(V_LongLiteral, bResult);
}

// Pushes number of user props
void SF_NBRUSERPROPS()
{
	long	nbrUserProps;
	nbrUserProps = gRoomWin->mePtr->user.nbrProps;
	PushAtom(V_LongLiteral, nbrUserProps);
}

// X USERPROP  - pushes ID# of user prop #x     (0 - n-1)
void SF_USERPROP()
{
	short		iType;
	long		iValue,iResult=0;
	AssetSpec	*as = gRoomWin->mePtr->user.propSpec;
	if ((iType = PopAtom(&iValue)) == V_Error)		// spot id#
		return;
	AtomToValue(&iType,&iValue);
	
	if (iValue >= 0 && iValue < gRoomWin->mePtr->user.nbrProps) {
		iResult = as[iValue].id;
	}
	PushAtom(V_LongLiteral, iResult);
}

// pushes user's ID Number
//
void SF_USERID()
{
	SF_WHOME();
}

// "name" WHOPOS
// userID WHOPOS
// pushes X Y position of user
void SF_WHOPOS()
{
	LocalUserRecPtr	uRec;
	short	sType;
	long	sValue,x=0,y=0;

	if ((sType = PopAtom(&sValue)) == V_Error)
		return;
	AtomToValue(&sType,&sValue);

	if (sType == V_StringLiteral)
		uRec = GetUserByName(gPV.strList+sValue);
	else
		uRec = GetUser(sValue);
	if (uRec) {
		x = uRec->user.roomPos.h;
		y = uRec->user.roomPos.v;
	}
	PushAtom(V_LongLiteral, x);
	PushAtom(V_LongLiteral, y);
}

void SF_NBRROOMUSERS()
{
	PushAtom(V_LongLiteral, gRoomWin->curRoom.nbrPeople);
}

// N ROOMUSER
// pushes userID of user N
void SF_ROOMUSER()
{
	short	iType;
	long	iValue,rValue=0;

	if ((iType = PopAtom(&iValue)) == V_Error)		// spot id#
		return;
	AtomToValue(&iType,&iValue);
	if (iValue >= 0 && iValue < gRoomWin->curRoom.nbrPeople)
		rValue = gRoomWin->userList[iValue].user.userID;

	PushAtom(V_LongLiteral, rValue);
}

void SF_MOUSEPOS()
{
	void GetPalaceMouse(Point *pt);
	Point	where;
	GetPalaceMouse(&where);
	PushAtom(V_LongLiteral, where.h);
	PushAtom(V_LongLiteral, where.v);
}

void SF_SAYAT()	// "thing" X Y SAYAT
{	// 6/7/95 modified
	short	type;
	long	value;
	Point	pt;

	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		pt.v = (short)value;

	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_LongLiteral)
		pt.h = (short)value;

	if ((type = PopAtom(&value)) == V_Error)	// prop id#
		return;
	AtomToValue(&type,&value);

	if (type == V_StringLiteral) {	// 7/6/95
		sprintf(gPV.tBuf,"@%d,%d %s",pt.h,pt.v,gPV.strList+value);
		DoPalaceCommand(PC_Chat, 0, gPV.tBuf);
	}
}

// userID WHONAME
// converts userID to user name
void SF_WHONAME()
{
	short			iType;
	long			iValue,rValue=0;
	LocalUserRecPtr	uRec;
	
	if ((iType = PopAtom(&iValue)) == V_Error)		// spot id#
		return;
	AtomToValue(&iType,&iValue);
	uRec = GetUser(iValue);
	if (uRec)
		strcpy(gPV.tBuf,CvtToCString(uRec->user.name));
	else
		gPV.tBuf[0] = 0;
	PushAtom(V_StringLiteral, AddToStringTable(gPV.tBuf));
}

void SF_WHOTARGET()
{
	PushAtom(V_LongLiteral, gRoomWin->targetID);
}

void SF_ROOMNAME()
{
	PushAtom(V_StringLiteral, 
			AddToStringTable(
				CvtToCString(
					(StringPtr) &gRoomWin->curRoom.varBuf[
									gRoomWin->curRoom.roomNameOfst])));
}

void SF_ROOMID()
{
	PushAtom(V_LongLiteral, gRoomWin->curRoom.roomID);
}

void SF_ATOI()
{
	long	result = 0;
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_StringLiteral)
		result = atol(gPV.strList+value);
	PushAtom(V_LongLiteral, result);
}

void SF_STRTOATOM()
{
	long	result = 0;
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
	if (type == V_StringLiteral)
		strcpy(gPV.tBuf,gPV.strList+value);
	else
		gPV.tBuf[0] = 0;
	PushAtom(V_AtomList, AddToStringTable(gPV.tBuf));
}

// Launch a plug-in  JAB 6/27/96
void SF_LAUNCHPPA()
{
	short	type;
	long	value;
	if ((type = PopAtom(&value)) == V_Error)
		return;
	AtomToValue(&type,&value);
#if PPASUPPORT
	if (type == V_StringLiteral) {
		PPAMgrPPACommand(gPV.strList+value);
	}
#endif
}

// Send a message to a plug-in
void SF_TALKPPA()
{
	SF_LAUNCHPPA();	// Same thing...
}