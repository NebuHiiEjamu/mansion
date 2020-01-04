/******************************************************************************************/
/* MANSION.H   SHARED - Jim Bumgardner (JAB)  1994-1997 */

/******************************************************************************************/

#ifndef _MANSION_H
#define _MANSION_H	1

/******************************************************************************************/

#include "local.h"
#include "m-events.h"
#include "m-assets.h"	/* 6/7/95 JAB */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

/******************************************************************************************/

#define TIMEBOMB		0			/* clear before final release!! */
#define DEBUG			0			/* clear before final release!! */
#define RELEASE			1			/* set for final release!! */

#define MansionVersion	0x00010016L	/* 9/4/95 JAB */

#define MaxHotPts		32
#define MaxPictureRecs	64
#define RoomWidth		512
#define RoomHeight		384
#define NbrColors		16
#define MaxUserProps	9		/* 6/7/95 JAB */
#define MinReservedProp	0L			/* 0  6/23 */
#define MaxReservedProp	99999999L	/* 100 Million Reserved Props */
#define MinGuestRoom	0L
#define MaxGuestRoom	199L		/* Guest Room ID#s */
#define MaxPeoplePerRoom  64		/* 7/29 Probably should be way lower - allow */
									/* Sysop to adjust downwards */

/******************************************************************************************/

enum {C_None,
	  C_AppleTalk,
	  C_IRCTCP,
	  C_IRCSerial,
	  C_PalaceTCP,
	  C_PalaceTelnet,
	  C_DDE,
	  C_IPX,
	  C_WinSock,
	  C_BerkSockets,
	  C_MacTCP,
	  C_IRC,
	  C_Serial,
	  C_CompuServe,
	  C_NbrConnectionTypes};


/* !! may be able to get rid of this... */
enum {HS_Normal,
			HS_Door,
			HS_ShutableDoor,
			HS_LockableDoor,
			HS_Bolt,
			HS_NavArea};

/* Hotspot Events moved to U-CMDS.H */

/* Hotspot States */
#define HS_Unlock	0
#define HS_Lock		1

/* Draw Commands */
enum {DC_Path,
      DC_Shape,
      DC_Text,
      DC_Detonate,
      DC_Delete,
      DC_Ellipse,	/* guess */
	    DC_NbrDrawCmds};

/* Shape Types */
enum {ST_Rect,
			ST_Oval};

/* Face Types		6/15/95 */
enum {FACE_Closed, FACE_Smile, FACE_TiltDown, FACE_Talk,
		FACE_WinkLeft, FACE_Normal, FACE_WinkRight, FACE_TiltLeft,
		FACE_TiltUp, FACE_TiltRight, FACE_Sad, FACE_Blotto,
		FACE_Angry};

/* 7/22/95		- values for whyKilled */
/* */

/* Disconnect Messages on MSG_SERVERDOWN (put into refnum) */
enum	{K_Unknown,
		 K_LoggedOff,
		 K_CommError,
		 K_Flood,
		 K_KilledByPlayer,
		 K_ServerDown,
		 K_Unresponsive,
		 K_KilledBySysop,
		 K_ServerFull,
		 K_InvalidSerialNumber,
		 K_DuplicateUser,
		 K_DeathPenaltyActive,
		 K_Banished,			/* logon attempt by banned user */
		 K_BanishKill,			/* act of banishing */
		 K_NoGuests,			/* 8/17/95		 */
		 K_DemoExpired,			/* 1/16/97 JAB demo membership has expired */
		 K_Verbose};			/* 1/16/97 JAB server provides verbal explanation */

/******************************************************************************************/

typedef LONG	ObjectID;
typedef LONG	PersonID;
typedef short	RoomID;

typedef struct {
	LONG	refCon;
	short	eventType;
	short	scriptTextOfst;
} EventHandlerRec, *EventHandlerPtr;


typedef struct {
	LONG	refCon;
	short	picID;
	short	picNameOfst;
	short	transColor;
	short	reserved;		/* 9/4/95 */
} PictureRec, *PictureRecPtr;

typedef struct {
	short	pictID;
	short	reserved;		/* 9/4/95 */
	Point	picLoc;	/* offset from hotspot->loc */
} StateRec, *StateRecPtr;

typedef struct {
	short	nextOfst;
	short	reserved;		/* 9/4/95 */
} LLRec, *LLPtr;

typedef struct {
	LLRec		link;		/* Link to next Loose Prop */
	AssetSpec	propSpec;	/* 6/7/95 replaced prop id */
	LONG		flags;
	LONG		refCon;
	Point		loc;
} LPropRec, *LPropPtr;

typedef struct {
	LLRec	link;			/* 7/14/95 */
	short	drawCmd;
	short	cmdLength;
	short	dataOfst;	/* Points or TextRecord */
} DrawRecord, *DrawRecPtr;

#define HS_Draggable	0x01
#define HS_DontMoveHere	0x02
#define HS_Invisible	0x04		/* 8/23/95 */
#define HS_ShowName		0x08		/* 8/23/95 */
#define HS_ShowFrame	0x0010		/* 8/23/95 */
#define HS_Shadow		0x0020		/* 8/23 */
#define HS_Fill			0x0040		/* 8/23 */

typedef struct {
	LONG		scriptEventMask;
	LONG		flags;		/* unused at the moment */
	LONG		secureInfo;	/* ditto */
	LONG		refCon;
	Point		loc;		/* Computed automatically */
	short		id;			/* may not correspond to order */
	short		dest;		/* or destDoor */
	short		nbrPts;
	short		ptsOfst;
	short		type;		/* 0 = navRgn,  1 = door, 2 = lock */
	short		groupID;
	short		nbrScripts;
	short		scriptRecOfst;
	short		state;
	short		nbrStates;
	short		stateRecOfst;
	short		nameOfst;	/* 8/23 was Visible, now using Flag instead... */
	short		scriptTextOfst;	/* 7/27/95 */
	short		alignReserved;
} Hotspot,*HotspotPtr;

/* Note these flags are used in ServerUserRecord */
#define U_SuperUser			0x01 	/* old name for U_Wizard */
#define U_God				0x02	/* Sysop level access */
#define U_Kill				0x04	/* Local flag for killing */
#define U_Guest				0x08	/* Local Guest Flag */
#define U_Banished			0x10
#define U_Penalized			0x20
#define U_CommError			0x40
#define U_Gag				0x0080
#define U_Pin				0x0100	/* 1/15/96 */
#define U_Hide				0x0200	/* 1/29/96 */
#define U_RejectESP			0x0400	/* 1/29/96 */
#define U_RejectPrivate		0x0800	/* 1/29/96 */
#define U_PropGag			0x1000  /* 2/7/96 */

typedef struct UserRec {
/* Public Stuff */
	PersonID	userID;		/* Unique User ID */
	Point		roomPos;	/* Applies to gObject */
	AssetSpec	propSpec[MaxUserProps];	/* 6/7/95 JAB */
	RoomID		roomID;		/* Applies to socket, not gObject */
	short		faceNbr;	/* Applies to Person (gObject subtype) */
	short		colorNbr;
	short		awayFlag;	/* Applies to Person */
	short		openToMsgs;	/* Applies to Person */
	short		nbrProps;	/* 6/7/95 JAB */
	Str31 		name;		/* Applies to Person */
} UserRec, *UserRecPtr;

/* #define RF_Locked		0x01		// Room is security-locked by author */
#define RF_AuthorLocked		0x0001		/* Same Thing - new name to avoid confusion */
#define RF_Private			0x0002		/* Room is private */
#define RF_NoPainting		0x0004		/* 7/25/95 No Painting Allowed */
#define RF_Closed			0x0008		/* Room Door has been locked */
#define RF_CyborgFreeZone	0x0010		/* No Cyborgs in this room */
#define RF_Hidden			0x0020		/* doesn't show up in goto list*/
#define RF_NoGuests			0x0040
#define RF_WizardsOnly		0x0080  	/* 1/27/95 */
#define RF_DropZone			0x0100

typedef struct RoomRec {
	LONG			roomFlags;
	LONG			facesID;
	RoomID			roomID;			/* Room Number */
	short			roomNameOfst,
					pictNameOfst,
					artistNameOfst,
					passwordOfst;
	short			nbrHotspots;	/* Number of hot spots (doors & such) */
	short			hotspotOfst;
	short			nbrPictures;	/* Number of attached pictures */
	short			pictureOfst;
	short			nbrDrawCmds;	/* Number of draw commands */
	short			firstDrawCmd;
	short			nbrPeople;		/* Number of people in room */
	short			nbrLProps;		/* Loose Prop Objects */
	/* short		lPropOfst;		6/28/95 */
	short			firstLProp;		/* 6/28/95 - Changed to a Linked List */
	short			reserved;		/* keep structure LONG aligned */
	/* 6/7/95 deleted FProps */
	/* short			nbrFProps;		// Favorite Props (for prop list) */
	/* short			fPropOfst;	 */
	short			lenVars;
	char			varBuf[1];
} RoomRec, *RoomRecPtr;

typedef struct {		/* 6/14/95 */
	LONG		userID;
	short		flags;
	short		roomID;
	char		name[1];		/* even aligned pascal string */
} UserListRec, *UserListPtr;

typedef struct {		/* 6/14/95 */
	LONG		roomID;
	short		flags;
	short		nbrUsers;
	char		name[1];		/* even aligned pascal string */
} RoomListRec, *RoomListPtr;

/* Server Permissions Flags 7/22/95 */
#define PM_AllowGuests			0x0001
#define PM_AllowCyborgs			0x0002
#define PM_AllowPainting		0x0004
#define PM_AllowCustomProps		0x0008
#define PM_AllowWizards			0x0010
#define PM_WizardsMayKill		0x0020
#define PM_WizardsMayAuthor		0x0040
#define PM_PlayersMayKill		0x0080
#define PM_CyborgsMayKill		0x0100
#define PM_DeathPenalty			0x0200
#define PM_PurgeInactiveProps	0x0400
#define PM_KillFlooders			0x0800
#define PM_NoSpoofing			0x1000	/* 1/27/95 */
#define PM_MemberCreatedRooms	0x2000	/* 5/8/96 */

typedef struct {
	LONG			serverPermissions;	/* Server Permissions */
	unsigned char	serverName[64];	/* Server Name - P String */

	/* 1/27/97 JAB Added per Kevin Hazzard's Recommendation */
	unsigned long 	serverOptions;		/* clients really need this info */
	unsigned long 	ulUploadCaps;		/* see LI_ULCAPS_ defines */
	unsigned long 	ulDownloadCaps;		/* see LI_DLCAPS_ defines */
} ServerInfo, *ServerInfoPtr;

/* 1/27/97 JAB Added per Kevin Hazzard's Recommendation */
typedef struct {
	unsigned long ulMaxProtocolVersion;	// HIWORD is major, LOWORD is minor
	unsigned long ulNegotiatedProtocolVersion;
} ServerVersionInfo, *ServerVersionPtr;

typedef struct {
	unsigned LONG	crc;		/* 1/14/97 JAB changed to unsigned */
	unsigned LONG	counter;	/* 1/14/97 JAB changed to unsigned */
	Str63			userName;
} LogonInfo, *LogonInfoPtr;

/* 1/14/97 JAB Added support for new style registration event */
typedef struct {
	unsigned LONG	crc;		/* 1/14/97 JAB changed to unsigned */
	unsigned LONG	counter;	/* 1/14/97 JAB changed to unsigned */
	Str31			userName;
	Str31			wizPassword;
	LONG			auxFlags;
	unsigned LONG	puidCtr;
	unsigned LONG	puidCRC;
	unsigned LONG	demoElapsed;
	unsigned LONG	totalElapsed;
	unsigned LONG	demoLimit;

	short	desiredRoom;
	char	reserved[6];

	/* 1/27/97 Kevin Hazzard's Requested Changes */
// HIWORD is major, LOWORD is minor
	unsigned long ulRequestedProtocolVersion;

	unsigned long ulUploadCaps;			// see LI_ULCAPS_ defines
#define LI_ULCAPS_ASSETS_PALACE		0x00000001UL
#define LI_ULCAPS_ASSETS_FTP			0x00000002UL
#define LI_ULCAPS_ASSETS_HTTP		0x00000004UL
#define LI_ULCAPS_ASSETS_OTHER		0x00000008UL
#define LI_ULCAPS_FILES_PALACE		0x00000010UL
#define LI_ULCAPS_FILES_FTP			0x00000020UL
#define LI_ULCAPS_FILES_HTTP			0x00000040UL
#define LI_ULCAPS_FILES_OTHER		0x00000080UL

	unsigned long ulDownloadCaps;		// see LI_DLCAPS_ defines
#define LI_DLCAPS_ASSETS_PALACE		0x00000001UL
#define LI_DLCAPS_ASSETS_FTP		0x00000002UL
#define LI_DLCAPS_ASSETS_HTTP		0x00000004UL
#define LI_DLCAPS_ASSETS_OTHER		0x00000008UL
#define LI_DLCAPS_FILES_PALACE		0x00000010UL
#define LI_DLCAPS_FILES_FTP			0x00000020UL
#define LI_DLCAPS_FILES_HTTP			0x00000040UL
#define LI_DLCAPS_FILES_OTHER		0x00000080UL

	unsigned long ul2DEngineCaps;		// see LI_2DENGINECAP_ defines
#define LI_2DENGINECAP_PALACE		0x00000001UL

	unsigned long ul2DGraphicsCaps;		// see LI_2DGRAPHCAP_ defines
#define LI_2DGRAPHCAP_GIF87			0x00000001UL
#define LI_2DGRAPHCAP_GIF89a			0x00000002UL
#define LI_2DGRAPHCAP_JPG			0x00000004UL
#define LI_2DGRAPHCAP_TIFF			0x00000008UL
#define LI_2DGRAPHCAP_TARGA			0x00000010UL
#define LI_2DGRAPHCAP_BMP			0x00000020UL
#define LI_2DGRAPHCAP_PCT			0x00000040UL

	unsigned long ul3DEngineCaps;		// see LI_3DENGINECAP_ defines
#define LI_3DENGINECAP_VRML1			0x00000001UL
#define LI_3DENGINECAP_VRML2			0x00000002UL

} AuxRegistrationRec;


typedef struct {
 		LONG	        transactionID;	/* Made unique by server */
        LONG            blockSize;
        short           blockNbr;
        short           nbrBlocks;
        union {
                struct {
                        LONG    size;
                        Str63   name;
                        char    data[1];
                } firstBlockRec;
                struct {
                        char    data[1];
                } nextBlockRec;
        } varBlock;
} FileBlockHeader, *FileBlockPtr;     /* Used to send files over net */

/******************************************************************************************/

/* Globals */
extern Boolean gQuitFlag;

/******************************************************************************************/

#endif

