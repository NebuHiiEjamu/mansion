/******************************************************************************
 ScriptMgr.c - implements the Iptscray scripting language

	Author - Jim Bumgardner

  7/27/95
  	Added Arrays
  	Added 	array N GET					// get a value from an array
  	Added	value array N PUT			// put a value into an array
	Added   n ARRAY						// allocate an array
	Added   [ n n n n ]					// An array
	
 ******************************************************************************/
#include "U-USER.H"
#include "U-SCRIPT.H"
#include "U-REGEXP.H"	//  6/2/95 JAB

#include <string.h>
#include <ctype.h>
#include <stdlib.h>


#define mymalloc(s)			NewPtrClear(s)
#define myfree(p)			DisposPtr((Ptr) (p))

ParseVars gPV;

char	gGlobalStrings[GS_NbrGlobalStrings][256];		// 6/6/96 JBUM
long	gWhoChat;

void InitScriptMgr(void)	// must be called after gRoomWin is setup
{
	gPV.allocStrList = 2048;
	if (gPV.strList == NULL)
		gPV.strList = mymalloc((long) gPV.allocStrList);
	if (gPV.tBuf == NULL)
		gPV.tBuf = mymalloc(MaxTBuf);
	InitInterpreter();
	LoadUserScript();		// 7/20/95 call after gRoomWin is setup
}

void CloseScriptMgr(void)
{
	if (gPV.strList)
		myfree(gPV.strList);
	if (gPV.tBuf)
		myfree(gPV.tBuf);
}

// Use this for setting string global from outside of script manager
//
void SetExternStringGlobal(char *string, short nbr)
{
	long	len;
	if (nbr >= 0 && nbr < GS_NbrGlobalStrings) {
		len = strlen(string);
		if (len > 255)
			len = 255;
		BlockMove(string,gGlobalStrings[nbr],len+1);	// 6/6/95
	}
}

// 6/6/95 JAB
void GetExternStringGlobal(char *string, short nbr)
{
	if (nbr >= 0 && nbr < GS_NbrGlobalStrings) {
		BlockMove(gGlobalStrings[nbr],string,strlen(gGlobalStrings[nbr])+1);
	}
}

// 6/6/95 JAB
void RetrieveExternStringGlobal(VariablePtr v, short nbr)
{
	v->value = AddToStringTable(gGlobalStrings[nbr]);
	v->type = V_StringLiteral;
}

// 6/6/95 JAB
void UpdateExternStringGlobal(VariablePtr v)
{
	short	nbr;
	if (strcmp(v->name,"CHATSTR") == 0)
		nbr = GS_ChatString;
	else
		return;
	BlockMove(gPV.strList+v->value,gGlobalStrings[nbr],strlen(gPV.strList+v->value)+1);
}

// 8/27/96 Modified to return a value
long DoHotspotScript(char *sBuf, HotspotPtr ab)
{
	gPV.curSpot = ab;
	return RunMessageScript(sBuf);
}

// 8/27/96 Modified to return a value
long DoUserScript(char *sBuf)
{
	gPV.curSpot = NULL;
	return RunMessageScript(sBuf);
}

void InitInterpreter(void)
{
	gPV.scriptRunning = false;
	gPV.nLevel = 0;
	gPV.sLevel = 0;
	gPV.pMode = P_GetAtom;
	gPV.nbrVars = 0;
	gPV.strListLen = 0;
}


GlobalVarHandle NewGlobal(VariablePtr v)
{
	GlobalVarHandle	gv;
	long			len;
	len = sizeof(GlobalVar);
	if (v->type == V_AtomList || v->type == V_StringLiteral)
		len += strlen(gPV.strList+v->value)+1;
	gv = (GlobalVarHandle) NewHandleClear(len);
	if (gv == NULL) {
		AbortScript("Out of memory for globals");
		return NULL;
	}
	else {
		(*gv)->v = *v;
		if (v->type == V_AtomList || v->type == V_StringLiteral)
			strcpy((*gv)->data,gPV.strList+v->value);
		(*gv)->nextVar = gPV.gList;
		gPV.gList = gv;
		return gv;
	}
}

// Get Global Variable Value and fill it in
void RetrieveGlobal(VariablePtr v)
{
	GlobalVarHandle	gv;
	for (gv = gPV.gList; gv; gv = (*gv)->nextVar) {
		if (strcmp((*gv)->v.name,v->name) == 0) {
			*v = (*gv)->v;
			if (v->type == V_AtomList || v->type == V_StringLiteral)
				v->value = AddToStringTable((*gv)->data);
		}
	}
}

// Write Variable value to Global variable
void UpdateGlobal(VariablePtr v)
{
	GlobalVarHandle	gv;
	long			len;
	for (gv = gPV.gList; gv; gv = (*gv)->nextVar) {
		if (strcmp((*gv)->v.name,v->name) == 0) {
			(*gv)->v = *v;
			len = sizeof(GlobalVar);
			if (v->type == V_AtomList || v->type == V_StringLiteral)
				len += strlen(gPV.strList+v->value)+1;
			if (len > GetHandleSize((Handle) gv))
				SetHandleSize((Handle) gv,len);
			if (v->type == V_AtomList || v->type == V_StringLiteral)
				strcpy((*gv)->data,gPV.strList+v->value);
			return;
		}
	}
	NewGlobal(v);
}

void GlobalizeVariable(VariablePtr v)
{
	// Variable has been defined as global - set it's flag
	v->flag |= VF_Global;
}

VariablePtr NewVariable(char *sym)
{
	VariablePtr vp = &gPV.vList[gPV.nbrVars];

	if (gPV.nbrVars >= MaxVars) {
		AbortScript("Exceeded Maximum Script Variables");
		return NULL;
	}
	else {
		strcpy(vp->name,sym);
		vp->value = 0L;
		vp->type = V_LongLiteral;
		vp->flag = 0;
		if (strcmp(sym,"CHATSTR") == 0)	{ // Auto assign extern string globals 6/6/95
			vp->flag |= VF_EGlobal;
			RetrieveExternStringGlobal(vp, GS_ChatString);
		}
		++gPV.nbrVars;
		return vp;
	}
}

Variable	*GetVariable(char *sym)
{
	// Search through table for variable
	short	i;
	for (i = 0; i < gPV.nbrVars; ++i) {
		if (strcmp(gPV.vList[i].name,sym) == 0) {
			if (gPV.vList[i].flag & VF_Global)
				RetrieveGlobal(&gPV.vList[i]);
			else if (gPV.vList[i].flag & VF_EGlobal) {	// 6/6/95
				if (strcmp(sym,"CHATSTR") == 0)
					RetrieveExternStringGlobal(&gPV.vList[i],GS_ChatString);
			}
			return &gPV.vList[i];
		}
	}
	return NewVariable(sym);
}

void AtomToValue(short *type, long *value)
{
	if (*type == V_Symbol) {
		Variable	*vt;
		vt = GetVariable(gPV.strList+(*value));
		if (vt) {	// 5/16/96 JAB additional error checking
			*type = vt->type;
			*value = vt->value;
		}
		else {	
			*type = V_LongLiteral;
			*value = 0;
		}
	}
}


void AssignVariable(VariablePtr v, short type, long value)
{
	// V is an entry in variable table
	// Assign it
	AtomToValue(&type,&value);
	v->value = value;
	v->type = type;
	if (v->flag & VF_Global)
		UpdateGlobal(v);
	else if (v->flag & VF_EGlobal) {
		UpdateExternStringGlobal(v);
	}
}

void PushAtom(short type, long value)
{
	// 5/16/96 additional error checking
	if (gPV.sLevel >= MaxStack) {
		AbortScript("Script stack overflow");
	}
	else {
		gPV.stack[gPV.sLevel].type = type;
		gPV.stack[gPV.sLevel].value = value;
		++gPV.sLevel;
	}
}

short PopAtom(long *value)
{
	if (gPV.sLevel > 0) {
		--gPV.sLevel;
		*value = gPV.stack[gPV.sLevel].value;
		return gPV.stack[gPV.sLevel].type;
	}
	else
		return V_Error;
}

void ParseNumber()
{
	long	n=0L;
	Boolean	negFlag=false;
	if (SC == '-') {
		negFlag = true;
		INC_SC;
	}
	while (SC >= '0' && SC <= '9') {
		n *= 10;
		n += SC - '0';
		INC_SC;
	}
	if (negFlag)
		n = -n;
	PushAtom(V_LongLiteral, n);
}


#define LONGALIGN(x)	x += (4 - (x & 3)) & 3

long AddToDataTable(Ptr str, long len)
{
	Ptr		sp,dp;
	long	offset;
	
	LONGALIGN(gPV.strListLen);
	
	if (gPV.strListLen + len > gPV.allocStrList) {
		long	nLen;
		Ptr		nList;
		nLen = gPV.allocStrList;
		while (nLen < gPV.strListLen+len)
			nLen += 1024;
		nList = mymalloc(nLen);
		if (nList == NULL) {
			// Out of Memory
			AbortScript("Out of memory for script data");
			return 0;
		}
		memcpy(nList,gPV.strList,gPV.strListLen);
		myfree(gPV.strList);
		gPV.strList = nList;
		gPV.allocStrList = (LONG) nLen;	// 5/16/96 changed to a LONG
	}
	sp = str;
	dp = gPV.strList + gPV.strListLen;
	offset = gPV.strListLen;
	BlockMove(sp, dp, len);
	dp += len;
	gPV.strListLen += (LONG)len; // 5/16/96 changed to a LONG
	return offset;
}

long AddToStringTable(char *str)
{
	Ptr		sp,dp;
	long	offset;
	long	len;
	len = strlen(str) + 1;
	if (gPV.strListLen + len > gPV.allocStrList) {
		long	nLen;
		Ptr		nList;
		
		nLen = gPV.allocStrList;
		while (nLen < gPV.strListLen+len)
			nLen += 1024;
		nList = mymalloc(nLen);
		if (nList == NULL) {
			// Out of Memory
			AbortScript("Out of memory for strings");
			// DebugStr("\pOut of Mem");
			return 0;
		}
		memcpy(nList,gPV.strList,gPV.strListLen);
		myfree(gPV.strList);
		gPV.strList = nList;
		gPV.allocStrList = (LONG) nLen;	// 5/16/96 changed to a LONG
	}
	sp = str;
	dp = gPV.strList + gPV.strListLen;
	offset = gPV.strListLen;
	while (*sp) {
		*(dp++) = *(sp++);
	}
	*dp = 0;
	gPV.strListLen += len;
	return offset;
}

Boolean	FindInStringTable(char *str, long *offset)
{
	long	i;
	Ptr		p;
	i = 0;
	p = gPV.strList;
	while (i < gPV.strListLen) {
		if (strcmp(str,p) == 0) {
			*offset = i;
			return true;
		}
		while (*p) {
			++p;
			++i;
		}
		++p;
		++i;
	}
	return false;
}

void ParseAtomList()
{
	short	nest;
	short	qFlag = false;
	long	sOffset;
	Ptr		dp;
	nest = 0;
	dp = gPV.tBuf;
	if (SC == '{')
		INC_SC;
	while (SC && (SC != '}' || nest > 0)) {
		if (qFlag) {
			if (SC == '\\') {	// Don't terminate on \"
				*(dp)++ = SCHAR_INC;	// 6/2/95 bug fix - don't lose slash
			}
			else if (SC == '\"')
				qFlag = false;
		}
		else {
			switch (SC) {
			case '\"':
				qFlag = true;
				break;
			// case '\\':		// 6/2/95 don't skip slashes
			//	INC_SC;
			//	break;
			case '{':
				++nest;
				break;
			case '}':
				--nest;
				break;
			}
		}
		*(dp)++ = SCHAR_INC;
	}
	*dp = 0;
	if (SC == '}')
		INC_SC;
	if (!FindInStringTable(gPV.tBuf,&sOffset))
		sOffset = AddToStringTable(gPV.tBuf);
	PushAtom(V_AtomList, sOffset);
	
}

void ParseStringLiteral()
{
	long	sOffset;
	Ptr		dp;
	dp = gPV.tBuf;
	if (SC == '\"')
		INC_SC;
	while (SC && SC != '\"') {
		if (SC == '\\') {
			INC_SC;
			if (SC == 'x') {
				unsigned char c = 0;
				INC_SC;
				c = (SC >= '0' && SC <= '9'? SC - '0' : 
					 (SC >= 'a' && SC <= 'f'? 10 + SC - 'a' :
					  (SC >= 'A' && SC <= 'F'? 10 + SC - 'A' : 0)));
				c <<= 4;
				INC_SC;
				c |= (SC >= '0' && SC <= '9'? SC - '0' : 
					 (SC >= 'a' && SC <= 'f'? 10 + SC - 'a' :
					  (SC >= 'A' && SC <= 'F'? 10 + SC - 'A' : 0)));
				INC_SC;
				// c <<= 4;
				*(dp)++ = c;	// 9/22/95
				continue;
			}
			else {
				*(dp)++ = SCHAR_INC;
			}
		}
		else {
			*(dp)++ = SCHAR_INC;
		}
	}
	*dp = 0;
	if (SC == '\"')
		INC_SC;
	if (!FindInStringTable(gPV.tBuf,&sOffset))
		sOffset = AddToStringTable(gPV.tBuf);
	PushAtom(V_StringLiteral, sOffset);
}

long Substr(long frag, long whole)
{
	char	*a,*b,*c,*d;
	a = gPV.strList + frag;
	b = gPV.strList + whole;
	while (*b) {
		c = a;
		d = b;
		while (*c && *d && (islower(*c)? toupper(*c) : *c) == (islower(*d)? toupper(*d) : *d)) {
			++c;
			++d;
		}
		if (*c == 0)
			return 1L;
		else if (*d == 0)
			return 0L;
		++b;
	}
	return 0L;
}

long grepstr(long pattern, long string)
{
	char	*pat,*str;
	pat = gPV.strList + pattern;
	str = gPV.strList + string;
	if (re_comp(pat))	// Compile Error?
		return 0L;
	return (long) re_exec(str);
}

// Note: this function must follow a grep
long grepsub(long replace)
{
	char	*rep;
	rep = gPV.strList + replace;
	if (re_subs(rep,gPV.tBuf) == 0)		// 6/13/95 - added error check
		return replace;
	else
		return (long) AddToStringTable(gPV.tBuf);;
}

void BinaryOp(unsigned char opType, 
				short type1, long v1, 
			  	short type2, long v2)
{
	long	result;
	short	resultType = V_LongLiteral;
	// Convert Variables (Symbols) to their contents
	AtomToValue(&type1,&v1);
	AtomToValue(&type2,&v2);

	// Perform Operation
	switch (opType) {
	case '=':
		if (type1 == V_LongLiteral && type2 == V_LongLiteral)	// 7/21/95
			result = (v1 == v2);
		else if (type1 == V_StringLiteral && type2 == V_StringLiteral) {
			result = stricmp(gPV.strList+v1,gPV.strList+v2) == 0;
		}
		else
			result = 0;
		break;
	case '+':
		if (type1 == V_LongLiteral && type2 == V_LongLiteral)	// 7/21/95
			result = (v1 + v2);
		else if (type1 == V_StringLiteral && type2 == V_StringLiteral) {	// 7/21/95
			strcpy(gPV.tBuf,gPV.strList+v1);
			strcat(gPV.tBuf,gPV.strList+v2);
			result = AddToStringTable(gPV.tBuf);
			resultType = V_StringLiteral;
		}
		else
			result = 0;
		break;
	case '-':
		result = (v1 - v2);
		break;
	case '*':
		result = (v1 * v2);
		break;
	case '/':
		result = (v2 == 0? 0 : v1 / v2);
		break;
	case '%':
		result = (v2 == 0? 0 : v1 % v2);
		break;
	case '<':
		result = (v1 < v2);
		break;
	case '>':
		result = (v1 > v2);
		break;
	case OP_LESSEQUAL:
		result = (v1 <= v2);
		break;
	case OP_GTREQUAL:
		result = (v1 >= v2);
		break;
	case OP_INEQUALITY:
		result = (v1 != v2);
		break;
	case '&':	// String Concat
		if (type1 == V_StringLiteral && type2 == V_StringLiteral) {	// 7/21/95
			strcpy(gPV.tBuf,gPV.strList+v1);
			strcat(gPV.tBuf,gPV.strList+v2);
			result = AddToStringTable(gPV.tBuf);
			resultType = V_StringLiteral;
		}
		else
			result = 0;
		break;
	}
	// Return Results
	PushAtom(resultType, result);
}

void UnaryOp(unsigned char opType, 
				short type1, long v1)
{
	long	result;
	short	resultType = V_LongLiteral;
	// Convert Variables (Symbols) to their contents
	AtomToValue(&type1,&v1);

	// Perform Operation
	switch (opType) {
	case '!':
		result = !v1;
		break;
	}
	// Return Results
	PushAtom(resultType, result);
}

void UnaryAssignmentOp(unsigned char opType, short type1, long v1)
{
	short 	vType;
	long	vValue;
	Variable	*v;

	vType = type1;
	vValue = v1;
	AtomToValue(&vType,&vValue);

	if (type1 != V_Symbol)
		return;
	if ((v = GetVariable(gPV.strList+v1)) == NULL)
		return;
	switch (opType) {
	case '+':	// Increment Operator
		AssignVariable(v,vType,vValue+1);
		break;
	case '-':	// Decrement Operator
		AssignVariable(v,vType,vValue-1);	// 8/31/95
		break;
	}
}

void BinaryAssignmentOp(unsigned char opType, short nType, long nValue, short type2, long v2)
{
	short 	vType;
	long	vValue;
	Variable	*v;

	AtomToValue(&nType,&nValue);
	vType = type2;
	vValue = v2;
	AtomToValue(&vType,&vValue);
	if (type2 != V_Symbol)
		return;
	if ((v = GetVariable(gPV.strList+v2)) == NULL)
		return;
	switch (opType) {
	case '+':	// +=
		AssignVariable(v,vType,vValue+nValue);
		break;
	case '-':	// -=
		AssignVariable(v,vType,vValue-nValue);
		break;
	case '*':	// *=
		AssignVariable(v,vType,vValue*nValue);
		break;
	case '/':	// /=
		AssignVariable(v,vType,nValue==0? 0 : vValue/nValue);
		break;
	case '%':	// %=
		AssignVariable(v,vType,nValue == 0? 0 : vValue%nValue);
		break;
	case '&':	// &= (String Assignment)
		strcpy(gPV.tBuf,gPV.strList+vValue);
		strcat(gPV.tBuf,gPV.strList+nValue);
		AssignVariable(v, V_StringLiteral, AddToStringTable(gPV.tBuf));
		break;
	}
}


void RunScript(void)
{
	short	type1,type2;
	long	value1,value2;
	if (gPV.abortScript)
		return;

	if (UserIsTryingToInterrupt()) {	// 7/25/95 local function (Mac Users typically use Cmd-.)
		StdStatusMessage(SM_UserAbortScript);
		gPV.abortScript = A_UserBreak;
		return;
	}

	while (SC && !gPV.abortScript) {
		// Skip white space and delimiters
		if (SC == ' ' || SC == '\t' || SC == '\r' || SC == '\n' ||
			SC == ';')
			INC_SC;
		// Skip comments
		else if (SC == '#') {
			// Comment
			while (SC && SC != '\r' && SC != '\n')
				INC_SC;
		}
		// Encode Subroutines into a string
		else if (SC == '{') {
			ParseAtomList();
		}
		// Encode strings
		else if (SC == '\"') {
			ParseStringLiteral();
		}
		// Shouldn't see any of these
		else if (SC == '}') {
			goto ScriptError;
		}
		else if (SC == '[') {
			INC_SC;
			PushAtom(V_ArrayMark, 0L);
		}
		else if (SC == ']') {
			INC_SC;
			PushAtom(V_Array, PopArray());
		}
		// Parse = and ==
		else if (SC == '!') {
			if (SCHAR(1) == '=') {
				// Equality
				if ((type2 = PopAtom(&value2)) == V_Error)
					goto ScriptError;
				if ((type1 = PopAtom(&value1)) == V_Error)
					goto ScriptError;
				BinaryOp(OP_INEQUALITY, type1,value1,type2,value2);
				INC_SC;
				INC_SC;
			}
			else {
				if ((type1 = PopAtom(&value1)) == V_Error)
					goto ScriptError;
				UnaryOp('!', type1, value1);
				INC_SC;
			}
		}
		else if (SC == '=') {
			if (SCHAR(1) == '=') {
				// Equality
				if ((type2 = PopAtom(&value2)) == V_Error)
					goto ScriptError;
				if ((type1 = PopAtom(&value1)) == V_Error)
					goto ScriptError;
				BinaryOp('=', type1,value1,type2,value2);
				INC_SC;
				INC_SC;
			}
			else {
				// Assignment
				short		type;
				long		sOffset;
				long		value;
				Variable	*v;
				if ((type = PopAtom(&sOffset)) != V_Symbol)
					goto ScriptError;
				if ((type = PopAtom(&value)) == V_Error)
					goto ScriptError;
				if ((v = GetVariable(gPV.strList+sOffset)) == NULL)
					goto ScriptError;
				AssignVariable(v,type,value);
				INC_SC;
			}
		}
		else if (SC == '+')	{
			if (SCHAR(1) == '+') {	// Increment
				if ((type1 = PopAtom(&value1)) == V_Error)
					goto ScriptError;
				UnaryAssignmentOp('+', type1,value1);
				INC_SC;
				INC_SC;
			}
			else if (SCHAR(1) == '=') {	// Plus Equal
				if ((type2 = PopAtom(&value2)) == V_Error)
					goto ScriptError;
				if ((type1 = PopAtom(&value1)) == V_Error)
					goto ScriptError;
				BinaryAssignmentOp('+', type1,value1,type2,value2);
				INC_SC;
				INC_SC;
			}
			else {
				// Addition
				if ((type2 = PopAtom(&value2)) == V_Error)
					goto ScriptError;
				if ((type1 = PopAtom(&value1)) == V_Error)
					goto ScriptError;
				BinaryOp('+', type1,value1,type2,value2);
				INC_SC;
			}
		}
		else if (SC == '-' && !(SCHAR(1) >= '0' && SCHAR(1) <= '9')) {
			if (SCHAR(1) == '-') {	// Decrement
				if ((type1 = PopAtom(&value1)) == V_Error)
					goto ScriptError;
				UnaryAssignmentOp('-', type1,value1);
				INC_SC;
				INC_SC;
			}
			else if (SCHAR(1) == '=') {	// Minus-Equal
				if ((type2 = PopAtom(&value2)) == V_Error)
					goto ScriptError;
				if ((type1 = PopAtom(&value1)) == V_Error)
					goto ScriptError;
				BinaryAssignmentOp('-', type1,value1,type2,value2);
				INC_SC;
				INC_SC;
			}
			else {
				// Subtraction
				if ((type2 = PopAtom(&value2)) == V_Error)
					goto ScriptError;
				if ((type1 = PopAtom(&value1)) == V_Error)
					goto ScriptError;
				BinaryOp('-', type1,value1,type2,value2);
				INC_SC;
			}
		}
		else if (SC == '<') {
			short	oper = '<';
			if ((type2 = PopAtom(&value2)) == V_Error)
				goto ScriptError;
			if ((type1 = PopAtom(&value1)) == V_Error)
				goto ScriptError;
			if (SCHAR(1) == '>') {
				oper = OP_INEQUALITY;	// Inequality
				INC_SC;
			}
			else if (SCHAR(1) == '=') {
				oper = OP_LESSEQUAL;
				INC_SC;
			}
			BinaryOp((unsigned char)oper,(char) type1,value1,(char)type2,value2);
			INC_SC;
		}
		else if (SC == '>') {
			short	oper = '>';
			if ((type2 = PopAtom(&value2)) == V_Error)
				goto ScriptError;
			if ((type1 = PopAtom(&value1)) == V_Error)
				goto ScriptError;
			if (SCHAR(1) == '=') {
				oper = OP_GTREQUAL;
				INC_SC;
			}
			BinaryOp((unsigned char)oper,type1,value1,type2,value2);
			INC_SC;
		}
		// Parse common operators
		else if ( SC == '*' ||
				 SC == '/' ||
				 SC == '&' ||
				 SC == '%') {				// 8/21/95 MOD
			short	oper;
			// Binary Operator
			oper = SC;
			if ((type2 = PopAtom(&value2)) == V_Error)
				goto ScriptError;
			if ((type1 = PopAtom(&value1)) == V_Error)
				goto ScriptError;
			if (SCHAR(1) == '=') {
				BinaryAssignmentOp((unsigned char)oper, type1, value1, type2, value2);
				INC_SC;
			}
			else
				BinaryOp((unsigned char)oper, type1,value1,type2,value2);
			INC_SC;
		}
		// Parse Numbers
		else if (SC == '-' || (SC >= '0' && SC <= '9'))
			ParseNumber();
		// Parse Symbols and Keywords
		else if (SC == '_' ||
				 (SC >= 'a' && SC <= 'z') ||
				 (SC >= 'A' && SC <= 'Z'))
			ParseSymbol();
		else
			goto ScriptError;		// 7/14/95
	}
	return;

ScriptError:
#ifdef WIN32
	LogMsg(0,WarningText,"Ixnay on the Iptscray!\n");
#else
	LogMessage("An error has been encountered while executing the iptscrae script.\r");
#endif
	return;
}

// Recursive Script Run Routine - Bottleneck for nested Scripts
//
void CallSubroutine(long offset)
{
	if (gPV.abortScript)	// 5/16/96 JAB - additional error checking
		return;

	if (gPV.nLevel >= MaxScriptNesting) {
		// DebugStr("\pExceeded Maximum Script Nesting");
		return;
	}
	gPV.tsp[gPV.nLevel] = gPV.so;
	gPV.nLevel++;
	gPV.so = offset;
	RunScript();
	--gPV.nLevel;
	gPV.so = gPV.tsp[gPV.nLevel];
	if (gPV.abortScript == A_Return)
		gPV.abortScript = A_OK;
}

// Top Level Script Run Routine - Bottleneck for all Level 1 Scripts
long RunMessageScript(char *msg)
{
	long	offset = AddToStringTable(msg);
	long	retVal=0;
	gPV.so = offset;
	if (!gPV.scriptRunning) {
		gPV.scriptRunning = true;
		gPV.abortScript = A_OK;
		RunScript();
		if (gPV.abortScript >= A_UserBreak)
			ClearAlarms();		// Clear Pending Alarms if User is trying to interrupt
		gPV.scriptRunning = false;

		// 8/27/96 Return Value
		if (gPV.sLevel > 0) {
			short	type;
			long	value;
			if ((type = PopAtom(&value)) != V_Error) {
				AtomToValue(&type,&value);
				if (type == V_LongLiteral)
					retVal = value;
			}
		}

		InitInterpreter();
	}
	else {
		CallSubroutine(offset);
	}
	return retVal;
}

long PopArray()
{
	short  arraymark = --gPV.sLevel;
	// count 
	while (gPV.sLevel && gPV.stack[gPV.sLevel].type != V_ArrayMark)
		gPV.sLevel --;

	if ( gPV.stack[gPV.sLevel].type == V_ArrayMark )
		return AllocateArray((short)(arraymark - gPV.sLevel), &gPV.stack[gPV.sLevel+1]);
	else
		return 0L;
}

long AllocateArray(short nbrItems, PStackPtr ps)
{
	ArrayPtr	newArray;
	long		value,size;

	size = sizeof(long)+nbrItems*sizeof(PStack);
	newArray = (ArrayPtr) gPV.tBuf;
	newArray->nbrItems = nbrItems;
	BlockMove((Ptr)ps,(Ptr) &newArray->firstItem[0],nbrItems * sizeof(PStack));
	value = AddToDataTable((Ptr) newArray, size);
	return value;
}

void AbortScript(char *msg)
{
#ifdef WIN32
	LogMsg(0,WarningText,"%s\n",msg);
#else
	SetLogColor(&gRedColor);
	LogMessage("%s\r",msg);
	SetLogColor(&gBlackColor);
#endif
	StdStatusMessage(SM_ScriptError);
	gPV.abortScript = A_Abort;
}
