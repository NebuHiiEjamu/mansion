// Keyword Function Enums
// Automatically generated by ParserMaker.c
//
enum {
	sf__DPLAYVERSION,sf__NBRGAMES,sf__GETGAMENAME,sf__GETGAMEGUID,sf__ISHOST,
	sf__ISPLAYER,sf__CANHOST,sf__CANJOIN,sf__CANSTART,sf__NBRPLAYERS,
	sf__GETSPOTDPSTATE,sf__SETSPOTGAMENAME,sf__SETSPOTGAMEGUID,sf__GETSPOTGAMENAME,sf__GETSPOTGAMEGUID,
	sf__GLOBAL,sf__DEF,sf__EXEC,sf__IF,sf__IFELSE,
	sf__WHILE,sf__ALARMEXEC,sf__NOT,sf__AND,sf__OR,
	sf__GOTOROOM,sf__SAY,sf__DEST,sf__ME,sf__ITOA,
	sf__GET,sf__PUT,sf__ARRAY,sf__LOCK,sf__SETPICLOC,
	sf__SETLOC,sf__SETSPOTSTATE,sf__SETSPOTSTATELOCAL,sf__SETALARM,sf__GETSPOTSTATE,
	sf__ADDLOOSEPROP,sf__DOFFPROP,sf__DONPROP,sf__REMOVEPROP,sf__CLEARPROPS,
	sf__CLEARLOOSEPROPS,sf__SETCOLOR,sf__SETFACE,sf__UNLOCK,sf__ISLOCKED,
	sf__GLOBALMSG,sf__ROOMMSG,sf__SUSRMSG,sf__LOCALMSG,sf__DELAY,
	sf__RANDOM,sf__SUBSTR,sf__GREPSTR,sf__GREPSUB,sf__LAUNCHAPP,
	sf__SHELLCMD,sf__KILLUSER,sf__NETGOTO,sf__MACRO,sf__MOVE,
	sf__SETPOS,sf__INSPOT,sf__LOGMSG,sf__SHOWLOOSEPROPS,sf__SERVERNAME,
	sf__USERNAME,sf__SELECT,sf__NBRSPOTS,sf__NBRDOORS,sf__DOORIDX,
	sf__SPOTIDX,sf__EXIT,sf__RETURN,sf__BREAK,sf__FOREACH,
	sf__DUP,sf__SWAP,sf__POP,sf__BEEP,sf__LENGTH,
	sf__WHOCHAT,sf__WHOME,sf__POSX,sf__POSY,sf__PRIVATEMSG,
	sf__STATUSMSG,sf__SPOTDEST,sf__UPPERCASE,sf__LOWERCASE,sf__DROPPROP,
	sf__TOPPROP,sf__ISGUEST,sf__DIMROOM,sf__DATETIME,sf__TICKS,
	sf__SPOTNAME,sf__SOUND,sf__MIDIPLAY,sf__MIDILOOP,sf__MIDISTOP,
	sf__PENFRONT,sf__PENBACK,sf__PENCOLOR,sf__PENSIZE,sf__PENPOS,
	sf__PENTO,sf__LINETO,sf__LINE,sf__PAINTCLEAR,sf__PAINTUNDO,
	sf__HASPROP,sf__NBRUSERPROPS,sf__USERPROP,sf__USERID,sf__WHOPOS,
	sf__NBRROOMUSERS,sf__ROOMUSER,sf__SAYAT,sf__MOUSEPOS,sf__WHONAME,
	sf__WHOTARGET,sf__ROOMNAME,sf__STRTOATOM,sf__ATOI,sf__ROOMID,
	sf__SETPROPS,sf__ISWIZARD,sf__ISGOD,sf__LAUNCHPPA,sf__TALKPPA,
sf_NbrScriptFunctions };

// Function Templates
// Automatically generated by ParserMaker.c
//
void SF_DPLAYVERSION();
void SF_NBRGAMES();
void SF_GETGAMENAME();
void SF_GETGAMEGUID();
void SF_ISHOST();
void SF_ISPLAYER();
void SF_CANHOST();
void SF_CANJOIN();
void SF_CANSTART();
void SF_NBRPLAYERS();
void SF_GETSPOTDPSTATE();
void SF_SETSPOTGAMENAME();
void SF_SETSPOTGAMEGUID();
void SF_GETSPOTGAMENAME();
void SF_GETSPOTGAMEGUID();
void SF_GLOBAL();
void SF_DEF();
void SF_EXEC();
void SF_IF();
void SF_IFELSE();
void SF_WHILE();
void SF_ALARMEXEC();
void SF_NOT();
void SF_AND();
void SF_OR();
void SF_GOTOROOM();
void SF_SAY();
void SF_DEST();
void SF_ME();
void SF_ITOA();
void SF_GET();
void SF_PUT();
void SF_ARRAY();
void SF_LOCK();
void SF_SETPICLOC();
void SF_SETLOC();
void SF_SETSPOTSTATE();
void SF_SETSPOTSTATELOCAL();
void SF_SETALARM();
void SF_GETSPOTSTATE();
void SF_ADDLOOSEPROP();
void SF_DOFFPROP();
void SF_DONPROP();
void SF_REMOVEPROP();
void SF_CLEARPROPS();
void SF_CLEARLOOSEPROPS();
void SF_SETCOLOR();
void SF_SETFACE();
void SF_UNLOCK();
void SF_ISLOCKED();
void SF_GLOBALMSG();
void SF_ROOMMSG();
void SF_SUSRMSG();
void SF_LOCALMSG();
void SF_DELAY();
void SF_RANDOM();
void SF_SUBSTR();
void SF_GREPSTR();
void SF_GREPSUB();
void SF_LAUNCHAPP();
void SF_SHELLCMD();
void SF_KILLUSER();
void SF_NETGOTO();
void SF_MACRO();
void SF_MOVE();
void SF_SETPOS();
void SF_INSPOT();
void SF_LOGMSG();
void SF_SHOWLOOSEPROPS();
void SF_SERVERNAME();
void SF_USERNAME();
void SF_SELECT();
void SF_NBRSPOTS();
void SF_NBRDOORS();
void SF_DOORIDX();
void SF_SPOTIDX();
void SF_EXIT();
void SF_RETURN();
void SF_BREAK();
void SF_FOREACH();
void SF_DUP();
void SF_SWAP();
void SF_POP();
void SF_BEEP();
void SF_LENGTH();
void SF_WHOCHAT();
void SF_WHOME();
void SF_POSX();
void SF_POSY();
void SF_PRIVATEMSG();
void SF_STATUSMSG();
void SF_SPOTDEST();
void SF_UPPERCASE();
void SF_LOWERCASE();
void SF_DROPPROP();
void SF_TOPPROP();
void SF_ISGUEST();
void SF_DIMROOM();
void SF_DATETIME();
void SF_TICKS();
void SF_SPOTNAME();
void SF_SOUND();
void SF_MIDIPLAY();
void SF_MIDILOOP();
void SF_MIDISTOP();
void SF_PENFRONT();
void SF_PENBACK();
void SF_PENCOLOR();
void SF_PENSIZE();
void SF_PENPOS();
void SF_PENTO();
void SF_LINETO();
void SF_LINE();
void SF_PAINTCLEAR();
void SF_PAINTUNDO();
void SF_HASPROP();
void SF_NBRUSERPROPS();
void SF_USERPROP();
void SF_USERID();
void SF_WHOPOS();
void SF_NBRROOMUSERS();
void SF_ROOMUSER();
void SF_SAYAT();
void SF_MOUSEPOS();
void SF_WHONAME();
void SF_WHOTARGET();
void SF_ROOMNAME();
void SF_STRTOATOM();
void SF_ATOI();
void SF_ROOMID();
void SF_SETPROPS();
void SF_ISWIZARD();
void SF_ISGOD();
void SF_LAUNCHPPA();
void SF_TALKPPA();


#if _U_SFUNC
// Keyword Function Table
// Automatically generated by ParserMaker.c
//
void (*SFuncArray[sf_NbrScriptFunctions])() = {
	SF_DPLAYVERSION,SF_NBRGAMES,SF_GETGAMENAME,SF_GETGAMEGUID,SF_ISHOST,
	SF_ISPLAYER,SF_CANHOST,SF_CANJOIN,SF_CANSTART,SF_NBRPLAYERS,
	SF_GETSPOTDPSTATE,SF_SETSPOTGAMENAME,SF_SETSPOTGAMEGUID,SF_GETSPOTGAMENAME,SF_GETSPOTGAMEGUID,
	SF_GLOBAL,SF_DEF,SF_EXEC,SF_IF,SF_IFELSE,
	SF_WHILE,SF_ALARMEXEC,SF_NOT,SF_AND,SF_OR,
	SF_GOTOROOM,SF_SAY,SF_DEST,SF_ME,SF_ITOA,
	SF_GET,SF_PUT,SF_ARRAY,SF_LOCK,SF_SETPICLOC,
	SF_SETLOC,SF_SETSPOTSTATE,SF_SETSPOTSTATELOCAL,SF_SETALARM,SF_GETSPOTSTATE,
	SF_ADDLOOSEPROP,SF_DOFFPROP,SF_DONPROP,SF_REMOVEPROP,SF_CLEARPROPS,
	SF_CLEARLOOSEPROPS,SF_SETCOLOR,SF_SETFACE,SF_UNLOCK,SF_ISLOCKED,
	SF_GLOBALMSG,SF_ROOMMSG,SF_SUSRMSG,SF_LOCALMSG,SF_DELAY,
	SF_RANDOM,SF_SUBSTR,SF_GREPSTR,SF_GREPSUB,SF_LAUNCHAPP,
	SF_SHELLCMD,SF_KILLUSER,SF_NETGOTO,SF_MACRO,SF_MOVE,
	SF_SETPOS,SF_INSPOT,SF_LOGMSG,SF_SHOWLOOSEPROPS,SF_SERVERNAME,
	SF_USERNAME,SF_SELECT,SF_NBRSPOTS,SF_NBRDOORS,SF_DOORIDX,
	SF_SPOTIDX,SF_EXIT,SF_RETURN,SF_BREAK,SF_FOREACH,
	SF_DUP,SF_SWAP,SF_POP,SF_BEEP,SF_LENGTH,
	SF_WHOCHAT,SF_WHOME,SF_POSX,SF_POSY,SF_PRIVATEMSG,
	SF_STATUSMSG,SF_SPOTDEST,SF_UPPERCASE,SF_LOWERCASE,SF_DROPPROP,
	SF_TOPPROP,SF_ISGUEST,SF_DIMROOM,SF_DATETIME,SF_TICKS,
	SF_SPOTNAME,SF_SOUND,SF_MIDIPLAY,SF_MIDILOOP,SF_MIDISTOP,
	SF_PENFRONT,SF_PENBACK,SF_PENCOLOR,SF_PENSIZE,SF_PENPOS,
	SF_PENTO,SF_LINETO,SF_LINE,SF_PAINTCLEAR,SF_PAINTUNDO,
	SF_HASPROP,SF_NBRUSERPROPS,SF_USERPROP,SF_USERID,SF_WHOPOS,
	SF_NBRROOMUSERS,SF_ROOMUSER,SF_SAYAT,SF_MOUSEPOS,SF_WHONAME,
	SF_WHOTARGET,SF_ROOMNAME,SF_STRTOATOM,SF_ATOI,SF_ROOMID,
	SF_SETPROPS,SF_ISWIZARD,SF_ISGOD,SF_LAUNCHPPA,SF_TALKPPA,
};
#endif
