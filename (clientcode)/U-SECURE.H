// U-SECURE.H

#define SecureVersion	1L	// Version number of this structure

#if RAINBOWRED

#define MAGIC_LONG      0x9242cb3fL
#define CRC_MAGIC       0xa9face74L
#define DefaultCRC      0x5304f903L
#define DefaultCounter  0xc146323cL
#define OBFUSCATE_LONG	0xD7AA3702L	// 8/7/96
#define ENCODE_OBFUSCATE	(OBFUSCATE_LONG ^ 0xAAAAAAAA)
#define DECODE_OBFUSCATE	0x55555555
#define HIGHSECURITY    1  /* this forces range checking on reg#s */

#else

#define MAGIC_LONG      0x9602c9bfL
#define CRC_MAGIC       0xa95ade76L
#define DefaultCRC      0x5905f923L
#define DefaultCounter  0xcf07309cL
#define OBFUSCATE_LONG	0xD7AA3702L
#define ENCODE_OBFUSCATE	(OBFUSCATE_LONG ^ 0xAAAAAAAA)
#define DECODE_OBFUSCATE	0x55555555
#endif

#define OLD_MAGIC_LONG	    0x21E8E72CL
#define OLD_CRC_MAGIC	    0xd9216290L		/* used to compute crc */
#define OLD_DefaultCRC		0x5ebe3744L
#define OLD_DefaultCounter	0x21e8e72cL		/* same as OLD_MAGIC_LONG */

typedef struct {
	unsigned long secureVersion;
	unsigned long crc;
	unsigned long counter;
	char	isRegistered;	// 1/14/97 JAB
	char	guestAccess;	// 1/14/97 JAB
							// Changed "short guestAccess" to
							// two unique fields.
							// We need to distinguish between 
							// "registered" users and guests
							// because now there may be unregistered
							// members.
	Str63	ownerName;
	Str63	ownerOrg;
} SecureRec;

extern SecureRec	gSecure;

// Locally Implemented Functions
void RegistrationDialog(void);
void RegInfo();
void RegCompleteDialog(short dialogNbr);
void RegComplete1Dialog();
void RegComplete2Dialog();

void GetSerialNumber(Ptr buffer);
void LoadSecureInfo(void);	// Called from StorePreferences()
void SaveSecureInfo(void);	// Called from StorePreferences()
void NewbieMenus(void);


// Portable Functions

Boolean CheckSecurity(void);
Boolean ValidSerialNumber(void);		// checks gSecure structure
Boolean NewbieSerialNumber(void);		// checks gSecure structure
unsigned long ComputeLicenseCRC(Ptr pt, long len);
int ConvertCodeToAscii(short code);
int ConvertAsciiToCode(short asc);
void ConvertCodeToString(unsigned char *s, char *sa, short nbrBits);
void ConvertStringToCode(char *sa, unsigned char *s, short nbrBits);
void GetSerialNumberString(char *str);	// c string
void GetOwnerString(char *str);	// c string

unsigned LONG ComputeOldLicenseCRC(Ptr pt, LONG len);
Boolean ValidOldSerialNumber(void);
Boolean OldNewbieSerialNumber(void);
Boolean MemberSerialNumber(unsigned LONG crc, unsigned LONG counter);

