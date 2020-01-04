/* S-SECURE.H */

#define SecureVersion	1L	/* Version number of this structure */

#if RAINBOWRED

#define MAGIC_LONG      0x9242cb3fL
#define CRC_MAGIC       0xa9face74L
#define DefaultCRC      0x5304f903L
#define DefaultCounter  0xc146323cL
#define HIGHSECURITY    1  /* this forces range checking on reg#s */

#else

#define MAGIC_LONG      0x9602c9bfL
#define CRC_MAGIC       0xa95ade76L
#define DefaultCRC      0x5905f923L
#define DefaultCounter  0xcf07309cL

#endif

#define OLD_MAGIC_LONG	    0x21E8E72CL
#define OLD_CRC_MAGIC	    0xd9216290L		/* used to compute crc */
#define OLD_DefaultCRC		0x5ebe3744L
#define OLD_DefaultCounter	0x21e8e72cL		/* same as OLD_MAGIC_LONG */

typedef struct {
	/* LONG			crc,counter;		// 8/3/95 */
	LONG			lastIPAddress;
	unsigned LONG	timeLastActive;				/* via GetDateTime() (seconds) */
	unsigned LONG	timeLastPenalized;
	LONG			nbrDishonorableDeaths;
	LONG			nbrLogins;
	LONG			nbrFailedAttempts;
	short			lastFlags;			/* banned? wizard? god? penalized? */
	short			whyLastKilled;
	unsigned char	lastName[1];
} UserBaseRec, *UserBasePtr;

/* 1/14/97 JAB Added special records for demo users */
typedef struct {
	LONG			lastIPAddress;
	unsigned LONG	timeLastActive;
	unsigned LONG	timeLastPenalized;
	LONG			nbrDishonorableDeaths;
	LONG			nbrLogins;
	LONG			nbrFailedAttempts;
	short			lastFlags;
	short			whyLastKilled;
	unsigned LONG	totalElapsed,demoElapsed;
	unsigned char	lastName[1];
} AltBaseRec, *AltBasePtr;

#define BR_SiteBan1		0x01
#define BR_SiteBan2		0x02
#define BR_AllInclusive	0x04	/* Members Too */
#define BR_Tracking		0x08	/* used for tracking ips */

/* 1/14/97 JAB */
#define BR_PuidKey		0x10	/* used to track guests */

typedef struct IPBanRec {
	struct IPBanRec	*nextRec;
	LONG	banFlags;
	LONG	ipAddress;
	unsigned LONG	regKey;
	LONG	timeLastPenalized;
	LONG	penaltyTime;
	LONG	whyBanned;
	char	verbalIP[128];
	char	userName[32];
	char	whoKilled[32];	/* 4/15/96 */
	char	comment[256];	/* 4/15/96 */
} IPBanRec, *IPBanRecPtr;

Boolean NewbieUserSerialNumber(unsigned LONG crc, unsigned LONG counter);
Boolean ValidUserSerialNumber(unsigned LONG crc, unsigned LONG counter);
unsigned LONG ComputeLicenseCRC(unsigned LONG ctr);
Handle GetUserBaseRec(LONG counter);
Handle GetAltBaseRec(LONG counter);	/* 1/14/97 JAB */

unsigned LONG ComputeOldLicenseCRC(Ptr pt, LONG len);
Boolean ValidOldSerialNumber(unsigned LONG crc, unsigned LONG counter);
Boolean OldNewbieUserSerialNumber(unsigned LONG crc, unsigned LONG counter);
void UserOldSerialCheck(ServerUserPtr user);


IPBanRecPtr GetIPBanRecByIP(unsigned LONG ipAddress, Boolean trackerFlag);
IPBanRecPtr GetIPBanRecByCounter(unsigned LONG counter, Boolean trackerFlag);
/* 1/14/97 JAB */
IPBanRecPtr GetIPBanRecByPuidKey(unsigned LONG counter, Boolean trackerFlag);

void InsertBanRec(IPBanRecPtr	recPtr);
void BanIP(ServerUserPtr whoKilled, unsigned LONG ipAddress, LONG penaltyMinutes, short siteFlag);
void DoBanList(ServerUserPtr cUser, char *str);
void DoUnBan(ServerUserPtr cUser, char *str, Boolean trackerFlag);
void PurgeBanList(ServerUserPtr cUser);
void ListUsers(ServerUserPtr cUser, char *arg, Boolean globalFlag);
void CommentBanRec(ServerUserPtr cUser, char *str);
void ExtendBanRec(ServerUserPtr cUser, char *str);

