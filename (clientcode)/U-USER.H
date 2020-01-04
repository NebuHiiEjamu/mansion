////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// User.h
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MANSION.H"
#include "C-LOCAL.h"
#include "M-ASSETS.H"
#include "Local.h"	// Port-specific functions for client

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MaxHotSpots       64
#define NbrShades	     256
#define NbrFades		  16
#define MaxPictures      128
#define RoomWidth        512
#define RoomHeight       384
#define FaceWidth         44
#define FaceHeight        44
#define PropWidth			    44
#define PropHeight        44
#define ToolWidth         32
#define ToolHeight        32
#define NbrDissolveSteps  16
#define ToolCellHeight    42
#define ToolCellWidth     42
#define MaxPenSize         8
#define MaxPatterns       38
#define MaxPoints        256

#if macintosh
#define LeftMargin        44
#define TopMargin         44
#else
#define LeftMargin        0
#define TopMargin         0
#endif

// 6/12/95
#define MaxBalloonRecs		32
#define BalloonGap			10
#define BalloonMargin		 8
#define PointHeight			 6
#define SpikeHeight		    16
#define SpikeWidth		    12
#define SpikeDisplace	    20
#define RandomAmplitude	    10
#define BalloonMinTicks	   (3L*TICK_SECONDS)	// 6/1/95 - increased minimum display time from 90 ticks
#define NameGap				16

// Error Numbers for Dialog Box Error Messages (STR Resource)

// 8/2/95 New- changed these to an enum
enum {RE_NoGuestAccess=1, RE_Unsupported, RE_ReadOnlyAssets, 
	  RE_ReadOnlyVolume, RE_InvalidSerialNumber, RE_ThankYou,
	  RE_Flooding, RE_KilledByPlayer, RE_KilledBySysop,
	  RE_KilledBySerialNumber, RE_KilledDuplicateUser, RE_DeathPenaltyActive,
	  RE_Banished, RE_Inactivity, RE_ServerShutDown, RE_OldVersion};
	  // 8/2/96 JAB Added new verbose error codes for common
	  // Connection problems
	  // RE_NoTCP,	RE_CantLocateSite, RE_ConnectionRefused, RE_ConnectionSevered,
	  // RE_BadData};

// 8/6/96 Verbose Error Messages - JAB
enum {VE_NoTCP=0, VE_CantLocateSite, VE_ConnectionRefused, VE_ConnectionSevered,
      VE_BadData,
	  // 1/14/97 JAB added new dialog boxes for demonstration timeouts...
      VE_GuestTimeOutOffline,
      VE_GuestTimeOutOnline,
      VE_MemberTimeOutOffline,
      VE_MemberTimeOutOnline
      };

// Error Numbers for Status Line Error Messages (STR Resource)
enum {SM_MembersOnly=1, SM_ServerShutDown, SM_GuestAccess, SM_MemberAccess, 
	  SM_CantLocateSite, SM_Connecting, SM_ConnectionRefused, SM_ConnectionSevered, 
	  SM_ConnectTCP, SM_ConnectAppletalk, SM_NoConnection, SM_ScriptLoaded, 
	  SM_ScriptError, SM_UserAbortScript, SM_DeniedByServer, SM_Killed, SM_Terminated, 
	  SM_ServerFull, 
	  SM_ErrorCreatingStream, 
	  SM_ErrorOpeningConnection,
	  SM_ReceiveError, SM_SendError, SM_NoPainting, SM_CyborgFreeZone, SM_StillTrying,
// 1/16/97 JAB
	  SM_DemoExpired,
// 2/19/97 JAB Kids Client
	  SM_SiteNotApproved
	  };

// Media Types used for requesting files 8/6/96 JAB
enum {MT_Picture, MT_Sound, MT_Movie};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Moved Icon IDs Tool IDs and Cursor IDs to C-LOCAL JAB 7/19/95


// User Modes
enum	{M_Normal,
       M_Authoring};


// Draw Tools
enum	{DT_Pencil,
       DT_Linesize,
       DT_Detonator,
       DT_Layer,
       NbrDrawTools};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct IconSetup
{
	short	nbrTools;
	short	firstTool;
	short	nbrFaces;
	short	firstFace;
	short	nbrProps;
	short	firstProp;
	Point	propOffsets[1];
} IconSetup;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PF_PropFaceFlag		0x02	// Prop is a face - don't show regular face prop
#define PF_PropGhostFlag	0x04	// Prop is transparent
#define PF_PropRareFlag		0x08	// Prop is rare - don't copy, delete from faves when dropped
#define PF_Animate			0x10	// 4/24/96 JAB
#define PF_Palindrome		0x20	// 4/24/96 JAB

typedef struct {
	short	width,height,hOffset,vOffset;
	short	scriptOffset;	// 6/22/95
	short	flags;			// 6/22/95
} PropHeader,*PropHeaderPtr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define UF_FadeIn				0x01
#define UF_FadeOut				0x02
#define UF_IRCSparkyAware		0x04
#define UF_PropFace				0x08
#define UF_Guest				0x10	// Newbie Flag
#define UF_IsAnimating			0x20	// 4/24/96 JAB
#define UF_IsPalindrome			0x40	// 4/24/96 JAB

typedef struct LocalUserRec
{
	UserRec	user;
	short   userFade;
	short	localFlags;	// 6/22 switched to flags system
	short	propFrameNumber,palindromeDirection;	// 4/24/96 JAB
	short	nbrAnimateProps;						// 4/24/96 JAB
	LONG	lastAnimate;							// 4/24/96 JAB
	// Boolean	fadeIn,fadeOut,ircSparkyAware,propFace;
	Handle	propHandle[MaxUserProps];
} LocalUserRec, *LocalUserRecPtr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	ObjectWindowRecord	objWin;
	GWorldPtr			offWorld,		// Offscreen Rendering
					    offPicture;
	PixMapHandle		offPixMap,
						offPictureMap;

	Handle				*faceHandles;
	Handle				*lPropHandles;
	// 4-bit gray -> 8-bit system color translations for colorizing faces
	LocalUserRec		userList[MaxPeoplePerRoom+4];
	LocalUserRecPtr		mePtr;
	Handle				faveProps;		// favorite props
	short				nbrFaveProps;
	short				nbrFadeIns,nbrFadeOuts,hasFaceAnimations;
	long				meID;
	long				targetID;
	Rect				msgRect,msgFrame,toolRect,controlFrame12;
	TEHandle			msgTEH;
	Boolean				msgActive;
	short				totalPeople;
	Boolean				noFades;
	Boolean				noRoomPicture;
	short				curHotspot;	// For Authoring
	short				expert,userFlags;	// 8/24 userflags & expert set from ThisIsYourStat
	Boolean				swapPackets;
	// Boolean				guestAccess; moved to gSecure
	Boolean				signOn;			// Set while signing on  7/20/95
	Boolean				navInProgress;	// Set while navigating  9/12/95
	unsigned char		*cTrans;		// Face Colors Table 6/27/95
	unsigned char		*fTrans;		// Fade Table		 6/27/95
	unsigned char		*gTrans;		// Ghost Table		 6/27/95
	long				userEventMask;
	Ptr					userScriptBuffer;
	Ptr					userScriptPtrs[32];
	ServerInfo			serverInfo;		// 7/25/95 - info on current server
	unsigned char		targetUserName[32];
	RoomRec				curRoom;
	char				varBuf[32767]; // needed by previous roomrec
} RoomWindowRec, *RoomWindowPtr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	short		nbrProps;				// 6/7/95
	char		colorNbr;
	char		faceNbr;
	AssetSpec	propSpec[MaxUserProps];	// 6/7/95
} MRec;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MaxMacros		80	// 5/9/96
#define MaxScriptMacros	10
#define OldMaxMacros	10
typedef struct {
	long	versID;
	MRec	mac[OldMaxMacros];
} OldMacroRec;

typedef struct {
	long	versID;
	long	nbrMacros;
	MRec	mac[MaxMacros];
} MacroRec;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MaxNetaddresses	10

typedef struct {
	char			name[64];
	long			ip_addr;
	short			portNumber;
} NetAddress;

#define UPF_UseFadeEffects		0x01
#define UPF_DownloadGraphics	0x02
#define UPF_NetscapeConfigured	0x04
#define UPF_TextToSpeech		0x08
#define UPF_AutoShowNames		0x10
#define UPF_TintedBalloons		0x20
#define UPF_UsePropAnimations	0x40
#define UPF_ClubMode			0x0080
#define UPF_DontShowRegInfo		0x0100
#define UPF_ShowOrigCode		0x0200
#define UPF_ShowOverlay			0x0400

typedef struct {
	long	versID;
	long	userPrefsFlags;		// UPF_ flags 7/27/95
	Str31	name;
	short	readingSpeed;
	short	fontID;
	short	fontSize;
	short	remotePort;
	char	remoteHost[256];
	short	serPort,serBaud;
	short	soundLevel;
	char	picFolder[64];
	Str31	userPassword;	// Lock Kiddie Access
	short	nbrNetaddresses;
	NetAddress nAddr[MaxNetaddresses];
	short	voiceNumber;
	Str15	originationCode;
} UserPrefs;


typedef struct {
	PersonID	speakerID;
	long		 exitTime;
	Rect	  	r,	// r = balloon outline rect
				    rr;	// rr = refresh rect, encompasses balloon and point
	Point		target;
	short		len;
	char		str[255];
	Boolean		active;
	Boolean 	private;
	Boolean 	thought;
	Boolean		excited;
	Boolean		perm;		// 6/1/95 added perm flag
	Boolean		system;
	short		tint;		// 0-15 = face color, -1 = System
} BalloonRec;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "PROTOS.H"  // local function prototypes
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

