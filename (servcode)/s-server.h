
/* Server.h */
#include "mansion.h"
#include "s-local.h"


#define MaxHotspots			128
#define MaxSpotEvents		32
#define MaxSpotStates		32
#define MaxDrawCmds			128
#define MaxLProps			128
#define MaxFProps			64

#if STPAT
#define GuestColor			5
#else
#define GuestColor			3
#endif

#define InitLenGroupBuffer		16384	/* Shouldn't be larger than send buffer! */
#define MaxLenGroupBuffer		131072L
#define GroupBufferIncrement	16384

#define UA_HideFrom		0x01
#define UA_Mute			0x02
#define UA_Follow		0x04
#define UA_Kick			0x08

typedef struct UserActionRec {
	struct UserActionRec	*nextRec;
	PersonID		target;
	LONG			actionFlags;
} UserActionRec,*UserActionPtr;

typedef struct ServerUserRec {
	struct ServerUserRec	*nextUser;
	short			connectionType;
	union			{
		LONG		ipAddress;
#ifdef macintosh
		TargetID	appleTalkAddress;				/* Switch to Union */
#else
 /*windows */
		SOCKET		socket;
#endif
	} netAddress;
	unsigned LONG	crc,counter;		/* Serial Number# */
	time_t			lastActive;			/* seconds 11/18 */
	time_t			signonTime;
	short			oldSerialFlag;
	short			nbrPings;
	short			nbrFloodEvents;
	short			whyKilled;
	unsigned LONG	deathPenalty;
	short			flags;
	UserActionPtr	actionList;
	char			navPassword[32];	/* 5/8/96 used for password protected rooms */

#ifdef macintosh
	/* TCP Stuff */
	unsigned LONG	tcpStream;
	LONG			tcpRemoteHost;
	short			tcpRemotePort;
	TCPiopb			*tcpReceiveBlock,*tcpSendBlock;
	Boolean			tcpReceiveFlag,tcpSendFlag;
	Ptr				tcpReceiveBuffer,tcpSendBuffer,tcpSendPtr;
	LONG			tcpReceiveIdx,tcpSendLength;
#else
	short			socketIdx;	/* Windows */
#endif
#ifdef unix
	Boolean			tcpReceiveFlag,tcpSendFlag;
	Ptr				tcpReceiveBuffer,tcpSendBuffer,tcpSendPtr;
	LONG			tcpReceiveIdx;
#if BISERVER
	void			*frontEndPtr;
	long			feIPAddr;
	short			feIPPort;
#else
	int				sockfd;
#endif
#endif
	/* Group stuff - can be shared by both platforms... */
	short			groupFlag;
	Ptr				groupBuffer;
	LONG			groupLen;
	LONG			groupAlloc;
	/* save verbal ip here for shortcut (c-string) */
	char			verbalIP[256];
	LONG			lastESPsender;
	RoomID			lastPasswordRoom;
	RoomID			lastOwnedRoom;
	int				nbrFailedPasswordAttempts;
	/* 1/14/97 JAB new fields to support unique user IDs for guests and timeouts */
	unsigned LONG	puidCtr,puidCRC;
	unsigned LONG	demoElapsed,totalElapsed;
	UserRec			user;
} ServerUserRec, *ServerUserPtr;

/* 5/8/96 - member room commands - see s-memrooms.c */
enum {MR_Title, MR_Picture, MR_Owner, MR_Kick, MR_Unkick, MR_Password, MR_Close, MR_Open,
		MR_Scripts, MR_Guests, MR_Painting, MR_Hide, MR_Unhide, MR_Delete, MR_NbrCommands};

typedef struct ServerRoomRec {
	struct ServerRoomRec *nextRoom;
	LONG	personID[MaxPeoplePerRoom];
	LONG	lastPainter;		/* 11/24/95 */
	LONG	maxOccupancy;	/* 1/27/96 if 0 defer to global roomOccupancy */
	LONG	maxGuests;		/* 1/27/96 if 0, don't use */

	unsigned LONG	memberOwner;		/* 5/8/96 member owned rooms (0 for normal public rooms) */
	char	roomPassword[32];	/* 5/8/96 room password */
	
	LONG	varBufAlloc;		/* 6/28/95 */
	RoomRec	room;
} ServerRoomRec, *ServerRoomPtr;

/* 6/21/96 JAB setup new list of server-specific options flags */
#define SO_SaveSessionKeys	0x0001
#define SO_PasswordSecurity	0x0002
#define SO_AllowDemoMembers	0x0004

typedef struct {
	LONG	versID;
	Str31	serverName;
	short	fontID;
	short	fontSize;
	LONG	textFileCreator;	/* Mac Specific */
	Str31	wizardPassword;		/* Encrypted */
	Str31	godPassword;
	Str31	ownerPassword;		/* Encrypted */
	short	allowTCP,allowAppletalk;
	short	localPort;
	LONG	permissions;		/* 7/22/95 JAB */
	short	deathPenaltyMinutes;	/* 7/22 Death Penalty is N Minutes */
	short	purgePropDays;			/* 7/22 Purge props after N Days */
	short	minFloodEvents;			/* 7/22 Kill flooders after N events */
	short	maxOccupancy;			/* 7/29	- total server max occupancy */
	short	roomOccupancy;			/* 7/29 - per room occupancy */
	Str63	oldPicFolder;				/* 8/14/95 pictures folder */
	/* char	reserved[22];			// 7/22 REMOVED it's easier */
									/* to add new fields without it */
	/* Yellow Pages Stuff - C Strings */
	char	sysop[64];
	char	url[128];				/* palace://myname:1313 NULL if dynamic */
	char	machineType[128];
	char	description[256];
	char	announcement[256];
	char	ypIPAddr[128];		/* ipaddress of yellow pages service, with optional :port */
	short	autoRegister;
	Str255	picFolder;		/* 12/7/95 */
	LONG	recycleLimit;		/* 4/9/96 maximum guest # + 1 */
	short	reserved;		/* 6/21/96 was saveSessionKeys - moved to serverOptions */
	LONG	serverOptions;
	char	autoAnnounce[256];
} ServerPrefs;

extern ServerUserPtr	gUserList,gToBeKilled;
extern ServerRoomPtr	gRoomList;
extern ServerPrefs		gPrefs;
extern short			gEntrance;
extern LONG				gNbrUsers;
extern Boolean			gModified;
extern short           	gNbrRooms;
extern HugePtr			gBigBuffer;  /* 100k - used for temporary stuff */
extern short			gNbrFileSends;
extern LONG				gLastMemberPager;
extern ServerUserPtr	*gUserArray;

/******************************************************************************************/

#include "s-funcs.h"

#if BISERVER
#include "biserver.h"		/* 3/18/95 */
#endif


/******************************************************************************************/

