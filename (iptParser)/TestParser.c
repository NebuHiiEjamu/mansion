#include "bad.h"

#define P_MatchFail	0x00
#define P_MatchFork	0x80

#include "bad.c"

Boolean ParseSymbol();

Boolean ParseSymbol(char *token)
{
	Ptr		sp;
	char	c;
	short	keyVal;
	short	i;

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

	if (keyVal == -1) {	// Unknown Word - push as a SYmbol
		printf("%s NO MATCH\n",token);
		return false;
	}
	else {
		return true;
	}
}

main()
{
	ParseSymbol("CONTROLTEST");

	ParseSymbol("GLOBAL");
	ParseSymbol("DEF");
	ParseSymbol("EXEC");
	ParseSymbol("IF");
	ParseSymbol("IFELSE");
	ParseSymbol("WHILE");
	ParseSymbol("ALARMEXEC");
	ParseSymbol("NOT");
	ParseSymbol("AND");
	ParseSymbol("OR");
	ParseSymbol("GOTOROOM");
	ParseSymbol("SAY");
	ParseSymbol("CHAT");
	ParseSymbol("DEST");
	ParseSymbol("ME");
	ParseSymbol("ID");
	ParseSymbol("ITOA");
	ParseSymbol("GET");
	ParseSymbol("PUT");
	ParseSymbol("ARRAY");
	ParseSymbol("LOCK");
	ParseSymbol("SETPICLOC");
	ParseSymbol("SETLOC");
	ParseSymbol("SETSPOTSTATE");
	ParseSymbol("SETSPOTSTATELOCAL");
	ParseSymbol("SETALARM");
	ParseSymbol("GETSPOTSTATE");
	ParseSymbol("ADDLOOSEPROP");
	ParseSymbol("DOFFPROP");
	ParseSymbol("DONPROP");
	ParseSymbol("REMOVEPROP");
	ParseSymbol("CLEARPROPS");
	ParseSymbol("NAKED");
	ParseSymbol("CLEARLOOSEPROPS");
	ParseSymbol("SETCOLOR");
	ParseSymbol("SETFACE");
	ParseSymbol("UNLOCK");
	ParseSymbol("ISLOCKED");
	ParseSymbol("GLOBALMSG");
	ParseSymbol("ROOMMSG");
	ParseSymbol("SUSRMSG");
	ParseSymbol("LOCALMSG");
	ParseSymbol("DELAY");
	ParseSymbol("RANDOM");
	ParseSymbol("RND");
	ParseSymbol("SUBSTR");
	ParseSymbol("GREPSTR");
	ParseSymbol("GREPSUB");
	ParseSymbol("LAUNCHAPP");
	ParseSymbol("SHELLCMD");
	ParseSymbol("KILLUSER");
	ParseSymbol("NETGOTO");
	ParseSymbol("GOTOURL");
	ParseSymbol("MACRO");
	ParseSymbol("MOVE");
	ParseSymbol("SETPOS");
	ParseSymbol("INSPOT");
	ParseSymbol("LOGMSG");
	ParseSymbol("SHOWLOOSEPROPS");
	ParseSymbol("SERVERNAME");
	ParseSymbol("USERNAME");
	ParseSymbol("SELECT");		// 7/28/95
	ParseSymbol("NBRSPOTS");		// 7/28/95
	ParseSymbol("NBRDOORS");		// 7/28/95
	ParseSymbol("DOORIDX");		// 7/28/95
	ParseSymbol("SPOTIDX");		// 7/28/95
	ParseSymbol("EXIT");
	ParseSymbol("RETURN");
	ParseSymbol("BREAK");
	ParseSymbol("FOREACH");
	ParseSymbol("DUP");
	ParseSymbol("SWAP");
	ParseSymbol("POP");
	ParseSymbol("BEEP");
	ParseSymbol("LENGTH");
	ParseSymbol("WHOCHAT");
	ParseSymbol("WHOME");
	ParseSymbol("POSX");
	ParseSymbol("POSY");
	ParseSymbol("PRIVATEMSG");
	ParseSymbol("STATUSMSG");
	ParseSymbol("SPOTDEST");
	ParseSymbol("UPPERCASE");
	ParseSymbol("LOWERCASE");
	ParseSymbol("DROPPROP");
	ParseSymbol("TOPPROP");
	ParseSymbol("ISGUEST");
	ParseSymbol("DIMROOM");
	ParseSymbol("DATETIME");
    ParseSymbol("TICKS");
	ParseSymbol("SPOTNAME");
	ParseSymbol("SOUND");
	ParseSymbol("MIDIPLAY");
	ParseSymbol("MIDILOOP");
	ParseSymbol("MIDISTOP");
	ParseSymbol("PENFRONT");	// 10/19
	ParseSymbol("PENBACK");
	ParseSymbol("PENCOLOR");
	ParseSymbol("PENSIZE");
	ParseSymbol("PENPOS");
	ParseSymbol("PENTO");
	ParseSymbol("LINETO");
	ParseSymbol("LINE");
	ParseSymbol("PAINTCLEAR"); // 10/23
	ParseSymbol("PAINTUNDO");
	ParseSymbol("HASPROP");
	ParseSymbol("NBRUSERPROPS");
	ParseSymbol("USERPROP");
	ParseSymbol("USERID");
	ParseSymbol("WHOPOS");
	ParseSymbol("NBRROOMUSERS");
	ParseSymbol("ROOMUSER");
	ParseSymbol("SAYAT");
	ParseSymbol("MOUSEPOS");
	ParseSymbol("WHONAME");
	ParseSymbol("WHOTARGET");
	ParseSymbol("ROOMNAME");
	ParseSymbol("STRTOATOM");
	ParseSymbol("ATOI");
	ParseSymbol("ROOMID");
	ParseSymbol("SETPROPS");
	ParseSymbol("ISWIZARD");
	ParseSymbol("ISGOD");
	ParseSymbol("LAUNCHPPA");	// 6/27/96
	ParseSymbol("TALKPPA");	// 6/27/96
	ParseSymbol("DP_DPLAYVERSION");
	ParseSymbol("DP_NBRGAMES");
	ParseSymbol("DP_GETGAMENAME");
	ParseSymbol("DP_GETGAMEGUID");
	ParseSymbol("DP_ISHOST");
	ParseSymbol("DP_ISPLAYER");
	ParseSymbol("DP_CANHOST");
	ParseSymbol("DP_CANJOIN");
	ParseSymbol("DP_CANSTART");
	ParseSymbol("DP_NBRPLAYERS");
	ParseSymbol("DP_GETSPOTDPSTATE");
	ParseSymbol("DP_SETSPOTGAMENAME");
	ParseSymbol("DP_SETSPOTGAMEGUID");
	ParseSymbol("DP_GETSPOTGAMENAME");
	ParseSymbol("DP_GETSPOTGAMEGUID");
	ParseSymbol("CONTROL");
}