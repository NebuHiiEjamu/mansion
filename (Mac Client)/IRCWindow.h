// IRCWindow.h
#ifndef _H_IRCWindow
#define _H_IRCWindow	1

#include "TermWindow.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <serial.h>

#include "MacTCPCommonTypes.h"
#include "TCPPB.h"
#include "TCPHi.h"
#include "TCPRoutines.h"
#include "GetMyIPAddr.h"

#define RcveBufferLength	512
#define LineBufferLength	2048
#define SendBufferLength	512
#define MaxTokens			32
#define TokLength			32

enum	{C_TCP, C_Serial};

typedef struct {
		unsigned long		remoteHost;		// IP Address
		unsigned long		tcpStream;		// tcp Stream
		TCPiopb 			*rcveBlock;		// Receive Async Rec
		Boolean				receiveFlag;	// Receiving?
} TCPVars;

typedef struct {
		short				inComm,outComm;
		char				*serBuf;
		SerShk				commFlags;
} SerVars;

typedef struct {
	TermWindowRecord	oWin;
	short				connectType; // 0 = TCP/IP, 1=Comm
	union {
		TCPVars			tcpVars;
		SerVars			serVars;
	} commVars;
	Boolean				connectionOpen,receiveFlag;
	char				remoteHostname[127];
	short				remotePort;
	char				*rcveBuffer;	// Receive Buffer
	char				*cp;	// Pointer to current recieve char
	long				cCnt;	// Character Count

						// IRC packets fill the following buffer
	char				lineBuffer[LineBufferLength],*lp;

						// The following buffer may be used to send packets
	char				sendBuffer[SendBufferLength];
	char				msgSource[128],msgNick[16];
	char				msgType[16];
	char				object[256];
	char 				tokens[MaxTokens][TokLength];
	char				myName[16];
	char				arg[10][128];
} IRCWindowRecord, *IRCWindowPtr;

#define IRCWIND			130
#define IRCBufSize		16384
#define IRCPort			6667
#define IRCTimeOut		30

extern WindowPtr	gIRCWindow;

void InitIRC(WindowPtr theWin);
void CloseIRC();

void IRCIdle(WindowPtr theWin, EventRecord *theEvent);
void DisposeIRCWindow(WindowPtr theWindow);
void IRCKey(WindowPtr theWin, EventRecord *theEvent);

void IRCRecordChar(WindowPtr theWin, char c);
void IRCRecordLine(WindowPtr theWin);
Boolean IRCCharAvail(WindowPtr theWin, char *c);

void CommString(char *str);

enum {ERR_NOSUCHNICK=401,ERR_NOSUCHSERVER,ERR_NOSUCHCHANNEL,ERR_CANNOTSENDTOCHAN,
		ERR_TOOMANYCHANNELS,ERR_WASNOSUCHNICK,ERR_TOOMANYTARGETS,ERR_408,ERR_NOORIGIN,
		ERR_NORECIPIENT=411,ERR_NOTEXTTOSEND,ERR_NOTOPLEVEL,ERR_WILDTOPLEVEL,
		ERR_UNKNOWNCOMMAND=421,ERR_NOMOTD,ERR_NOADMININFO,ERR_FILEERROR,
		ERR_NONICKNAMEGIVEN=431,ERR_ERRONEUSNICKNAME,ERR_NICKNAMEINUSE,
		ERR_NICKCOLLISION=436,ERR_USERNOTINCHANNEL=441,ERR_NOTONCHANNEL,
		ERR_USERONCHANNEL,ERR_NOLOGIN,ERR_SUMMONDISABLED,ERR_USERSDISABLED,
		ERR_NOTREGISTERED=451,ERR_NEEDMOREPARAMS=461,ERR_MAYNOTREGISTER,
		ERR_NOPERMFORHOST,ERR_PASSWDMISMATCH,ERR_YOUREBANNEDCREEP,
		ERR_KEYSET=467,
		ERR_CHANNELISFULL=471,ERR_UNKNOWNMODE,ERR_INVITEONLYCHAN,ERR_BANNEDFROMCHAN,
		ERR_BADCHANNELKEY,
		ERR_NOPRIVILEGES=481,ERR_CHANOPRIVSNEEDED,ERR_CANTKILLSERVER,
		ERR_NOOPERHOST=491,
		ERR_UMODEUNKNOWNFLAG=501,ERR_USERSDONTMATCH};

enum {	RPL_NONE=300,
		RPL_AWAY,
		RPL_USERHOST,
		RPL_ISON,
		RPL_UNAWAY=305,
		RPL_NOWAWAY,
		RPL_WHOISUSER=311,
		RPL_WHOISSERVER,
		RPL_WHOISOPERATOR,
		RPL_WHOWASUSER,
		RPL_ENDOFWHO,

		RPL_WHOISIDLE=317,
		RPL_ENDOFWHOIS,
		RPL_WHOISCHANNELS,
		RPL_LISTSTART=321,
		RPL_LIST,
		RPL_LISTEND,
		RPL_CHANNELMODEIS,

		RPL_NOTOPIC=331,
		RPL_TOPIC,

		RPL_INVITING=341,
		RPL_SUMMONING,
		
		RPL_VERSION=351,
		RPL_WHOREPLY,
		RPL_NAMREPLY,
		
		RPL_LINKS=364,
		RPL_ENDOFLINKS=365,

		RPL_ENDOFNAMES=366,
		RPL_BANLIST,
		RPL_ENDOFBANLIST,
		RPL_ENDOFWHOWAS	=369,
		RPL_INFO=371,
		RPL_MOTD,
		RPL_ENDOFINFO=374,
		RPL_MOTDSTART,
		RPL_ENDOFMOTD,
		RPL_YOUREOPER=381,
		RPL_REHASHING,
		
		RPL_TIME=391,
		RPL_USERSSTART,
		RPL_USERS,
		RPL_ENDOFUSERS,
		RPL_NOUSERS,
		
		RPL_TRACELINK=200,
		RPL_TRACECONNECTING,
		RPL_TRACEHANDSHAKE,
		RPL_TRACEUNKNOWN,
		RPL_TRACEOPERATOR,
		RPL_TRACEUSER,
		RPL_TRACENEWTYPE=208,
		RPL_STATSLINKINFO=211,
		RPL_STATSCOMMANDS,
		RPL_STATSCLINE,
		RPL_STATSNLINE,
		RPL_STATSILINE,
		RPL_STATSKLINE,
		RPL_STATSYLINE=218,
		RPL_ENDOFSTATS,
		RPL_UMODEIS=221,
		RPL_STATSLLINE=241,
		RPL_STATSUPTIME,
		RPL_STATSOLINE,
		RPL_STATSHLINE,
		RPL_LUSERCLIENT=251,
		RPL_LUSERONLINE,
		RPL_LUSERUNKNOWN,
		RPL_LUSERCHANNELS,
		RPL_LUSERME,
		RPL_ADMINME,RPL_ADMINLOC1,RPL_ADMINLOC2,RPL_ADMINEMAIL,
		RPL_TRACELOG=261};		

#endif
		
