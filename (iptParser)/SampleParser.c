
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

void ParseSymbol(char *token)
{
	Ptr		sp;
	short	keyVal;
	short	i;


	// Point sp to the beginning of the token
	sp = token;
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

	// If a match was found, call the corresponding function
	if (keyVal != -1) {	
		(*SFuncArray[keyVal])();
	}
}


// A sample function (all functions should have the same arguments)
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
