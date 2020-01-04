/* s-magicss.h */

/*
A portable library for server reg numbers.

	A serial number consists of 3 32-bit numbers: a,b,c.

	It is computed from the following information:	
		    seed           (32-bits)  - A unique user ID seed number

			options        (32-bits) consisting of:
				flags		   (16-bits)  - currently undefined
				capacity   	   (8-bits    - stored as multiples of 10 users e.g. 2 = 20 users)	
										  - 0 = minimum # of users (3?)
										  - 255 = maximum # of users (2550+)
				platform type  (8-bits)   - 0=mac,1=windows,2=NT,3=unix

	The three numbers are computed as follows:

		a = ComputeCRC(seed,options);    This is how we check for validity.
		b = seed XOR MAGIC_SEED_NUMBER XOR a
		c = options XOR MAGIC_OPTIONS_NUMBER XOR b

   These three numbers (a,b,c) are converted to a 20-digit alpha-numeric representation
   by converting groups of 5 bits into a letter.

todo:  Make sure YPD is fixed to deal with new reg numbers.

*/
#define SS_LENGTH			96				/* number of bits */
#define SS_BINLENGTH		12				/* length of binary encoded number */
#define SS_ASCILENGTH		20				/* length of ascii encoded number (NO HYPHENS) */
#define SEED_MAGIC_LONG		0x9602c9bfL
#define FLAGS_MAGIC_LONG	0xC73088E2L
#define CRC_MAGIC			0xa95ade76L
#define OBFUSCATE_LONG		0xD7AA3702L
#define ROTATE_OBFUSCATE	0x02D7AA37L
#define ENCODE_OBFUSCATE	(ROTATE_OBFUSCATE ^ 0xAAAAAAAA)
#define CRC_ENCODE			(CRC_MAGIC ^ ENCODE_OBFUSCATE)
#define DECODE_OBFUSCATE	0x55555555

/* Exported Routines */
	/* converstion routines */
void SSAsciiToBinary(char *ascBuf, unsigned char *binBuf);
void SSBinaryToAscii(unsigned char *binBuf, char *ascBuf);

	/* validity check */
int SSIsValidSerialNumber(unsigned char *binBuf);

	/* serial number parsing */
void SSParseSerialNumber(unsigned char *binBuf, int *platform,int *capacity,int *flags, unsigned LONG *seed);

	/* serial number generation */
	/* (not included with server) */
int SSUpgradeSerial(char *oldSerial, char *newSerial,int newPlatform, int newCapacity, int newFlags);
void SSGenSerialNumber(unsigned char *binBuf,int platform,int capacity,int flags, unsigned LONG seed);

/* Internal Routines */
int SSConvertCodeToAscii(short code);
int SSConvertAsciiToCode(short asc);
unsigned LONG SSComputeCRC(unsigned long *v);
unsigned LONG SSDecodeCRC(unsigned long *v);

/* Internal Data */
extern unsigned LONG gSSCRCMask[256];
